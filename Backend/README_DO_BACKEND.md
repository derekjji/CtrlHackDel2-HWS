# DigitalOcean Backend - Production Ready

This is a **production-ready** C backend for uploading and downloading encrypted files to DigitalOcean Spaces using libcurl with **full AWS S3 Signature Version 4 authentication**.

## Features

✅ **AWS S3 Signature V4 Authentication** - Fully implemented  
✅ **Secure HTTPS Upload/Download** - TLS encryption in transit  
✅ **Environment-based Configuration** - No hardcoded credentials  
✅ **Error Handling** - Proper error messages and return codes  
✅ **Content Integrity** - SHA-256 hashing for verification  

## Prerequisites

- Install [libcurl](https://curl.se/libcurl/)
- Install [OpenSSL](https://www.openssl.org/)
- Get your DigitalOcean Spaces credentials

## Quick Start

### 1. Setup Environment

**Windows PowerShell:**
```powershell
. .\setup_env.ps1
```

**Linux/Mac:**
```bash
source setup_env.sh
```

### 2. Compile

```bash
make do_backend
```

### 3. Upload/Download

```bash
./do_backend upload local_file.txt remote_name.txt
./do_backend download local_output.txt remote_name.txt
```

## What's New?

✅ Fixed `#include <curl/curl.h>` typo  
✅ Implemented full S3 Signature V4 authentication  
✅ Added proper error handling  
✅ Environment-based configuration  

For healthcare data, use **healthcare_backend** instead (includes automatic encryption).

See [README.md](README.md) for full documentation.

---

**Status: Production Ready ✅**
