#include<stdlib.h>
#include<stdio.h>
#include<cjson/cJSON.h>
#include<string.h>
#include"repo.h"

char* read_file(const char* path){
    FILE* f = fopen(path, "rb");
    if(!f){
        perror("error readinnng file");
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    rewind(f);

    char *buffer = malloc(n+1);
    if(!buffer){
        perror("eroare malloc");
        fclose(f);
        return NULL;
    }
    fread(buffer, 1, n, f);
    buffer[n] = '\0';

    fclose(f);
    return buffer;
}

cJSON* repo_get_parsed_tree(const char* path){
    char *json_text = read_file(path);
    if(!json_text){
        fprintf(stderr, "Failed to read file: %s\n", path);
        return NULL;
    }

    cJSON *root = cJSON_Parse(json_text);

    if (!root) {
        fprintf(stderr, "JSON parse error\n");
        free(json_text);
        return NULL;
    }

    free(json_text);
    //memoria din root apartine caller-ului!! trebuie cJSON_Delete(root) dupa call
    return root;
}

cJSON* repo_get_packages_root(cJSON* root) {
    cJSON* pkgs = cJSON_GetObjectItemCaseSensitive(root, "packages");
    if (!pkgs || !cJSON_IsObject(pkgs)) return NULL;
    return pkgs;
}

cJSON* repo_get_package(cJSON* pkgs, const char* name) {
    if (!pkgs || !name) return NULL;
    cJSON* pkg = cJSON_GetObjectItemCaseSensitive(pkgs, name);
    if (!pkg || !cJSON_IsObject(pkg)) return NULL;
    return pkg;
}

cJSON* repo_get_versions(cJSON* pkg) {
    cJSON* versions = cJSON_GetObjectItemCaseSensitive(pkg, "versions");
    if (!versions || !cJSON_IsObject(versions)) return NULL;
    return versions;
}

cJSON* repo_get_version_node(cJSON* pkg, const char* version_str){
    if (!pkg || !version_str) {
        fprintf(stderr, "repo_get_version_node: invalid input\n");
        return NULL;
    }

    cJSON* versions = repo_get_versions(pkg);
    if (!versions || !cJSON_IsObject(versions)) {
        fprintf(stderr, "repo_get_version_node: missing/invalid versions\n");
        return NULL;
    }

    cJSON* version = cJSON_GetObjectItemCaseSensitive(versions, version_str);
    if (!version) {
        fprintf(stderr, "repo_get_version_node: version %s not found\n", version_str);
        return NULL;
    }

    return version;
}

cJSON* repo_get_dependencies(cJSON* version_node) {
    if (!version_node) {
        fprintf(stderr, "repo_get_dependencies: invalid input\n");
        return NULL;
    }

    cJSON* deps = cJSON_GetObjectItemCaseSensitive(version_node, "dependencies");
    if (!deps) {
        // Treat as no dependencies (not an error)
        return NULL;
    }

    if (!cJSON_IsObject(deps)) {
        fprintf(stderr, "repo_get_dependencies: dependencies is not an object\n");
        return NULL;
    }

    return deps;
}

int compare_versions(const char* v1, const char* v2){ 
    int maj1 = 0, min1 = 0, pat1 = 0; 
    int maj2 = 0, min2 = 0, pat2 = 0; 
    sscanf(v1, "%d.%d.%d", &maj1, &min1, &pat1); 
    sscanf(v2, "%d.%d.%d", &maj2, &min2, &pat2); 
    
    if (maj1 != maj2) return maj1 - maj2; 
    if (min1 != min2) return min1 - min2; 
    return pat1 - pat2; 
} 

cJSON* repo_get_latest_version(cJSON* pkg) {
    if (!pkg) return NULL;

    cJSON* versions = cJSON_GetObjectItemCaseSensitive(pkg, "versions");
    if (!versions || !cJSON_IsObject(versions)) return NULL;

    cJSON* ver = NULL;
    cJSON* latest = NULL;

    cJSON_ArrayForEach(ver, versions) {
        if (!latest || compare_versions(ver->string, latest->string) > 0) {
            latest = ver;
        }
    }

    return latest; // this is the version NODE: key= "1.2.3"
}

int repo_get_github_owner_repo(cJSON *pkg, const char **out_owner, const char **out_repo) {
    if (!pkg || !out_owner || !out_repo) return 0;

    cJSON *source = cJSON_GetObjectItemCaseSensitive(pkg, "source");
    if (!source || !cJSON_IsObject(source)) return 0;

    cJSON *type = cJSON_GetObjectItemCaseSensitive(source, "type");
    if (!type || !cJSON_IsString(type) || strcmp(type->valuestring, "github") != 0) return 0;

    cJSON *owner = cJSON_GetObjectItemCaseSensitive(source, "owner");
    cJSON *repo  = cJSON_GetObjectItemCaseSensitive(source, "repo");

    if (!owner || !cJSON_IsString(owner) || !repo || !cJSON_IsString(repo)) return 0;

    *out_owner = owner->valuestring; // pointer owned by cJSON tree DO NOT free()
    *out_repo  = repo->valuestring;
    return 1;
}

int repo_get_tag_name(cJSON *version_node, const char **out_tag) {
    if (!version_node || !out_tag) return 0;

    cJSON *ref = cJSON_GetObjectItemCaseSensitive(version_node, "ref");
    if (!ref || !cJSON_IsObject(ref)) return 0;

    cJSON *type = cJSON_GetObjectItemCaseSensitive(ref, "type");
    if (!type || !cJSON_IsString(type) || strcmp(type->valuestring, "tag") != 0) return 0;

    cJSON *name = cJSON_GetObjectItemCaseSensitive(ref, "name");
    if (!name || !cJSON_IsString(name)) return 0;

    *out_tag = name->valuestring; //  pointer owned by cJSON tree DO NOT free()
    return 1;
}

