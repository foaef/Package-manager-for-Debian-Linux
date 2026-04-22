#ifndef FS_H
#define FS_H

typedef struct {
    char *name;
    char *version;
    char *depends;
    char *filename;
} DebianPackage;

typedef struct{
    char* pkgname;
    int offset;
}Stanza;

typedef struct{
    Stanza* MMAP;
    int pkgcount;
}PKGMAP;

int search_index(char* keyword);
void flush_cache(void);
void fs_mmap_index(PKGMAP* PKGMAP);
int fs_init_directories(void);
int ensure_cache_path_dir(void);
char* build_cache_path_for_package(DebianPackage* pkg);

#endif