# Encryption Utility for C Backend

This utility provides AES-256-CBC encryption and decryption using OpenSSL. Use it to encrypt files before uploading to DigitalOcean Spaces and decrypt after downloading.

## Prerequisites
- Install [OpenSSL](https://www.openssl.org/) development libraries

## Usage
1. Compile:
   ```sh
   gcc encrypt_util.c -o encrypt_util -lcrypto
   ```
2. Edit the key and IV in the code or load them securely from a file or environment variable.
3. Use the `encrypt()` and `decrypt()` functions in your backend to protect sensitive data.

## Example
See the `main()` function in `encrypt_util.c` for a demonstration of encrypting and decrypting a string.

---

**Note:**
- Never hardcode keys in production. Use a secure key management system.
- Always use a random IV for each encryption and store it with the ciphertext.
- For file encryption, read the file into a buffer and use the provided functions.
