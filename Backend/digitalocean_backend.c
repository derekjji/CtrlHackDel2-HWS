// digitalocean_backend.c
// Basic backend for uploading and downloading encrypted files to DigitalOcean Spaces
// Requires libcurl for HTTP requests
// Compile with: gcc digitalocean_backend.c -o do_backend -lcurl

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define DO_SPACES_URL "https://<your-space-name>.<region>.digitaloceanspaces.com/"
#define DO_ACCESS_KEY "<your-access-key>"
#define DO_SECRET_KEY "<your-secret-key>"

// Uploads a file to DigitalOcean Spaces
int upload_file(const char *local_path, const char *remote_name) {
    CURL *curl;
    CURLcode res;
    FILE *fd = fopen(local_path, "rb");
    if (!fd) {
        perror("File open failed");
        return 1;
    }
    curl = curl_easy_init();
    if (curl) {
        char url[512];
        snprintf(url, sizeof(url), "%s%s", DO_SPACES_URL, remote_name);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_READDATA, fd);
        // TODO: Add authentication headers (AWS S3 signature v4)
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
    }
    fclose(fd);
    return 0;
}

// Downloads a file from DigitalOcean Spaces
int download_file(const char *remote_name, const char *local_path) {
    CURL *curl;
    CURLcode res;
    FILE *fd = fopen(local_path, "wb");
    if (!fd) {
        perror("File open failed");
        return 1;
    }
    curl = curl_easy_init();
    if (curl) {
        char url[512];
        snprintf(url, sizeof(url), "%s%s", DO_SPACES_URL, remote_name);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fd);
        // TODO: Add authentication headers (AWS S3 signature v4)
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
    }
    fclose(fd);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s upload|download <local> <remote>\n", argv[0]);
        return 1;
    }
    if (strcmp(argv[1], "upload") == 0) {
        return upload_file(argv[2], argv[3]);
    } else if (strcmp(argv[1], "download") == 0) {
        return download_file(argv[3], argv[2]);
    } else {
        printf("Unknown command\n");
        return 1;
    }
}
