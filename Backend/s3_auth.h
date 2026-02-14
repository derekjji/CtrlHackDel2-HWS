// s3_auth.h
// AWS S3 Signature Version 4 authentication for DigitalOcean Spaces
// Provides functions to generate signed headers for S3-compatible APIs

#ifndef S3_AUTH_H
#define S3_AUTH_H

#include <time.h>

// Structure to hold S3 credentials
typedef struct {
    const char *access_key;
    const char *secret_key;
    const char *region;
    const char *service;
} S3Credentials;

// Structure to hold request information
typedef struct {
    const char *method;        // HTTP method (GET, PUT, POST, etc.)
    const char *host;          // Host header
    const char *uri;           // Request URI
    const char *query_string;  // Query string (can be NULL)
    const char *content_hash;  // SHA256 hash of payload (hex)
    time_t timestamp;          // Request timestamp
} S3Request;

// Generate authorization header for S3 request
char* s3_generate_auth_header(const S3Credentials *creds, const S3Request *req);

// Generate date string in ISO8601 format (YYYYMMDDTHHMMSSZ)
void s3_get_iso8601_timestamp(time_t t, char *buffer, size_t len);

// Generate date string in YYYYMMDD format
void s3_get_date(time_t t, char *buffer, size_t len);

// Calculate SHA256 hash of data and return as hex string
char* sha256_hex(const unsigned char *data, size_t len);

// Calculate HMAC-SHA256
void hmac_sha256(const unsigned char *key, size_t key_len,
                 const unsigned char *data, size_t data_len,
                 unsigned char *out);

#endif // S3_AUTH_H
