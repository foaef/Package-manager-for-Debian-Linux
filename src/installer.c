#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include"index.h"
#include"resolver.h"

int install_package_list(){
    char command[1024] = "dpkg -R -i ";
    char* HOME = getenv("HOME");
    if(!HOME){
        fprintf(stderr, "couldnt get home env variable");
        return -1;
    }
    char* fmt = "%s/Desktop/pacman/cache";
    size_t path_size = (size_t)snprintf(NULL, 0, fmt, HOME)+ 1;
    char* path = malloc(path_size);
    if(!path){
        fprintf(stderr,"malloc error: install_package_list");
        return -1;
    }
    int verificare = snprintf(path, path_size, fmt, HOME);
    if(verificare < 0){
        fprintf(stderr, "couldnt get path");
        free(path);
        return -1;
    }
    strcat(command, path);
    system(command);
    return 0;
}

int uninstall_package(char* pkgname){
    char* fmt = "dpkg -P %s";
    char* command = malloc(sizeof(char)*100);
    if(!command){
        perror("uninstall: malloc");
        return -1;
    }
    sprintf(command, fmt, pkgname);
    system(command);
    free(command);
    return 0;//careful if uninstalling a package with a really long name
}