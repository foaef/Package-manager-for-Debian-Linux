#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "index.h"
#include "fs.h"
#include "resolver.h"


/*
Package: 0xffff
Version: 0.9-1
Installed-Size: 167
Maintainer: Sebastian Reichel <sre@debian.org>
Architecture: amd64
Depends: libc6 (>= 2.14), libusb-0.1-4 (>= 2:0.1.12)
Description: Open Free Fiasco Firmware Flasher
Multi-Arch: foreign
Homepage: https://github.com/pali/0xFFFF
Description-md5: 183c49f6505eb3432d1b069800f1f5b6
Tag: admin::hardware, hardware::usb, implemented-in::c,
 interface::commandline, role::program, scope::utility, works-with::file
Section: misc
Priority: optional
Filename: pool/main/0/0xffff/0xffff_0.9-1_amd64.deb
Size: 59232
MD5sum: 3de8282ef18b53b11c8ad6d6481b3a14
SHA256: 2c5a35bc4830379b565369ccbca608535d64577fb3244869a17cb6de8d9bda7d
*/

int compare_stanzas(const void* a, const void* b) {
    const Stanza* s1 = (const Stanza*)a;
    const Stanza* s2 = (const Stanza*)b;
    
    // Compare the package names alphabetically
    //for qsort() ad bsearch()
    return strcmp(s1->pkgname, s2->pkgname);
}

void clean_newline(char* s) {
    if (!s) return;
    s[strcspn(s, "\r\n")] = 0;
}

void free_debian_package(DebianPackage* pkg) {
    if (pkg == NULL) return;

    if (pkg->name)     free(pkg->name);
    if (pkg->version)  free(pkg->version);
    if (pkg->depends)  free(pkg->depends);
    if (pkg->filename) free(pkg->filename);

    free(pkg);
}

DebianPackage* index_get_package_info(const char* pkgname, FILE* f, PKGMAP* MAP){
    //search for pkgname in MMAP
    int offset = 0;
    
    //using bsearch for fast lookup in MMAP (assuming MMAP is sorted alphabetically already)
    Stanza key;
    key.pkgname = (char*)pkgname;
    Stanza* rez = bsearch(&key,  MAP->MMAP, MAP->pkgcount, sizeof(Stanza), compare_stanzas);
    if(!rez){
        fprintf(stderr, "couldnt find package: %s\n", pkgname);
        return NULL;
    }
    offset = rez->offset;

    int flag = fseek(f, offset, SEEK_SET);
    if(flag != 0){
        perror("plm eroare");
        return NULL;
    }

    DebianPackage* result = NULL;
    char line[1024];
    char* p = line;
    while(fgets(line, 1024, f)){
        if(strncmp(line, "Package: ", 9) == 0){
            char current_name[256];
            sscanf(line + 9, "%s", current_name);

            if(strcmp(pkgname, current_name) == 0){
                result = calloc(1, sizeof(DebianPackage));
                result->name = strdup(current_name);
            }
        }
        if(strncmp(line, "Version: ", 9) == 0){
                p = line + 9;
                result->version = strdup(p);
                clean_newline(result->version);
        }
        if(strncmp(line, "Depends: ", 9) == 0){
                p = line + 9;
                result->depends = strdup(p);
                clean_newline(result->depends);
        }
        if(strncmp(line, "Filename: ", 10) == 0){
                p = line + 10;
                result->filename = strdup(p);
                clean_newline(result->filename);
        }
        if(line[0] == '\n' || line[0] == '\r'){
                break;
        }
    }
    
    return result; //must free() and also fclose f after
}

char* build_url_from_filename(DebianPackage* pkg){
    char* fmt = "https://ftp.debian.org/debian/%s";
    int url_size = (snprintf(NULL, 0, fmt, pkg->filename));
    if(url_size<0){
        perror("ceva nu merge");
        return NULL;
    }

    char* url = malloc(url_size + 10);

    if(!url){
        perror("build_url: malloc");
        return NULL;
    }
    sprintf(url, fmt, pkg->filename);
    return url;//must free()
}

//Depends: libc6 (>= 2.14), libusb-0.1-4 (>= 2:0.1.12)

char** split_dependencies(char* deps_str, int* cnt){
    if(!deps_str){
        *cnt = 0;
        return NULL;
    }
    char** list = malloc((size_t)(sizeof(char*)*64));
    if(!list){
        perror("split_deps: malloc");
        return NULL;
    }
    char* copy = strdup(deps_str);
    char* saveptr;
    int i = 0;
    char* token = strtok_r(copy, ",", &saveptr);
    while(token != NULL && i < 64){
        while(*token && isspace((unsigned char)*token)){
            token++;
        }
        if(*token == '('){
            token = strtok_r(NULL, ",", &saveptr);
            continue;
        }
        char* p = strchr(token, ' ');
        if(p){
            *p = '\0';
        }
        list[i] = strdup(token);
        i++;
        token = strtok_r(NULL, ",", &saveptr);
    }
    *cnt = i;
    free(copy);
    return list;//must free()
}
//, ziblg\0(version cct)\0