# PowerShell test suite for healthcare backend
# Run this script to test all functionality

Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "Healthcare Backend Test Suite" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host ""

# Check if executable exists
if (-not (Test-Path "healthcare_backend.exe")) {
    Write-Host "Error: healthcare_backend.exe not found" -ForegroundColor Red
    Write-Host "Please compile first" -ForegroundColor Yellow
    exit 1
}

# Check environment variables
$envConfigured = $env:DO_SPACE_NAME -and $env:DO_REGION -and $env:DO_ACCESS_KEY -and $env:DO_SECRET_KEY -and $env:ENCRYPTION_KEY

if (-not $envConfigured) {
    Write-Host "Warning: Environment variables not set" -ForegroundColor Yellow
    Write-Host "Tests will be skipped" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "To run full tests, run: . .\setup_env.ps1" -ForegroundColor Yellow
    $fullTest = $false
} else {
    Write-Host "✓ Environment configured" -ForegroundColor Green
    $fullTest = $true
}

Write-Host ""
Write-Host "Running Tests..." -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan

# Test function
function Test-Record {
    param(
        [string]$TestName,
        [string]$InputFile,
        [string]$PatientId,
        [string]$OutputFile
    )
    
    Write-Host ""
    Write-Host $TestName -ForegroundColor Yellow
    Write-Host "----------------------------------------" -ForegroundColor Gray
    
    if (-not $fullTest) {
        Write-Host "Skipping (no env config)" -ForegroundColor Gray
        return
    }
    
    # Store
    & .\healthcare_backend.exe store $InputFile $PatientId
    if ($LASTEXITCODE -ne 0) {
        Write-Host "✗ $TestName FAILED (upload)" -ForegroundColor Red
        return
    }
    
    # Retrieve
    & .\healthcare_backend.exe retrieve $PatientId $OutputFile
    if ($LASTEXITCODE -ne 0) {
        Write-Host "✗ $TestName FAILED (download)" -ForegroundColor Red
        return
    }
    
    # Compare
    $original = Get-Content $InputFile -Raw
    $retrieved = Get-Content $OutputFile -Raw
    
    if ($original -eq $retrieved) {
        Write-Host "✓ $TestName PASSED" -ForegroundColor Green
    } else {
        Write-Host "✗ $TestName FAILED (content mismatch)" -ForegroundColor Red
    }
    
    # Cleanup
    Remove-Item $OutputFile -ErrorAction SilentlyContinue
    Remove-Item "$InputFile.encrypted" -ErrorAction SilentlyContinue
}

# Run all tests
Test-Record "Test 1: Simple Patient Record" "test_simple_record.json" "TEST_SIMPLE_001" "output_simple.json"
Test-Record "Test 2: Emergency Room Record" "test_emergency_record.json" "EMERGENCY_002" "output_emergency.json"
Test-Record "Test 3: Laboratory Results" "test_lab_results.json" "LAB_TEST_003" "output_lab.json"
Test-Record "Test 4: E-Prescription" "test_prescription.json" "RX_TEST_004" "output_rx.json"
Test-Record "Test 5: Radiology Report" "test_imaging_report.json" "IMAGING_005" "output_imaging.json"
Test-Record "Test 6: Full Patient Record" "sample_patient_record.json" "SAMPLE_PATIENT" "output_sample.json"

Write-Host ""
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "Test Suite Complete" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan

if ($fullTest) {
    Write-Host ""
    Write-Host "All tests executed with full upload/download cycle" -ForegroundColor Green
    Write-Host "Check for any FAILED tests above" -ForegroundColor Yellow
} else {
    Write-Host ""
    Write-Host "Tests skipped - environment not configured" -ForegroundColor Yellow
    Write-Host "Run: . .\setup_env.ps1 to enable full testing" -ForegroundColor Yellow
}

Write-Host ""
