#ifndef INDEX_H
#define INDEX_H

#include"fs.h"

void clean_newline(char* s);
DebianPackage* index_get_package_info(const char* pkgname, FILE* f, PKGMAP* MAP);
void free_debian_package(DebianPackage* pkg);
char* build_url_from_filename(DebianPackage* pkg);
char** split_dependencies(char* deps_str, int* cnt);
int compare_stanzas(const void* a, const void* b);

#endif