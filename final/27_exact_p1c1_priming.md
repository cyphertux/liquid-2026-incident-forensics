# Mission 27 — Exact historical priming of `(P1, C1)`

**Date:** 2026-09-07  
**Question:** Did any Liquid runtime ever successfully verify exact `(P1, C1)` (enabling `cache.Set`) before / during acceptance of `f24a` in block **4050336**?  
**Stance:** Seek falsification. Do not equate lab incomplete-key vulnerability with incident causation.

---

## Executive summary

```
CACHE MECHANISM REFUTED AS INCIDENT CAUSE
(for the honest Verify→Set priming of exact P1/C1)

CACHE VULNERABILITY (incomplete key) REMAINS CONFIRMED AS A LAB FACT
4050336 ACCEPTANCE MECHANISM: STILL UNKNOWN
FIRST ACCEPTOR: UNKNOWN
```

**Why the classic priming story fails for f24a:**

1. Full **P1** (4234 bytes) is **not** a verifying rangeproof under the real context, nor under related dry-run contexts, nor when paired with the commitment that *does* verify the P0 prefix.  
2. Elements inserts into the rangeproof cache **only after** `secp256k1_rangeproof_verify` succeeds.  
3. Therefore a historical honest path cannot create the cache entry for exact `(P1, C1)`.  
4. On-chain / local search finds **P1 only inside f24a**; **C1 as value commitment only inside f24a** (elsewhere: OP_RETURN embed only).  
5. No mempool/debug artifact of a priming tx was found.

Artificial lab `Set(key)` can still make OLD-key f24a appear to “hit” — that is the vulnerability property, **not** a demonstrated Elements insertion sequence for this `(P1,C1)`.

---

## Answers A–F

| # | Question | Answer |
|---|---|---|
| **A** | Was P1/C1 ever found in a valid priming transaction? | **NO** (not in scanned chain data; only f24a carries both as RP field + value commitment) |
| **B** | Was P1/C1 found in mempool-only data? | **NO ARTIFACT** (no production mempool/debug logs available) |
| **C** | Can P1/C1 produce a cache hit via honest Verify→Set? | **NO** — verify fails; Set unreachable |
| **D** | Can an artificial cache entry make f24a pass (OLD key)? | **YES (lab)** — not historical proof |
| **E** | Does that explain 4050336? | **NO** for honest `(P1,C1)` priming |
| **F** | Is the cache vulnerability definitely causal for the incident? | **NO** |

**Final verdict label:** `CACHE MECHANISM REFUTED AS INCIDENT CAUSE`  
**Nuance:** Incomplete-key bug remains real; it is **not** shown to be the mechanism that admitted f24a/4050336.

---

## 1. Exact search for P1

Fingerprint: `SHA256(P1)=6619fa29ce0967ae93baefef0bc6233972b05249a9861a96695a106bbcc098b2`  
Length: **4234**.

| Source | Full P1 present? |
|---|---|
| Local hex extracts / f24a tx hex | **YES** (same tx) |
| `final/data/scan_cache` (3031 txs, heights 4049384–4050335) | **NO** full P1; **0** outs with `rp_len=4234` |
| Prefix-only hits | P0 family (expected: `P1[:4166]==P0`) |
| Mempool datasets in repo | **NONE** containing P1 |

Artifact: [`final/data/p1_c1_occurrence_search.json`](data/p1_c1_occurrence_search.json)

**FACT:** Within the scanned pre-incident window, the only full 4234-byte P1 is **f24a itself** (height 4050336).

---

## 2. Exact search for C1

`C1=086f5d67160fc4b477954fb09ef321e5b589d7a07740a1a6df494ed2335b1d01d8`

| Occurrence class | Where |
|---|---|
| **VALUE COMMITMENT** | **Only** `f24a` vout1 (in available parses) |
| **SCRIPT / OP_RETURN embed only** | `71c93d43…`, `27114710…` out0 |
| In 68× Q0 family txs | **Absent** |

**FACT:** Script embedding ≠ `VerifyRangeProof` commitment argument (Mission 25).

---

## 3. Cache key / salt limitation

Pre-fix:

```text
entry = salted_SHA256( proof || value_commitment )
```

- Salt = per-process random (`m_salted_hasher_range_proof`) → **historically unknowable** from chain data.  
- What *can* be proven: equality of `(proof, commitment)` ⇒ same entry **within one process**.  
- Unsalted fingerprint `SHA256(P1||C1)=86bc93c9…` is a **lab surrogate for equality tests only**.

**Do not claim a historical cache hit from an unsalted hash appearing in a file.**

No artifact in the repo contains salted cache dump entries.

---

## 4. Mempool-only search (priority)

| Artifact | Result |
|---|---|
| `data/f24a.mempool.json` | Text: `Transaction not found` — not a primer |
| `debug.log` / functionary logs / `testmempoolaccept` traces | **Not present** in repository |
| Public archived Liquid mempool dumps | **Not available** to this investigation |
| Window 4050335→4050336 (~60s) | Only on-chain P0 dry-runs + f24a; no separate P1 carrier |

**LIMITATION (FACT):** Absence of evidence in *our* corpus ≠ proof no private mempool message existed.  
**Combined with §5–7:** even a private T_prime carrying exact `(P1,C1)` would **fail VerifyAmounts** and therefore would **not** `Set` under stock Elements.

---

## 5. Required valid context `(G', S')`

Sought: `Verify(P1, C1, G', S') = TRUE` while real L-BTC+`OP_RETURN` is FALSE.

### New Mission 27 harness results

Log: [`final/logs/p1_context_search_m27.log`](logs/p1_context_search_m27.log)

| Test | Result |
|---|---|
| P1+C1+L+OP_RETURN | **0** |
| P1+C1+L+71c9 script | **0** |
| P1+C1+L+empty / OP_TRUE / `6a0100` | **0** |
| P1+C1+GEN_as_asset+6a | **0** |
| P1+C0+71c9 script (plen=4234) | **0** |
| P1[:4166](=P0)+C0+71c9 script | **1** |
| P0+C0+71c9 script | **1** |
| P0+C1+… | **0** |
| Prior batch (128 contexts, Mission 04) | **0 hits** |

**Structural point (FACT):** Appending the 68-byte tail breaks verification even for the commitment/script that validate the P0 prefix. Stock `VerifyRangeProof` uses `vchRangeProof.size()` for **both** crypto and cache key — no “verify prefix / hash full” split in Elements.

**INFERENCE (strong):** Exact `(P1,C1)` has **no** honest verifying context compatible with the bulletproof verifier used by Elements; therefore **honest cache insertion of that pair is not achievable**.

(Exhaustive proof over all `(G,S)` is impossible; failure of all related contexts + malformed-length behavior is treated as decisive for incident analysis.)

---

## 6. Semantics of `P1 = P0 || C0 || GEN || 0x6a || 0x43`

Confirmed again; details in [`final/data/p1_tail_semantics.json`](data/p1_tail_semantics.json).

| Piece | Meaning |
|---|---|
| **P0** | Valid RP for `(C0, L-BTC, 71c9 OP_RETURN script)` |
| **C0** | That output’s value commitment |
| **GEN** | 33-byte generator serialization (`0a…`) also inside 71c9 OP_RETURN after C1 |
| **0x6a / 0x43** | Match OP_RETURN / push framing bytes from that script template |

**semantic role UNKNOWN** beyond: *concatenation of prior dry-run material into an invalid longer “proof” field.*  
Not shown to be a Fiat–Shamir message, valid `extra_commit`, or cache primer.

---

## 7. Reconstruct T_prime

Required T_prime:

```text
vout.rangeproof == P1 (4234 B exact)
vout.value_commitment == C1
VerifyRangeProof → TRUE → Set(OLD_KEY(P1||C1))
```

| Requirement | Status |
|---|---|
| Exact bytes | Only appear together on f24a itself |
| Verify TRUE | **Fails** in all tests |
| Mempool insert | **Would fail** stock validation |
| Lab reproduction of honest path | **NON REPRODUIT / REFUTED** |

Artificial reproduction (manual cache insert of key then present f24a) demonstrates vulnerability class only.

---

## 8. Version differences

| Version / path | Behavior relevant to f24a |
|---|---|
| Pre-`c26d719` (OLD key) | Incomplete key; **still rejects** f24a on empty cache / failed verify |
| Post-`c26d719` (NEW key) | Key includes asset+script; f24a still rejects on crypto fail |
| Liquid default `assumevalid` | Empty in tree studied → does not alone skip checks |
| Production functionary binaries | **UNKNOWN** |

Patch dates (Mission 01): author **2026-08-03**; merges early **Sep 2026** — deployment on acceptors **UNKNOWN**.

---

## 9. Acceptance matrix (harness + source; not full `elementsd` ConnectBlock)

From [`final/data/m27_acceptance_matrix.json`](data/m27_acceptance_matrix.json):

| Node | Cache | Version | f24a:1 | 4050336 (expected stock path) |
|---|---|---|---|---|
| A | empty | vulnerable | **REJECT** | **REJECT** |
| B | artificial `Set(P1\|\|C1)` | vulnerable | **ACCEPT (lab)** | ACCEPT *if* that were the only failure — **Set not historically obtainable via Verify** |
| C | empty | patched | **REJECT** | **REJECT** |
| D | artificial old entry | patched | **REJECT** (key mismatch → crypto fail) | **REJECT** |
| E | any | `fScriptChecks=false` | skipped | **ACCEPT possible in code** — **prod UNPROVEN** |

Full `elementsd` ConnectBlock of 4050336: still **not completed** in this repo (Mission 06).

---

## 10. First acceptor

No new runtime identity evidence (logs, versions, peer traces).

```
FIRST ACCEPTOR = UNKNOWN
FIRST ACCEPTANCE MECHANISM = UNKNOWN
```

Signed (11/15) ≠ ConnectBlock success (Missions 18–21).

---

## 11. Alternative hypotheses (if not `(P1,C1)` cache)

| Hypothesis | Status |
|---|---|
| Honest `(P1,C1)` cache priming | **REFUTED** as viable stock path |
| `fScriptChecks=false` / non-default assumevalid | **CODE-POSSIBLE**, **historically UNPROVEN** |
| Non-stock / modified validation binary | **UNPROVEN** |
| `combineblocksigs` without local TestBlockValidity then receivers with broken checks | **PARTIAL** (assembly without amounts check is real; receiver ConnectBlock still required for tip) |
| Explorer-only artifact | **REFUTED** for chain continuity through peg-outs (Mission 22) |
| Different cache than `CachingRangeProofChecker` | **No alternate amount cache found** in prior source reads |

---

## 12. Epistemic rule compliance

We do **not** describe the incident as a “cache-key collision” between distinct verifying `(P,C)` pairs for f24a.  

Established vulnerability:

```text
cache key omitted verification context
```

Incident still requires, for *this* story:

```text
exact cached (P1,C1) via successful verify
+ historical cache state
+ f24a acceptance
```

**First link fails:** successful verify of `(P1,C1)` is not available.

---

## 13. Updated confidence

| Item | Score | Note |
|---|---:|---|
| Incomplete-key vulnerability | **95** | unchanged |
| Lab A→B exploit for *verifying* pairs | **90** | unchanged |
| Honest historical `(P1,C1)` priming | **0–5** | **refuted as stock path** |
| Cache as cause of f24a/4050336 | **5–10** | **refuted for priming narrative** |
| Alternate skip-VerifyAmounts path | **25** | open |
| Root cause known | **10** | unresolved |
| First acceptor known | **0** | unknown |

---

## 14. Final statement

```
CACHE MECHANISM REFUTED AS INCIDENT CAUSE

P1 is byte-derived from P0 but is not a verifying proof at full length.
Stock Elements cannot cache-insert (P1,C1) because verify never succeeds.
No pre-f24a carrier of exact (P1,C1) was found on-chain in the scanned window.
No mempool priming artifact was found.

The rangeproof-cache incompleteness remains a confirmed vulnerability.
It does not, on present evidence, explain acceptance of f24a or block 4050336.

FIRST ACCEPTOR: UNKNOWN
FIRST ACCEPTANCE MECHANISM: UNKNOWN
ROOT CAUSE: UNRESOLVED
```

**Open research priority shifts to:** non-cache acceptance paths (`fScriptChecks` / deployment configs / non-stock binaries) and acquisition of functionary or peer logs for `ConnectBlock(4050336)`.
