#!/bin/bash
# Automated test suite for healthcare backend
# Run this script to test all functionality

set -e  # Exit on error

echo "=========================================="
echo "Healthcare Backend Test Suite"
echo "=========================================="
echo ""

# Check if executables exist
if [ ! -f "./healthcare_backend" ] && [ ! -f "./healthcare_backend.exe" ]; then
    echo "Error: healthcare_backend executable not found"
    echo "Please compile first with: make"
    exit 1
fi

# Determine executable name (Linux vs Windows)
if [ -f "./healthcare_backend.exe" ]; then
    BACKEND="./healthcare_backend.exe"
else
    BACKEND="./healthcare_backend"
fi

# Check environment variables
if [ -z "$DO_SPACE_NAME" ] || [ -z "$DO_REGION" ] || [ -z "$DO_ACCESS_KEY" ] || [ -z "$DO_SECRET_KEY" ] || [ -z "$ENCRYPTION_KEY" ]; then
    echo "Warning: Environment variables not set"
    echo "Tests will run in encryption-only mode (no upload/download)"
    echo ""
    echo "To run full tests, run: source setup_env.sh"
    FULL_TEST=false
else
    echo "✓ Environment configured"
    FULL_TEST=true
fi

echo ""
echo "Running Tests..."
echo "=========================================="

# Test 1: Simple Record
echo ""
echo "Test 1: Simple Patient Record"
echo "----------------------------------------"
if $FULL_TEST; then
    $BACKEND store test_simple_record.json TEST_SIMPLE_001
    $BACKEND retrieve TEST_SIMPLE_001 output_simple.json
    diff test_simple_record.json output_simple.json && echo "✓ Test 1 PASSED" || echo "✗ Test 1 FAILED"
    rm -f output_simple.json test_simple_record.json.encrypted
else
    echo "Skipping (no env config)"
fi

# Test 2: Emergency Record
echo ""
echo "Test 2: Emergency Room Record"
echo "----------------------------------------"
if $FULL_TEST; then
    $BACKEND store test_emergency_record.json EMERGENCY_002
    $BACKEND retrieve EMERGENCY_002 output_emergency.json
    diff test_emergency_record.json output_emergency.json && echo "✓ Test 2 PASSED" || echo "✗ Test 2 FAILED"
    rm -f output_emergency.json test_emergency_record.json.encrypted
else
    echo "Skipping (no env config)"
fi

# Test 3: Lab Results
echo ""
echo "Test 3: Laboratory Results"
echo "----------------------------------------"
if $FULL_TEST; then
    $BACKEND store test_lab_results.json LAB_TEST_003
    $BACKEND retrieve LAB_TEST_003 output_lab.json
    diff test_lab_results.json output_lab.json && echo "✓ Test 3 PASSED" || echo "✗ Test 3 FAILED"
    rm -f output_lab.json test_lab_results.json.encrypted
else
    echo "Skipping (no env config)"
fi

# Test 4: Prescription
echo ""
echo "Test 4: E-Prescription"
echo "----------------------------------------"
if $FULL_TEST; then
    $BACKEND store test_prescription.json RX_TEST_004
    $BACKEND retrieve RX_TEST_004 output_rx.json
    diff test_prescription.json output_rx.json && echo "✓ Test 4 PASSED" || echo "✗ Test 4 FAILED"
    rm -f output_rx.json test_prescription.json.encrypted
else
    echo "Skipping (no env config)"
fi

# Test 5: Imaging Report
echo ""
echo "Test 5: Radiology Report"
echo "----------------------------------------"
if $FULL_TEST; then
    $BACKEND store test_imaging_report.json IMAGING_005
    $BACKEND retrieve IMAGING_005 output_imaging.json
    diff test_imaging_report.json output_imaging.json && echo "✓ Test 5 PASSED" || echo "✗ Test 5 FAILED"
    rm -f output_imaging.json test_imaging_report.json.encrypted
else
    echo "Skipping (no env config)"
fi

# Test 6: Original Sample Record
echo ""
echo "Test 6: Full Patient Record (Original)"
echo "----------------------------------------"
if $FULL_TEST; then
    $BACKEND store sample_patient_record.json SAMPLE_PATIENT
    $BACKEND retrieve SAMPLE_PATIENT output_sample.json
    diff sample_patient_record.json output_sample.json && echo "✓ Test 6 PASSED" || echo "✗ Test 6 FAILED"
    rm -f output_sample.json sample_patient_record.json.encrypted
else
    echo "Skipping (no env config)"
fi

echo ""
echo "=========================================="
echo "Test Suite Complete"
echo "=========================================="

if $FULL_TEST; then
    echo ""
    echo "All tests executed with full upload/download cycle"
    echo "Check for any FAILED tests above"
else
    echo ""
    echo "Tests skipped - environment not configured"
    echo "Run: source setup_env.sh to enable full testing"
fi

echo ""
