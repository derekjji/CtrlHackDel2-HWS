// digitalocean_backend.c
// Production backend for uploading and downloading encrypted files to DigitalOcean Spaces
// Requires libcurl for HTTP requests and OpenSSL for S3 authentication
// Compile with: gcc digitalocean_backend.c s3_auth.c -o do_backend -lcurl -lcrypto

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <openssl/sha.h>
#include "s3_auth.h"

// Configuration - Load from environment or config file in production
#define DO_SPACE_NAME getenv("DO_SPACE_NAME")
#define DO_REGION getenv("DO_REGION")
#define DO_ACCESS_KEY getenv("DO_ACCESS_KEY")
#define DO_SECRET_KEY getenv("DO_SECRET_KEY")

// Calculate SHA256 hash of file content
char* hash_file_content(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    
    unsigned char buffer[8192];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), f)) != 0) {
        SHA256_Update(&sha256, buffer, bytes);
    }
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256);
    fclose(f);
    
    char *hex = malloc(SHA256_DIGEST_LENGTH * 2 + 1);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hex + (i * 2), "%02x", hash[i]);
    }
    hex[SHA256_DIGEST_LENGTH * 2] = '\0';
    return hex;
}

// Uploads a file to DigitalOcean Spaces with S3 authentication
int upload_file(const char *local_path, const char *remote_name) {
    // Validate environment variables
    if (!DO_SPACE_NAME || !DO_REGION || !DO_ACCESS_KEY || !DO_SECRET_KEY) {
        fprintf(stderr, "Error: Missing required environment variables\n");
        fprintf(stderr, "Please set: DO_SPACE_NAME, DO_REGION, DO_ACCESS_KEY, DO_SECRET_KEY\n");
        return 1;
    }
    
    FILE *fd = fopen(local_path, "rb");
    if (!fd) {
        perror("File open failed");
        return 1;
    }
    
    // Get file size
    fseek(fd, 0, SEEK_END);
    long file_size = ftell(fd);
    rewind(fd);
    
    // Calculate content hash
    char *content_hash = hash_file_content(local_path);
    if (!content_hash) {
        fclose(fd);
        return 1;
    }
    
    // Build URL and host
    char url[1024];
    char host[512];
    snprintf(host, sizeof(host), "%s.%s.digitaloceanspaces.com", DO_SPACE_NAME, DO_REGION);
    snprintf(url, sizeof(url), "https://%s/%s", host, remote_name);
    
    // Setup S3 credentials
    S3Credentials creds = {
        .access_key = DO_ACCESS_KEY,
        .secret_key = DO_SECRET_KEY,
        .region = DO_REGION,
        .service = "s3"
    };
    
    // Setup S3 request
    time_t now = time(NULL);
    char uri[512];
    snprintf(uri, sizeof(uri), "/%s", remote_name);
    
    S3Request req = {
        .method = "PUT",
        .host = host,
        .uri = uri,
        .query_string = NULL,
        .content_hash = content_hash,
        .timestamp = now
    };
    
    // Generate auth header
    char *auth_header = s3_generate_auth_header(&creds, &req);
    
    // Setup curl
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize curl\n");
        free(content_hash);
        free(auth_header);
        fclose(fd);
        return 1;
    }
    
    // Build headers
    char timestamp[32];
    s3_get_iso8601_timestamp(now, timestamp, sizeof(timestamp));
    
    char date_header[128];
    char hash_header[128];
    char auth_header_full[2048];
    
    snprintf(date_header, sizeof(date_header), "x-amz-date: %s", timestamp);
    snprintf(hash_header, sizeof(hash_header), "x-amz-content-sha256: %s", content_hash);
    snprintf(auth_header_full, sizeof(auth_header_full), "Authorization: %s", auth_header);
    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, date_header);
    headers = curl_slist_append(headers, hash_header);
    headers = curl_slist_append(headers, auth_header_full);
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
    
    // Configure curl
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_READDATA, fd);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_size);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    
    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "Upload failed: %s\n", curl_easy_strerror(res));
    } else {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code == 200) {
            printf("Upload successful: %s -> %s\n", local_path, remote_name);
        } else {
            fprintf(stderr, "Upload failed with HTTP code: %ld\n", response_code);
            res = CURLE_HTTP_RETURNED_ERROR;
        }
    }
    
    // Cleanup
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(content_hash);
    free(auth_header);
    fclose(fd);
    
    return (res == CURLE_OK) ? 0 : 1;
}

// Downloads a file from DigitalOcean Spaces with S3 authentication
int download_file(const char *remote_name, const char *local_path) {
    // Validate environment variables
    if (!DO_SPACE_NAME || !DO_REGION || !DO_ACCESS_KEY || !DO_SECRET_KEY) {
        fprintf(stderr, "Error: Missing required environment variables\n");
        fprintf(stderr, "Please set: DO_SPACE_NAME, DO_REGION, DO_ACCESS_KEY, DO_SECRET_KEY\n");
        return 1;
    }
    
    FILE *fd = fopen(local_path, "wb");
    if (!fd) {
        perror("File open failed");
        return 1;
    }
    
    // Build URL and host
    char url[1024];
    char host[512];
    snprintf(host, sizeof(host), "%s.%s.digitaloceanspaces.com", DO_SPACE_NAME, DO_REGION);
    snprintf(url, sizeof(url), "https://%s/%s", host, remote_name);
    
    // Setup S3 credentials
    S3Credentials creds = {
        .access_key = DO_ACCESS_KEY,
        .secret_key = DO_SECRET_KEY,
        .region = DO_REGION,
        .service = "s3"
    };
    
    // Setup S3 request (GET has empty content hash)
    time_t now = time(NULL);
    char uri[512];
    snprintf(uri, sizeof(uri), "/%s", remote_name);
    
    char *empty_hash = sha256_hex((unsigned char*)"", 0);
    
    S3Request req = {
        .method = "GET",
        .host = host,
        .uri = uri,
        .query_string = NULL,
        .content_hash = empty_hash,
        .timestamp = now
    };
    
    // Generate auth header
    char *auth_header = s3_generate_auth_header(&creds, &req);
    
    // Setup curl
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize curl\n");
        free(empty_hash);
        free(auth_header);
        fclose(fd);
        return 1;
    }
    
    // Build headers
    char timestamp[32];
    s3_get_iso8601_timestamp(now, timestamp, sizeof(timestamp));
    
    char date_header[128];
    char hash_header[128];
    char auth_header_full[2048];
    
    snprintf(date_header, sizeof(date_header), "x-amz-date: %s", timestamp);
    snprintf(hash_header, sizeof(hash_header), "x-amz-content-sha256: %s", empty_hash);
    snprintf(auth_header_full, sizeof(auth_header_full), "Authorization: %s", auth_header);
    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, date_header);
    headers = curl_slist_append(headers, hash_header);
    headers = curl_slist_append(headers, auth_header_full);
    
    // Configure curl
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fd);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    
    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "Download failed: %s\n", curl_easy_strerror(res));
    } else {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code == 200) {
            printf("Download successful: %s -> %s\n", remote_name, local_path);
        } else {
            fprintf(stderr, "Download failed with HTTP code: %ld\n", response_code);
            res = CURLE_HTTP_RETURNED_ERROR;
        }
    }
    
    // Cleanup
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(empty_hash);
    free(auth_header);
    fclose(fd);
    
    return (res == CURLE_OK) ? 0 : 1;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("DigitalOcean Spaces Backend for Healthcare Data\n");
        printf("Usage: %s upload|download <local_file> <remote_name>\n", argv[0]);
        printf("\nEnvironment variables required:\n");
        printf("  DO_SPACE_NAME - Your DigitalOcean Space name\n");
        printf("  DO_REGION - Region (e.g., nyc3, sfo3)\n");
        printf("  DO_ACCESS_KEY - Your access key\n");
        printf("  DO_SECRET_KEY - Your secret key\n");
        return 1;
    }
    
    if (strcmp(argv[1], "upload") == 0) {
        return upload_file(argv[2], argv[3]);
    } else if (strcmp(argv[1], "download") == 0) {
        return download_file(argv[3], argv[2]);
    } else {
        fprintf(stderr, "Error: Unknown command '%s'\n", argv[1]);
        fprintf(stderr, "Valid commands: upload, download\n");
        return 1;
    }
}
