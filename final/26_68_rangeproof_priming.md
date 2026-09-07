# Mission 26 — ~68 identical rangeproofs and cache priming

**Date:** 2026-09-07  
**Goal:** Independently test Bitquery’s claim of ~68 identical 4166-byte Liquid rangeproofs (~14h before the incident) as the missing cache-priming link for `f24a` / block 4050336.  
**Method:** Full raw-tx scan of Liquid heights **4049384–4050335** (Blockstream API + failover), local parse of 4050336, byte-level comparison with P0/P1, OLD cache-key fingerprints, secp verification.

---

## Executive summary

```
CACHE VULNERABILITY CONFIRMED, INCIDENT CAUSALITY UNPROVEN

Bitquery’s ~68 copies: EXIST, but they are NOT the P0 used to build P1.
They do NOT share OLD cache key material with f24a:1.
They do NOT explain acceptance of f24a / 4050336.
```

| Finding | Verdict |
|---|---|
| ~68 identical 4166-byte proofs in 4049384–4050246 | **FACT** (exact count **68**) |
| Those 68 are byte-identical to each other | **FACT** (SHA256 `8cdcb808…cc1f`) |
| Those 68 equal our attack-related P0 (`0e018e2b…`) | **REFUTED** (differ at byte offset 2) |
| Same `(P,C)` across the 68 (Q0,CQ) | **FACT** |
| Q0 cryptographically verifies | **FACT** (lab) |
| Q0/CQ can prime *its own* OLD cache entry | **FACT** (structurally) |
| Q0/CQ equals / collides with P1/C1 key | **REFUTED** |
| Attack P0 (71c9/2711) appears 68× in that window | **REFUTED** (0 in window; 2 standalone at 4050335) |
| `P1 == P0 \|\| C0 \|\| GEN \|\| 6a \|\| 43` | **FACT** (byte-level) |
| That derivation ⇒ cache collision with P0 | **REFUTED** |
| Closes causal gap to 4050336 | **NO** |

**Overall classification:**  
`CACHE PRIMING VIA THE 68-FAMILY → f24a` = **REFUTED**  
`CACHE PRIMING VIA P0-FAMILY → f24a` = **REFUTED** (Mission 25, reconfirmed)  
`HISTORICAL (P1,C1) PRIMING` = still **UNPROVEN**  
`FIRST ACCEPTOR` = **UNKNOWN**

---

## 1. Two different 4166-byte families

Do **not** conflate them.

| Family | Proof SHA256 | Len | Count | Heights | Value commitment | Script | OLD key fingerprint `SHA256(P\|\|C)` |
|---|---|---:|---:|---|---|---|---|
| **Q0** (Bitquery window) | `8cdcb80895572fabbd4b93ce4aeda95c86be51d4e6cea22501d48d480917cc1f` | 4166 | **68** | 4049384→4050246 | `0879be667ef9…f81798` (**same ×68**) | `6a0100` | `e1722355382a47649206ff1a3c1c16e61979c841cef27b2144aca634dae1017e` |
| **P0** (71c9/2711) | `0e018e2b9345483472c95cfb7589bba4b55b8b072bf1a81ac38e4c7ae8e8509a` | 4166 | **2** standalone | **4050335 only** | `09d6c6150e95…e683f5` (=C0) | `6a43\|\|C1\|\|GEN\|\|6a` | `47b9fde7d8539c3544e4481c0edeaa66f4f1b85c17530dafb4d04f35b72d55f5` |
| **P1** (f24a:1) | `6619fa29ce0967ae93baefef0bc6233972b05249a9861a96695a106bbcc098b2` | **4234** | 1 | 4050336 | C1 `086f5d67…1d01d8` | `6a` | `86bc93c9c3eb514e71c5748e2318fe6ca8a41d1254f9e9258edeb69f158c37cc` |

LCP(Q0,P0)=**2** bytes only (`4033` then diverge). Both start with `4033…` — easy to confuse in a length-only search.

Artifacts:

- [`final/data/68_rangeproof_occurrences.json`](data/68_rangeproof_occurrences.json)  
- [`final/data/68_rangeproof_cache_keys.json`](data/68_rangeproof_cache_keys.json)  
- [`final/data/p0_p1_byte_comparison.json`](data/p0_p1_byte_comparison.json)  
- [`final/data/cache_priming_candidates.json`](data/cache_priming_candidates.json)  
- [`final/logs/68_rangeproof_reproduction.log`](logs/68_rangeproof_reproduction.log)

Scan: **953** heights, **3031** txs cached under `final/data/scan_cache/` (working cache; gitignored).

---

## 2. Bitquery claim audit

| Claim | Rating | Evidence |
|---|---|---|
| ~68 copies of a 4166-byte rangeproof | **FACT** (exactly 68) | full-window scan |
| Spread ~14h before attack | **STRONGLY SUPPORTED** | first ts `2026-09-05 22:01:10Z` → f24a `2026-09-06 13:53:10Z` (~15.9h); last Q0 `12:23:10Z` same day |
| Heights ~4049384–4050246 | **FACT** | matches |
| These primed caches for the attack tx | **REFUTED** as f24a `(P1,C1)` primers | wrong proof + wrong commitment + wrong key |
| “crafted cache-key collision” (generic) | **FACT** lab / **UNPROVEN** for incident | |
| “valid tx first” (Q0 family) | **FACT** they precede f24a; **REFUTED** as f24a primers | |
| “nodes with primed caches accepted / unprimed rejected” | **UNPROVEN** | no dual-node logs |
| “network split because of cache state” | **UNPROVEN** | |

---

## 3. Q0 family detail (the actual 68)

Parsed with `liquidjs-lib` for all 68 txs:

- **proof:** identical Q0 (4166 B)  
- **value commitment CQ:** identical `0879be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798`  
- **asset:** explicit L-BTC `016d521c38…026f`  
- **script:** `6a0100` (identical)  
- **nonce:** `00`  
- **out_index:** 1 in sampled/decoded set  

So they reuse exact `(Q0, CQ, G_L, S=6a0100)` — a real repeated dry-run / spam pattern.

**Lab:** `secp256k1_rangeproof_verify(Q0,CQ,G_L,6a0100) = TRUE`  
⇒ mempool validation **could** `Set(OLD_KEY(Q0\|\|CQ))`.

That entry is **useless** for f24a:1:

`OLD_KEY(Q0||CQ) ≠ OLD_KEY(P1||C1)`  
(fingerprints `e1722355…` vs `86bc93c9…`).

Neither C0 nor C1 nor P0 nor P1 appears in any of the 68 Q0 txs.

---

## 4. P0 family (71c9 / 2711) — not the 68

Standalone P0 fields: **only 2** (both height **4050335**, ~60s before f24a).  
Embedded: P0 is also the **prefix** of f24a’s 4234-byte P1 (see §5).

Mission 25 scenarios A–F: after storing P0/C0 keys, **f24a:1 still REJECT**. Reconfirmed.

---

## 5. P0 ↔ P1 byte relationship (**CONFIRMED**)

Recalculated (do not take public blogs on faith):

```
len(P0)=4166
len(P1)=4234
P1[0:4166] == P0          → TRUE
tail = P1[4166:]          → 68 bytes
tail[0:33] == C0          → TRUE
tail[33:66] == GEN        → TRUE  (generator ser. from 71c9 OP_RETURN)
tail[66] == 0x6a
tail[67] == 0x43
```

**Formula (FACT):**

`P1 == P0 || C0 || GEN || 0x6a || 0x43`

**71c9/2711 OP_RETURN (FACT):**

`script = 0x6a || 0x43 || C1 || GEN || 0x6a`

So C1 is embedded as a **script data push**, not as the value-commitment argument to `VerifyRangeProof`.

| Interpretation of C1-in-blob | Status |
|---|---|
| Cache priming of `(P1,C1)` | **REFUTED** |
| Marks / links construction to later C1 | **PLAUSIBLE** (structural) |
| Generator GEN identified | **FACT** (33-byte `0a…` ser.) |
| Proving nonce / challenge reuse proven | **UNPROVEN** |
| Semantic role beyond structure | **UNKNOWN** |

**Critical separation:**

- **Byte-level derivation** of P1 from P0: **CONFIRMED**  
- **Cache-key collision** P0/C0 vs P1/C1: **REFUTED** (`47b9fde7…` ≠ `86bc93c9…`)

Appending bytes changes the proof argument; OLD key hashes the **entire** proof.

---

## 6. OLD cache key algorithm (reminder)

Pre-fix Elements:

```text
entry = salted_SHA256( proof || value_commitment )
# asset + script omitted
```

Salt is **per-process random** → absolute salted entries not recoverable from chain.  
**Equality of `(proof, commitment)`** across txs is necessary and sufficient for collision **within one process**.

No SHA256 collision between distinct `(P,C)` pairs is claimed or observed (would be cryptographically absurd at this size).

---

## 7. Reproduction

Log: `final/logs/68_rangeproof_reproduction.log`  
Script: `final/scripts/q0_f24a_prime_test.c` (+ Mission 25 harness)

| Scenario | Result |
|---|---|
| Empty → f24a:1 | REJECT (crypto 0) |
| Store Q0/CQ → f24a:1 | keys differ → MISS → REJECT |
| Store P0/C0 (71c9/2711) → f24a:1 | keys differ → MISS → REJECT |
| Artificial A→B same `(P,C)` different context | ACCEPT on OLD key (vuln lab) — **not** these families vs f24a |

Node A/B/C fork narrative from cache priming of f24a: **not demonstrated** by Q0 or P0 families.

---

## 8. Timeline

| When (UTC) | Event | Class |
|---|---|---|
| 2026-09-05 22:01:10 | First Q0 tx @ 4049384 | ON-CHAIN FACT |
| … | 68× `(Q0,CQ)` through 4050246 | ON-CHAIN FACT |
| 2026-09-06 12:23:10 | Last Q0 @ 4050246 | ON-CHAIN FACT |
| 2026-09-06 13:52:10 | 4050335: 2711 then 71c9 (P0/C0) | ON-CHAIN FACT |
| 2026-09-06 13:53:10 | 4050336: f24a (P1=P0\|\|…) | ON-CHAIN FACT |
| later | 4050344 / 4050349 peg-outs | ON-CHAIN FACT |
| Aug 3 / Sep 1–6 | cache-key fix authored / merged / cherry-picks | CODE FACT |

Whitehat / “bridge patched” messages: **PUBLIC REPORTING** (not re-verified here).

---

## 9. Does this close the causal chain?

```
[A] cache vuln                         CONFIRMED
[B] lab exploit                        CONFIRMED
[C] 68× Q0 primed f24a key             REFUTED
[C'] P0 primed f24a key                 REFUTED
[D] some historical (P1,C1) prime      UNPROVEN
[E] f24a accepted via cache            UNPROVEN
[F] ConnectBlock(4050336) via [E]      UNPROVEN
```

**First broken arrow for the Bitquery-68 story: C.**

P1’s construction from P0 is an important **attack-craft** clue, not a cache-hit proof.

---

## 10. Final verdict table

| Proposition | Verdict | Proof |
|---|---|---|
| 68 copies exist | **YES** | scan count=68 |
| They are byte-identical | **YES** | single SHA256 `8cdcb808…` |
| Same P0/C0 as 71c9 | **NO** | different proof+commit |
| Same `(Q0,CQ)` across 68 | **YES** | liquidjs parse |
| They validate cryptographically | **YES** | secp verify=1 |
| They can primer *their* cache key | **YES** | store=true path |
| Same cache key as P1/C1 | **NO** | fingerprints differ |
| P1 derives from P0 (bytes) | **YES** | `P1[:4166]==P0` + tail parse |
| They explain f24a acceptance | **NO** | |
| They explain 4050336 | **NO** | |
| They prove consensus split | **NO** | |

---

## 11. What would still close the gap

1. On-chain or mempool artifact with **exact** `(P1,C1)` verified under `store=true` before ConnectBlock(4050336).  
2. Or a non-cache acceptance path with version/config evidence.  
3. Dual-node ACCEPT/REJECT logs for block `e1d9a2aa…`.

---

## 12. Status line

```
CACHE VULNERABILITY: CONFIRMED
68-RANGEPROOF FAMILY: CONFIRMED AS ON-CHAIN PATTERN
68-FAMILY → f24a CACHE PRIMING: REFUTED
P0 → P1 BYTE DERIVATION: CONFIRMED
P0 → f24a CACHE PRIMING: REFUTED
HISTORICAL (P1,C1) PRIMING: UNPROVEN
4050336 ACCEPTANCE MECHANISM: UNKNOWN
FIRST ACCEPTOR: UNKNOWN
ROOT CAUSE: UNRESOLVED
```

**No causal narrative from the 68 rangeproofs to f24a acceptance is justified.**
