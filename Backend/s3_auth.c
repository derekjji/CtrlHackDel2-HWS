// s3_auth.c
// AWS S3 Signature Version 4 implementation for DigitalOcean Spaces

#include "s3_auth.h"
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Calculate SHA256 hash and return as hex string
char* sha256_hex(const unsigned char *data, size_t len) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(data, len, hash);
    
    char *hex = malloc(SHA256_DIGEST_LENGTH * 2 + 1);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hex + (i * 2), "%02x", hash[i]);
    }
    hex[SHA256_DIGEST_LENGTH * 2] = '\0';
    return hex;
}

// Calculate HMAC-SHA256
void hmac_sha256(const unsigned char *key, size_t key_len,
                 const unsigned char *data, size_t data_len,
                 unsigned char *out) {
    unsigned int len;
    HMAC(EVP_sha256(), key, key_len, data, data_len, out, &len);
}

// Generate ISO8601 timestamp
void s3_get_iso8601_timestamp(time_t t, char *buffer, size_t len) {
    struct tm *tm_info = gmtime(&t);
    strftime(buffer, len, "%Y%m%dT%H%M%SZ", tm_info);
}

// Generate date in YYYYMMDD format
void s3_get_date(time_t t, char *buffer, size_t len) {
    struct tm *tm_info = gmtime(&t);
    strftime(buffer, len, "%Y%m%d", tm_info);
}

// Create canonical request
static char* create_canonical_request(const S3Request *req) {
    char *canonical = malloc(4096);
    
    // Canonical Request format:
    // HTTP_METHOD\n
    // CANONICAL_URI\n
    // CANONICAL_QUERY_STRING\n
    // CANONICAL_HEADERS\n
    // SIGNED_HEADERS\n
    // HASHED_PAYLOAD
    
    char timestamp[32];
    s3_get_iso8601_timestamp(req->timestamp, timestamp, sizeof(timestamp));
    
    snprintf(canonical, 4096,
             "%s\n"           // HTTP method
             "%s\n"           // URI
             "%s\n"           // Query string
             "host:%s\n"      // Canonical headers
             "x-amz-content-sha256:%s\n"
             "x-amz-date:%s\n"
             "\n"             // End of headers
             "host;x-amz-content-sha256;x-amz-date\n"  // Signed headers
             "%s",            // Hashed payload
             req->method,
             req->uri,
             req->query_string ? req->query_string : "",
             req->host,
             req->content_hash,
             timestamp,
             req->content_hash);
    
    return canonical;
}

// Create string to sign
static char* create_string_to_sign(const S3Credentials *creds, const S3Request *req,
                                   const char *canonical_request_hash) {
    char *string_to_sign = malloc(1024);
    char timestamp[32], date[16];
    
    s3_get_iso8601_timestamp(req->timestamp, timestamp, sizeof(timestamp));
    s3_get_date(req->timestamp, date, sizeof(date));
    
    // String to Sign format:
    // Algorithm\n
    // RequestDateTime\n
    // CredentialScope\n
    // HashedCanonicalRequest
    
    snprintf(string_to_sign, 1024,
             "AWS4-HMAC-SHA256\n"
             "%s\n"
             "%s/%s/%s/aws4_request\n"
             "%s",
             timestamp,
             date,
             creds->region,
             creds->service,
             canonical_request_hash);
    
    return string_to_sign;
}

// Calculate signing key
static void get_signing_key(const S3Credentials *creds, time_t t, unsigned char *signing_key) {
    char date[16];
    s3_get_date(t, date, sizeof(date));
    
    // Signing Key = HMAC(HMAC(HMAC(HMAC("AWS4" + secret_key, date), region), service), "aws4_request")
    
    char secret_key_with_prefix[256];
    snprintf(secret_key_with_prefix, sizeof(secret_key_with_prefix), "AWS4%s", creds->secret_key);
    
    unsigned char k_date[32];
    hmac_sha256((unsigned char*)secret_key_with_prefix, strlen(secret_key_with_prefix),
                (unsigned char*)date, strlen(date), k_date);
    
    unsigned char k_region[32];
    hmac_sha256(k_date, 32, (unsigned char*)creds->region, strlen(creds->region), k_region);
    
    unsigned char k_service[32];
    hmac_sha256(k_region, 32, (unsigned char*)creds->service, strlen(creds->service), k_service);
    
    hmac_sha256(k_service, 32, (unsigned char*)"aws4_request", 12, signing_key);
}

// Generate authorization header
char* s3_generate_auth_header(const S3Credentials *creds, const S3Request *req) {
    // Step 1: Create canonical request
    char *canonical_request = create_canonical_request(req);
    
    // Step 2: Hash canonical request
    char *canonical_hash = sha256_hex((unsigned char*)canonical_request, strlen(canonical_request));
    
    // Step 3: Create string to sign
    char *string_to_sign = create_string_to_sign(creds, req, canonical_hash);
    
    // Step 4: Calculate signing key
    unsigned char signing_key[32];
    get_signing_key(creds, req->timestamp, signing_key);
    
    // Step 5: Calculate signature
    unsigned char signature_raw[32];
    hmac_sha256(signing_key, 32, (unsigned char*)string_to_sign, strlen(string_to_sign), signature_raw);
    
    // Convert signature to hex
    char signature[65];
    for (int i = 0; i < 32; i++) {
        sprintf(signature + (i * 2), "%02x", signature_raw[i]);
    }
    signature[64] = '\0';
    
    // Step 6: Build authorization header
    char date[16];
    s3_get_date(req->timestamp, date, sizeof(date));
    
    char *auth_header = malloc(1024);
    snprintf(auth_header, 1024,
             "AWS4-HMAC-SHA256 Credential=%s/%s/%s/%s/aws4_request, "
             "SignedHeaders=host;x-amz-content-sha256;x-amz-date, "
             "Signature=%s",
             creds->access_key,
             date,
             creds->region,
             creds->service,
             signature);
    
    // Cleanup
    free(canonical_request);
    free(canonical_hash);
    free(string_to_sign);
    
    return auth_header;
}
