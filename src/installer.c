#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include"index.h"
#include"resolver.h"

int install_package_list(){
    char command[1024] = "dpkg -R -i /var/baciu/cache";
    int res = system(command);
    return res;
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