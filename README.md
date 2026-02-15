# HWS (Health Web Services)

**Secure, encrypted healthcare records with blockchain verification**

HWS is a full-stack healthcare record management system that stores patient data encrypted in the cloud (DigitalOcean Spaces) with tamper-proof audit trails on Solana blockchain. Built with React frontend, Node.js API, and high-performance C backend.

## 🚀 Quick Start

See **[START.md](START.md)** for setup instructions.

---

## 📖 How to Use the Program

### 1. Open the Application

1. Make sure both servers are running (see [START.md](START.md)):
   - Web API on port 4000
   - Frontend on port 5173
2. Open your browser to **http://localhost:5173**
3. You'll see a splash screen, then the main interface

### 2. Connect Your Solana Wallet

1. Click **"Select Wallet"** button in the top-right corner
2. Choose your wallet provider (Phantom, Solflare, etc.)
3. Approve the connection request in your wallet
4. Your wallet address will appear in the button

> **Why Solana?** Every record store/retrieve operation creates a tamper-proof audit entry on Solana blockchain for compliance and security.

### 3. Store a Patient Record

**Fill out the form:**
- **Patient ID:** Unique identifier (e.g., `PATIENT_001`)
- **Patient Name:** Full name
- **Date of Birth:** YYYY-MM-DD format (e.g., `2000-01-01`)
- **Diagnosis:** Medical diagnosis or condition
- **Medications:** Comma-separated list (e.g., `Aspirin, Lisinopril`)
- **Allergies:** Comma-separated list (e.g., `Penicillin, Shellfish`)
- **Last Visit:** Date of last visit (YYYY-MM-DD)
- **Additional Notes:** Any extra information
- **Allowed Viewer Wallets:** Optional comma-separated Solana wallet addresses who can access this record

**Click "Store Record"**

What happens:
1. ✅ Your data is encrypted with AES-256-CBC
2. ✅ A proof transaction is written to Solana blockchain
3. ✅ Encrypted file is uploaded to DigitalOcean Spaces
4. ✅ You'll see: `Stored successfully. Hash: [record-hash]`

### 4. Retrieve a Patient Record

**Enter the Patient ID** in the "Retrieve Patient Record" section and click **"Retrieve Record"**.

What happens:
1. ✅ System checks if you're authorized (owner or allowed viewer)
2. ✅ Encrypted file is downloaded from DigitalOcean Spaces
3. ✅ Data is decrypted on the server
4. ✅ An access audit proof is written to Solana
5. ✅ Decrypted record appears on screen

**If you're not authorized:** You'll see an error message.

### 5. View Patient History

Recent patient lookups appear in the **"Recent Patient History"** section:
- Patient ID and name
- Date of birth and last visit
- Record hash (blockchain verification)
- Timestamp when you accessed it

Click **"Clear History"** to remove all entries from browser storage.

### 6. View Audit Trail

After retrieving a record, click **"Show Audits"** to see:
- Who accessed the record (wallet address)
- When they accessed it
- Solana transaction signature (blockchain proof)
- Result (SUCCESS/DENIED)

---

## 🏗️ Architecture

```
┌─────────────────────┐
│   React Frontend    │  ← User Interface
│   localhost:5173    │     Solana Wallet Integration
└──────────┬──────────┘
           │ HTTP/REST
           ↓
┌─────────────────────┐
│   Node.js Web API   │  ← Access Control
│   localhost:4000    │     Metadata Management
└──────────┬──────────┘
           │ execFile()
           ↓
┌─────────────────────┐
│   C Backend         │  ← AES-256 Encryption
│   healthcare_backend│     S3 Authentication
└──────────┬──────────┘
           │ HTTPS
           ↓
┌─────────────────────┐     ┌──────────────┐
│ DigitalOcean Spaces │     │   Solana     │
│  (Encrypted Data)   │     │  (Blockchain │
│   Off-Chain Storage │     │  Audit Logs) │
└─────────────────────┘     └──────────────┘
```

### Why This Design?

**Security Layers:**
- 🔐 **AES-256-CBC Encryption:** Patient data encrypted before leaving your computer
- 🌐 **DigitalOcean Spaces:** Secure cloud storage with private ACLs
- ⛓️ **Solana Blockchain:** Immutable audit trail that cannot be tampered with
- 🎫 **Access Control:** Only record owner and approved viewers can decrypt

**Performance:**
- ⚡ C backend for fast encryption/decryption
- 📦 Efficient S3-compatible object storage
- 🔄 React with real-time updates

---

## 📂 Project Structure

- **`Backend/`** - C backend source code
  - `healthcare_backend.c` - Main encryption + storage logic
  - `s3_auth.c/h` - AWS S3 Signature V4 authentication
  - `encrypt_util.c` - AES-256-CBC encryption utilities
  - `.env` - DigitalOcean credentials (not in Git)

- **`Backend/web_api/`** - Node.js Express API
  - `src/server.js` - REST endpoints
  - `data/records_meta.json` - Access control metadata

- **`Frontend/`** - React + Vite application
  - `src/App.jsx` - Main UI component
  - `src/solana.js` - Blockchain interaction

- **Documentation:**
  - `START.md` - Quick start guide
  - `INTEGRATION.md` - Full stack integration details
  - `TESTING.md` - Test suite documentation

---

## 🔒 Security & Privacy

**Your data is protected by:**
- Military-grade AES-256-CBC encryption
- Unique encryption key per deployment
- Wallet-based authentication
- Whitelist access control
- Blockchain audit trails
- No plaintext data in cloud storage

**⚠️ Never commit your `.env` files to Git!** They contain your encryption keys and cloud credentials.

---

## 🛠️ Development Setup

### Prerequisites
- **Windows:** MSYS2 UCRT64 terminal with GCC, libcurl, OpenSSL
- **Node.js:** v18+ for Web API and Frontend
- **Solana Wallet:** Phantom or Solflare browser extension
- **DigitalOcean Spaces:** Account with access/secret keys

### Installation

**1. Compile C Backend:**
```bash
cd Backend
cp .env.example .env
# Edit .env with your DigitalOcean credentials
make
```

**2. Start Web API:**
```powershell
cd Backend\web_api
Copy-Item .env.example .env
# Edit .env with same credentials
npm install
npm run dev
```

**3. Start Frontend:**
```powershell
cd Frontend
Copy-Item .env.example .env
npm install
npm run dev
```

**4. Open Browser:**
http://localhost:5173

See [START.md](START.md) for detailed setup instructions.

---

## 📋 API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| `POST` | `/api/records/store` | Store encrypted patient record |
| `GET` | `/api/records/:patientId` | Retrieve and decrypt record |
| `POST` | `/api/records/:patientId/audit` | Add audit log entry |
| `GET` | `/api/records/:patientId/audits` | Get all audit logs |

---

## 🧪 Testing

Run test suite:
```bash
cd Backend
./run_tests.sh    # Linux/Mac
./run_tests.ps1   # Windows
```

See [TESTING.md](Backend/TESTING.md) for comprehensive test cases.

---

## 📄 License

This project is for educational and development purposes. Ensure HIPAA compliance before using in production healthcare environments.
