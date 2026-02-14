// encrypt_util.c
// Simple AES-256-CBC encryption/decryption using OpenSSL
// Compile with: gcc encrypt_util.c -o encrypt_util -lcrypto

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define AES_KEYLEN 32 // 256 bits
#define AES_IVLEN 16  // 128 bits

void handleErrors(void) {
    ERR_print_errors_fp(stderr);
    abort();
}

int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
            unsigned char *iv, unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;

    if (!(ctx = EVP_CIPHER_CTX_new())) handleErrors();
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) handleErrors();
    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) handleErrors();
    ciphertext_len = len;
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
    ciphertext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
            unsigned char *iv, unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;

    if (!(ctx = EVP_CIPHER_CTX_new())) handleErrors();
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) handleErrors();
    if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)) handleErrors();
    plaintext_len = len;
    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) handleErrors();
    plaintext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    return plaintext_len;
}

// Example usage
int main() {
    unsigned char key[AES_KEYLEN] = "01234567890123456789012345678901"; // 32 bytes
    unsigned char iv[AES_IVLEN] = "0123456789012345"; // 16 bytes
    unsigned char plaintext[] = "Sensitive medical data.";
    unsigned char ciphertext[128];
    unsigned char decryptedtext[128];

    int ciphertext_len = encrypt(plaintext, strlen((char *)plaintext), key, iv, ciphertext);
    int decryptedtext_len = decrypt(ciphertext, ciphertext_len, key, iv, decryptedtext);
    decryptedtext[decryptedtext_len] = '\0';

    printf("Original: %s\n", plaintext);
    printf("Encrypted (hex): ");
    for (int i = 0; i < ciphertext_len; i++) printf("%02x", ciphertext[i]);
    printf("\nDecrypted: %s\n", decryptedtext);
    return 0;
}
