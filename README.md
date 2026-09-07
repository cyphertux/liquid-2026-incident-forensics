# Liquid Network — 2026 Incident Forensics

> **Status:** Ongoing investigation — **root cause unresolved**
> **FIRST ACCEPTOR:** UNKNOWN
> **FIRST ACCEPTANCE MECHANISM:** UNKNOWN
> **Repository intent:** Independent, falsifiable technical forensics for the Liquid Network incident of **2026-09-06**
> **Scope boundary:** This repository establishes reproducible facts. It does **not** claim a root cause.

---

## Executive Summary

On **2026-09-06**, the Liquid Network experienced a major incident involving withdrawals on the order of **~4,000 BTC** from federation reserves via peg-outs (public reporting). This repository independently reconstructs **three** Liquid peg-outs in blocks 4050344–4050349 (A+B = **3998.66973280 BTC**; third peg-out separate) and focuses on confidential transaction

`f24a4b179b5cc7e88b25a763911f7cbdf2bf45d1d1b5ab611e94461cef0a183f`

in block **4050336** (`e1d9a2aa…a0d5`).

**Current technical state (after Mission 28 red-team):**

- f24a out1 rangeproof **P1** is cryptographically **invalid** under its real L-BTC + `OP_RETURN` context (**CONFIRMED**).
- Primer transactions `71c93d43…` / `27114710…` (block 4050335) carry a **valid** rangeproof **P0** under their OP_RETURN script (**CONFIRMED**).
- Under **post-fix** Elements cache keying (`c26d719c29`: unframed `proof‖value‖asset/generator‖script`), the primer and f24a:1 produce **byte-identical 4301-byte cache preimages** — **preimage aliasing**, not a SHA-256 collision (**CONFIRMED**, independently red-teamed).
- At the `CachingRangeProofChecker` locus: a **post-fix** node that Set the entry from the primer then **hits** on f24a:1; a cold post-fix node **rejects**; a pre-fix primed node **rejects** (**CONFIRMED** in lab).
- **Historical incident causality remains UNPROVEN:** whether accepting production runtimes ran post-`c26d719` code with that primer in cache is unknown. Tagged `elements-23.3.3` is still **pre-fix**.

```
The cache exploit mechanism is experimentally confirmed;
historical incident causality remains unproven.
```

**FIRST ACCEPTOR / FIRST ACCEPTANCE MECHANISM:** still **UNKNOWN**.
**No production causal narrative is claimed beyond the reproduced locus.**

---

## Current Status Matrix

| Question | Status |
|---|---|
| Historical / lab incomplete pre-fix cache keying | **CONFIRMED** |
| Post-fix non-injective (unframed) keying | **CONFIRMED** |
| Primer/attack post-fix preimages identical (4301 B) | **CONFIRMED** |
| “SHA-256 collision” as description | **REFUTED** (identical preimage) |
| Primer verifies (TRUE) | **CONFIRMED** |
| Attack f24a:1 verifies (FALSE) | **CONFIRMED** |
| Cache Set → Hit reproduction (checker) | **CONFIRMED** |
| `erase=true` retains entry until later GC | **CONFIRMED** |
| Framed keying breaks the alias | **CONFIRMED** |
| ~68 Q0 proofs prime f24a | **REFUTED** |
| Direct honest Verify→Set of exact `(P1,C1)` | **REFUTED** |
| Pre-fix `71c9/2711` primes `H(P1‖C1)` | **REFUTED** |
| Post-fix primed node can pass RP locus for f24a:1 | **CONFIRMED** |
| Full historical `ConnectBlock(4050336)` via this path | **UNPROVEN** |
| Production nodes ran post-fix at incident | **UNPROVEN** |
| Primer reached accepting nodes’ mempools | **UNPROVEN** |
| First accepting runtime | **UNKNOWN** |
| Cache mechanism caused the incident | **UNPROVEN** |
| Root cause identified | **No** |

Evidence labels: **FACT** / **INFERENCE** / **HYPOTHESIS** / **UNKNOWN** / **REFUTED**.

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
- Artificial A→B under post-fix key (different contexts, same `(P,C)`): B rejected: **reproduced**
- Direct honest Verify→Set of exact `(P1,C1)`: **refuted** as a viable stock path ([`final/27_exact_p1c1_priming.md`](final/27_exact_p1c1_priming.md))
- Post-fix **preimage aliasing** primer↔f24a:1: **reproduced** and red-teamed ([`final/28_red_team_review.md`](final/28_red_team_review.md))

### G. Post-fix cache preimage aliasing (Missions 26–28)

- Byte constructions (**FACT**): `P1 = P0 ‖ C0 ‖ G ‖ 0x6a ‖ 0x43` and primer script `S0 = 0x6a ‖ 0x43 ‖ C1 ‖ G ‖ 0x6a` ([`final/data/p0_p1_byte_comparison.json`](final/data/p0_p1_byte_comparison.json), [`final/data/28_byte_identity.json`](final/data/28_byte_identity.json)).
- For explicit assets, cache/crypto use **serialized generator**, not wire `0x01‖asset_id` (`confidential_validation.cpp`).
- Post-fix preimages `P1‖C1‖G‖S1` and `P0‖C0‖G‖S0` are **byte-identical** (4301 bytes) **before** hashing ([`final/data/28_key_reconstruction.json`](final/data/28_key_reconstruction.json)).
- ~68 other 4166-byte Q0 proofs in 4049384–4050246 are a **different** family and **do not** prime f24a ([`final/26_68_rangeproof_priming.md`](final/26_68_rangeproof_priming.md)).

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
| **Method** | OLD key `H(proof‖value_commitment)` vs NEW key including asset + script |
| **Result** | OLD A→B: B returns success via **cache hit without crypto**; NEW A→B: B fails ([`final/logs/elements_cache_harness_run.log`](final/logs/elements_cache_harness_run.log)) |
| **Limitation** | Artificial contexts; **not** the Mission 28 primer↔f24a alias |

### 3b. Post-fix primer ↔ f24a:1 preimage aliasing (Mission 28)

| | |
|---|---|
| **Input** | Real hex from `71c93d43` / `f24a` outs |
| **Environment** | [`final/scripts/28_cache_replay.c`](final/scripts/28_cache_replay.c) |
| **Method** | Elements-faithful `CachingRangeProofChecker` with post-fix unframed keying |
| **Result** | Primed post-fix: **RP-locus ACCEPT** (cache hit); cold post-fix: **REJECT**; pre-fix primed: **REJECT** ([`final/logs/28_red_team_reproduction.log`](final/logs/28_red_team_reproduction.log)) |
| **Limitation** | Checker-level only — **not** a full `ConnectBlock` / `ActivateBestChain` of block 4050336 |

### 4. Clean-cache f24a out1 under Elements-faithful flow

| | |
|---|---|
| **Result** | Reject for OLD and NEW keying without the colliding primer entry |
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

| When | Event | Class |
|---|---|---|
| 2026-04-13 | `elements-23.3.3` released — **still PRE-FIX** cache keying (tag does **not** contain `c26d719`) | CODE |
| 2026-08-03 | `c26d719c29` authored (post-fix keying) | CODE |
| **2026-09-01** | `c26d719c29` committed / merged lineage on master | CODE |
| **2026-09-02** | `6253d7e103` backport toward `elements-23.x` | CODE |
| **2026-09-03** | `212c43f475` cherry-pick toward `elements-23.3.x` | CODE |
| 2026-09-05 → 09-06 | ~68 copies of a *different* 4166-byte Q0 proof (not P0) | ON-CHAIN |
| 2026-09-06 **13:52:10 UTC** | Block **4050335**: primers `27114710…`, `71c93d43…` | ON-CHAIN |
| 2026-09-06 **13:53:10 UTC** | Block **4050336** contains `f24a` | ON-CHAIN |
| +60s | Block **4050337** parents 4050336 | ON-CHAIN |
| Same day | Peg-outs @4050344 / 4050349; BTC `8db751…` | ON-CHAIN |
| 2026-09-06 | Public incident reporting; bridge paused | PUBLIC REPORTING |

The reconstructed **lab** exploit requires **post-fix** keying.
**Whether accepting production nodes were running that code remains unverified.**
Do **not** read “the attacker exploited the patch” as a FACT.

---

## Technical Mechanism (current)

### Keying

**Before** the fix:

```text
H(salt || proof || value_commitment)
```

**After** `c26d719c29` (unframed concatenation):

```text
H(
  salt ||
  proof ||
  value_commitment ||
  asset_commitment/generator ||
  scriptPubKey
)
```

No length prefixes or domain separators between fields → **non-injective** encoding: distinct quadruplets can share one byte stream (**preimage aliasing caused by non-injective unframed concatenation**).
**Not** a SHA-256 collision.

### Reproduced locus

```text
PRIMER 71c93d43 / 27114710
          │
          │ Verify = TRUE
          ▼
       Cache.Set
          │
          ▼
    SAME 4301-BYTE PREIMAGE
          ▲
          │
    f24a :1
          │
          │ Verify = FALSE (would fail if checked)
          ▼
       Cache.Hit
          │
          ▼
    crypto verification skipped
          │
          ▼
    rangeproof-validation locus passes
```

**This is experimentally reproduced** at `CachingRangeProofChecker` fidelity ([`final/28_red_team_review.md`](final/28_red_team_review.md)).

**The rangeproof-validation locus of 4050336 is reproducible:** a primed post-fix node accepts the attack transaction at that validation stage, while a cold node rejects it.

**Historical acceptance by a production Liquid node through this exact path remains unproven** pending operational evidence.

### Revision of Mission 27

Mission 27 ([`final/27_exact_p1c1_priming.md`](final/27_exact_p1c1_priming.md)) correctly showed:

- full **P1** never passes `secp256k1_rangeproof_verify` in tested contexts;
- therefore honest **Verify→Set of exact `(P1,C1)`** cannot insert that pair.

It tested the hypothesis *“the cache must already contain `(P1,C1)` via verifying P1.”*
That hypothesis is **REFUTED**.

Mission 28 tested a **different** mechanism:

```text
valid primer (P0,C0,G,S0)
  → same post-fix preimage as (P1,C1,G,S1)
  → same cache entry
  → attack hits without verifying P1
```

Mission 27 is **kept** as falsification of the first priming story — not deleted. Its broader wording that the cache was “refuted as incident cause” is **superseded**: the exploit **mechanism** is experimentally confirmed; **historical causality** remains unproven.

### The Remaining Historical Link

The cryptographic and cache mechanism is independently reproduced. The remaining question is **historical**, not cryptographic:

> Were the nodes that accepted block 4050336 actually running the post-fix keying introduced by `c26d719c29`, and had the `71c9`/`2711` primer transactions traversed their mempools before the block arrived?

Still needed:

- functionary / bridge-node version inventory
- mempool acceptance logs
- `ProcessNewBlock` / `ActivateBestChain` logs
- first acceptor identity
- cache state at acceptance time

```
FIRST ACCEPTOR: UNKNOWN
FIRST ACCEPTANCE MECHANISM: UNKNOWN
```

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

### Pre-fix keying (FACT)

- Pre-fix `CachingRangeProofChecker` keyed primarily on **`proof || value_commitment`** ([`final/01_code_history.md`](final/01_code_history.md)).
- Artificial A→B (same `(P,C)`, different generator/script): OLD hit without crypto; NEW rejects B ([`final/05_cache_reproduction.md`](final/05_cache_reproduction.md)).

### Post-fix unframed keying (FACT — Mission 28)

- After `c26d719c29`, keying is unframed `proof‖value‖asset/generator‖script` ([`final/patches/c26d719c29.diff`](final/patches/c26d719c29.diff)).
- Primer `71c9`/`2711` and f24a:1 share a **byte-identical** 4301-byte post-fix preimage (**preimage aliasing**, not SHA-256 collision).
- Lab: Set after primer → Hit on attack; cold reject; pre-fix primed reject; framing breaks alias ([`final/28_red_team_review.md`](final/28_red_team_review.md)).

### What remains unproven historically

- That accepting production nodes ran **post-fix** code with the primer entry present.
- Full `ConnectBlock(4050336)` via this path.
- Pre-fix `H(P1‖C1)` priming by `71c9`/`2711`: **REFUTED** ([`final/25_71c9_2711_priming.md`](final/25_71c9_2711_priming.md)).
- Direct honest Verify→Set of exact `(P1,C1)`: **REFUTED** ([`final/27_exact_p1c1_priming.md`](final/27_exact_p1c1_priming.md)).

```
The cache exploit mechanism is experimentally confirmed;
historical incident causality remains unproven.
```

**Do not treat the cache mechanism as the proven root cause of the incident.**

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
| Post-fix preimage-alias cache hit caused historical acceptance | **STRONGLY SUPPORTED** as mechanism; **UNPROVEN** as incident cause | Mission 28 lab | Production post-fix + primer in acceptor cache |
| Pre-fix cache poisoning via same `(P1,C1)` | Lab A→B **CONFIRMED**; historical `(P1,C1)` prime **REFUTED** for `71c9` | Missions 5, 12, 25 | — |
| Direct honest Verify→Set of exact `(P1,C1)` | **REFUTED** | Mission 27 | — |
| ~68 Q0 proofs primed f24a | **REFUTED** | Mission 26 | — |
| Direct CT/rangeproof issue (invalid P1 under L-BTC) | **CONFIRMED** as crypto property | Native verify | How acceptance occurred |
| Pedersen accounting / numeric inflation | Tally depends on C1 **CONFIRMED**; cleartext **UNKNOWN** | Pedersen harness | Opening of C1 |
| `assumevalid` / `fScriptChecks=false` | Default empty; production use **UNKNOWN** | chainparams | Production config |
| Version mismatch (pre- vs post-fix on acceptor) | Fix dates known; acceptor binary **UNKNOWN** | git history | Version inventory |
| Producer without `TestBlockValidity` | Path **CONFIRMED** in code | mining.cpp | ≠ acceptance |
| Functionary compromise / consensus split / other bugs | **Not demonstrated** | — | Positive evidence |

[`REPORT.md`](REPORT.md) and early `final/11` scores are **historical**. Authoritative current state: this README + Missions 27–28.

---

## What Has Been Ruled Out

| Claim | Status |
|---|---|
| Truncated commitment form `…5b1d8` on the wire | **REFUTED** |
| `71c9`/`2711` primed pre-fix `H(P1‖C1)` | **REFUTED** ([`final/25_71c9_2711_priming.md`](final/25_71c9_2711_priming.md)) |
| ~68 Q0 proofs are primers of f24a | **REFUTED** ([`final/26_68_rangeproof_priming.md`](final/26_68_rangeproof_priming.md)) |
| Exact `(P1,C1)` must Verify→Set for the exploit | **REFUTED** (Mission 27); alias path does not need it |
| “SHA-256 collision” | **REFUTED** — identical preimage before hash |
| “`elements-23.3.3` contains the fix” | **REFUTED** — tag is still pre-fix |
| Liquid default `assumevalid` alone explains tip acceptance | **REFUTED** as default-config explanation |
| OP_RETURN skips confidential rangeproof checks | **REFUTED** |
| Explorer HTML / federation sig ⇒ `ConnectBlock` | **REFUTED** as sufficient proof |

---

## What Has NOT Been Proven

- first accepting node / runtime / version / configuration;
- production nodes ran post-`c26d719` at incident time;
- primer `71c9`/`2711` traversed accepting nodes’ mempools;
- historical cache entry present at `ConnectBlock(4050336)`;
- full historical `ConnectBlock` via the Mission 28 path;
- `fScriptChecks=false` / `assumevalid` as production mechanism;
- consensus split; functionary compromise;
- numeric amount in C1;
- that the cache mechanism **caused** the incident.

---

## The Remaining Unknown

```
f24a
  ↓
invalid rangeproof (P1) under tested L-BTC context     [FACT]
  ↓
cold / pre-fix: RP locus REJECT                     [FACT / lab]
primed post-fix: RP locus ACCEPT                     [FACT / lab]
  ↓
?   ← HISTORICAL LINK UNPROVEN
      (post-fix binary + primer in acceptor cache?)
  ↓
4050336 present in some active chainstate
  ↓
4050337 (prev = hash(4050336))                          [FACT]
  ↓
4050338 → … → 4050349                                   [FACT]
```

**SIGNATURE ≠ VALIDATION** · **BLOCK PRODUCTION ≠ BLOCK ACCEPTANCE**

See **The Remaining Historical Link** above.

---

## Why The Investigation Stops Here (for now)

Cryptographic + cache **mechanism** is independently reproduced (Mission 28).
What is missing is **operational** evidence binding that mechanism to production acceptors ([`final/21_first_runtime_acceptor.md`](final/21_first_runtime_acceptor.md)).

```
Investigation status: ONGOING / ROOT CAUSE UNRESOLVED
Mechanism (lab): CONFIRMED
Historical causality: UNPROVEN
FIRST ACCEPTOR: UNKNOWN
FIRST ACCEPTANCE MECHANISM: UNKNOWN
```

**No causal narrative beyond the reproduced locus is justified.**

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
│   ├── 01_…29_*.md           ← chronological investigation reports (+ documentation update)
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
17. [`final/24_gitignore_readiness.md`](final/24_gitignore_readiness.md) — git hygiene
18. [`final/25_71c9_2711_priming.md`](final/25_71c9_2711_priming.md) — pre-fix `71c9` priming of `H(P1‖C1)` **REFUTED**
19. [`final/26_68_rangeproof_priming.md`](final/26_68_rangeproof_priming.md) — ~68 Q0 family **REFUTED** as f24a primers
20. [`final/27_exact_p1c1_priming.md`](final/27_exact_p1c1_priming.md) — exact `(P1,C1)` honest Verify→Set **REFUTED**
21. [`final/28_red_team_review.md`](final/28_red_team_review.md) — **CONFIRMED** post-fix preimage aliasing (red-team)
22. [`final/29_documentation_update.md`](final/29_documentation_update.md) — integrate Mission 28 into public docs

### Mission index (24–29)

| # | Title | Result |
|---|---|---|
| 24 | Gitignore / publication hygiene | GO |
| 25 | `71c9`/`2711` pre-fix priming of `H(P1‖C1)` | **REFUTED** |
| 26 | ~68 Q0 rangeproof priming | **REFUTED** |
| 27 | Exact `(P1,C1)` honest Verify→Set | **REFUTED** (kept; hypothesis revised by 28) |
| 28 | Red-team validation of post-fix cache aliasing | **CONFIRMED** (mechanism); historical causality **UNPROVEN** |
| 29 | Documentation update integrating Mission 28 | Documentary |

Mission 28 summary: Independent red-team reproduction confirms the byte-identical post-fix cache preimage, native cryptographic TRUE/FALSE split, cache hit behavior, salt independence, and failure of framed keying to collide. Historical causality remains **UNPROVEN**.

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

### B2. Post-fix primer ↔ f24a alias (Mission 28)

```bash
# See source for include paths; requires Elements secp objects + hex under final/hex/
gcc -O2 -I elements/src/secp256k1/include \
  -o build/28_cache_replay final/scripts/28_cache_replay.c \
  build/secp256k1.o build/precomputed_ecmult.o build/precomputed_ecmult_gen.o -lpthread
./build/28_cache_replay
```

Expected directional result: primed post-fix **ACCEPT** at RP locus; cold post-fix **REJECT**; pre-fix primed **REJECT**.
Reference: [`final/logs/28_red_team_reproduction.log`](final/logs/28_red_team_reproduction.log), [`final/28_red_team_review.md`](final/28_red_team_review.md).

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
| Historical reports `final/01`–`28` | may contain superseded emphasis | **KEEP** as investigation history; README is authoritative for current conclusions |

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

## Limitations

- Checker-level / harness reproduction ≠ full `elementsd` `ConnectBlock` / `ActivateBestChain` of block 4050336.
- Lab primed ACCEPT at the rangeproof locus ≠ proven historical acceptance path.
- No production version inventory, mempool logs, or cache dumps bind Mission 28 to accepting nodes.
- Tagged `elements-23.3.3` is still pre-fix — deployment of `c26d719` on Liquid validators is **unverified**.
- C1 numeric opening remains unknown; Pedersen tally ≠ cleartext honesty.
- Historical reports (`final/01`–`27`, early `REPORT.md`) may use superseded emphasis; prefer this README + Mission 28/29.

---

## Current Conclusion

The investigation establishes independently reproducible facts: invalid f24a out1 under the tested L-BTC context; pre- and post-fix cache keying behavior; **post-fix preimage aliasing** between primer `71c9`/`2711` and f24a:1 (Mission 28); inclusion of f24a in 4050336; federation signatures; chain continuity through 4050349; UTXO links; three Liquid peg-outs.

```
The cache exploit mechanism is experimentally confirmed;
historical incident causality remains unproven.
```

The rangeproof-validation locus of 4050336 is reproducible in lab (primed post-fix accepts; cold rejects). Historical acceptance by a production Liquid node through that exact path remains unproven. First acceptor / first acceptance mechanism: **UNKNOWN**. Root cause: **unresolved**.

```
Investigation status: ONGOING / ROOT CAUSE UNRESOLVED
Mechanism (lab): CONFIRMED
Historical causality: UNPROVEN
FIRST ACCEPTOR: UNKNOWN
FIRST ACCEPTANCE MECHANISM: UNKNOWN
```

**Next investigation step:** operational evidence for post-fix deployment and primer mempool presence on accepting nodes (see **The Remaining Historical Link**).

---

## Open Questions

1. Did accepting runtimes for 4050336 run post-`c26d719` keying?
2. Did primer `71c9`/`2711` traverse those nodes’ mempools before the block?
3. What is the first accepting runtime / version / config?
4. Can full `ConnectBlock(4050336)` be demonstrated end-to-end under the primed post-fix path?
5. Can an independent ACCEPT vs REJECT peer pair be documented?
6. Can C1 be opened or bounded for numeric inflation?

Contributions that add **artifacts** (logs, RPC dumps, version inventories) are far more valuable than narrative restatements.

---

## Current Verdict (compact)

| Layer | Status |
|---|---|
| Crypto (P1 FALSE / P0 TRUE) | **CONFIRMED** |
| Post-fix preimage aliasing | **CONFIRMED** |
| Cache Set → Hit (checker) | **CONFIRMED** |
| Full historical ConnectBlock path | **UNPROVEN** |
| Incident root cause | **UNRESOLVED** |
