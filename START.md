# Start All Services - Full Stack

## Quick Start (PowerShell)

```powershell
# Terminal 1: Web API
cd Backend\web_api
npm run dev

# Terminal 2: Frontend  
cd Frontend
npm run dev

# Open browser: http://localhost:5173
```

## Quick Start (MSYS2/Linux)

```bash
# Terminal 1: Web API
cd Backend/web_api
npm run dev

# Terminal 2: Frontend
cd Frontend
npm run dev

# Open browser: http://localhost:5173
```

## Prerequisites

### 1. Compile C Backend

```bash
# MSYS2 UCRT64 terminal
cd Backend
source load_env.sh
make
```

### 2. Configure Web API

```bash
cd Backend/web_api
cp .env.example .env
# Edit .env with your credentials
notepad .env  # Windows
nano .env      # Linux
```

### 3. Configure Frontend

```bash
cd Frontend  
cp .env.example .env
# Usually defaults are fine
```

## Verifying Services

### Check C Backend
```bash
cd Backend
./healthcare_backend store sample_patient_record.json TEST
# Should encrypt and upload
```

### Check Web API
```bash
# Open new terminal
curl http://localhost:4000/health
# Should return: {"status":"ok"}
```

### Check Frontend
```
Open browser: http://localhost:5173
Should see "HWS - Health Web Services" interface
```

## Full Architecture Running

```
┌─────────────────────────┐
│   Frontend:5173         │ ← Open in browser
└───────────┬─────────────┘
            │ HTTP
            ↓
┌─────────────────────────┐
│   Web API:4000          │ ← Terminal 1
└───────────┬─────────────┘
            │ execFile()
            ↓
┌─────────────────────────┐
│   C Backend             │ ← Compiled binary
│   healthcare_backend    │
└───────────┬─────────────┘
            │ HTTPS
            ↓
    DigitalOcean Spaces
            +
    Solana Blockchain
```

## Stopping Services

```bash
# Press Ctrl+C in each terminal running npm
```

## Environment Summary

You need 3 environment files:

1. **Backend/.env** - C backend credentials
   ```bash
   DO_SPACE_NAME, DO_REGION, DO_ACCESS_KEY, 
   DO_SECRET_KEY, ENCRYPTION_KEY
   ```

2. **Backend/web_api/.env** - API config + same credentials
   ```bash
   PORT, CORS_ORIGIN, BACKEND_BINARY,
   + all Backend/.env variables
   ```

3. **Frontend/.env** - Frontend config
   ```bash
   VITE_API_URL, VITE_SOLANA_NETWORK
   ```

## Troubleshooting

**"Cannot find healthcare_backend.exe"**
- Compile C backend first: `cd Backend && make`
- Check BACKEND_BINARY path in web_api/.env

**"CORS error" in browser**
- Verify web_api is running
- Check CORS_ORIGIN in web_api/.env includes http://localhost:5173

**"Connection refused" to API**
- Web API must be running on port 4000
- Check VITE_API_URL in Frontend/.env

See [INTEGRATION.md](INTEGRATION.md) for detailed troubleshooting.

---

**Ready!** Open http://localhost:5173 and connect your Solana wallet 🚀
