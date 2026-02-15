# Backend Setup & Startup Guide

## Prerequisites

1. **Node.js** installed (v16+)
2. **OpenSSL** installed (for encryption key generation)
3. **DigitalOcean Spaces** credentials (optional for development)
4. **healthcare_backend.exe** already compiled in Backend/ folder

## Step 1: Install Dependencies

```powershell
cd Backend\web_api
npm install
```

Expected output:
```
added 50 packages in 2s
```

## Step 2: Create .env File

Create `Backend\web_api\.env`:

```env
# Server Configuration
PORT=4000
CORS_ORIGIN=http://localhost:5173,http://127.0.0.1:5173

# CRITICAL: Generate this with: openssl rand -hex 32
ENCRYPTION_KEY=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa

# DigitalOcean Spaces (for production storage)
DO_SPACE_NAME=your-space-name
DO_REGION=nyc3
DO_ACCESS_KEY=your-access-key
DO_SECRET_KEY=your-secret-key

# Path to compiled backend binary
BACKEND_BINARY=..\healthcare_backend.exe
```

## Step 3: Generate Encryption Key

```powershell
# In PowerShell
$ENCRYPTION_KEY = & openssl rand -hex 32
Write-Host "Generated key: $ENCRYPTION_KEY"
# Copy this to your .env file
```

## Step 4: Start Backend Server

```powershell
cd Backend\web_api
npm run dev
# or
node src/server.js
```

Expected output:
```
Server running on http://localhost:4000
Health check available at GET /health
```

## Common Issues & Fixes

### Issue 1: "Cannot find module 'express'"
**Cause**: Dependencies not installed
**Fix**:
```powershell
cd Backend\web_api
rm -r node_modules package-lock.json
npm install
npm run dev
```

### Issue 2: "ENCRYPTION_KEY must be 64 hex chars"
**Cause**: Missing or invalid ENCRYPTION_KEY in .env
**Fix**:
```powershell
# Generate fresh key
$ENCRYPTION_KEY = & openssl rand -hex 32
echo $ENCRYPTION_KEY  # Copy and paste into .env
```

### Issue 3: "Cannot find healthcare_backend.exe"
**Cause**: Either binary not compiled or wrong path
**Fix**:
```powershell
# Check file exists
ls Backend\healthcare_backend.exe

# If missing, compile it
cd Backend
gcc healthcare_backend.c s3_auth.c -o healthcare_backend.exe -lcurl -lcrypto

# Update .env if needed
BACKEND_BINARY=..\healthcare_backend.exe
```

### Issue 4: "Port 4000 already in use"
**Cause**: Another process using port 4000
**Fix**:
```powershell
# Find process using port 4000
netstat -ano | findstr :4000

# Kill it (replace PID with actual process ID)
taskkill /PID <PID> /F

# Or use different port in .env
PORT=4001
```

### Issue 5: "Cannot download from DigitalOcean Spaces"
**Cause**: Missing or invalid credentials
**Fix**:
```env
# Verify these are correct and paste into .env
DO_SPACE_NAME=your-actual-space
DO_REGION=nyc3
DO_ACCESS_KEY=your-actual-key
DO_SECRET_KEY=your-actual-secret
```

## Development Workflow

**Terminal 1 - Backend:**
```powershell
cd Backend\web_api
npm run dev
# Watch for: "Server running on http://localhost:4000"
```

**Terminal 2 - Frontend:**
```powershell
cd frontend
npm run dev
# Visit: http://localhost:5173
```

**Test Connection:**
```powershell
# In third terminal, test backend is running
curl http://localhost:4000/health
# Should return: {"status":"ok"}
```

## Verify Everything Works

1. **Backend server starts:**
   ```powershell
   npm run dev
   # Should print: "Server running on http://localhost:4000"
   ```

2. **Health check passes:**
   ```powershell
   curl http://localhost:4000/health
   # Returns: {"status":"ok"}
   ```

3. **Frontend connects:**
   - Open http://localhost:5173
   - Connect wallet (Solflare)
   - Try to store a record
   - Check backend logs for encryption messages

4. **Check backend logs:**
   ```
   File encrypted successfully
   Patient record stored successfully
   ```

## Troubleshooting Script

```powershell
# Run this to diagnose issues
Write-Host "=== Backend Startup Checklist ===" -ForegroundColor Cyan
Write-Host "✓ Node.js: $(node --version)"
Write-Host "✓ npm: $(npm --version)"
Write-Host "✓ OpenSSL: $(openssl version)"

$envFile = "Backend\web_api\.env"
if (Test-Path $envFile) {
    Write-Host "✓ .env file exists" -ForegroundColor Green
    $key = Select-String "ENCRYPTION_KEY=" $envFile | ForEach-Object { $_.Line.Split('=')[1] }
    $keyLen = $key.Length
    Write-Host "  ENCRYPTION_KEY length: $keyLen (should be 64)" -ForegroundColor $(if ($keyLen -eq 64) { "Green" } else { "Red" })
} else {
    Write-Host "✗ .env file NOT found" -ForegroundColor Red
}

if (Test-Path "Backend\healthcare_backend.exe") {
    Write-Host "✓ healthcare_backend.exe found" -ForegroundColor Green
} else {
    Write-Host "✗ healthcare_backend.exe NOT found" -ForegroundColor Red
}

Write-Host ""
Write-Host "Ready to start? Run:" -ForegroundColor Cyan
Write-Host "cd Backend\web_api && npm run dev"
```

## Next Steps

1. Create Backend/web_api/.env file with your credentials
2. Run `npm install` to install dependencies
3. Run `npm run dev` to start the server
4. Verify it starts successfully by checking logs
