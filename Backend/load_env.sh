#!/bin/bash
# Load environment variables from .env file
# Usage: source load_env.sh

if [ -f .env ]; then
    echo "Loading environment variables from .env..."
    export $(cat .env | grep -v '^#' | grep -v '^[[:space:]]*$' | xargs)
    echo "✓ Environment variables loaded"
    echo ""
    echo "Loaded variables:"
    echo "  DO_SPACE_NAME=$DO_SPACE_NAME"
    echo "  DO_REGION=$DO_REGION"
    echo "  DO_ACCESS_KEY=[set]"
    echo "  DO_SECRET_KEY=[hidden]"
    echo "  ENCRYPTION_KEY=[hidden]"
else
    echo "Error: .env file not found!"
    echo ""
    echo "Please create .env from the template:"
    echo "  cp .env.example .env"
    echo "  nano .env  # Edit with your credentials"
    exit 1
fi
