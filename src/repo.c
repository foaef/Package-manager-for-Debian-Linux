#include<stdlib.h>
#include<stdio.h>
#include<string.h>

int append_user_packages(){
    char* cmd = "dpkg-scanpackages /var/baciu/user_built /dev/null >> /var/baciu/packages/Packages";
    int res = system(cmd);
    return res;
}

int package_folder(char* path){
    char* fmt = "dpkg-deb --build %s /var/baciu/user_built";
    int cmd_size = strlen(fmt) + strlen(path) + 1;
    char* cmd = malloc(cmd_size*sizeof(char));
    if(!cmd){
        perror("malloc error");
        return -1;
    }
    sprintf(cmd, fmt, path);
    int res = system(cmd);
    free(cmd);
    return res;
}

