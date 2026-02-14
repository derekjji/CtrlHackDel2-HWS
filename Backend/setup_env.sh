#!/bin/bash
# setup_env.sh - Setup script for configuring the healthcare backend
# Source this file: source setup_env.sh

echo "Healthcare Data Backend - Environment Setup"
echo "============================================"
echo ""

# DigitalOcean Spaces Configuration
echo "Enter your DigitalOcean Spaces configuration:"
read -p "Space Name: " DO_SPACE_NAME
read -p "Region (e.g., nyc3, sfo3): " DO_REGION
read -p "Access Key: " DO_ACCESS_KEY
read -sp "Secret Key: " DO_SECRET_KEY
echo ""
echo ""

# Export DigitalOcean credentials
export DO_SPACE_NAME
export DO_REGION
export DO_ACCESS_KEY
export DO_SECRET_KEY

# Generate encryption key if not set
if [ -z "$ENCRYPTION_KEY" ]; then
    echo "Generating new 256-bit encryption key..."
    ENCRYPTION_KEY=$(openssl rand -hex 32)
    export ENCRYPTION_KEY
    echo "Generated encryption key: $ENCRYPTION_KEY"
    echo "⚠️  IMPORTANT: Save this key securely! You'll need it to decrypt data."
    echo ""
    
    # Save to file
    echo "$ENCRYPTION_KEY" > .encryption_key
    chmod 600 .encryption_key
    echo "Encryption key saved to .encryption_key (secure permissions)"
else
    echo "Using existing ENCRYPTION_KEY from environment"
fi

echo ""
echo "✓ Environment configured successfully!"
echo ""
echo "Environment variables set:"
echo "  DO_SPACE_NAME=$DO_SPACE_NAME"
echo "  DO_REGION=$DO_REGION"
echo "  DO_ACCESS_KEY=$DO_ACCESS_KEY"
echo "  DO_SECRET_KEY=***hidden***"
echo "  ENCRYPTION_KEY=***hidden***"
echo ""
echo "To persist these settings, add them to your ~/.bashrc or ~/.bash_profile"
