import { useMemo, useState } from "react";
import { useConnection, useWallet } from "@solana/wallet-adapter-react";
import { WalletMultiButton } from "@solana/wallet-adapter-react-ui";
import { postMemoProof, sha256Hex } from "./solana";

const API_URL = import.meta.env.VITE_API_URL || "http://localhost:4000";
const HISTORY_STORAGE_KEY = "hws_patient_history";
const MAX_HISTORY_ITEMS = 20;

const DEFAULT_RECORD_FORM = {
  patientName: "John Doe",
  dob: "1980-02-03",
  diagnosis: "Type 2 Diabetes",
  medications: "Metformin",
  allergies: "",
  lastVisit: "2026-02-14",
  notes: "Patient responding well to treatment."
};

function normalizeCsv(value) {
  return value
    .split(",")
    .map((item) => item.trim())
    .filter(Boolean);
}

function formatDate(value) {
  if (!value) return "N/A";
  const parsed = new Date(value);
  if (Number.isNaN(parsed.getTime())) return value;
  return parsed.toLocaleDateString();
}

export default function App({ endpoint }) {
  const { connection } = useConnection();
  const wallet = useWallet();
  const walletAddress = wallet.publicKey?.toBase58() || "";

  const [patientId, setPatientId] = useState("patient-001");
  const [viewersInput, setViewersInput] = useState("");
  const [recordForm, setRecordForm] = useState(DEFAULT_RECORD_FORM);
  const [lookupPatientId, setLookupPatientId] = useState("patient-001");
  const [status, setStatus] = useState("Ready");
  const [responseData, setResponseData] = useState(null);
  const [audits, setAudits] = useState([]);
  const [patientHistory, setPatientHistory] = useState(() => {
    try {
      const raw = localStorage.getItem(HISTORY_STORAGE_KEY);
      const parsed = raw ? JSON.parse(raw) : [];
      return Array.isArray(parsed) ? parsed : [];
    } catch {
      return [];
    }
  });

  const viewerWallets = useMemo(() => viewersInput.split(",").map((w) => w.trim()).filter(Boolean), [viewersInput]);

  function updateRecordField(field, value) {
    setRecordForm((prev) => ({ ...prev, [field]: value }));
  }

  async function storeRecord(e) {
    e.preventDefault();
    if (!wallet.connected || !wallet.publicKey) {
      setStatus("Connect your Solana wallet first.");
      return;
    }

    const parsed = {
      patientName: recordForm.patientName.trim(),
      dob: recordForm.dob,
      diagnosis: recordForm.diagnosis.trim(),
      medications: normalizeCsv(recordForm.medications),
      allergies: normalizeCsv(recordForm.allergies),
      lastVisit: recordForm.lastVisit,
      notes: recordForm.notes.trim()
    };

    const canonical = JSON.stringify(parsed);
    const recordHash = await sha256Hex(canonical);

    setStatus("Writing Solana proof then storing encrypted record off-chain...");
    let storeProofTx = null;
    try {
      storeProofTx = await postMemoProof({
        wallet,
        connection,
        payload: {
          event: "STORE_RECORD",
          patientId,
          recordHash,
          owner: walletAddress,
          viewers: viewerWallets,
          ts: new Date().toISOString()
        }
      });
    } catch (err) {
      setStatus(`Solana proof failed: ${err.message}`);
      return;
    }

    const res = await fetch(`${API_URL}/api/records/store`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({
        patientId,
        record: parsed,
        ownerPubkey: walletAddress,
        allowedViewers: viewerWallets,
        storeProofTx
      })
    });

    const data = await res.json();
    if (!res.ok) {
      setStatus(data.error || "Store request failed.");
      setResponseData(data);
      return;
    }

    setResponseData(data);
    setStatus(`Stored successfully. Hash: ${data.recordHash}`);
  }

  async function retrieveRecord(e) {
    e.preventDefault();
    if (!wallet.connected || !wallet.publicKey) {
      setStatus("Connect your Solana wallet first.");
      return;
    }

    setStatus("Retrieving and decrypting record...");
    const res = await fetch(`${API_URL}/api/records/${encodeURIComponent(lookupPatientId)}?requestor=${walletAddress}`);
    const data = await res.json();

    setResponseData(data);
    if (!res.ok) {
      setStatus(data.error || "Retrieve request failed.");
      return;
    }

    const historyItem = {
      patientId: lookupPatientId,
      recordHash: data.recordHash || null,
      record: data.record || null,
      timestamp: new Date().toISOString()
    };
    setPatientHistory((prev) => {
      const next = [historyItem, ...prev].slice(0, MAX_HISTORY_ITEMS);
      localStorage.setItem(HISTORY_STORAGE_KEY, JSON.stringify(next));
      return next;
    });

    try {
      const auditTx = await postMemoProof({
        wallet,
        connection,
        payload: {
          event: "ACCESS_RECORD",
          patientId: lookupPatientId,
          recordHash: data.recordHash,
          requestor: walletAddress,
          ts: new Date().toISOString()
        }
      });

      await fetch(`${API_URL}/api/records/${encodeURIComponent(lookupPatientId)}/audit`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
          actorPubkey: walletAddress,
          action: "READ",
          result: "SUCCESS",
          solanaTx: auditTx
        })
      });
    } catch (err) {
      setStatus(`Retrieved, but Solana access proof failed: ${err.message}`);
      return;
    }

    setStatus("Record retrieved. Access proof added to Solana.");
    await loadAudits(lookupPatientId);
  }

  function clearPatientHistory() {
    setPatientHistory([]);
    localStorage.removeItem(HISTORY_STORAGE_KEY);
  }

  async function loadAudits(targetPatientId = lookupPatientId) {
    const res = await fetch(`${API_URL}/api/records/${encodeURIComponent(targetPatientId)}/audits`);
    const data = await res.json();
    if (res.ok) {
      setAudits(data.audits || []);
    }
  }

  return (
    <div className="appShell">
      <div className="bgOrbs" aria-hidden="true">
        <span className="orb orbA" />
        <span className="orb orbB" />
        <span className="orb orbC" />
      </div>

      <header className="topNav">
        <div className="brandRow">
          <img src="/hws_logo.png" alt="HWS Logo" className="brandLogo" />
          <span className="brandDivider">|</span>
          <div className="brandText">
            <h1 className="productTitle">Health Web Services</h1>
            <span className="brandSub">Accessible Online Healthcare Records</span>
          </div>
        </div>
        <WalletMultiButton className="walletBtn" />
      </header>

      <section className="utilityBar">
        <article className="utilityCard">
          <strong>1. Connect Wallet</strong>
          <span>Use Solflare to sign Solana proof memos.</span>
        </article>
        <article className="utilityCard">
          <strong>2. Store Encrypted Record</strong>
          <span>Upload encrypted patient data and save hash proof.</span>
        </article>
        <article className="utilityCard">
          <strong>3. Retrieve + Verify</strong>
          <span>Check permissions, decrypt data, and log audit proof.</span>
        </article>
        <div className="rpcLine">RPC: {endpoint}</div>
      </section>

      <main className="workspace">
        <section className="panel panelStore">
          <div className="panelHead">
            <h2>Store Record</h2>
            <span>Encrypt + off-chain storage + proof anchor</span>
          </div>
          <form onSubmit={storeRecord}>
            <label>Patient ID</label>
            <input value={patientId} onChange={(e) => setPatientId(e.target.value)} required />

            <label>Allowed viewer wallets</label>
            <input
              value={viewersInput}
              onChange={(e) => setViewersInput(e.target.value)}
              placeholder="wallet1,wallet2"
            />

            <label>Medical Record Details</label>
            <div className="recordGrid">
              <div>
                <label>Patient Name</label>
                <input
                  value={recordForm.patientName}
                  onChange={(e) => updateRecordField("patientName", e.target.value)}
                  required
                />
              </div>
              <div>
                <label>Date of Birth</label>
                <input
                  type="date"
                  value={recordForm.dob}
                  onChange={(e) => updateRecordField("dob", e.target.value)}
                  required
                />
              </div>
              <div className="fieldFull">
                <label>Diagnosis</label>
                <input
                  value={recordForm.diagnosis}
                  onChange={(e) => updateRecordField("diagnosis", e.target.value)}
                  required
                />
              </div>
              <div className="fieldFull">
                <label>Medications (comma separated)</label>
                <input
                  value={recordForm.medications}
                  onChange={(e) => updateRecordField("medications", e.target.value)}
                  placeholder="Metformin, Insulin"
                />
              </div>
              <div className="fieldFull">
                <label>Allergies (comma separated)</label>
                <input
                  value={recordForm.allergies}
                  onChange={(e) => updateRecordField("allergies", e.target.value)}
                  placeholder="Penicillin, Peanuts"
                />
              </div>
              <div>
                <label>Last Visit</label>
                <input
                  type="date"
                  value={recordForm.lastVisit}
                  onChange={(e) => updateRecordField("lastVisit", e.target.value)}
                />
              </div>
              <div className="fieldFull">
                <label>Clinical Notes</label>
                <textarea
                  rows={4}
                  value={recordForm.notes}
                  onChange={(e) => updateRecordField("notes", e.target.value)}
                  placeholder="Summary notes for this visit"
                />
              </div>
            </div>

            <button type="submit">Encrypt + Upload</button>
          </form>
        </section>

        <section className="panel panelRetrieve">
          <div className="panelHead">
            <h2>Retrieve Record</h2>
            <span>Permission check + decrypt + verify</span>
          </div>
          <form onSubmit={retrieveRecord}>
            <label>Patient ID</label>
            <input value={lookupPatientId} onChange={(e) => setLookupPatientId(e.target.value)} required />
            <button type="submit">Verify Access + Decrypt</button>
          </form>

          <div className="status">{status}</div>
          <pre className="output">{responseData ? JSON.stringify(responseData, null, 2) : "No response yet."}</pre>
        </section>

        <section className="panel panelHistory">
          <div className="historyHeader">
            <div className="panelHead">
              <h2>Past Patient Info</h2>
              <span>Saved retrieval snapshots</span>
            </div>
            <button type="button" className="ghostBtn" onClick={clearPatientHistory}>Clear</button>
          </div>
          {patientHistory.length === 0 ? <p>No past patient data yet.</p> : null}
          <div className="historyList">
            {patientHistory.map((entry, idx) => (
              <article className="historyCard" key={`${entry.patientId}-${entry.timestamp}-${idx}`}>
                <strong>{entry.patientId}</strong>
                <span>Retrieved: {formatDate(entry.timestamp)}</span>
                <span>Hash: {entry.recordHash || "No hash"}</span>
                <div className="historyDetails">
                  <span><b>Name:</b> {entry.record?.patientName || "N/A"}</span>
                  <span><b>DOB:</b> {formatDate(entry.record?.dob)}</span>
                  <span><b>Diagnosis:</b> {entry.record?.diagnosis || "N/A"}</span>
                  <span><b>Last Visit:</b> {formatDate(entry.record?.lastVisit)}</span>
                  <span><b>Medications:</b> {Array.isArray(entry.record?.medications) && entry.record.medications.length > 0 ? entry.record.medications.join(", ") : "None listed"}</span>
                  <span><b>Allergies:</b> {Array.isArray(entry.record?.allergies) && entry.record.allergies.length > 0 ? entry.record.allergies.join(", ") : "None listed"}</span>
                  <span><b>Notes:</b> {entry.record?.notes || "No notes"}</span>
                </div>
              </article>
            ))}
          </div>
        </section>
      </main>

      <section className="panel panelAudit">
        <div className="auditHeader">
          <div className="panelHead">
            <h2>Audit Proofs</h2>
            <span>On-chain evidence trail</span>
          </div>
          <button type="button" onClick={() => loadAudits()}>Refresh Audit Log</button>
        </div>
        {audits.length === 0 ? <p>No audit entries loaded.</p> : null}
        <div className="auditGrid">
          {audits.map((audit, idx) => (
            <article className="audit" key={`${audit.solanaTx || "none"}-${idx}`}>
              <strong>{audit.action}</strong>
              <span>{audit.result}</span>
              <span>{audit.actorPubkey}</span>
              <span>{audit.solanaTx || "No tx"}</span>
              <span>{audit.timestamp}</span>
            </article>
          ))}
        </div>
      </section>
    </div>
  );
}
