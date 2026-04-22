#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "repo.h"
#include "resolver.h"

#define MAX_QUEUE 2048

// Track what we've already dealt with to prevent infinite loops
char* VISITED[MAX_QUEUE]; 
int visited_count = 0;

// The final ordered list of packages to download/install
DebianPackage* INSTALL_QUEUE[MAX_QUEUE];
int queue_count = 0;

void add_to_install_list(DebianPackage* pkg) {
    for(int i = 0; i < queue_count; i++) {
        if(strcmp(INSTALL_QUEUE[i]->name, pkg->name) == 0) return;
    }
    INSTALL_QUEUE[queue_count++] = pkg;
    return;
}

int is_in_install_queue(DebianPackage* pkg){
    for(int i = 0; i < queue_count; i++) {
        if(strcmp(INSTALL_QUEUE[i]->name, pkg->name) == 0) return 1;
    }
    return 0;
}

void add_to_visited(DebianPackage* pkg) {
    for(int i = 0; i < visited_count; i++) {
        if(strcmp(VISITED[i], pkg->name) == 0) return;
    }
    VISITED[visited_count++] = pkg->name;
    return;
}

int is_visited(DebianPackage* pkg){
    for(int i = 0; i < visited_count; i++) {
        if(strcmp(VISITED[i], pkg->name) == 0) return 1;
    }
    return 0;
}

int is_installed(const char* pkgname) {
    // The real Debian status database
    FILE* f = fopen("/var/lib/dpkg/status", "r");
    if (!f) {
        // Fallback for testing if not on a Debian system
        return 0; 
    }

    char line[1024];
    int found_pkg = 0;
    int is_actually_installed = 0;

    while (fgets(line, sizeof(line), f)) {
        // 1. Find the package block
        if (strncmp(line, "Package: ", 9) == 0) {
            char name[256];
            sscanf(line + 9, "%s", name);
            if (strcmp(name, pkgname) == 0) {
                found_pkg = 1;
            } else {
                found_pkg = 0;
            }
            continue;
        }

        // 2. Look for the Status line inside that block
        if (found_pkg && strncmp(line, "Status: ", 8) == 0) {
            // Debian uses "install ok installed" to mean it's fully there
            if (strstr(line, "installed")) {
                is_actually_installed = 1;
                break; 
            }
        }
    }

    fclose(f);
    return is_actually_installed;
}

void resolve_package(DebianPackage* package, FILE* f, PKGMAP* MAP){
    if(!f){
        perror("resolve_package got NULL file pointer as innput");
        return;
    }
    if(is_in_install_queue(package) || is_visited(package)){
        return;
    }
    if(is_installed(package->name)){
        return;
    }

    add_to_visited(package);

    int cnt = 0;
    char** deps = split_dependencies(package->depends, &cnt);
    for(int i = 0; i< cnt; i++){
        DebianPackage* pkg = index_get_package_info(deps[i], f, MAP);
        if(!pkg){
            return;
        }
        resolve_package(pkg, f, MAP);
        printf("resolved package: %s\n", pkg->name);
        free(deps[i]);
    }
    free(deps);
    add_to_install_list(package);
    return;
    //INSTALLED_QUEUE now contains the dependecies in instalation order
    //after using INSTALLED_QUEUE, iterate and free() every package
}
