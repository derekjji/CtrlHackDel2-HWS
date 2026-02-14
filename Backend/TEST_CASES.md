# Test Cases Overview

## Healthcare Backend Test Suite

This document describes all test cases included in the healthcare backend test suite.

## Test Case Summary

| Test # | Type | File | Patient ID | Purpose |
|--------|------|------|------------|---------|
| 1 | Simple Record | `test_simple_record.json` | TEST_SIMPLE_001 | Basic encryption/workflow |
| 2 | Emergency | `test_emergency_record.json` | EMERGENCY_002 | Critical care data |
| 3 | Lab Results | `test_lab_results.json` | LAB_TEST_003 | Structured numeric data |
| 4 | Prescription | `test_prescription.json` | RX_TEST_004 | Controlled substance handling |
| 5 | Imaging | `test_imaging_report.json` | IMAGING_005 | Large text reports |
| 6 | Full EHR | `sample_patient_record.json` | SAMPLE_PATIENT | Complete health record |

---

## Test Case 1: Simple Patient Record

**File:** `test_simple_record.json`  
**Patient ID:** `TEST_SIMPLE_001`  
**Size:** ~200 bytes  

### Coverage
- ✅ Basic patient demographics
- ✅ Simple diagnosis
- ✅ Minimal data structure
- ✅ Quick encryption test

### Use Case
Perfect for:
- Initial setup verification
- Quick smoke tests
- Development debugging
- CI/CD pipeline tests

---

## Test Case 2: Emergency Room Record

**File:** `test_emergency_record.json`  
**Patient ID:** `EMERGENCY_002`  
**Size:** ~2.5 KB  

### Coverage
- ✅ Critical care scenario
- ✅ Time-sensitive data
- ✅ Complex nested structures
- ✅ Vital signs monitoring
- ✅ Emergency interventions
- ✅ Drug allergy overrides

### Use Case
Tests:
- High-priority data encryption
- Complex JSON structures
- Emergency workflow
- Allergy documentation
- Critical value handling

### Medical Scenario
Patient presenting with chest pain and difficulty breathing. Suspected acute coronary syndrome. Demonstrates emergency department workflow with STAT interventions.

---

## Test Case 3: Laboratory Results

**File:** `test_lab_results.json`  
**Patient ID:** `LAB_TEST_003`  
**Size:** ~3 KB  

### Coverage
- ✅ Complete Blood Count (CBC)
- ✅ Metabolic Panel (CMP)
- ✅ Lipid Panel
- ✅ Thyroid Function Tests
- ✅ Reference ranges
- ✅ Status indicators (Normal/Abnormal)
- ✅ Clinical interpretation

### Use Case
Tests:
- Numeric data with precision
- Reference range validation
- Multi-test panel structures
- Lab certification data (CLIA)
- Pathologist signatures

### Medical Scenario
Routine annual physical exam lab work. All values normal except borderline high LDL cholesterol. Demonstrates typical preventive care lab results.

---

## Test Case 4: E-Prescription

**File:** `test_prescription.json`  
**Patient ID:** `RX_TEST_004`  
**Size:** ~2 KB  

### Coverage
- ✅ Multiple medications
- ✅ DEA numbers (controlled substances)
- ✅ NPI (National Provider Identifier)
- ✅ Pharmacy information (NCPDP)
- ✅ NDC codes (drug identification)
- ✅ Dosage instructions
- ✅ Refill information
- ✅ Electronic signatures

### Use Case
Tests:
- Sensitive prescription data encryption
- Drug identifiers (NDC codes)
- Provider credentials (DEA, NPI)
- Pharmacy routing
- Digital signature verification
- Substitution permissions

### Medical Scenario
Patient with hypertension, hyperlipidemia, and type 2 diabetes receiving maintenance medications. Demonstrates chronic disease management with multiple prescription drugs.

### Security Note
This test case is critical for HIPAA compliance as it contains:
- Controlled substance information
- Provider DEA numbers
- Patient medication history

---

## Test Case 5: Radiology Report

**File:** `test_imaging_report.json`  
**Patient ID:** `IMAGING_005`  
**Size:** ~4 KB  

### Coverage
- ✅ CT scan findings
- ✅ Detailed anatomical descriptions
- ✅ DICOM metadata
- ✅ Radiation dose information
- ✅ Clinical impressions
- ✅ Recommendations
- ✅ Radiologist credentials
- ✅ Study identifiers (Accession, Study UID)

### Use Case
Tests:
- Large text content encryption
- Medical terminology handling
- DICOM integration references
- Multi-section report structure
- Image study metadata
- Radiation tracking

### Medical Scenario
Patient with persistent cough undergoing CT chest scan to rule out pulmonary mass. Negative findings with mild inflammatory changes. Demonstrates typical radiology workflow and reporting.

### Integration Note
This test references DICOM images stored separately. In production, DICOM images would be:
1. Stored in PACS (Picture Archiving System)
2. Referenced in this JSON report
3. Hash of DICOM study stored on Solana blockchain

---

## Test Case 6: Full Patient Record

**File:** `sample_patient_record.json`  
**Patient ID:** `SAMPLE_PATIENT`  
**Size:** ~3 KB  

### Coverage
- ✅ Complete demographics
- ✅ Medical history
- ✅ Chronic conditions
- ✅ Medication list
- ✅ Surgical history
- ✅ Allergies
- ✅ Current visit details
- ✅ Vital signs
- ✅ Lab results (A1C, glucose, lipids)
- ✅ Insurance information
- ✅ Metadata tracking

### Use Case
Tests:
- Comprehensive EHR encryption
- Multi-section document structure
- Protected Health Information (PHI)
- Financial information (insurance)
- Complete patient journey
- Longitudinal health records

### Medical Scenario
Annual physical examination for patient with controlled type 2 diabetes and hypertension. Demonstrates complete primary care visit with all standard documentation components.

### HIPAA Identifiers Present
This record includes 18 HIPAA identifiers that must be protected:
1. Name
2. Address
3. Dates (DOB, visit dates)
4. Phone number
5. Email
6. Social Security Number (redacted in sample)
7. Medical Record Number
8. Health plan number
9. Provider identifiers

---

## Running Test Cases

### Automated Test Suite

**Windows PowerShell:**
```powershell
.\run_tests.ps1
```

**Linux/Mac:**
```bash
chmod +x run_tests.sh
./run_tests.sh
```

### Individual Test Execution

```bash
# Test 1: Simple Record
./healthcare_backend store test_simple_record.json TEST_SIMPLE_001
./healthcare_backend retrieve TEST_SIMPLE_001 output.json
diff test_simple_record.json output.json

# Test 2: Emergency
./healthcare_backend store test_emergency_record.json EMERGENCY_002
./healthcare_backend retrieve EMERGENCY_002 output.json

# Test 3: Lab Results
./healthcare_backend store test_lab_results.json LAB_TEST_003
./healthcare_backend retrieve LAB_TEST_003 output.json

# Test 4: Prescription
./healthcare_backend store test_prescription.json RX_TEST_004
./healthcare_backend retrieve RX_TEST_004 output.json

# Test 5: Imaging
./healthcare_backend store test_imaging_report.json IMAGING_005
./healthcare_backend retrieve IMAGING_005 output.json

# Test 6: Full Record
./healthcare_backend store sample_patient_record.json SAMPLE_PATIENT
./healthcare_backend retrieve SAMPLE_PATIENT output.json
```

---

## Test Success Criteria

Each test must:

1. ✅ **Encrypt successfully** - File encrypted with AES-256-CBC
2. ✅ **Generate hash** - SHA-256 hash created for Solana
3. ✅ **Upload to S3** - File stored in DigitalOcean Spaces
4. ✅ **Download from S3** - File retrieved intact
5. ✅ **Decrypt successfully** - Original data recovered
6. ✅ **Data integrity** - Output matches input exactly

### Verification Commands

```bash
# Verify data integrity
diff original.json decrypted.json

# Should output nothing if files are identical

# Calculate hash for verification
sha256sum original.json
sha256sum decrypted.json
# Hashes should match
```

---

## Performance Benchmarks

Expected performance (on standard hardware):

| Test Case | Encryption | Upload | Download | Decryption | Total |
|-----------|-----------|--------|----------|------------|-------|
| Simple    | <10ms     | 100ms  | 100ms    | <10ms      | ~220ms |
| Emergency | <20ms     | 150ms  | 150ms    | <20ms      | ~340ms |
| Lab       | <30ms     | 200ms  | 200ms    | <30ms      | ~460ms |
| Rx        | <20ms     | 150ms  | 150ms    | <20ms      | ~340ms |
| Imaging   | <40ms     | 250ms  | 250ms    | <40ms      | ~580ms |
| Full      | <30ms     | 200ms  | 200ms    | <30ms      | ~460ms |

*Note: Upload/download times depend on network speed*

---

## Security Testing

Each test validates:

### Encryption Security
- ✅ AES-256-CBC encryption applied
- ✅ Random IV generated per file
- ✅ No plaintext visible in encrypted file
- ✅ Different IV produces different ciphertext

### Access Control
- ✅ S3 authentication required
- ✅ Private ACL enforced
- ✅ HTTPS transport encryption
- ✅ Environment-based credentials

### Data Integrity
- ✅ SHA-256 hash verification
- ✅ Byte-for-byte comparison
- ✅ No data loss during encryption/decryption
- ✅ No data corruption during transfer

---

## Troubleshooting Test Failures

### Test Fails at Encryption
```bash
# Check OpenSSL installation
openssl version

# Verify encryption key is set
echo $ENCRYPTION_KEY  # Should be 64 hex characters
```

### Test Fails at Upload
```bash
# Check environment variables
echo $DO_SPACE_NAME
echo $DO_REGION
echo $DO_ACCESS_KEY

# Test DigitalOcean connectivity
curl https://$DO_SPACE_NAME.$DO_REGION.digitaloceanspaces.com
```

### Test Fails at Decryption
```bash
# Ensure same encryption key is used
# Compare key used for encryption vs decryption

# Check file wasn't corrupted
sha256sum encrypted_file.bin
```

### Test Fails at Comparison
```bash
# Check for encoding issues
file original.json
file decrypted.json

# Look for differences
diff -u original.json decrypted.json
```

---

## Continuous Integration

To integrate into CI/CD:

```yaml
# .github/workflows/test.yml
name: Healthcare Backend Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y libcurl4-openssl-dev libssl-dev
      
      - name: Compile
        run: |
          cd Backend
          make
      
      - name: Run tests
        env:
          DO_SPACE_NAME: ${{ secrets.DO_SPACE_NAME }}
          DO_REGION: ${{ secrets.DO_REGION }}
          DO_ACCESS_KEY: ${{ secrets.DO_ACCESS_KEY }}
          DO_SECRET_KEY: ${{ secrets.DO_SECRET_KEY }}
          ENCRYPTION_KEY: ${{ secrets.ENCRYPTION_KEY }}
        run: |
          cd Backend
          ./run_tests.sh
```

---

## Test Data Privacy

**⚠️ IMPORTANT:** All test data is **synthetic/fictional**

- No real patient information
- Fictional names and identifiers
- Safe for development/testing
- Can be committed to version control

**NEVER use real patient data in testing!**

To create test data from real records:
1. Use synthetic data generators
2. De-identify all PHI
3. Randomize all identifiers
4. Anonymize all dates
5. Replace all names with fictional ones

---

## Next Steps

After successful testing:

1. ✅ Integrate with Solana smart contract
2. ✅ Add hash verification before retrieval
3. ✅ Implement access control via blockchain
4. ✅ Add audit logging
5. ✅ Deploy to production environment

See [README.md](README.md) for Solana integration details.
