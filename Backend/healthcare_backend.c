// healthcare_backend.c
// Integrated backend for encrypted healthcare data storage on DigitalOcean Spaces
// Combines AES-256-CBC encryption with S3-compatible storage
// Compile with: gcc healthcare_backend.c s3_auth.c -o healthcare_backend -lcurl -lcrypto

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include "s3_auth.h"

#define AES_KEYLEN 32  // 256 bits
#define AES_IVLEN 16   // 128 bits
#define BUFFER_SIZE 8192

// Configuration from environment
#define DO_SPACE_NAME getenv("DO_SPACE_NAME")
#define DO_REGION getenv("DO_REGION")
#define DO_ACCESS_KEY getenv("DO_ACCESS_KEY")
#define DO_SECRET_KEY getenv("DO_SECRET_KEY")
#define ENCRYPTION_KEY getenv("ENCRYPTION_KEY")  // Should be 32 bytes (64 hex chars)

// Error handling
void handle_error(const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(1);
}

// Convert hex string to bytes
int hex_to_bytes(const char *hex, unsigned char *bytes, int len) {
    if (strlen(hex) != len * 2) return -1;
    for (int i = 0; i < len; i++) {
        sscanf(hex + 2*i, "%2hhx", &bytes[i]);
    }
    return 0;
}

// Convert bytes to hex string
void bytes_to_hex(const unsigned char *bytes, int len, char *hex) {
    for (int i = 0; i < len; i++) {
        sprintf(hex + 2*i, "%02x", bytes[i]);
    }
    hex[len * 2] = '\0';
}

// Encrypt file and return encrypted content + IV
// Format: [16-byte IV][encrypted data]
int encrypt_file(const char *input_path, const char *output_path, const unsigned char *key) {
    FILE *in = fopen(input_path, "rb");
    if (!in) {
        perror("Failed to open input file");
        return -1;
    }
    
    FILE *out = fopen(output_path, "wb");
    if (!out) {
        perror("Failed to open output file");
        fclose(in);
        return -1;
    }
    
    // Generate random IV
    unsigned char iv[AES_IVLEN];
    if (RAND_bytes(iv, AES_IVLEN) != 1) {
        fprintf(stderr, "Failed to generate IV\n");
        fclose(in);
        fclose(out);
        return -1;
    }
    
    // Write IV to output file (needed for decryption)
    fwrite(iv, 1, AES_IVLEN, out);
    
    // Initialize encryption context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handle_error("Failed to create cipher context");
    
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        handle_error("Failed to initialize encryption");
    }
    
    // Encrypt file in chunks
    unsigned char buffer_in[BUFFER_SIZE];
    unsigned char buffer_out[BUFFER_SIZE + EVP_CIPHER_block_size(EVP_aes_256_cbc())];
    int bytes_read, bytes_encrypted;
    
    while ((bytes_read = fread(buffer_in, 1, BUFFER_SIZE, in)) > 0) {
        if (EVP_EncryptUpdate(ctx, buffer_out, &bytes_encrypted, buffer_in, bytes_read) != 1) {
            handle_error("Encryption failed");
        }
        fwrite(buffer_out, 1, bytes_encrypted, out);
    }
    
    // Finalize encryption
    if (EVP_EncryptFinal_ex(ctx, buffer_out, &bytes_encrypted) != 1) {
        handle_error("Encryption finalization failed");
    }
    fwrite(buffer_out, 1, bytes_encrypted, out);
    
    // Cleanup
    EVP_CIPHER_CTX_free(ctx);
    fclose(in);
    fclose(out);
    
    printf("File encrypted successfully: %s -> %s\n", input_path, output_path);
    return 0;
}

// Decrypt file (expects format: [16-byte IV][encrypted data])
int decrypt_file(const char *input_path, const char *output_path, const unsigned char *key) {
    FILE *in = fopen(input_path, "rb");
    if (!in) {
        perror("Failed to open encrypted file");
        return -1;
    }
    
    FILE *out = fopen(output_path, "wb");
    if (!out) {
        perror("Failed to open output file");
        fclose(in);
        return -1;
    }
    
    // Read IV from file
    unsigned char iv[AES_IVLEN];
    if (fread(iv, 1, AES_IVLEN, in) != AES_IVLEN) {
        fprintf(stderr, "Failed to read IV from encrypted file\n");
        fclose(in);
        fclose(out);
        return -1;
    }
    
    // Initialize decryption context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handle_error("Failed to create cipher context");
    
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        handle_error("Failed to initialize decryption");
    }
    
    // Decrypt file in chunks
    unsigned char buffer_in[BUFFER_SIZE];
    unsigned char buffer_out[BUFFER_SIZE + EVP_CIPHER_block_size(EVP_aes_256_cbc())];
    int bytes_read, bytes_decrypted;
    
    while ((bytes_read = fread(buffer_in, 1, BUFFER_SIZE, in)) > 0) {
        if (EVP_DecryptUpdate(ctx, buffer_out, &bytes_decrypted, buffer_in, bytes_read) != 1) {
            handle_error("Decryption failed");
        }
        fwrite(buffer_out, 1, bytes_decrypted, out);
    }
    
    // Finalize decryption
    if (EVP_DecryptFinal_ex(ctx, buffer_out, &bytes_decrypted) != 1) {
        handle_error("Decryption finalization failed - data may be corrupted");
    }
    fwrite(buffer_out, 1, bytes_decrypted, out);
    
    // Cleanup
    EVP_CIPHER_CTX_free(ctx);
    fclose(in);
    fclose(out);
    
    printf("File decrypted successfully: %s -> %s\n", input_path, output_path);
    return 0;
}

// Calculate SHA256 hash of file
char* hash_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    
    unsigned char buffer[BUFFER_SIZE];
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
    bytes_to_hex(hash, SHA256_DIGEST_LENGTH, hex);
    return hex;
}

// Upload encrypted file to DigitalOcean Spaces
int upload_to_spaces(const char *local_path, const char *remote_name) {
    if (!DO_SPACE_NAME || !DO_REGION || !DO_ACCESS_KEY || !DO_SECRET_KEY) {
        fprintf(stderr, "Missing DigitalOcean credentials in environment\n");
        return -1;
    }
    
    FILE *fd = fopen(local_path, "rb");
    if (!fd) {
        perror("Failed to open file for upload");
        return -1;
    }
    
    // Get file size
    fseek(fd, 0, SEEK_END);
    long file_size = ftell(fd);
    rewind(fd);
    
    // Calculate content hash
    char *content_hash = hash_file(local_path);
    if (!content_hash) {
        fclose(fd);
        return -1;
    }
    
    // Build URL and host
    char url[1024], host[512];
    snprintf(host, sizeof(host), "%s.%s.digitaloceanspaces.com", DO_SPACE_NAME, DO_REGION);
    snprintf(url, sizeof(url), "https://%s/%s", host, remote_name);
    
    // Setup S3 credentials and request
    S3Credentials creds = {
        .access_key = DO_ACCESS_KEY,
        .secret_key = DO_SECRET_KEY,
        .region = DO_REGION,
        .service = "s3"
    };
    
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
    
    char *auth_header = s3_generate_auth_header(&creds, &req);
    
    // Setup curl
    CURL *curl = curl_easy_init();
    if (!curl) {
        free(content_hash);
        free(auth_header);
        fclose(fd);
        return -1;
    }
    
    // Build headers
    char timestamp[32];
    s3_get_iso8601_timestamp(now, timestamp, sizeof(timestamp));
    
    struct curl_slist *headers = NULL;
    char header_buf[2048];
    
    snprintf(header_buf, sizeof(header_buf), "x-amz-date: %s", timestamp);
    headers = curl_slist_append(headers, header_buf);
    
    snprintf(header_buf, sizeof(header_buf), "x-amz-content-sha256: %s", content_hash);
    headers = curl_slist_append(headers, header_buf);
    
    snprintf(header_buf, sizeof(header_buf), "Authorization: %s", auth_header);
    headers = curl_slist_append(headers, header_buf);
    
    headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
    headers = curl_slist_append(headers, "x-amz-acl: private");
    
    // Configure curl
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_READDATA, fd);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_size);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    
    // Perform upload
    CURLcode res = curl_easy_perform(curl);
    int result = 0;
    
    if (res != CURLE_OK) {
        fprintf(stderr, "Upload failed: %s\n", curl_easy_strerror(res));
        result = -1;
    } else {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code == 200) {
            printf("Uploaded to DigitalOcean Spaces: %s\n", remote_name);
        } else {
            fprintf(stderr, "Upload failed with HTTP %ld\n", response_code);
            result = -1;
        }
    }
    
    // Cleanup
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(content_hash);
    free(auth_header);
    fclose(fd);
    
    return result;
}

// Download from DigitalOcean Spaces
int download_from_spaces(const char *remote_name, const char *local_path) {
    if (!DO_SPACE_NAME || !DO_REGION || !DO_ACCESS_KEY || !DO_SECRET_KEY) {
        fprintf(stderr, "Missing DigitalOcean credentials in environment\n");
        return -1;
    }
    
    FILE *fd = fopen(local_path, "wb");
    if (!fd) {
        perror("Failed to open file for download");
        return -1;
    }
    
    // Build URL and host
    char url[1024], host[512];
    snprintf(host, sizeof(host), "%s.%s.digitaloceanspaces.com", DO_SPACE_NAME, DO_REGION);
    snprintf(url, sizeof(url), "https://%s/%s", host, remote_name);
    
    // Setup S3 credentials and request
    S3Credentials creds = {
        .access_key = DO_ACCESS_KEY,
        .secret_key = DO_SECRET_KEY,
        .region = DO_REGION,
        .service = "s3"
    };
    
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
    
    char *auth_header = s3_generate_auth_header(&creds, &req);
    
    // Setup curl
    CURL *curl = curl_easy_init();
    if (!curl) {
        free(empty_hash);
        free(auth_header);
        fclose(fd);
        return -1;
    }
    
    // Build headers
    char timestamp[32];
    s3_get_iso8601_timestamp(now, timestamp, sizeof(timestamp));
    
    struct curl_slist *headers = NULL;
    char header_buf[2048];
    
    snprintf(header_buf, sizeof(header_buf), "x-amz-date: %s", timestamp);
    headers = curl_slist_append(headers, header_buf);
    
    snprintf(header_buf, sizeof(header_buf), "x-amz-content-sha256: %s", empty_hash);
    headers = curl_slist_append(headers, header_buf);
    
    snprintf(header_buf, sizeof(header_buf), "Authorization: %s", auth_header);
    headers = curl_slist_append(headers, header_buf);
    
    // Configure curl
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fd);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    
    // Perform download
    CURLcode res = curl_easy_perform(curl);
    int result = 0;
    
    if (res != CURLE_OK) {
        fprintf(stderr, "Download failed: %s\n", curl_easy_strerror(res));
        result = -1;
    } else {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code == 200) {
            printf("Downloaded from DigitalOcean Spaces: %s\n", remote_name);
        } else {
            fprintf(stderr, "Download failed with HTTP %ld\n", response_code);
            result = -1;
        }
    }
    
    // Cleanup
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(empty_hash);
    free(auth_header);
    fclose(fd);
    
    return result;
}

// Main workflow functions
int store_patient_record(const char *patient_file, const char *patient_id) {
    printf("\n=== Storing Patient Record ===\n");
    printf("Patient ID: %s\n", patient_file);
    
    // Validate encryption key
    const char *key_hex = ENCRYPTION_KEY;
    if (!key_hex || strlen(key_hex) != 64) {
        fprintf(stderr, "Invalid ENCRYPTION_KEY (must be 64 hex chars / 32 bytes)\n");
        return -1;
    }
    
    unsigned char key[AES_KEYLEN];
    if (hex_to_bytes(key_hex, key, AES_KEYLEN) != 0) {
        fprintf(stderr, "Failed to parse encryption key\n");
        return -1;
    }
    
    // Step 1: Encrypt the file
    char encrypted_path[512];
    snprintf(encrypted_path, sizeof(encrypted_path), "%s.encrypted", patient_file);
    
    if (encrypt_file(patient_file, encrypted_path, key) != 0) {
        return -1;
    }
    
    // Step 2: Calculate hash for Solana verification
    char *file_hash = hash_file(encrypted_path);
    if (!file_hash) {
        fprintf(stderr, "Failed to calculate file hash\n");
        return -1;
    }
    
    printf("Encrypted file hash (for Solana): %s\n", file_hash);
    
    // Step 3: Upload to DigitalOcean Spaces
    char remote_name[512];
    snprintf(remote_name, sizeof(remote_name), "patients/%s.encrypted", patient_id);
    
    if (upload_to_spaces(encrypted_path, remote_name) != 0) {
        free(file_hash);
        return -1;
    }
    
    printf("\n✓ Patient record stored successfully!\n");
    printf("  Remote path: %s\n", remote_name);
    printf("  Hash for blockchain: %s\n", file_hash);
    printf("\nNext step: Store this hash on Solana blockchain for verification\n");
    
    free(file_hash);
    return 0;
}

int retrieve_patient_record(const char *patient_id, const char *output_file) {
    printf("\n=== Retrieving Patient Record ===\n");
    printf("Patient ID: %s\n", patient_id);
    
    // Validate encryption key
    const char *key_hex = ENCRYPTION_KEY;
    if (!key_hex || strlen(key_hex) != 64) {
        fprintf(stderr, "Invalid ENCRYPTION_KEY (must be 64 hex chars / 32 bytes)\n");
        return -1;
    }
    
    unsigned char key[AES_KEYLEN];
    if (hex_to_bytes(key_hex, key, AES_KEYLEN) != 0) {
        fprintf(stderr, "Failed to parse encryption key\n");
        return -1;
    }
    
    // Step 1: Download from DigitalOcean Spaces
    char remote_name[512];
    char encrypted_path[512];
    snprintf(remote_name, sizeof(remote_name), "patients/%s.encrypted", patient_id);
    snprintf(encrypted_path, sizeof(encrypted_path), "%s.encrypted.tmp", output_file);
    
    if (download_from_spaces(remote_name, encrypted_path) != 0) {
        return -1;
    }
    
    // Step 2: Verify hash against Solana blockchain (placeholder)
    char *downloaded_hash = hash_file(encrypted_path);
    if (downloaded_hash) {
        printf("Downloaded file hash: %s\n", downloaded_hash);
        printf("TODO: Verify this hash against Solana blockchain\n");
        free(downloaded_hash);
    }
    
    // Step 3: Decrypt the file
    if (decrypt_file(encrypted_path, output_file, key) != 0) {
        return -1;
    }
    
    // Cleanup temp file
    remove(encrypted_path);
    
    printf("\n✓ Patient record retrieved successfully!\n");
    printf("  Decrypted to: %s\n", output_file);
    
    return 0;
}

int main(int argc, char *argv[]) {
    printf("Healthcare Data Backend - Secure Storage System\n");
    printf("================================================\n");
    
    if (argc < 2) {
        printf("\nUsage:\n");
        printf("  %s store <patient_file> <patient_id>\n", argv[0]);
        printf("      - Encrypt and upload patient record\n");
        printf("  %s retrieve <patient_id> <output_file>\n", argv[0]);
        printf("      - Download and decrypt patient record\n");
        printf("\nRequired environment variables:\n");
        printf("  DO_SPACE_NAME    - DigitalOcean Space name\n");
        printf("  DO_REGION        - Region (nyc3, sfo3, etc.)\n");
        printf("  DO_ACCESS_KEY    - Access key\n");
        printf("  DO_SECRET_KEY    - Secret key\n");
        printf("  ENCRYPTION_KEY   - 256-bit key (64 hex chars)\n");
        printf("\nExample:\n");
        printf("  export ENCRYPTION_KEY=$(openssl rand -hex 32)\n");
        printf("  %s store patient_data.json patient_12345\n", argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "store") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: %s store <patient_file> <patient_id>\n", argv[0]);
            return 1;
        }
        return store_patient_record(argv[2], argv[3]);
        
    } else if (strcmp(argv[1], "retrieve") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: %s retrieve <patient_id> <output_file>\n", argv[0]);
            return 1;
        }
        return retrieve_patient_record(argv[2], argv[3]);
        
    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        fprintf(stderr, "Valid commands: store, retrieve\n");
        return 1;
    }
}
