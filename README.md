# Liquid Network — 2026 Incident Forensics

> **Status:** Ongoing investigation — **root cause unresolved**  
> **FIRST ACCEPTOR:** UNKNOWN  
> **FIRST ACCEPTANCE MECHANISM:** UNKNOWN  
> **Repository intent:** Independent, falsifiable technical forensics for the Liquid Network incident of **2026-09-06**  
> **Scope boundary:** This repository establishes reproducible facts. It does **not** claim a root cause.

---

## Executive Summary

On **2026-09-06**, the Liquid Network experienced a major incident involving the withdrawal of on the order of **~4,000 BTC** from federation reserves via peg-outs (public reporting). Independently, this repository reconstructs **three** Liquid peg-outs in blocks 4050344–4050349:

| Peg-out | Block | Amount (BTC) |
|---|---:|---:|
| `46f117…` | 4050344 | **2.65138358** |
| `ce4…` | 4050349 | **3996.01834922** |
| `731f…` | 4050349 | **3.99601658** |

**A + B = 3998.66973280 BTC** (do not fold C into that sum). All three amounts+addresses structurally match outputs of Bitcoin federation payout `8db751a650…b140` ([`final/22_full_onchain_flow.md`](final/22_full_onchain_flow.md)).

This investigation focuses on a confidential transaction

`f24a4b179b5cc7e88b25a763911f7cbdf2bf45d1d1b5ab611e94461cef0a183f`

included in Liquid block **4050336**

(`e1d9a2aae69e0fc3ca18f7f7f84e0615e92a5e3b5000d66c10c34043346da0d5`).

Independent verification shows that **one rangeproof in that transaction fails** under the on-chain L-BTC + `OP_RETURN` context tested here. Along the stock Elements validation path reconstructed in this repository, that failure would cause rejection when the relevant rangeproof cache is empty.

Separately, a **historical Elements rangeproof-cache key incompleteness** was confirmed in source and **reproduced as an exploitable laboratory condition** (artificial priming). **This investigation has not established that that cache vulnerability was used during the incident.**

Block 4050336 carries an **11-of-15** federation signblock quorum. Block **4050337** cryptographically references 4050336 as its parent. Parent continuity is confirmed through **4050349**. UTXO relationships from `f24a` toward the peg-outs are documented. **The first runtime that accepted 4050336 into an active chainstate remains unidentified.**

**Accordingly, the root cause remains unresolved.**

**No causal narrative is justified beyond this point.**

---

## Investigation Status

| Item | Status |
|---|---|
| Root cause identified | **No** |
| First accepting node / runtime identified | **UNKNOWN** |
| Historical rangeproof-cache hit for `(P1,C1)` demonstrated | **No** |
| Consensus split demonstrated | **No** |
| Functionary key compromise demonstrated | **No** |
| Laboratory reproductions of key crypto/cache facts | **Yes** (see below) |

Evidence labels used throughout:

| Label | Meaning |
|---|---|
| **FACT** | Directly supported by on-chain data, source code, or executed reproduction |
| **INFERENCE** | Reasonable conclusion from facts; not independently proven |
| **HYPOTHESIS** | Candidate explanation under test |
| **UNKNOWN** | Not established by available evidence |
| **REFUTED** | Affirmatively contradicted by available evidence |

---

## What Is Confirmed

### A. On-chain facts

- Transaction `f24a…183f` is confirmed in Liquid block **4050336**  
  ([`final/02_transaction_f24.md`](final/02_transaction_f24.md), [`final/hex/f24a4b179b5cc7e88b25a763911f7cbdf2bf45d1d1b5ab611e94461cef0a183f.hex`](final/hex/f24a4b179b5cc7e88b25a763911f7cbdf2bf45d1d1b5ab611e94461cef0a183f.hex)).
- Block hash: `e1d9a2aae69e0fc3ca18f7f7f84e0615e92a5e3b5000d66c10c34043346da0d5`  
  ([`final/data/block_4050336.json`](final/data/block_4050336.json)).
- Previous block: `aad24e4fb64ca8adf4961667da87820cd48e553957ac64e75de7cdb298b5d66b` (height 4050335).
- Block timestamp (API `timestamp`): `1788702790` → **2026-09-06 13:53:10 UTC** ([`final/09_timeline.md`](final/09_timeline.md), [`final/data/chain_4050335_4050349.json`](final/data/chain_4050335_4050349.json)).
- Peg-outs A/B/C and Bitcoin payout correspondence ([`final/22_full_onchain_flow.md`](final/22_full_onchain_flow.md), [`final/data/pegout_flow.json`](final/data/pegout_flow.json)).

### B. Cryptographic facts

- Output **1** of `f24a` (commitment **C1**, rangeproof **P1**, script `OP_RETURN` / `6a`, explicit L-BTC asset) fails `secp256k1_rangeproof_verify` in the tested real context  
  ([`final/03_crypto_validation.md`](final/03_crypto_validation.md), [`final/logs/verify_rangeproof_out.log`](final/logs/verify_rangeproof_out.log)).
- Sibling confidential outputs **out0** and **out2** verify **TRUE** under their real contexts (sanity checks).
- `SHA256(P1) = 6619fa29ce0967ae93baefef0bc6233972b05249a9861a96695a106bbcc098b2`  
  ([`final/data/critical_hex_checksums.txt`](final/data/critical_hex_checksums.txt)).
- Pedersen tally over published commitments: **TRUE with C1**, **FALSE without C1**  
  ([`final/13_pedersen_balance_f24a.md`](final/13_pedersen_balance_f24a.md), [`final/logs/pedersen_tally_f24a.log`](final/logs/pedersen_tally_f24a.log)).
- Cleartext / blinding opening of C1: **not recovered**; numeric inflation amount: **not demonstrated** ([`final/14_c1_constraints_no_blind.md`](final/14_c1_constraints_no_blind.md)).

### C. Validation facts

- Stock path (Elements source): `ConnectBlock` → `CheckTxInputs` → `if (fScriptChecks) VerifyAmounts` → `CRangeCheck` → `secp256k1_rangeproof_verify`  
  ([`final/15_validation_divergence.md`](final/15_validation_divergence.md), [`final/17_block_lifecycle_4050336.md`](final/17_block_lifecycle_4050336.md)).
- With empty rangeproof cache, Elements-faithful verification of f24a out1 returns reject (`ret=0`) for both old and new cache keying  
  ([`final/05_cache_reproduction.md`](final/05_cache_reproduction.md), [`final/logs/elements_cache_harness_run.log`](final/logs/elements_cache_harness_run.log)).
- Liquid v1 default `assumevalid` is empty in the Elements tree inspected here; it does **not**, by default, explain tip acceptance ([`final/18_environment_4050336.md`](final/18_environment_4050336.md), [`final/21_first_runtime_acceptor.md`](final/21_first_runtime_acceptor.md)).
- `fScriptChecks=false` is reachable via the assumevalid gate in `ConnectBlock`; **production use of that path is not demonstrated** ([`final/21_first_runtime_acceptor.md`](final/21_first_runtime_acceptor.md)).

### D. Federation signature facts

- Signblock challenge is 11-of-15 P2WSH CHECKMULTISIG ([`final/data/block_4050336_fed_pubkeys.txt`](final/data/block_4050336_fed_pubkeys.txt)).
- Individual ECDSA matching of witness DER signatures to federation pubkeys (message = block hash, little-endian internal order, `SIGHASH_ALL`) was executed for heights 4050335–4050343  
  ([`final/data/signblock_pubkey_matches.json`](final/data/signblock_pubkey_matches.json), [`final/20_validation_path_ancestor.md`](final/20_validation_path_ancestor.md)).

### E. Chain reconstruction facts

- Independently checked: `4050337.prev == hash(4050336)`, and continuous parent links through **4050349**  
  ([`final/data/chain_4050335_4050349.json`](final/data/chain_4050335_4050349.json), [`final/22_full_onchain_flow.md`](final/22_full_onchain_flow.md)).
- Confirmed UTXO relationships (Mission 22):
  - `f24a:2 → c6ea:0 → 46f117 vin11 → 46f117:1 → ce4 vin4`
  - `f24a:0 → 3875… → c6ea… → 46f117…`
  - `c6ea:1 → 3289… → ce4…`
  - `f24a:1` **UNSPENT**
- Three Liquid peg-outs in 4050336–4050349 only; exhaustive scan found no others ([`final/data/pegout_flow.json`](final/data/pegout_flow.json)).

### F. Reproduction facts

- Artificial old-cache A→B false positive: **reproduced**  
- Patched (new) key A→B: B rejected: **reproduced**  
- Historical priming of exact `(P1,C1)`: **not found** in scanned material ([`final/04_context_search.md`](final/04_context_search.md), [`final/12_structural_falsification.md`](final/12_structural_falsification.md)).

---

## What Was Reproduced

Distinguish carefully: **laboratory reproduction** ≠ **observed production behavior**.

### 1. Independent rangeproof verification (f24a out1)

| | |
|---|---|
| **Input** | `final/hex/out1_{commitment,rangeproof,asset,script}.hex` |
| **Environment** | Elements-bundled secp256k1-zkp objects; harness [`final/scripts/verify_rangeproof.c`](final/scripts/verify_rangeproof.c) / [`tests/verify_rangeproof.c`](tests/verify_rangeproof.c) |
| **Method** | `secp256k1_rangeproof_verify` with L-BTC generator + `extra_commit = 0x6a` |
| **Result** | **FALSE** ([`final/logs/verify_rangeproof_out.log`](final/logs/verify_rangeproof_out.log)) |
| **Limitation** | Does not by itself explain why the block became an ancestor |

### 2. Sibling rangeproofs (out0 / out2)

| | |
|---|---|
| **Result** | **TRUE** under real contexts ([`final/03_crypto_validation.md`](final/03_crypto_validation.md)) |
| **Limitation** | Confirms selective failure of out1, not acceptance mechanism |

### 3. Rangeproof cache key behavior + laboratory poisoning

| | |
|---|---|
| **Input** | Synthetic contexts A/B sharing `(P,C)` but differing generator/script |
| **Environment** | [`final/scripts/elements_cache_harness.c`](final/scripts/elements_cache_harness.c), [`final/scripts/cache_repro.c`](final/scripts/cache_repro.c) |
| **Method** | OLD key `H(proof\|\|value_commitment)` vs NEW key including asset + script |
| **Result** | OLD A→B: B returns success via **cache hit without crypto**; NEW A→B: B fails ([`final/logs/elements_cache_harness_run.log`](final/logs/elements_cache_harness_run.log)) |
| **Limitation** | Artificial contexts; **not** a historical priming of f24a’s `(P1,C1)` |

### 4. Clean-cache f24a out1 under Elements-faithful flow

| | |
|---|---|
| **Result** | Reject for OLD and NEW keying without priming |
| **Limitation** | Full `elementsd` ConnectBlock of block 4050336 was **not** completed in this workspace ([`final/06_full_node_reproduction.md`](final/06_full_node_reproduction.md)) |

### 5. Pedersen balance

| | |
|---|---|
| **Result** | Tally holds iff C1 included; opening of C1 unknown |
| **Limitation** | No numeric inflation proof without blinds |

### 6. UTXO / peg-out relationship

| | |
|---|---|
| **Result** | Confirmed UTXO links and three peg-outs through 4050349 ([`final/22_full_onchain_flow.md`](final/22_full_onchain_flow.md), [`final/data/pegout_flow.json`](final/data/pegout_flow.json)); earlier notes in [`final/08_pegout_chain.md`](final/08_pegout_chain.md) |
| **Limitation** | Links are **on-chain relationships**, not a closed causal proof that `f24a` “caused” the BTC payout |

### 7. Block validation call path

| | |
|---|---|
| **Method** | Source reading of Elements `validation.cpp` / `tx_verify.cpp` / `confidential_validation.cpp` |
| **Result** | Path documented in [`final/15_validation_divergence.md`](final/15_validation_divergence.md) et seq. |
| **Limitation** | Code path ≠ production runtime identity |

---

## Incident Timeline

Condensed from [`final/09_timeline.md`](final/09_timeline.md) (mix of **CODE** and **ON-CHAIN** facts):

| When | Event | Class |
|---|---|---|
| 2026-04-13 | `elements-23.3.3` released (still uses incomplete rangeproof cache key in the tree examined) | CODE |
| 2026-08–09 | Cache-key fix authored / merged / cherry-picked (`c26d719`, `6253d7e`, `212c43f`) | CODE |
| 2026-09-06 **13:53:10 UTC** | Block **4050336** contains `f24a` (`timestamp` 1788702790) | ON-CHAIN |
| +60s | Block **4050337** parents 4050336 | ON-CHAIN |
| Same day | Peg-outs A `46f117…` @4050344; B `ce4…` + C `731f…` @4050349; BTC `8db751…` | ON-CHAIN |
| 2026-09-06 | Public incident reporting; bridge paused (external sources) | PUBLIC REPORTING |

Whether production functionary nodes had deployed the cache-key fix by block 4050336 is **NOT VERIFIED**.

---

## Transaction `f24a…183f`

| Field | Value | Artifact |
|---|---|---|
| TXID | `f24a4b179b5cc7e88b25a763911f7cbdf2bf45d1d1b5ab611e94461cef0a183f` | [`final/02_transaction_f24.md`](final/02_transaction_f24.md) |
| Block | 4050336 / `e1d9…a0d5` | |
| Inputs | 1 (`0fbde521…3b95:2`) | [`final/13_pedersen_balance_f24a.md`](final/13_pedersen_balance_f24a.md) |
| Outputs | 4 (incl. explicit fee 58) | |
| Confidential values | out0, out1, out2 | |
| Rangeproof out0 | **PASS** | |
| Rangeproof out1 (**P1**/C1, `OP_RETURN`) | **FAIL** (L-BTC context tested) | [`final/logs/verify_rangeproof_out.log`](final/logs/verify_rangeproof_out.log) |
| Rangeproof out2 | **PASS** (+ surjection **PASS**) | |
| Pedersen tally | Holds with C1; fails without | [`final/13_pedersen_balance_f24a.md`](final/13_pedersen_balance_f24a.md) |
| C1 opening / signed value | **Not determined** | [`final/14_c1_constraints_no_blind.md`](final/14_c1_constraints_no_blind.md) |
| UTXO role | out1 unspent (OP_RETURN); out0/out2 later spent | [`final/data/f24a_descendant_probe.json`](final/data/f24a_descendant_probe.json) |

**Do not read “Pedersen tally TRUE” as “amounts are honest.”** The tally is an elliptic-curve relation among commitments; without a valid rangeproof (or an opening), it does not establish a safe cleartext value for C1.

---

## Block 4050336

| Field | Value |
|---|---|
| Height | 4050336 |
| Hash | `e1d9a2aae69e0fc3ca18f7f7f84e0615e92a5e3b5000d66c10c34043346da0d5` |
| Previous | `aad24e4fb64ca8adf4961667da87820cd48e553957ac64e75de7cdb298b5d66b` |
| Tx count | 7 |
| f24a position | index **1** ([`final/data/block_4050336_txids.json`](final/data/block_4050336_txids.json)) |
| Signblock | P2WSH `00207f1a37f6…e6be74`, redeem `OP_11` + 15 pubs + `OP_15` + `CHECKMULTISIG` |
| Quorum | **11** DER signatures |
| Timestamp | 1788702790 |

The block is real and contains `f24a`. Federation signatures authenticate the **signblock challenge** (block hash under dynafed rules).  

**SIGNATURE ≠ VALIDATION.**  
Signatures do **not** by themselves prove that any particular signer executed successful `ConnectBlock` / `VerifyAmounts` on f24a.

Only confidential rangeproof failure found in a full scan of block 4050336: **f24a:1** ([`final/data/block_4050336_tx_rp_scan.json`](final/data/block_4050336_tx_rp_scan.json)).

---

## Chain Reconstruction

Parent links verified through **4050349** ([`final/data/chain_4050335_4050349.json`](final/data/chain_4050335_4050349.json)):

```
4050335  aad24e4fb64ca8adf4961667da87820cd48e553957ac64e75de7cdb298b5d66b
    ↓
4050336  e1d9a2aae69e0fc3ca18f7f7f84e0615e92a5e3b5000d66c10c34043346da0d5
    ↓
4050337  c212cdcb6b2e68d4f56a7ddfee48bd3c02b2bfbb703b9f4506ac1597a40d38be
    ↓
    …
    ↓
4050343  f7add9732519fd2737a1be40a86a96191db0ea126338cc934eb686834ddc98f8
    ↓
4050344  3a4afa5b5a01119f6ecd8073374fb1418fa220c00b1ac0cd582d6adfc0be6c68   ← peg-out A (46f117)
    ↓
    …
    ↓
4050349  90a8c1ae477530b1e50c30ab7dc709eacab5a68a8b3652c147441508b1244730   ← peg-outs B (ce4), C (731f)
```

Full hashes for every height: see the JSON artifact / [`final/22_full_onchain_flow.md`](final/22_full_onchain_flow.md).

This proves 4050336 was used as the parent of subsequently signed blocks on the published tip examined here.  

It does **not** identify the first validator / accepting runtime.

**BLOCK PRODUCTION ≠ BLOCK ACCEPTANCE.**  
**ON-CHAIN RELATIONSHIP ≠ CAUSAL RELATIONSHIP.**

### UTXO relationships (confirmed only)

```
f24a:0 → 3875… → c6ea… → 46f117…
f24a:1 UNSPENT
f24a:2 → c6ea:0 → 46f117 vin11 → 46f117:1 → ce4 vin4
         c6ea:1 → 3289… → ce4…
```

These are **UTXO links**, not a claim that `f24a` “caused” the BTC payout.

### Peg-outs → Bitcoin

Structural correspondence (exact sats + BTC address) with `8db751a650ae2f12006b7e8c69a75e4df360e8afd6b9e05ae0b9fa6458a7b140` for peg-outs A, B, and C. Liquid→Bitcoin outpoint spend is **not** available; do not treat amount match alone as a full cryptographic bridge proof beyond what Mission 22 states.

---

## Federation Signature Analysis

ECDSA matching results ([`final/data/signblock_pubkey_matches.json`](final/data/signblock_pubkey_matches.json)):

| Block | Signer pubkey positions (0–14) |
|---|---|
| 4050336 | `0,1,3,4,5,6,7,8,11,13,14` |
| 4050337 | `0,1,3,5,6,7,8,10,11,13,14` |

- Intersection 4050336 ∩ 4050337: **10** keys  
- Common across the observed 4050335–4050343 series: `{1,5,6,8,11}`

These observations establish **which federation public keys signed the blocks**.  
They do **not** establish which signers executed full `ConnectBlock` validation.  
This repository does **not** map pubkeys to human or organizational identities.

---

## Confidential Transaction / Rangeproof Analysis

Investigation path (reports [`final/02`](final/02_transaction_f24.md)–[`final/04`](final/04_context_search.md), [`final/13`](final/13_pedersen_balance_f24a.md)–[`final/14`](final/14_c1_constraints_no_blind.md)):

1. Parse Elements serialization of `f24a`.
2. Extract commitments and proofs.
3. Verify rangeproofs with Elements’ secp256k1-zkp API.
4. Search alternate generators/scripts (128+ contexts; blocks around the incident) for a context where `(P1,C1)` verifies TRUE — **no hit in scanned material**.
5. Evaluate Pedersen conservation with/without C1.
6. Attempt constraints without blinds — value remains non-determined.

---

## Rangeproof Cache Investigation

**This section is deliberately conservative.**

### What exists (FACT)

- Pre-fix Elements `CachingRangeProofChecker` keyed rangeproof results primarily on **`proof || value_commitment`** (asset generator and `scriptPubKey` omitted from the key) — see [`final/01_code_history.md`](final/01_code_history.md), patches under [`final/patches/`](final/patches/).
- Fix commit family `c26d719` / cherry-picks adds asset commitment + scriptPubKey to the key ([`final/patches/c26d719c29.diff`](final/patches/c26d719c29.diff)).
- Laboratory A→B demonstrates that under the **old** key, a successful verify in context A can make context B return success via **cache hit without re-running cryptography**.
- Under the **new** key, the same artificial sequence rejects B.

This demonstrates **exploitability as a laboratory condition**.

### What is not established for the incident

> **However, this investigation has not established that the historical cache was primed with the exact proof/commitment pair used by f24a before block 4050336 was validated.**

Further:

- P1 bytes were **not** found before f24a in the scanned window ([`final/12_structural_falsification.md`](final/12_structural_falsification.md)).
- Candidate transactions `71c93d43…` / `27114710…` embed **C1 in script** with **different** rangeproofs — classified **NOT PRIMING OF P1/C1** for the pre-fix key `H(P1||C1)`.
- Therefore: **cache vulnerability demonstrated; historical use in this incident not established.**

**Do not treat the cache bug as the proven root cause of the incident.**

---

## Validation Path

Reconstructed stock path (source-level):

```
ProcessNewBlock
  → CheckBlock                    (structural; does not run VerifyAmounts)
  → ActivateBestChain
    → ConnectBlock
      → CheckTxInputs
        → if (fScriptChecks) VerifyAmounts
          → CRangeCheck
            → secp256k1_rangeproof_verify   ← f24a out1 fails here (empty cache)
```

Also documented:

- `CheckBlock` alone does not establish full confidential validity.
- Signblock signatures do not establish `ConnectBlock` success.
- `BLOCK_VALID_SCRIPTS` is raised **after** successful `ConnectBlock` — it is not a substitute for a missing production acceptance log.
- `assumevalid` / `fScriptChecks=false` / historical cache hit: **not demonstrated** as the production mechanism for 4050336 ([`final/21_first_runtime_acceptor.md`](final/21_first_runtime_acceptor.md)).

Official Elements RPC path note: `combineblocksigs` assembles signatures and checks proof **without** `TestBlockValidity` / amount checks — so **production of a signed invalid block is possible in code**. That is not proof of **acceptance** into chainstate ([`final/18_environment_4050336.md`](final/18_environment_4050336.md)).

---

## Hypotheses Tested

| Hypothesis | Status | Evidence | What remains missing |
|---|---|---|---|
| Rangeproof cache poisoning caused acceptance of f24a | **UNKNOWN** as historical cause; lab exploitability **CONFIRMED** | Harness + source; no historical `(P1,C1)` prime | Production cache state / priming event |
| Direct CT/rangeproof issue (invalid P1 under L-BTC) | **CONFIRMED** as crypto property of f24a out1 | Native verify | How that property passed consensus acceptance |
| Pedersen accounting / numeric inflation | Tally dependence on C1 **CONFIRMED**; cleartext inflation **UNKNOWN** | Pedersen harness | Blinding factors / opening |
| `assumevalid` skipped checks at tip | Default Liquid empty: cannot explain by default; runtime override **UNKNOWN** | chainparams + validation.cpp | Production config |
| `fScriptChecks=false` in production | Theoretically possible via assumevalid gate; **UNKNOWN** historically | Source | Production proof |
| Version mismatch (vulnerable vs patched) | Fix timing known; deployment on acceptor **UNKNOWN** | git history ([`final/09_timeline.md`](final/09_timeline.md)) | Acceptor binary identity |
| Configuration mismatch | **UNKNOWN** | — | Production config |
| Producer workflow without `TestBlockValidity` | Path **CONFIRMED** in code (`combineblocksigs`) | mining.cpp | Whether used; still ≠ acceptance |
| Functionary compromise (keys) | **Not demonstrated**; not assumed | — | Positive evidence (none here) |
| Consensus split (dual tips) | **UNKNOWN** (not established) | Single published tip lineage examined | Independent rejecting node evidence |
| Other validation bug | **UNKNOWN** | Divergence after CheckProof remains open | Runtime evidence |
| Combination of factors | **PLAUSIBLE** as narrative class; **not demonstrated** | — | Causal closure |

Early reports ([`final/11_final_verdict.md`](final/11_final_verdict.md), [`REPORT.md`](REPORT.md)) emphasize the cache mechanism and score “root cause” as incomplete. Later reports ([`final/19`](final/19_first_acceptor.md)–[`final/21`](final/21_first_runtime_acceptor.md)) explicitly fail to identify any accepting runtime. **This README follows the stricter closure rule of the later work:** no root-cause claim.

---

## What Has Been Ruled Out

| Claim | Status |
|---|---|
| Truncated commitment form `…5b1d8` appears on the wire | **REFUTED** (actual ends `…5b1d01d8`) |
| `71c93d43` / `27114710` primed `H(P1\|\|C1)` | **REFUTED** as P1/C1 primers ([`final/12_structural_falsification.md`](final/12_structural_falsification.md)) |
| Liquid default `assumevalid` alone explains tip acceptance of 4050336 | **REFUTED** as default-config explanation |
| OP_RETURN outputs skip confidential rangeproof checks in Elements | **REFUTED** (path still queues `CRangeCheck`) |
| Explorer HTML alone proves `ConnectBlock` | **REFUTED** as sufficient proof |
| “Federation signature ⇒ each signer ConnectBlock’d” | **REFUTED** as a logical inference |

---

## What Has NOT Been Proven

This investigation has **not** proven:

- the first accepting node;
- the first accepting runtime / version / configuration;
- a historical rangeproof-cache hit on `(P1,C1)`;
- historical cache priming of `(P1,C1)`;
- `fScriptChecks=false` in production on 2026-09-06;
- `assumevalid` as the mechanism used;
- a consensus split;
- functionary compromise;
- the root cause of the incident;
- that f24a’s invalid rangeproof was accepted *because of* the cache bug.

---

## The Remaining Unknown

```
f24a
  ↓
invalid rangeproof (P1) under tested L-BTC context     [FACT]
  ↓
stock empty-cache validation rejects                    [FACT / lab]
  ↓
?   ← FIRST UNPROVEN LINK
  ↓
4050336 present in some active chainstate
  ↓
4050337 (prev = hash(4050336))                          [FACT]
  ↓
4050338 → … → 4050349                                   [FACT]
```

**SIGNATURE ≠ VALIDATION**  
**BLOCK PRODUCTION ≠ BLOCK ACCEPTANCE**

The first unproven causal link is the **runtime/environment that put 4050336 into an active chainstate**.

---

## Why The Investigation Stops Here

No runtime-level artifact (node log, RPC `getblockchaininfo` / `getbestblockhash` dump, versioned production binary, or dual-node ACCEPT/REJECT pair) identifying the first acceptor was found ([`final/21_first_runtime_acceptor.md`](final/21_first_runtime_acceptor.md)).

This is a **deliberate forensic boundary**, not an invitation to fill the gap with the most compatible story.

```
Investigation status: ONGOING / ROOT CAUSE UNRESOLVED
FIRST ACCEPTOR: UNKNOWN
FIRST ACCEPTANCE MECHANISM: UNKNOWN
FIRST UNPROVEN LINK: the first runtime that placed block 4050336
                     into an active chainstate.
```

**No causal narrative is justified beyond this point.**

---

## Evidence & Repository Structure

Primary curated pack: [`final/`](final/).

```
.
├── README.md                 ← this document
├── REPORT.md                 ← earlier consolidated notes (see conflict note above)
├── artifacts/                ← checksums, sigcache excerpts, fix diffs
├── build/                    ← local object files / harness binaries (machine-specific)
├── data/                     ← raw hex / JSON captures (broader than final/data)
├── final/                    ← curated reports, hex, logs, patches, scripts, data
│   ├── 01_…23_*.md           ← chronological investigation reports (+ publication audit)
│   ├── data/                 ← block JSON, scans, signature matches, checksums
│   ├── hex/                  ← f24a and related extracted fields
│   ├── logs/                 ← harness outputs
│   ├── patches/              ← cache-fix related diffs / excerpts
│   └── scripts/              ← C harnesses (also mirrored under tests/)
├── logs/                     ← additional run logs
├── tests/                    ← C sources used for local builds
├── tools/                    ← JS tooling (includes node_modules; see disclosure)
├── elements/                 ← local Elements git clone (large; optional for readers)
└── secp256k1-zkp/            ← local secp256k1-zkp clone (optional)
```

Report reading order (investigation evolution):

1. [`final/01_code_history.md`](final/01_code_history.md) — Elements cache history  
2. [`final/02_transaction_f24.md`](final/02_transaction_f24.md) — parse f24a  
3. [`final/03_crypto_validation.md`](final/03_crypto_validation.md) — rangeproof results  
4. [`final/04_context_search.md`](final/04_context_search.md) — search for alternate verifying contexts  
5. [`final/05_cache_reproduction.md`](final/05_cache_reproduction.md) — lab cache exploit  
6. [`final/06_full_node_reproduction.md`](final/06_full_node_reproduction.md) — full-node limits  
7. [`final/07_consensus_split.md`](final/07_consensus_split.md) — split mechanism vs incident  
8. [`final/08_pegout_chain.md`](final/08_pegout_chain.md) — peg-out amounts / BTC linkage  
9. [`final/09_timeline.md`](final/09_timeline.md)  
10. [`final/10_alternative_hypotheses.md`](final/10_alternative_hypotheses.md)  
11. [`final/11_final_verdict.md`](final/11_final_verdict.md) / [`final/12_structural_falsification.md`](final/12_structural_falsification.md) — early closure on cache causality  
12. [`final/13`](final/13_pedersen_balance_f24a.md)–[`final/14`](final/14_c1_constraints_no_blind.md) — Pedersen / C1  
13. [`final/15`](final/15_validation_divergence.md)–[`final/18`](final/18_environment_4050336.md) — validation / lifecycle / environment  
14. [`final/19`](final/19_first_acceptor.md)–[`final/21`](final/21_first_runtime_acceptor.md) — acceptor / runtime still UNKNOWN  
15. [`final/22_full_onchain_flow.md`](final/22_full_onchain_flow.md) — chain to 4050349, UTXO/peg-out reconstruction  
16. [`final/23_publication_readiness.md`](final/23_publication_readiness.md) — pre-push publication audit  

---

## Reproduction

Requires: Linux, `gcc`, Elements tree (for `secp256k1` headers/objects with rangeproof/generator/surjection modules), and the hex under `final/hex/` or `data/`.

Prebuilt binaries may exist under `build/` and `final/scripts/` for the original analysis host; **rebuild on your machine**.

### A. Cryptographic verification

```bash
# Example pattern used in this workspace (adjust include/lib paths to your Elements build):
gcc -O2 -I elements/src/secp256k1/include \
  -o build/verify_rangeproof tests/verify_rangeproof.c \
  build/secp256k1.o build/precomputed_ecmult.o build/precomputed_ecmult_gen.o -lpthread
# Run from a cwd where the harness’s expected hex paths resolve (see source).
./build/verify_rangeproof
```

Expected (real L-BTC + `6a` context): `secp256k1_rangeproof_verify => 0` / `RESULT: FALSE`  
Reference log: [`final/logs/verify_rangeproof_out.log`](final/logs/verify_rangeproof_out.log).

### B. Cache reproduction

```bash
gcc -O2 -I elements/src/secp256k1/include \
  -o build/elements_cache_harness tests/elements_cache_harness.c \
  build/secp256k1.o build/precomputed_ecmult.o build/precomputed_ecmult_gen.o -lpthread
./build/elements_cache_harness
```

Expected directional result: OLD A→B cache hit success; NEW A→B reject; clean f24a out1 reject.  
Reference: [`final/logs/elements_cache_harness_run.log`](final/logs/elements_cache_harness_run.log).

### C. Transaction analysis

- Hex: [`final/hex/f24a4b179b5cc7e88b25a763911f7cbdf2bf45d1d1b5ab611e94461cef0a183f.hex`](final/hex/f24a4b179b5cc7e88b25a763911f7cbdf2bf45d1d1b5ab611e94461cef0a183f.hex)  
- Narrative parse: [`final/02_transaction_f24.md`](final/02_transaction_f24.md)  
- Optional: `tools/` + `liquidjs-lib` (see `tools/node_modules/liquidjs-lib`) for Elements serialization helpers.

### D. Block validation

Full `elementsd` ConnectBlock of 4050336: **not completed** in this repository ([`final/06_full_node_reproduction.md`](final/06_full_node_reproduction.md)).  
Source-path documentation: [`final/15_validation_divergence.md`](final/15_validation_divergence.md), [`final/17_block_lifecycle_4050336.md`](final/17_block_lifecycle_4050336.md).

### E. Chain reconstruction

```bash
python3 -c "import json; t=json.load(open('final/data/chain_4050335_4050349.json'));
print(all(t[i]['PREVHASH']==t[i-1]['HASH'] for i in range(1,len(t))))"
```

Expected: `True`.

Signature match table: [`final/data/signblock_pubkey_matches.json`](final/data/signblock_pubkey_matches.json).  
UTXO / peg-out graph: [`final/data/pegout_flow.json`](final/data/pegout_flow.json).

**Missing for complete end-to-end consensus reproduction:** a documented production (or lab) `elementsd` instance with known version/config that both accepts 4050336 and can be compared to a rejecting peer. Crypto/cache harnesses are **partial reproduction** of validation components, not full-node ConnectBlock.

---

## Methodology

1. Prefer primary artifacts (raw hex, block JSON, source, executed logs) over secondary commentary.  
2. Label every non-trivial claim as FACT / INFERENCE / HYPOTHESIS / UNKNOWN / REFUTED.  
3. Laboratory exploitability never upgrades to historical causation without a binding artifact.  
4. Compatible explanations are not causal claims.  
5. When reports disagree in emphasis, prefer the report that **withholds** the stronger claim unless new evidence appears.

---

## Publication Safety

This repository contains **public blockchain forensic data** and local reproducibility artifacts (hex, JSON, C harnesses, logs).

Confirmed by the publication audit ([`final/23_publication_readiness.md`](final/23_publication_readiness.md)):

- no credentials or private keys were intentionally included;
- federation **public** keys and blockchain artifacts are not treated as secrets;
- local hostname / absolute paths found during audit were redacted or relativized before publication;
- host-built binaries and upstream clones are listed in `.gitignore` (rebuild from sources).

This is **not** a formal security audit and does **not** claim the tree is “100% safe.”

### Sensitive data review

| FILE | CONTENT | ACTION TAKEN |
|---|---|---|
| `final/data/build_env.json` | hostname | **REDACTED** |
| `REPORT.md`, harness defaults | absolute local paths | **REDACTED / relativized** |
| `tools/node_modules/`, `elements/`, `secp256k1-zkp/`, `build/` | bulky / host-specific | **`.gitignore`** |
| Historical reports `final/01`–`22` | may contain superseded emphasis | **KEEP** as investigation history; README is authoritative for current conclusions |

---

## External References

Primary technical sources actually used by the investigation artifacts:

- Elements Project — https://github.com/ElementsProject/elements  
- Elements release `elements-23.3.3` — https://github.com/ElementsProject/elements/releases/tag/elements-23.3.3  
- secp256k1-zkp (Elements-bundled / related) — https://github.com/ElementsProject/secp256k1-zkp  
- Blockstream Liquid explorer API (block/tx captures) — `https://blockstream.info/liquid/api/`  
- Liquid / Elements documentation — https://docs.liquid.net/ · https://elementsproject.org/  
- Blockstream Help Center — Liquid functionary overview (workflow description; not used as ConnectBlock proof) — https://help.blockstream.com/liquid-network/faqs/what-is-a-liquid-network-functionary  

Public incident reporting (context only; not validation proofs) is cited in timeline notes such as SideSwap / press coverage of 2026-09-06; treat as **PUBLIC REPORTING**, not as cryptographic evidence.

---

## Current Conclusion

The investigation establishes several independently reproducible technical facts around the incident, including the invalid rangeproof behavior of `f24a` under the tested L-BTC context, the historical rangeproof-cache vulnerability and its laboratory exploitability, the inclusion of `f24a` in block 4050336, the 11-of-15 federation signatures, confirmed chain continuity through **4050349**, documented UTXO links toward the peg-outs, and three Liquid peg-outs with structural correspondence to Bitcoin payout `8db751…`.

It does not, however, identify the runtime that first accepted 4050336, nor does it establish that the rangeproof-cache vulnerability was used in the incident.

Accordingly, the root cause remains unresolved.

```
Investigation status: ONGOING / ROOT CAUSE UNRESOLVED
FIRST ACCEPTOR: UNKNOWN
FIRST ACCEPTANCE MECHANISM: UNKNOWN
FIRST UNPROVEN LINK: the first runtime that placed block 4050336
                     into an active chainstate.
```

**No causal narrative is justified beyond this point.**

---

## Open Questions

1. Which runtime first connected block 4050336 into an active chainstate?  
2. What exact Elements version and configuration did that runtime run?  
3. Was `VerifyAmounts` executed, skipped, or short-circuited — and by which condition?  
4. If a cache hit occurred, where did the priming `(P1,C1)` success originate?  
5. Can an independent full-node pair demonstrate ACCEPT vs REJECT on the same block bytes with documented configs?  
6. Can C1 be opened or otherwise bounded to demonstrate (or refute) numeric inflation?

Contributions that add **artifacts** (logs, RPC dumps, reproducible configs) are far more valuable than narrative restatements of existing hypotheses.
