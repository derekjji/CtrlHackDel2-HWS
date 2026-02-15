# Frontend & Backend Wallet Encryption Sync Guide

## Overview
The system uses:
- **Frontend**: Solana wallet (Solflare) to sign transactions and authenticate
- **Backend**: Stores wallet addresses in metadata for permission validation
- **Encryption**: AES-256-CBC with random IV for data protection
- **Proof Chain**: Wallet transactions posted to Solana memo program for audit trail

## Critical Setup Requirements

### 1. Backend Environment Variables

**Required for encryption to work:**
```bash
# Do NOT forget to set these BEFORE running the backend server
export ENCRYPTION_KEY=$(openssl rand -hex 32)
export DO_SPACE_NAME="your-space-name"
export DO_REGION="nyc3"  # or sfo3, etc
export DO_ACCESS_KEY="your-access-key"
export DO_SECRET_KEY="your-secret-key"
```

**Location**: Backend/.env or Backend/web_api/.env

**Verify:**
```bash
echo $ENCRYPTION_KEY  # Should show 64 hex characters (32 bytes)
```

### 2. Frontend Wallet Configuration

**File**: frontend/src/main.jsx
- ✅ Solflare wallet enabled (Phantom removed)
- ✅ autoConnect enabled (persistent wallet state)
- ✅ ConnectionProvider configured with correct endpoint

**Verify**: 
```bash
# In browser console after wallet connects
console.log(wallet.publicKey?.toBase58())  # Should show wallet address like: So1...
```

### 3. Frontend-to-Backend Communication

**Flow for Store:**
1. Frontend: `wallet.publicKey?.toBase58()` → wallet address
2. Frontend: POST `/api/records/store` with:
   ```json
   {
     "patientId": "patient-001",
     "record": { /* sanitized record data */ },
     "ownerPubkey": "SolflareWalletAddress...",
     "allowedViewers": ["OtherWallet..."],
     "storeProofTx": "solana-transaction-hash"
   }
   ```
3. Backend: Validates `ownerPubkey` format (should be valid Solana address)
4. Backend: Encrypts record with ENCRYPTION_KEY
5. Backend: Stores metadata with wallet addresses

**Flow for Retrieve:**
1. Frontend: GET `/api/records/:patientId?requestor=walletAddress`
2. Backend: Checks if `requestor` is `ownerPubkey` OR in `allowedViewers`
3. Backend: If permitted, decrypts and returns record
4. Backend: Logs access to audit trail with wallet address

### 4. Common Issues & Fixes

#### Issue: "Invalid ENCRYPTION_KEY (must be 64 hex chars / 32 bytes)"
**Cause**: ENCRYPTION_KEY not set or wrong format
**Fix**: 
```bash
# In Backend directory
export ENCRYPTION_KEY=$(openssl rand -hex 32)
node web_api/src/server.js
```

#### Issue: Permission Denied / Access Denied
**Cause**: Wallet address mismatch or not in allowed viewers
**Fix**:
1. Check frontend wallet matches stored `ownerPubkey`
2. Verify `allowedViewers` array contains requester's wallet
3. Get current wallet: `console.log(wallet.publicKey?.toBase58())`

#### Issue: Encryption/Decryption Failed
**Cause**: Different keys used between frontend and backend
**Fix**:
1. Frontend doesn't handle encryption (backend-only)
2. Save ENCRYPTION_KEY somewhere persistent
3. Verify it's set before each backend start

### 5. Testing Checklist

- [ ] Backend ENCRYPTION_KEY is set (`echo $ENCRYPTION_KEY`)
- [ ] Backend DigitalOcean Spaces credentials are correct
- [ ] Frontend connects to Solflare wallet successfully
- [ ] Frontend wallet address displays in browser console
- [ ] Store request completes with recordHash returned
- [ ] Retrieve request returns decrypted record data
- [ ] Access audit shows wallet address of requester
- [ ] Same wallet cannot decrypt other user's records

### 6. Data Flow Verification

**Terminal 1 - Backend Server**
```bash
cd Backend/web_api
export ENCRYPTION_KEY=$(openssl rand -hex 32)
# ... set DO_* env vars ...
node src/server.js
# Should show: "Server running on port 4000"
```

**Browser - Frontend**
```javascript
// After page loads
console.log('Wallet connected:', wallet.connected)
console.log('Wallet address:', wallet.publicKey?.toBase58())

// After clicking "Store Record"
// Check Network tab for POST to /api/records/store
// Response should include: { patientId, recordHash, ownerPubkey, ... }
```

**Console Logs to Monitor**
Backend should show:
- `File encrypted successfully`
- `Patient record stored successfully`
- `Downloaded file hash`
- `Patient record retrieved successfully`

### 7. Wallet Address Format

- **Solflare**: Base58 encoded Solana public key (44 characters)
- **Example**: `9B5X6wrjCHVJYNL2S4NFj9KYbS5jmdoYPjPKPxKN6RRX`
- **Validation**: Must be valid base58 string

### 8. Encryption Key Format

- **Required**: 64 hexadecimal characters (32 bytes)
- **Example**: `a1b2c3d4e5f6...` (64 chars exactly)
- **Generation**: `openssl rand -hex 32`
- **Storage**: Save in .env file, NOT in code

## Quick Resync Command

```bash
# Backend
cd Backend/web_api
export ENCRYPTION_KEY=$(openssl rand -hex 32)
export DO_SPACE_NAME="your-space"
export DO_REGION="nyc3"
export DO_ACCESS_KEY="your-key"
export DO_SECRET_KEY="your-secret"
node src/server.js

# Frontend (new terminal)
cd frontend
npm run dev  # Visit http://localhost:5173
```

## Next Steps

1. Verify ENCRYPTION_KEY is stored persistently
2. Test Store → Retrieve flow with same wallet
3. Test access control with different wallet
4. Monitor console logs for encryption operations
