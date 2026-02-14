# setup_env.ps1 - Setup script for configuring the healthcare backend (Windows PowerShell)
# Run this: . .\setup_env.ps1

Write-Host "Healthcare Data Backend - Environment Setup" -ForegroundColor Cyan
Write-Host "============================================" -ForegroundColor Cyan
Write-Host ""

# DigitalOcean Spaces Configuration
Write-Host "Enter your DigitalOcean Spaces configuration:" -ForegroundColor Yellow
$DO_SPACE_NAME = Read-Host "Space Name"
$DO_REGION = Read-Host "Region (e.g., nyc3, sfo3)"
$DO_ACCESS_KEY = Read-Host "Access Key"
$DO_SECRET_KEY = Read-Host "Secret Key" -AsSecureString
$DO_SECRET_KEY_PLAIN = [Runtime.InteropServices.Marshal]::PtrToStringAuto(
    [Runtime.InteropServices.Marshal]::SecureStringToBSTR($DO_SECRET_KEY))

Write-Host ""

# Export DigitalOcean credentials
$env:DO_SPACE_NAME = $DO_SPACE_NAME
$env:DO_REGION = $DO_REGION
$env:DO_ACCESS_KEY = $DO_ACCESS_KEY
$env:DO_SECRET_KEY = $DO_SECRET_KEY_PLAIN

# Generate encryption key if not set
if (-not $env:ENCRYPTION_KEY) {
    Write-Host "Generating new 256-bit encryption key..." -ForegroundColor Green
    
    # Generate random 32 bytes and convert to hex
    $bytes = New-Object byte[] 32
    $rng = [System.Security.Cryptography.RandomNumberGenerator]::Create()
    $rng.GetBytes($bytes)
    $ENCRYPTION_KEY = ($bytes | ForEach-Object { $_.ToString("x2") }) -join ''
    
    $env:ENCRYPTION_KEY = $ENCRYPTION_KEY
    Write-Host "Generated encryption key: $ENCRYPTION_KEY" -ForegroundColor Green
    Write-Host "⚠️  IMPORTANT: Save this key securely! You'll need it to decrypt data." -ForegroundColor Red
    Write-Host ""
    
    # Save to file
    $ENCRYPTION_KEY | Out-File -FilePath ".encryption_key" -NoNewline -Encoding ASCII
    Write-Host "Encryption key saved to .encryption_key" -ForegroundColor Green
} else {
    Write-Host "Using existing ENCRYPTION_KEY from environment" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "✓ Environment configured successfully!" -ForegroundColor Green
Write-Host ""
Write-Host "Environment variables set:"
Write-Host "  DO_SPACE_NAME=$DO_SPACE_NAME"
Write-Host "  DO_REGION=$DO_REGION"
Write-Host "  DO_ACCESS_KEY=$DO_ACCESS_KEY"
Write-Host "  DO_SECRET_KEY=***hidden***"
Write-Host "  ENCRYPTION_KEY=***hidden***"
Write-Host ""
Write-Host "Note: These variables are set for this session only." -ForegroundColor Yellow
Write-Host "To persist, add them to your PowerShell profile or use setx command." -ForegroundColor Yellow
