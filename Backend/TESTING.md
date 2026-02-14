# Testing Guide

## Test the Healthcare Backend

### Quick Test Run

**Windows PowerShell:**
```powershell
.\run_tests.ps1
```

**Linux/Mac:**
```bash
chmod +x run_tests.sh
./run_tests.sh
```

### 1. Setup Test Environment

```bash
# Windows PowerShell
. .\setup_env.ps1

# Linux/Mac
source setup_env.sh
```

### 2. Compile All Components

```bash
make clean
make
```

### 3. Test Encryption/Decryption

```bash
# Test the encryption utility
./encrypt_util
```

Expected output:
```
Original: Sensitive medical data.
Encrypted (hex): [hex characters]
Decrypted: Sensitive medical data.
```

### 4. Test Healthcare Backend (Local)

#### Test Storage
```bash
# Store the sample patient record
./healthcare_backend store sample_patient_record.json TEST_PATIENT_001
```

Expected output:
```
=== Storing Patient Record ===
Patient ID: sample_patient_record.json
File encrypted successfully: sample_patient_record.json -> sample_patient_record.json.encrypted
Encrypted file hash (for Solana): [64-char hash]
Uploaded to DigitalOcean Spaces: patients/TEST_PATIENT_001.encrypted

✓ Patient record stored successfully!
  Remote path: patients/TEST_PATIENT_001.encrypted
  Hash for blockchain: [hash]
```

#### Test Retrieval
```bash
# Retrieve and decrypt the record
./healthcare_backend retrieve TEST_PATIENT_001 retrieved_patient.json
```

Expected output:
```
=== Retrieving Patient Record ===
Patient ID: TEST_PATIENT_001
Downloaded from DigitalOcean Spaces: patients/TEST_PATIENT_001.encrypted
Downloaded file hash: [hash]
File decrypted successfully

✓ Patient record retrieved successfully!
  Decrypted to: retrieved_patient.json
```

#### Verify Data Integrity
```bash
# Compare original and retrieved files
# Windows PowerShell:
Compare-Object (Get-Content sample_patient_record.json) (Get-Content retrieved_patient.json)

# Linux/Mac:
diff sample_patient_record.json retrieved_patient.json
```

If no output, files are identical! ✓

### 5. Test Basic Backend (Optional)

```bash
# Upload a file
./do_backend upload sample_patient_record.json test_upload.json

# Download the file
./do_backend download downloaded.json test_upload.json
```

## Test Cases Included

The backend includes **6 comprehensive test cases** covering different healthcare scenarios:

### 1. **Simple Patient Record** ([test_simple_record.json](test_simple_record.json))
   - Basic patient information
   - Simple diagnosis
   - Tests basic encryption/decryption workflow

### 2. **Emergency Record** ([test_emergency_record.json](test_emergency_record.json))
   - Critical care scenario
   - Vital signs and emergency treatment
   - Complex nested data structures
   - Tests high-priority data handling

### 3. **Laboratory Results** ([test_lab_results.json](test_lab_results.json))
   - Complete blood count (CBC)
   - Metabolic panel
   - Lipid panel
   - Thyroid function tests
   - Tests structured numeric data with reference ranges

### 4. **E-Prescription** ([test_prescription.json](test_prescription.json))
   - Multiple medications
   - Prescriber information
   - Pharmacy details
   - DEA and NPI numbers
   - Tests sensitive prescription data handling

### 5. **Radiology Report** ([test_imaging_report.json](test_imaging_report.json))
   - CT scan findings
   - DICOM metadata
   - Detailed radiologist interpretation
   - Tests large text reports with medical terminology

### 6. **Full Patient Record** ([sample_patient_record.json](sample_patient_record.json))
   - Comprehensive health record
   - Medical history
   - Current visit details
   - Insurance information
   - Tests complete EHR workflow

## End-to-End Workflow Test

### Simulating Real Healthcare Scenario

```bash
# 1. Create patient record (JSON/XML/Text)
echo '{"patient": "Jane Smith", "diagnosis": "Flu"}' > patient_jane.json

# 2. Store encrypted to DigitalOcean
./healthcare_backend store patient_jane.json PATIENT_JANE_002

# 3. Note the hash returned (for Solana)
# Example: "abc123def456..."

# 4. [In Solana smart contract]
# Store hash on blockchain with access permissions

# 5. Later: Retrieve the record
./healthcare_backend retrieve PATIENT_JANE_002 jane_decrypted.json

# 6. Verify content
cat jane_decrypted.json
```

## Security Tests

### Test 1: Verify Encryption Works
```bash
# Encrypted file should not contain plaintext
grep "John Doe" sample_patient_record.json.encrypted
# Should return nothing (binary data)

# Original should contain plaintext
grep "John Doe" sample_patient_record.json
# Should return matches
```

### Test 2: Verify Different IVs Each Time
```bash
# Encrypt same file twice
./healthcare_backend store sample_patient_record.json TEST_A
./healthcare_backend store sample_patient_record.json TEST_B

# Files should be different (different IVs)
# Windows:
fc /b sample_patient_record.json.encrypted sample_patient_record.json.encrypted
# Linux/Mac:
cmp sample_patient_record.json.encrypted sample_patient_record.json.encrypted
```

### Test 3: Verify Wrong Key Fails
```bash
# Store with one key
export ORIGINAL_KEY=$ENCRYPTION_KEY
./healthcare_backend store sample_patient_record.json TEST_KEY

# Try to decrypt with wrong key
export ENCRYPTION_KEY=$(openssl rand -hex 32)
./healthcare_backend retrieve TEST_KEY wrong_key.json
# Should fail with decryption error

# Restore correct key
export ENCRYPTION_KEY=$ORIGINAL_KEY
./healthcare_backend retrieve TEST_KEY correct_key.json
# Should succeed
```

## Performance Test

```bash
# Create larger test file (10MB)
dd if=/dev/urandom of=large_test.bin bs=1M count=10

# Time the encryption and upload
time ./healthcare_backend store large_test.bin LARGE_TEST

# Time the download and decryption
time ./healthcare_backend retrieve LARGE_TEST large_decrypted.bin

# Verify integrity
# Linux/Mac:
sha256sum large_test.bin large_decrypted.bin
# Windows:
Get-FileHash large_test.bin, large_decrypted.bin -Algorithm SHA256
```

## Integration Test with Mock Solana

```bash
# 1. Store patient record and capture hash
HASH=$(./healthcare_backend store sample_patient_record.json MOCK_TEST | grep "Hash for blockchain" | awk '{print $4}')

echo "Captured hash: $HASH"

# 2. Simulate storing on Solana (mock)
echo "$HASH" > solana_mock_hash.txt

# 3. Before retrieval, verify hash exists (mock)
if [ -f solana_mock_hash.txt ]; then
    echo "✓ Hash found on blockchain (mock)"
    ./healthcare_backend retrieve MOCK_TEST verified_record.json
else
    echo "✗ Hash not found - access denied"
fi
```

## Cleanup Test Data

```bash
# Remove all test files
rm -f *.encrypted *.encrypted.tmp
rm -f patient_*.json jane_*.json retrieved_*.json
rm -f large_test.bin large_decrypted.bin
rm -f solana_mock_hash.txt

# Keep the sample file
# sample_patient_record.json is preserved
```

## Troubleshooting Tests

### Issue: Connection timeout
```bash
# Check internet connection
curl https://www.google.com

# Check DigitalOcean Spaces endpoint
curl https://$DO_SPACE_NAME.$DO_REGION.digitaloceanspaces.com
```

### Issue: Authentication error (403)
```bash
# Verify credentials are set
echo "Space: $DO_SPACE_NAME"
echo "Region: $DO_REGION"
echo "Access Key: $DO_ACCESS_KEY"
# Secret key should NOT be echoed for security

# Re-run setup
. .\setup_env.ps1  # Windows
source setup_env.sh  # Linux/Mac
```

### Issue: Decryption fails
```bash
# Check encryption key is consistent
echo $ENCRYPTION_KEY

# If lost, you cannot decrypt old files!
# Check if saved in .encryption_key file
cat .encryption_key
```

## Automated Test Script

Save as `run_tests.sh`:

```bash
#!/bin/bash
set -e

echo "Running Healthcare Backend Tests..."

# Compile
echo "1. Compiling..."
make clean && make

# Test encryption
echo "2. Testing encryption utility..."
./encrypt_util

# Test healthcare backend
echo "3. Testing healthcare backend..."
./healthcare_backend store sample_patient_record.json AUTO_TEST_001
./healthcare_backend retrieve AUTO_TEST_001 test_output.json

# Verify
echo "4. Verifying data integrity..."
diff sample_patient_record.json test_output.json

echo "✓ All tests passed!"

# Cleanup
rm -f test_output.json *.encrypted
```

Run with:
```bash
# Make executable
chmod +x run_tests.sh

# Run
./run_tests.sh
```

---

**Remember**: Never use real patient data in testing! Always use synthetic/mock data.
