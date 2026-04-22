#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include"index.h"
#include"fs.h"

int fs_init_directories(void) {
    int rc = system("mkdir -p \"$HOME/Desktop/pacman/cache\" \"$HOME/Desktop/pacman/packages\"");
    if (rc != 0) {
        fprintf(stderr, "error initializing directories (mkdir returned %d)\n", rc);
        return -1;
    }
    return 0;
}

int ensure_cache_path_dir(void){
    char* HOME = getenv("HOME");
    if(!HOME){
        fprintf(stderr, "couldnt get home env variable");
        return -1;
    }
    char* format = "%s \"%s/Desktop/pacman/cache\"";
    size_t path_size = (size_t)snprintf(NULL, 0, format, HOME)+ 1;
    char* path = malloc(path_size+20);
    snprintf(path, path_size + 10, format, "mkdir -p ", HOME);
    int res = system(path);
    if(res != 0){
        fprintf(stderr, "cache path does not exist  (mkdir returned %d)", res);
        free(path);
        return -1;
    }
    free(path);
    return 0;
}

char* build_cache_path_for_package(DebianPackage* pkg){
    char* HOME = getenv("HOME");
    char* base_path;

    if (HOME) {
        base_path = HOME;
    } else {
        // Fallback for sudo/root environment
        base_path = "/tmp"; 
    }
    if(!HOME){
        fprintf(stderr, "couldnt get home env variable");
        return NULL;
    }
    char* ongfrfrnocappath = "%s/Desktop/pacman/cache/%s-%s.deb";
    size_t path_size = (size_t)snprintf(NULL, 0, ongfrfrnocappath, base_path, pkg->name, pkg->version)+ 1;
    char* path = malloc(path_size);
    if(!path){
        fprintf(stderr,"malloc error: build_cache_path_for_package");
        return NULL;
    }

    int verificare = snprintf(path, path_size, ongfrfrnocappath, base_path, pkg->name, pkg->version);
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
    FILE* f = fopen("Packages", "r");
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
    char* cmd = "rm -rf cache/*.deb";
    system(cmd);
}