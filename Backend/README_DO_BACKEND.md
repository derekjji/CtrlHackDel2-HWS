# DigitalOcean C Backend Example

This is a basic C backend for uploading and downloading encrypted files to DigitalOcean Spaces using libcurl.

## Prerequisites
- Install [libcurl](https://curl.se/libcurl/)
- Get your DigitalOcean Spaces access key, secret key, and endpoint URL

## Usage
1. Edit `digitalocean_backend.c` and set your credentials and endpoint:
   - `DO_SPACES_URL`
   - `DO_ACCESS_KEY`
   - `DO_SECRET_KEY`
2. Compile:
   ```sh
   gcc digitalocean_backend.c -o do_backend -lcurl
   ```
3. Run:
   ```sh
   ./do_backend upload <local_file> <remote_name>
   ./do_backend download <local_file> <remote_name>
   ```

## Note
- This example does not implement AWS S3 signature v4 authentication (required for private buckets). For production, use a library or implement the signing process.
- Data should be encrypted before upload for security.

---

For more advanced features, consider using a higher-level language or a C library that supports S3-compatible authentication.
