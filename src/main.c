#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include<cjson/cJSON.h>
#include"resolver.h"
#include"fs.h"
#include"downloader.h"
#include"repo.h"
#include"index.h"
#include"installer.h"

int install(char** arguments){
    //initialising stuff
    FILE* f = fopen("Packages", "r");
    if(!f){
        perror("could not open file index");
        return -1;
    }
    fs_init_directories();
    ensure_cache_path_dir();
    PKGMAP* MAP = malloc(sizeof(PKGMAP));
    fs_mmap_index(MAP);
    
    //sort MMAP to use binary search in index_get_package_info
    qsort(MAP->MMAP, MAP->pkgcount, sizeof(Stanza), compare_stanzas);

    DebianPackage* pkg = index_get_package_info(arguments[2], f, MAP);
    resolve_package(pkg, f, MAP);

    download_to_file(INSTALL_QUEUE, queue_count);

    install_package_list();
    flush_cache();

    for(int i = 0;  i < queue_count; i++){
        free_debian_package(INSTALL_QUEUE[i]);
    }
    
    fclose(f);
    return 0;
}

int uninstall(char* name){
    return uninstall_package(name);
}

int update_index(void){
    return download_index();
}

int main(int argc, char **argv){
    //Usage
    if(argc < 2){
        printf("Usage: %s <option> <package name>\n", argv[0]);
        return -1;
    }
    //install, uninstall, update
    if(strcmp(argv[1], "install") == 0){
        return install(argv);
    }
    if(strcmp(argv[1], "uninstall") == 0){
        return uninstall(argv[2]);
    }
    if(strcmp(argv[1], "update") == 0){
        return update_index();
    }
    return -1;
}

