# Load environment variables from .env file
# Usage: . .\load_env.ps1

if (Test-Path .env) {
    Write-Host "Loading environment variables from .env..." -ForegroundColor Cyan
    
    Get-Content .env | ForEach-Object {
        if ($_ -match '^([^#].+?)=(.+)$') {
            $name = $matches[1].Trim()
            $value = $matches[2].Trim()
            Set-Item -Path "env:$name" -Value $value
        }
    }
    
    Write-Host "✓ Environment variables loaded" -ForegroundColor Green
    Write-Host ""
    Write-Host "Loaded variables:"
    Write-Host "  DO_SPACE_NAME=$env:DO_SPACE_NAME"
    Write-Host "  DO_REGION=$env:DO_REGION"
    Write-Host "  DO_ACCESS_KEY=[set]"
    Write-Host "  DO_SECRET_KEY=[hidden]"
    Write-Host "  ENCRYPTION_KEY=[hidden]"
} else {
    Write-Host "Error: .env file not found!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please create .env from the template:" -ForegroundColor Yellow
    Write-Host "  Copy-Item .env.example .env"
    Write-Host "  notepad .env  # Edit with your credentials"
}
