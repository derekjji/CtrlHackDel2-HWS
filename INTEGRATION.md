# Full Stack Integration Guide

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                         Frontend (React)                         │
│                     http://localhost:5173                        │
│            - Solana wallet integration                           │
│            - Patient record UI                                   │
│            - Blockchain proof submission                         │
└───────────────────────────┬─────────────────────────────────────┘
                            │ HTTP API
                            ↓
┌─────────────────────────────────────────────────────────────────┐
│                   Web API (Node.js/Express)                      │
│                     http://localhost:4000                        │
│            - REST API endpoints                                  │
│            - Access control logic                                │
│            - Calls C backend via execFile                        │
└───────────────────────────┬─────────────────────────────────────┘
                            │ Process execution
                            ↓
┌─────────────────────────────────────────────────────────────────┐
│                  C Backend (healthcare_backend.exe)              │
│            - AES-256-CBC encryption/decryption                   │
│            - S3 authentication                                   │
│            - DigitalOcean Spaces upload/download                 │
└───────────────────────────┬─────────────────────────────────────┘
                            │ HTTPS
                            ↓
                 DigitalOcean Spaces (Encrypted Storage)
                            +
                 Solana Blockchain (Hash Proofs & Audits)
```

## Quick Start - Full Stack

### Prerequisites
- Node.js 18+ installed
- MSYS2 with GCC (for C backend)
- DigitalOcean Spaces account
- Solana wallet (Phantom/Solflare)

### 1. Setup Backend (C)

```bash
# Open MSYS2 UCRT64 terminal
cd /c/Users/derek/Downloads/CtrlHackDel2/Backend

# Create environment file
cp .env.example .env
nano .env  # Add your DigitalOcean credentials

# Load environment
source load_env.sh

# Compile
make

# Test
./encrypt_util
```

### 2. Setup Web API (Node.js)

```bash
# In PowerShell or MSYS2
cd Backend/web_api

# Copy template
cp .env.example .env

# Edit .env (Windows)
notepad .env

# Or edit in MSYS2
nano .env

# Required variables:
# PORT=4000
# CORS_ORIGIN=http://localhost:5173
# BACKEND_BINARY=..\\healthcare_backend.exe  (Windows)
# or: ../healthcare_backend                   (Linux)

# The web API uses the SAME environment variables as C backend:
# DO_SPACE_NAME, DO_REGION, DO_ACCESS_KEY, DO_SECRET_KEY, ENCRYPTION_KEY

# Install dependencies
npm install

# Run
npm run dev
```

### 3. Setup Frontend (React/Vite)

```bash
# In PowerShell or terminal
cd Frontend

# Create .env file
copy .env.example .env  # Windows
# or: cp .env.example .env  # Linux

# Edit Frontend/.env
notepad .env

# Add:
# VITE_API_URL=http://localhost:4000
# VITE_SOLANA_NETWORK=devnet

# Install dependencies
npm install

# Run
npm run dev
```

### 4. Access the Application

Open browser: **http://localhost:5173**

## Environment Configuration

### Backend/. env (C Backend)
```bash
DO_SPACE_NAME=your-space-name
DO_REGION=nyc3
DO_ACCESS_KEY=your-access-key
DO_SECRET_KEY=your-secret-key
ENCRYPTION_KEY=your-64-char-hex-key
```

### Backend/web_api/.env (Node.js API)
```bash
PORT=4000
CORS_ORIGIN=http://localhost:5173
BACKEND_BINARY=..\\healthcare_backend.exe

# Same as C backend:
DO_SPACE_NAME=your-space-name
DO_REGION=nyc3
DO_ACCESS_KEY=your-access-key
DO_SECRET_KEY=your-secret-key
ENCRYPTION_KEY=your-64-char-hex-key
```

### Frontend/.env (React App)
```bash
VITE_API_URL=http://localhost:4000
VITE_SOLANA_NETWORK=devnet
```

## API Endpoints

### POST /api/records/store
Store an encrypted patient record.

**Request:**
```json
{
  "patientId": "patient-001",
  "record": {
    "patientName": "John Doe",
    "dob": "1980-02-03",
    "diagnosis": "Type 2 Diabetes"
  },
  "ownerPubkey": "solana-wallet-address",
  "allowedViewers": ["viewer-wallet-1", "viewer-wallet-2"],
  "storeProofTx": "solana-tx-signature"
}
```

**Response:**
```json
{
  "patientId": "patient-001",
  "recordHash": "abc123...",
  "ownerPubkey": "owner-wallet",
  "allowedViewers": ["viewer-1"],
  "storeProofTx": "solana-tx"
}
```

### GET /api/records/:patientId?requestor=wallet
Retrieve and decrypt a patient record.

**Response:**
```json
{
  "patientId": "patient-001",
  "recordHash": "abc123...",
  "ownerPubkey": "owner-wallet",
  "allowedViewers": ["viewer-1"],
  "record": {
    "patientName": "John Doe",
    "dob": "1980-02-03"
  }
}
```

### POST /api/records/:patientId/audit
Add audit log entry.

**Request:**
```json
{
  "actorPubkey": "wallet-address",
  "action": "READ",
  "result": "SUCCESS",
  "solanaTx": "solana-tx-signature"
}
```

### GET /api/records/:patientId/audits
Get audit trail for a record.

## Data Flow Example

### Storing a Record:

1. **User fills form** in Frontend
2. **Frontend connects** to Solana wallet
3. **Frontend posts** proof memo to Solana blockchain
4. **Frontend calls** `POST /api/records/store` with record data
5. **Web API receives** request
6. **Web API saves** record JSON to temp file
7. **Web API executes** `healthcare_backend.exe store tempfile.json patient-id`
8. **C backend encrypts** file with AES-256-CBC
9. **C backend uploads** to DigitalOcean Spaces with S3 auth
10. **C backend returns** SHA-256 hash
11. **Web API saves** metadata (hash, owner, viewers)
12. **Frontend displays** success + hash

### Retrieving a Record:

1. **User enters** patient ID
2. **Frontend checks** requestor's wallet is connected
3. **Frontend calls** `GET /api/records/:id?requestor=wallet`
4. **Web API checks** permissions (owner or allowed viewer)
5. **Web API executes** `healthcare_backend.exe retrieve patient-id output.json`
6. **C backend downloads** encrypted file from DigitalOcean
7. **C backend decrypts** file with AES-256-CBC
8. **C backend saves** plaintext to temp file
9. **Web API reads** decrypted JSON
10. **Web API returns** record data
11. **Frontend posts** access proof to Solana
12. **Frontend displays** decrypted record

## Security Features

### Encryption (C Backend)
- **AES-256-CBC** with random IVs
- **Military-grade** protection
- **Unique IV** per file

### Access Control (Web API)
- **Owner-based** permissions
- **Viewer whitelist** per record
- **Wallet signature** verification

### Blockchain (Solana)
- **Tamper-proof** audit trail
- **Hash verification** for integrity
- **Immutable** access logs

### Storage (DigitalOcean)
- **S3 authentication** (Signature V4)
- **HTTPS** transmission
- **Private ACLs** on all files

## Testing the Full Stack

### 1. Test C Backend Alone
```bash
cd Backend
./healthcare_backend store sample_patient_record.json TEST_001
./healthcare_backend retrieve TEST_001 output.json
diff sample_patient_record.json output.json
```

### 2. Test Web API
```bash
# Terminal 1: Start API
cd Backend/web_api
npm run dev

# Terminal 2: Test endpoints
curl http://localhost:4000/health
# Should return: {"status":"ok"}

# Test store (requires C backend compiled)
curl -X POST http://localhost:4000/api/records/store \
  -H "Content-Type: application/json" \
  -d '{"patientId":"test-001","record":{"name":"Test"},"ownerPubkey":"wallet123"}'
```

### 3. Test Frontend
```bash
# Terminal 1: API running (from step 2)
# Terminal 2: Start frontend
cd Frontend
npm run dev

# Open browser: http://localhost:5173
# 1. Connect Solana wallet
# 2. Fill in patient ID and record JSON
# 3. Click "Store Record"
# 4. Check console for transaction
```

## Troubleshooting

### C Backend Errors

**"Missing environment variables"**
```bash
cd Backend
source load_env.sh
echo $DO_SPACE_NAME  # Should show your space name
```

**"NoSuchBucket" (HTTP 404)**
- Verify Space exists in DigitalOcean
- Check DO_SPACE_NAME matches exactly
- Ensure DO_REGION is correct

### Web API Errors

**"Cannot find healthcare_backend.exe"**
```bash
# Check BACKEND_BINARY path in web_api/.env
# Windows: ..\\healthcare_backend.exe
# Linux: ../healthcare_backend

# Verify file exists
cd Backend/web_api
ls ../healthcare_backend.exe  # Windows
ls ../healthcare_backend       # Linux
```

**"ENOENT" or "spawn" errors**
- Make sure C backend is compiled
- Check file permissions (chmod +x on Linux)
- Verify path separators (\ for Windows, / for Linux)

### Frontend Errors

**"Network request failed"**
- Check Web API is running on port 4000
- Verify VITE_API_URL in Frontend/.env
- Check CORS settings in web_api

**"Wallet not connected"**
- Install Phantom or Solflare browser extension
- Switch to Devnet in wallet settings
- Click "Connect Wallet" button

### CORS Errors

If you see CORS errors in browser console:

1. Check `Backend/web_api/.env`:
   ```bash
   CORS_ORIGIN=http://localhost:5173
   ```

2. Restart Web API after changing CORS settings

## Production Deployment

### Environment Setup

**Never commit:**
- Any `.env` file with real credentials
- `.encryption_key` files
- Real patient data

**Do commit:**
- `.env.example` templates
- Documentation
- Test data (synthetic only)

### Deployment Checklist

- [ ] Use separate encryption keys per environment
- [ ] Configure production DigitalOcean Space
- [ ] Set up proper CORS origins
- [ ] Enable HTTPS for API
- [ ] Use environment variables for all secrets
- [ ] Set up monitoring and logging
- [ ] Configure rate limiting
- [ ] Implement backup strategy
- [ ] Set up CI/CD with GitHub Actions
- [ ] Configure Solana mainnet connection

## Development Workflow

```bash
# 1. Start all services
cd Backend/web_api && npm run dev &
cd Frontend && npm run dev &

# 2. Open browser to http://localhost:5173

# 3. Make changes to any component

# 4. Hot reload is enabled for:
#    - Frontend (Vite HMR)
#    - Web API (--watch flag)
#    - C backend (recompile with make)

# 5. Test changes immediately
```

## Next Steps

1. ✅ Backend encryption - Complete
2. ✅ Web API integration - Complete
3. ✅ Frontend UI - Complete
4. ✅ Solana proofs - Complete
5. [ ] Add more test cases
6. [ ] Implement FHIR compatibility
7. [ ] Add mobile app support
8. [ ] Enhanced analytics dashboard

---

**Full stack is ready!** 🚀

All three layers are working together:
- Frontend (React + Solana)
- Web API (Node.js/Express)
- C Backend (Encryption + Storage)
