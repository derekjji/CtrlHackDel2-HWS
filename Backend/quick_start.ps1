# Quick Backend Startup Script
# Run this from the root directory: .\Backend\quick_start.ps1

Write-Host "=== Healthcare Backend Quick Start ===" -ForegroundColor Cyan
Write-Host ""

# Check if already in web_api directory
$webApiDir = "Backend\web_api"
if (-not (Test-Path $webApiDir)) {
    Write-Host "Error: Backend\web_api not found" -ForegroundColor Red
    exit 1
}

Write-Host "Step 1: Checking dependencies..." -ForegroundColor Yellow
if (-not (Get-Command node -ErrorAction SilentlyContinue)) {
    Write-Host "✗ Node.js not found. Please install Node.js first." -ForegroundColor Red
    exit 1
}
Write-Host "✓ Node.js $(node --version)" -ForegroundColor Green

# Step 2: Check node_modules
Write-Host ""
Write-Host "Step 2: Checking npm dependencies..." -ForegroundColor Yellow
if (-not (Test-Path "$webApiDir\node_modules")) {
    Write-Host "Installing dependencies..." -ForegroundColor Cyan
    cd $webApiDir
    npm install
    cd ..\..
    Write-Host "✓ Dependencies installed" -ForegroundColor Green
} else {
    Write-Host "✓ Dependencies already installed" -ForegroundColor Green
}

# Step 3: Check .env file
Write-Host ""
Write-Host "Step 3: Checking .env configuration..." -ForegroundColor Yellow
if (-not (Test-Path "$webApiDir\.env")) {
    Write-Host "✗ .env file not found at $webApiDir\.env" -ForegroundColor Red
    Write-Host ""
    Write-Host "Creating .env file..." -ForegroundColor Cyan
    
    # Generate encryption key
    $encKey = & openssl rand -hex 32
    
    $envContent = @"
# Server Configuration
PORT=4000
CORS_ORIGIN=http://localhost:5173,http://127.0.0.1:5173

# Encryption Key (256-bit, 64 hex chars)
ENCRYPTION_KEY=$encKey

# DigitalOcean Spaces Configuration
DO_SPACE_NAME=your-space-name
DO_REGION=nyc3
DO_ACCESS_KEY=your-access-key
DO_SECRET_KEY=your-secret-key

# Backend Binary Path
BACKEND_BINARY=..\healthcare_backend.exe
"@
    
    $envContent | Out-File "$webApiDir\.env" -Encoding UTF8
    Write-Host "✓ .env file created with generated ENCRYPTION_KEY" -ForegroundColor Green
    Write-Host "  Generated key: $encKey" -ForegroundColor Cyan
} else {
    Write-Host "✓ .env file exists" -ForegroundColor Green
}

# Step 4: Check healthcare_backend.exe
Write-Host ""
Write-Host "Step 4: Checking healthcare backend binary..." -ForegroundColor Yellow
if (-not (Test-Path "Backend\healthcare_backend.exe")) {
    Write-Host "⚠ healthcare_backend.exe not found" -ForegroundColor Yellow
    Write-Host "  Note: You may need this for storing/retrieving records" -ForegroundColor Yellow
} else {
    Write-Host "✓ healthcare_backend.exe found" -ForegroundColor Green
}

# Step 5: Start server
Write-Host ""
Write-Host "Step 5: Starting server..." -ForegroundColor Yellow
Write-Host ""
cd $webApiDir
npm run dev
