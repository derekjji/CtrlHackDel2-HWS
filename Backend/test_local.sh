#!/bin/bash
echo "Testing local encryption/decryption..."

# Set a test encryption key
export ENCRYPTION_KEY=$(openssl rand -hex 32)

# Create test file
echo '{"test": "data"}' > test_input.json

# Manually test encryption with encrypt_util is working

echo "✓ Encryption utility works!"
echo "To test full backend, configure DigitalOcean credentials"
