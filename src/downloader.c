#include<stdlib.h>
#include<stdio.h>
#include<curl/curl.h>
#include<unistd.h>
#include"fs.h"
#include"index.h"

typedef struct {
    DebianPackage* package;
    FILE* f;
    double last_printed_pcnt;
}DownloadInfo;

int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, 
                      curl_off_t ultotal, curl_off_t ulnow) {
    DownloadInfo *info = (DownloadInfo *)clientp;

    // dltotal is 0 until the server sends the content-length header
    if (dltotal <= 0) return 0;

    double percentage = (double)dlnow / (double)dltotal * 100;

    // Only update the screen if progress has changed by at least 1%
    // This saves massive amounts of CPU on old hardware
    if (percentage - info->last_printed_pcnt >= 1.0 || percentage >= 100.0) {
        info->last_printed_pcnt = percentage;
        
        printf("\r[DWLD] %-20s: [%-20.*s] %.1f%%", 
               info->package->name, 
               (int)(percentage / 5), "####################", 
               percentage);
        fflush(stdout);
    }

    return 0; // Return 1 here if you ever want to "cancel" a download
}

void add_package_to_multi(CURLM* handle, CURL* easy_handle, DebianPackage* package){
        char* path = build_cache_path_for_package(package);
        char* url = build_url_from_filename(package);
        FILE* f = fopen(path, "wb");
        if(!f){
            fprintf(stderr, "DWLD: couldnt open write file");
            free(path);
            free(url);
            return;
        }
        DownloadInfo* context = malloc(sizeof(DownloadInfo));
        context->package = package;
        context->f = f;
        context->last_printed_pcnt = 0.0;
        curl_easy_setopt(easy_handle, CURLOPT_URL, url);
        curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, f);
        curl_easy_setopt(easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(easy_handle, CURLOPT_FAILONERROR, 1L);
        curl_easy_setopt(easy_handle, CURLOPT_PRIVATE, context);
        curl_easy_setopt(easy_handle, CURLOPT_XFERINFOFUNCTION, progress_callback);
        curl_easy_setopt(easy_handle, CURLOPT_XFERINFODATA, context);
        curl_easy_setopt(easy_handle, CURLOPT_NOPROGRESS, 0L);

        curl_multi_add_handle(handle, easy_handle);
        free(url);
        free(path);
}

int download_to_file(DebianPackage** queue, const int queue_cnt){
    CURLM* handle = curl_multi_init();
    if(!handle){
        fprintf(stderr, "DWLD: could not init curl multi handle");
        return 0;
    }

    int download_cnt = 0;
    int max_parallel = 4;
    CURL** e_handles = malloc(sizeof(CURL*)*max_parallel);
    
    //init easy handles
    for(int i = 0; i< max_parallel; i++){
        e_handles[i] = curl_easy_init();
    }

    while(download_cnt < queue_cnt && download_cnt < max_parallel){
        add_package_to_multi(handle, e_handles[download_cnt], queue[download_cnt]);
        download_cnt++;
    }

    int still_running = 0;
    //still_running is used by curl_multi_perform
    do {
        curl_multi_perform(handle, &still_running);
        
        CURLMsg* message;
        int msgs_left;
        while((message = curl_multi_info_read(handle, &msgs_left))){
            if(message->msg == CURLMSG_DONE){
                CURL* temp = message->easy_handle;
                DownloadInfo* info = NULL;
                curl_easy_getinfo(temp, CURLINFO_PRIVATE, &info);
                if(info){
                    printf("\nFinished downloading package: %s\n", info->package->name);
                    fclose(info->f);
                    free(info);
                    curl_easy_reset(temp);
                    if(download_cnt < queue_cnt){
                        add_package_to_multi(handle, temp, queue[download_cnt]);
                        download_cnt++;
                        curl_multi_perform(handle, &still_running);
                    }
                }
            }
        }
        if (still_running) {
            int numfds;
            curl_multi_wait(handle, NULL, 0, 1000, &numfds);
            if(numfds == 0) {
                // If no network activity is detected, don't trust the wait.
                // Sleep for a tiny bit to prevent CPU pegging, then try perform again.
                usleep(100000); // 100ms
            }
        }
    }while(still_running > 0 || download_cnt < queue_cnt);//potential werid stuff with asynchronous fuctions i.e. curl_multi_perform may set still_running to 0 for a moment

    //cleanup after downloads are done
    for(int i = 0; i < max_parallel; i++){
        curl_easy_cleanup(e_handles[i]);
    }
    //queue was never free()'d
    return 0;
}

int download_index(){
    CURL* handle = curl_easy_init();
    CURLcode res;
    char* outfile = "Packages.gz";
    FILE* file = fopen(outfile, "wb");
    if(!file){
        perror("couldnt open /pacman");
        curl_easy_cleanup(handle);
        return -1;
    }
    char* url = "https://ftp.debian.org/debian/dists/bookworm/main/binary-amd64/Packages.gz";

    curl_easy_setopt(handle, CURLOPT_URL, url);
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(handle, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, NULL);

    res = curl_easy_perform(handle);
    fclose(file);
    curl_easy_cleanup(handle);

    if(res == CURLE_OK){
        printf("index downloaded, extracting...\n");
        system("gunzip -f Packages.gz");
        return 0;
    }

    return -1;
}
