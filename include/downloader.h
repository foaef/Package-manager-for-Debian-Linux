#ifndef DWLD_H
#define DWLD_H

#include<curl/curl.h>

typedef struct {
    DebianPackage* package;
    FILE* f;
}DownloadInfo;

int download_index();
int download_to_file(DebianPackage** queue, const int queue_cnt);
void add_package_to_multi(CURLM* handle, CURL* easy_handle, DebianPackage* package);

#endif