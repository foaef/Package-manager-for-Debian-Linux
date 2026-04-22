#ifndef RESOLVER_H
#define RESOLVER_H

#include"index.h"

extern DebianPackage* INSTALL_QUEUE[2048];
extern int queue_count;

void add_to_install_list(DebianPackage* pkg);
int is_in_install_queue(DebianPackage* pkg);
void add_to_visited(DebianPackage* pkg); 
int is_visited(DebianPackage* pkg);
int is_installed(const char* pkgname); 
void resolve_package(DebianPackage* package, FILE* f, PKGMAP* MAP);

#endif