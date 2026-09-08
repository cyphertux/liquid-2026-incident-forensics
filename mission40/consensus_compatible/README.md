# CONSENSUS-COMPATIBLE RECONSTRUCTION (not produced)

**Do not confuse with `mission40/exact/` or top-level EXACT hexes.**

## Why EXACT Liquid txs cannot be ConnectBlock'd on regtest

| Blocker | Detail |
|---|---|
| Chain params | `decoderawtransaction` of 71c9/f24a **fails on regtest** with elements-23.3.3; succeeds on `liquidv1` |
| Prevouts | Spends Liquid UTXOs absent from regtest UTXO set |
| Asset genesis | Pegged asset / generators differ |
| Federation / signblock | Liquid dynafed ≠ regtest signed blocks |

## What a reconstruction would need

1. Fund regtest confidential UTXOs with known blinds.
2. Build two txs whose **rangeproof locus fields** equal Mission 28 `(P0,C0,G,S0)` / `(P1,C1,G,S1)` bit-exactly.
3. Keep Pedersen balance + surjection proofs valid for those commitments — generally requires regenerating companion witnesses (surjection, etc.), which **loses bit-identity** with historical 71c9/f24a outside the locus fields.
4. Mine with Elements `getnewblockhex` / `signblock` / `submitblock` on post-fix `elementsd`.

## Properties preserved vs lost

| Preserved if locus fields copied exactly | Lost |
|---|---|
| Unframed cache preimage identity | Historical txid/wtxid |
| Primer crypto TRUE / alias FALSE at rangeproof | Exact surjection / other witnesses |
| Cache HIT mechanism at checker | Historical block hashes / 4050336 causality |

## Status

`CONSENSUS_COMPATIBLE_ARTIFACTS = NOT_PRODUCED`
`BLOCKED_BY`: need known blinds + post-fix elementsd build + confidential tx builder
