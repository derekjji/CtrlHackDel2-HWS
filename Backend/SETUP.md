# Quick Setup Guide

## For Contributors/Users (Setting Up Your Local Environment)

### Step 1: Clone the Repository
```bash
git clone <your-repo-url>
cd CtrlHackDel2/Backend
```

### Step 2: Install Dependencies

**Windows (MSYS2 UCRT64):**
```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-curl mingw-w64-ucrt-x86_64-openssl
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt-get install build-essential libcurl4-openssl-dev libssl-dev
```

**macOS:**
```bash
brew install gcc curl openssl
```

### Step 3: Configure Your Environment

#### Option A: Using Setup Script (Recommended)
```bash
# Linux/Mac
source setup_env.sh

# Windows PowerShell
. .\setup_env.ps1
```

#### Option B: Manual Configuration
```bash
# Copy the example config
cp .env.example .env

# Edit .env with your actual credentials
nano .env  # or use any text editor

# Load environment variables
source .env  # Linux/Mac
# or
. .\.env  # PowerShell (you may need to convert format)
```

### Step 4: Get DigitalOcean Credentials

1. Go to https://cloud.digitalocean.com/
2. Create a Space (if you don't have one)
3. Navigate to **API** → **Spaces Keys**
4. Click **Generate New Key**
5. Copy the Access Key and Secret Key
6. Note your Space name and region (e.g., nyc3, sfo3)

### Step 5: Compile the Backend
```bash
make
```

### Step 6: Test It
```bash
# Quick test
./encrypt_util

# Full test (requires DigitalOcean configured)
./run_tests.sh  # Linux/Mac
.\run_tests.ps1  # Windows
```

## Security Notes

### ✅ Safe to Share (Already in Repo):
- `.env.example` - Template with placeholder values
- `setup_env.sh` / `setup_env.ps1` - Setup scripts
- All source code files
- Test data (synthetic patient records)
- Documentation

### ❌ NEVER Share (Gitignored):
- `.env` - Your actual credentials
- `.encryption_key` - Your encryption key
- `*.key`, `*.pem` - Any key files
- `config.ini` - Config with secrets

## Troubleshooting

### Environment Variables Not Set
```bash
# Check if variables are loaded
echo $DO_SPACE_NAME
echo $DO_REGION

# If empty, reload:
source setup_env.sh  # Linux/Mac
. .\setup_env.ps1     # Windows
```

### Compilation Errors
- Make sure all dependencies are installed
- Use the correct terminal (MSYS2 UCRT64 on Windows)
- Check that paths are correct

### Upload/Download Fails (HTTP 404)
- Verify your Space exists in DigitalOcean
- Check Space name and region are correct
- Ensure access key and secret key are valid

## For Project Maintainers

When sharing this project:

1. ✅ **Commit:**
   - `.env.example`
   - `.gitignore`
   - All source code
   - Documentation
   - Test data

2. ❌ **Never Commit:**
   - `.env`
   - `.encryption_key`
   - Any files with real credentials

3. 📝 **Document:**
   - How to get credentials
   - Setup process
   - Environment variables needed

4. 🔒 **Use GitHub Secrets** for CI/CD:
   - Add secrets in repo settings
   - Reference them in workflows
   - Never print them in logs

---

**Ready to go!** 🚀

For detailed information, see [README.md](README.md)
