import { useMemo, useState } from "react";
import { useConnection, useWallet } from "@solana/wallet-adapter-react";
import { WalletMultiButton } from "@solana/wallet-adapter-react-ui";
import { postMemoProof, sha256Hex } from "./solana";

const API_URL = import.meta.env.VITE_API_URL || "http://localhost:4000";

const EXAMPLE_JSON = `{
  "patientName": "John Doe",
  "dob": "1980-02-03",
  "diagnosis": "Type 2 Diabetes",
  "medications": ["Metformin"],
  "lastVisit": "2026-02-14"
}`;

export default function App({ endpoint }) {
  const { connection } = useConnection();
  const wallet = useWallet();
  const walletAddress = wallet.publicKey?.toBase58() || "";

  const [patientId, setPatientId] = useState("patient-001");
  const [viewersInput, setViewersInput] = useState("");
  const [recordJson, setRecordJson] = useState(EXAMPLE_JSON);
  const [lookupPatientId, setLookupPatientId] = useState("patient-001");
  const [status, setStatus] = useState("Ready");
  const [responseData, setResponseData] = useState(null);
  const [audits, setAudits] = useState([]);

  const viewerWallets = useMemo(() => viewersInput.split(",").map((w) => w.trim()).filter(Boolean), [viewersInput]);

  async function storeRecord(e) {
    e.preventDefault();
    if (!wallet.connected || !wallet.publicKey) {
      setStatus("Connect your Solana wallet first.");
      return;
    }

    let parsed;
    try {
      parsed = JSON.parse(recordJson);
    } catch {
      setStatus("Record JSON is invalid.");
      return;
    }

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
          <div className="brandText">
            <h1 className="productTitle">HWS</h1>
            <span className="brandSub">Health Web Services</span>
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

            <label>Record JSON</label>
            <textarea rows={11} value={recordJson} onChange={(e) => setRecordJson(e.target.value)} required />

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
