# 02 — Transaction f24a parsing

## Source

Raw hex fetched from Blockstream Liquid API (no local “fichiers fournis” were present).  
File: `final/hex/f24a4b179b5cc7e88b25a763911f7cbdf2bf45d1d1b5ab611e94461cef0a183f.hex`  
SHA256(file): `b1b97f3be9e089befc2c0dfd6fc95fcf91aefcf5ed22abb436122c99422694f6`

## FACT — Header / structure

| Field | Value |
|---|---|
| txid | `f24a4b179b5cc7e88b25a763911f7cbdf2bf45d1d1b5ab611e94461cef0a183f` |
| version | 2 |
| locktime | 0 |
| vin | 1 |
| vout | 4 |
| size | 13151 bytes |
| fee | 58 sats (explicit fee output) |
| status | confirmed height **4050336**, block `e1d9a2aae69e0fc3ca18f7f7f84e0615e92a5e3b5000d66c10c34043346da0d5` |

Parsed with `liquidjs-lib` (Elements serialization, not Bitcoin Core).

## FACT — Outputs

### out0
- asset explicit L-BTC wire `016d521c38…026f`
- value commitment `08360f95ce0a63e76daab6a05616103ba7462a9af7db0697e9c8fa6a65aba103f8`
- script P2WPKH `0014f5906a8572cf826a9d737ce1bc6a19a066868630`
- rangeproof 4174 B, SHA256 `3fe0fb01…d51321`
- surjection empty

### out1 — CRITICAL
- asset explicit L-BTC (display id `6f0279e9…526d`)
- **C1** value commitment: `086f5d67160fc4b477954fb09ef321e5b589d7a07740a1a6df494ed2335b1d01d8`
- nonce `00`
- scriptPubKey `6a` (OP_RETURN)
- **P1** rangeproof length **4234**
- **SHA256(P1) = `6619fa29ce0967ae93baefef0bc6233972b05249a9861a96695a106bbcc098b2`**
- surjection empty (required for explicit asset)
- artifacts: `final/hex/out1_*.hex`

### out2
- blinded asset + value; rangeproof 4174 B; surjection 67 B

### out3 (fee)
- explicit value 58; empty script; no proofs

## FACT — Note on truncated commitment form

User string `…5b1d8` (missing nibbles) does **not** appear on the wire. Full 33-byte commitment ends with `…5b1d01d8`.
