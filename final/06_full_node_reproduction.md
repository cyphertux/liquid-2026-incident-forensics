# 06 — Full node reproduction

## Attempted

- Build Elements from source: **blocked** (no `cmake`/`autoconf`/`libevent`/`boost` without root; no apt install rights)
- Docker `elementsproject/elementsd`: **blocked** (`docker.sock` unavailable)
- Prebuilt node binary: not available in environment

## What was possible instead (FACT)

In-process Elements-faithful validation path with real `libsecp256k1` rangeproof module (see 05).  
This is **not** a full `elementsd` consensus/mempool/block pipeline.

## Experiments requested vs status

| Experiment | Status |
|---|---|
| Vuln node, empty cache, submit f24a → REJECT | **PARTIAL**: clean Elements-flow verify of out1 rangeproof → REJECT. Full tx/block accept path **NON REPRODUIT** |
| Vuln node, prime, submit f24a → ACCEPT | **NON REPRODUIT** (no context A for P1/C1; no full node) |
| Patched node, same prime, f24a → REJECT | **NON REPRODUIT** as full node; artificial NEW cache rejects B after A |
| Restart clears cache | **INFERENCE from code**: `rangeProofCache` is process-static RAM |

## Cache locus (FACT from source)

- RAM cuckoo cache, process lifetime
- Not written to chainstate DB
- Shared across mempool + block validation within one process when `store=true`
