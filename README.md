# HWS (Health Web Services)

This repo now includes a working React frontend connected to your existing C backend through a lightweight JS API wrapper.

## What is where

- `Backend/`: your existing C backend (`healthcare_backend.c`, DigitalOcean Spaces storage, encryption)
- `Backend/web_api/`: Express wrapper API that calls the C binary and exposes HTTP endpoints for the frontend
- `frontend/`: React + Vite frontend with Solana wallet integration and memo proofs

## 1) Build your existing C backend

From `Backend/`:

```powershell
make
```

Ensure `healthcare_backend.exe` exists in `Backend/`.

## 2) Run web API wrapper

```powershell
cd Backend/web_api
npm install
Copy-Item .env.example .env
npm run dev
```

Default API: `http://localhost:4000`

### API endpoints

- `POST /api/records/store`
- `GET /api/records/:patientId?requestor=<walletPubkey>`
- `POST /api/records/:patientId/audit`
- `GET /api/records/:patientId/audits`

## 3) Run frontend

```powershell
cd frontend
npm install
Copy-Item .env.example .env
npm run dev
```

Frontend default: `http://localhost:5173`

## Solana in frontend

Frontend uses wallet adapter (Phantom/Solflare) and writes memo proofs to Solana Devnet for:
- record storage proof
- record access proof

Those signatures are sent to backend audit metadata.

## Important

The API wrapper stores permission/audit metadata in `Backend/web_api/data/records_meta.json`.
Your encrypted patient files remain in DigitalOcean Spaces via the existing C backend.
