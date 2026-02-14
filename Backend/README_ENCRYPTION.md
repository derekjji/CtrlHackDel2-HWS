# Encryption Utility - AES-256-CBC

This utility provides **AES-256-CBC encryption** and decryption using OpenSSL for securing patient healthcare records.

## Features

✅ **AES-256-CBC** - Military-grade 256-bit encryption  
✅ **Random IV Generation** - Unique initialization vector per file  
✅ **OpenSSL Integration** - Industry-standard cryptography  
✅ **File-based Encryption** - Handles files of any size  

## Prerequisites

- Install [OpenSSL](https://www.openssl.org/) development libraries

## Compilation

```bash
gcc encrypt_util.c -o encrypt_util -lcrypto
# Or: make encrypt_util
```

## Usage

### Standalone Demo
```bash
./encrypt_util
```

### Integrated Healthcare Backend (Recommended)

For production use, encryption is **automatically handled**:

```bash
# Automatically encrypts before upload
./healthcare_backend store patient_record.json patient_12345

# Automatically decrypts after download
./healthcare_backend retrieve patient_12345 output.json
```

## Key Management

### Generate Strong Keys
```bash
# Generate 256-bit key (64 hex characters)
openssl rand -hex 32
```

### ⚠️ Security Best Practices

✅ Never hardcode keys  
✅ Use environment variables or key management services  
✅ Generate random IV for each encryption  
✅ Rotate keys periodically  

## File Format

Encrypted files:
```
[16 bytes: IV][Encrypted Data with PKCS#7 padding]
```

## Integration with Solana

1. Encrypt patient record
2. Calculate SHA-256 hash
3. Store hash on Solana blockchain
4. Upload encrypted file to DigitalOcean
5. Verify hash before retrieval

See [README.md](README.md) for complete documentation.

---

**Status: Production Ready ✅**
