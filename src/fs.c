#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include"index.h"
#include"fs.h"

int fs_init_directories(void) {
    int rc = system("sudo mkdir -p /var/baciu/cache /var/baciu/packages /var/baciu/user_built && sudo chmod -R 777 /var/baciu");
    if (rc != 0) {
        fprintf(stderr, "error initializing directories (mkdir returned %d)\n", rc);
        return -1;
    }
    return 0;
}

int ensure_cache_path_dir(void) {
    int res = system("mkdir -p /var/baciu/cache"); 
    if (res != 0) {
        fprintf(stderr, "mkdir failed\n");
        return -1;
    }
    return 0;
}

char* build_cache_path_for_package(DebianPackage* pkg){
    char* ongfrfrnocappath = "/var/baciu/cache/%s-%s.deb";
    size_t path_size = (size_t)snprintf(NULL, 0, ongfrfrnocappath, pkg->name, pkg->version)+ 1;
    char* path = malloc(path_size);
    if(!path){
        fprintf(stderr,"malloc error: build_cache_path_for_package");
        return NULL;
    }

    int verificare = snprintf(path, path_size, ongfrfrnocappath, pkg->name, pkg->version);
    if(verificare < 0){
        fprintf(stderr, "couldnt get path");
        free(path);
        return NULL;
    }
    if((size_t)verificare >= path_size){
        fprintf(stderr, "truncated path: invalid");
        free(path);
        return NULL;
    }
    return path;//caller must free()
}

void fs_mmap_index(PKGMAP* PKGMAP){
    FILE* f = fopen("/var/baciu/packages/Packages", "r");
    if(!f){
        perror("couldnt open package index");
        return;
    }
    char line[1024];
    PKGMAP->pkgcount = 0;
    PKGMAP->MMAP = malloc(65000*sizeof(Stanza));
    while(fgets(line, 1024, f)){
        if(strncmp(line,  "Package: ", 9) == 0){
            char* name = strdup(line+ 9);
            PKGMAP->MMAP[PKGMAP->pkgcount].pkgname = strdup(name);
            clean_newline(PKGMAP->MMAP[PKGMAP->pkgcount].pkgname);
            PKGMAP->MMAP[PKGMAP->pkgcount].offset = ftell(f) - strlen(line);
            PKGMAP->pkgcount++;
            free(name);
        }
        
    }
    fclose(f);
}

void flush_cache(void){
    char* cmd = "rm -rf /var/baciu/cache/*.deb";
    system(cmd);
}

int search_index(char* keyword){
    char* fmt = "grep '%s' /var/baciu/packages/Packages";
    char* cmd = malloc(sizeof(char)*(strlen(fmt) + strlen(keyword) + 5));
    if(!cmd){
        perror("malloc error");
        return -1;
    }
    sprintf(cmd, fmt, keyword);
    system(cmd);
    return 0;
}