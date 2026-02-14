import fs from "fs/promises";
import path from "path";
import os from "os";
import { execFile } from "child_process";
import { promisify } from "util";
import express from "express";
import cors from "cors";
import dotenv from "dotenv";

dotenv.config();

const execFileAsync = promisify(execFile);
const app = express();

const PORT = Number(process.env.PORT || 4000);
const CORS_ORIGIN = process.env.CORS_ORIGIN || "http://localhost:5173";
const allowedOrigins = new Set([
  ...CORS_ORIGIN.split(",").map((o) => o.trim()).filter(Boolean),
  "http://127.0.0.1:5173",
  "http://localhost:5173"
]);
const BACKEND_BINARY = process.env.BACKEND_BINARY || "..\\healthcare_backend.exe";

const dataDir = path.resolve(process.cwd(), "data");
const metaPath = path.join(dataDir, "records_meta.json");

app.use(cors({
  origin(origin, callback) {
    if (!origin || allowedOrigins.has(origin)) {
      callback(null, true);
      return;
    }
    callback(new Error(`CORS blocked for origin: ${origin}`));
  }
}));
app.use(express.json({ limit: "2mb" }));

async function readMeta() {
  const text = await fs.readFile(metaPath, "utf8");
  return JSON.parse(text);
}

async function writeMeta(meta) {
  await fs.writeFile(metaPath, JSON.stringify(meta, null, 2));
}

function parseStoredHash(stdout) {
  const lines = stdout.split(/\r?\n/);
  for (const line of lines) {
    const match = line.match(/Hash for blockchain:\s*([a-fA-F0-9]{64})/);
    if (match) return match[1].toLowerCase();
  }
  return null;
}

async function runBackend(args) {
  const binaryPath = path.resolve(process.cwd(), BACKEND_BINARY);
  return execFileAsync(binaryPath, args, { windowsHide: true, maxBuffer: 1024 * 1024 * 5 });
}

async function ensureDataStore() {
  await fs.mkdir(dataDir, { recursive: true });
  try {
    await fs.access(metaPath);
  } catch {
    await fs.writeFile(metaPath, JSON.stringify({ records: {} }, null, 2));
  }
}

app.get("/health", (_req, res) => {
  res.json({ status: "ok" });
});

app.post("/api/records/store", async (req, res) => {
  const { patientId, record, ownerPubkey, allowedViewers = [], storeProofTx } = req.body;
  if (!patientId || !record || !ownerPubkey) {
    return res.status(400).json({ error: "patientId, record, ownerPubkey are required" });
  }

  const tempFile = path.join(os.tmpdir(), `${patientId}-${Date.now()}.json`);

  try {
    await fs.writeFile(tempFile, JSON.stringify(record, null, 2));
    const { stdout, stderr } = await runBackend(["store", tempFile, patientId]);
    if (stderr && stderr.trim()) {
      console.error(stderr);
    }

    const recordHash = parseStoredHash(stdout);
    if (!recordHash) {
      return res.status(500).json({ error: "Could not parse record hash from backend output", stdout });
    }

    const meta = await readMeta();
    const now = new Date().toISOString();
    meta.records[patientId] = {
      patientId,
      ownerPubkey,
      allowedViewers,
      recordHash,
      updatedAt: now,
      audits: [
        ...(meta.records[patientId]?.audits || []),
        {
          action: "STORE",
          result: "SUCCESS",
          actorPubkey: ownerPubkey,
          solanaTx: storeProofTx || null,
          timestamp: now
        }
      ]
    };
    await writeMeta(meta);

    res.json({
      patientId,
      recordHash,
      ownerPubkey,
      allowedViewers,
      storeProofTx: storeProofTx || null
    });
  } catch (error) {
    res.status(500).json({ error: "Store failed", details: error.message });
  } finally {
    await fs.unlink(tempFile).catch(() => {});
  }
});

app.get("/api/records/:patientId", async (req, res) => {
  const { patientId } = req.params;
  const { requestor } = req.query;
  if (!requestor) {
    return res.status(400).json({ error: "requestor query param is required" });
  }

  try {
    const meta = await readMeta();
    const recordMeta = meta.records[patientId];
    if (!recordMeta) {
      return res.status(404).json({ error: "No metadata for this patientId. Store a record first." });
    }

    const allowed = requestor === recordMeta.ownerPubkey || (recordMeta.allowedViewers || []).includes(requestor);
    if (!allowed) {
      return res.status(403).json({ error: "Access denied by permission metadata" });
    }

    const outputFile = path.join(os.tmpdir(), `${patientId}-decrypted-${Date.now()}.json`);
    const { stdout, stderr } = await runBackend(["retrieve", patientId, outputFile]);
    if (stderr && stderr.trim()) {
      console.error(stderr);
    }

    const plain = await fs.readFile(outputFile, "utf8");
    await fs.unlink(outputFile).catch(() => {});

    let record;
    try {
      record = JSON.parse(plain);
    } catch {
      record = { raw: plain };
    }

    return res.json({
      patientId,
      recordHash: recordMeta.recordHash,
      ownerPubkey: recordMeta.ownerPubkey,
      allowedViewers: recordMeta.allowedViewers || [],
      backendOutput: stdout,
      record
    });
  } catch (error) {
    return res.status(500).json({ error: "Retrieve failed", details: error.message });
  }
});

app.post("/api/records/:patientId/audit", async (req, res) => {
  const { patientId } = req.params;
  const { actorPubkey, action, result, solanaTx } = req.body;

  try {
    const meta = await readMeta();
    const recordMeta = meta.records[patientId];
    if (!recordMeta) {
      return res.status(404).json({ error: "Record metadata not found" });
    }

    recordMeta.audits = recordMeta.audits || [];
    recordMeta.audits.unshift({
      actorPubkey: actorPubkey || "unknown",
      action: action || "UNKNOWN",
      result: result || "UNKNOWN",
      solanaTx: solanaTx || null,
      timestamp: new Date().toISOString()
    });

    await writeMeta(meta);
    res.json({ ok: true });
  } catch (error) {
    res.status(500).json({ error: "Audit write failed", details: error.message });
  }
});

app.get("/api/records/:patientId/audits", async (req, res) => {
  const { patientId } = req.params;

  try {
    const meta = await readMeta();
    const recordMeta = meta.records[patientId];
    if (!recordMeta) {
      return res.status(404).json({ error: "Record metadata not found" });
    }

    res.json({ audits: recordMeta.audits || [] });
  } catch (error) {
    res.status(500).json({ error: "Audit read failed", details: error.message });
  }
});

ensureDataStore()
  .then(() => {
    app.listen(PORT, () => {
      console.log(`web_api listening on http://localhost:${PORT}`);
    });
  })
  .catch((error) => {
    console.error("Failed to initialize web_api datastore", error);
    process.exit(1);
  });
