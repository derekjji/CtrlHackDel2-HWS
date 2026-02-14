# CtrlHackDel2 - Healthcare Data Security System

Reengineering healthcare by redesigning data flow, standardizing programs, and enhancing security.

## 🏥 Overview

This project implements a secure healthcare data management system combining:
- **DigitalOcean Spaces**: Encrypted off-chain storage of patient records
- **Solana Blockchain**: Tamper-proof verification, access control, and audit trails

## 🔐 Key Features

- ✅ **AES-256-CBC Encryption** - Military-grade encryption for patient data
- ✅ **S3-Compatible Storage** - Secure cloud storage on DigitalOcean Spaces
- ✅ **Blockchain Verification** - Solana-based tamper detection and access control
- ✅ **HIPAA-Ready** - Encryption at rest and in transit
- ✅ **Production-Ready** - Complete error handling and logging

## 📁 Project Structure

```
CtrlHackDel2/
├── Backend/                    # C backend for encrypted storage
│   ├── healthcare_backend.c    # Main integrated backend
│   ├── digitalocean_backend.c  # Basic S3 operations
│   ├── s3_auth.c/h             # AWS S3 signature authentication
│   ├── encrypt_util.c          # Encryption utilities
│   ├── test_*.json             # Test patient records
│   ├── Makefile                # Build configuration
│   ├── .env.example            # Environment template (safe to share)
│   └── README.md               # Detailed documentation
└── .github/workflows/          # CI/CD configuration
```

## 🚀 Quick Start

### 1. Clone the Repository
```bash
git clone https://github.com/yourusername/CtrlHackDel2.git
cd CtrlHackDel2/Backend
```

### 2. Install Dependencies

**Windows (MSYS2):**
```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-curl mingw-w64-ucrt-x86_64-openssl
```

**Linux:**
```bash
sudo apt-get install build-essential libcurl4-openssl-dev libssl-dev
```

### 3. Configure Environment
```bash
# Copy the template
cp .env.example .env

# Edit with your credentials
nano .env

# Load environment
source load_env.sh  # Linux/Mac
. .\load_env.ps1     # Windows
```

### 4. Build & Test
```bash
# Compile
make

# Test encryption
./encrypt_util

# Run full tests (requires DigitalOcean configured)
./run_tests.sh  # Linux/Mac
.\run_tests.ps1  # Windows
```

## 📖 Documentation

- **[Backend/README.md](Backend/README.md)** - Complete backend documentation
- **[Backend/SETUP.md](Backend/SETUP.md)** - Detailed setup guide
- **[Backend/TESTING.md](Backend/TESTING.md)** - Testing guide
- **[Backend/TEST_CASES.md](Backend/TEST_CASES.md)** - Test case documentation

## 🔒 Security

### Safe to Share (Committed to Git):
- ✅ `.env.example` - Template configuration
- ✅ Source code
- ✅ Test data (synthetic records)
- ✅ Documentation

### Never Share (Gitignored):
- ❌ `.env` - Your actual credentials
- ❌ `.encryption_key` - Encryption keys
- ❌ Any files with real secrets

### For Contributors:
1. Never commit secrets
2. Always use `.env.example` as template
3. Keep `.env` local only
4. Use GitHub Secrets for CI/CD

## 🏗️ Architecture

```
Patient Record
    ↓
AES-256 Encryption
    ↓
    ├──→ DigitalOcean Spaces (Encrypted Data)
    └──→ Solana Blockchain (Hash + Access Control)
```

## 📊 Use Cases

1. **Electronic Health Records (EHR)** - Complete patient histories
2. **Lab Results** - Diagnostic test data with references
3. **Prescriptions** - E-prescribing with DEA/NPI validation
4. **Imaging Reports** - Radiology with DICOM metadata
5. **Emergency Records** - Time-sensitive critical care data

## 🧪 Testing

Six comprehensive test cases included:
1. Simple patient record
2. Emergency room visit
3. Laboratory results
4. E-prescription
5. Radiology report
6. Full EHR

Run all tests:
```bash
./run_tests.sh  # Linux/Mac
.\run_tests.ps1  # Windows
```

## 🤝 Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Never commit secrets (check `.gitignore`)
4. Commit your changes (`git commit -m 'Add amazing feature'`)
5. Push to the branch (`git push origin feature/amazing-feature`)
6. Open a Pull Request

## 📝 License

See LICENSE file for details.

## 🆘 Support

- Check [SETUP.md](Backend/SETUP.md) for setup troubleshooting
- See [TESTING.md](Backend/TESTING.md) for test issues
- Review [Backend/README.md](Backend/README.md) for detailed docs

## 🎯 Roadmap

- [x] Backend encryption system
- [x] DigitalOcean Spaces integration
- [x] Comprehensive test suite
- [ ] Solana smart contract integration
- [ ] Web interface
- [ ] Mobile app
- [ ] FHIR compatibility

---

**Built for Healthcare, Secured by Design, Verified by Blockchain** 🏥🔐⛓️