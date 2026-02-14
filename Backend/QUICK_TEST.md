# Quick Test Reference

## Run All Tests (Automated)

```powershell
# Windows PowerShell
.\run_tests.ps1

# Linux/Mac
./run_tests.sh
```

---

## Individual Test Commands

### Test 1: Simple Record (253 bytes)
```bash
./healthcare_backend store test_simple_record.json TEST_SIMPLE_001
./healthcare_backend retrieve TEST_SIMPLE_001 output_simple.json
diff test_simple_record.json output_simple.json
```

### Test 2: Emergency Record (2.1 KB)
```bash
./healthcare_backend store test_emergency_record.json EMERGENCY_002
./healthcare_backend retrieve EMERGENCY_002 output_emergency.json
diff test_emergency_record.json output_emergency.json
```

### Test 3: Lab Results (3.7 KB)
```bash
./healthcare_backend store test_lab_results.json LAB_TEST_003
./healthcare_backend retrieve LAB_TEST_003 output_lab.json
diff test_lab_results.json output_lab.json
```

### Test 4: E-Prescription (3.1 KB)
```bash
./healthcare_backend store test_prescription.json RX_TEST_004
./healthcare_backend retrieve RX_TEST_004 output_rx.json
diff test_prescription.json output_rx.json
```

### Test 5: Imaging Report (3.7 KB)
```bash
./healthcare_backend store test_imaging_report.json IMAGING_005
./healthcare_backend retrieve IMAGING_005 output_imaging.json
diff test_imaging_report.json output_imaging.json
```

### Test 6: Full Patient Record (3 KB)
```bash
./healthcare_backend store sample_patient_record.json SAMPLE_PATIENT
./healthcare_backend retrieve SAMPLE_PATIENT output_sample.json
diff sample_patient_record.json output_sample.json
```

---

## Quick Verification

### Check if test passes (Windows PowerShell):
```powershell
if (Compare-Object (Get-Content input.json) (Get-Content output.json)) {
    Write-Host "FAILED" -ForegroundColor Red
} else {
    Write-Host "PASSED" -ForegroundColor Green
}
```

### Check if test passes (Linux/Mac):
```bash
if diff input.json output.json; then
    echo "PASSED"
else
    echo "FAILED"
fi
```

---

## What Each Test Covers

| Test | Type | Use Case |
|------|------|----------|
| 1 | Simple | Basic encryption workflow |
| 2 | Emergency | Critical care, complex data |
| 3 | Lab | Numeric data, references |
| 4 | Rx | Prescriptions, DEA/NPI |
| 5 | Imaging | Radiology, DICOM metadata |
| 6 | Full EHR | Complete patient record |

---

## Expected Output

### Successful Store:
```
=== Storing Patient Record ===
Patient ID: test_simple_record.json
File encrypted successfully: test_simple_record.json -> test_simple_record.json.encrypted
Encrypted file hash (for Solana): a3f8d9e2c1b4567890abcdef...
Uploaded to DigitalOcean Spaces: patients/TEST_SIMPLE_001.encrypted

✓ Patient record stored successfully!
  Remote path: patients/TEST_SIMPLE_001.encrypted
  Hash for blockchain: a3f8d9e2c1b4567890abcdef...
```

### Successful Retrieve:
```
=== Retrieving Patient Record ===
Patient ID: TEST_SIMPLE_001
Downloaded from DigitalOcean Spaces: patients/TEST_SIMPLE_001.encrypted
Downloaded file hash: a3f8d9e2c1b4567890abcdef...
TODO: Verify this hash against Solana blockchain
File decrypted successfully

✓ Patient record retrieved successfully!
  Decrypted to: output_simple.json
```

---

## Cleanup Test Files

```powershell
# Windows
Remove-Item output_*.json, *.encrypted

# Linux/Mac
rm -f output_*.json *.encrypted
```

---

See [TEST_CASES.md](TEST_CASES.md) for detailed test documentation.
