# Mission 28 — Red-team / independent counter-expertise of the “post-fix preimage alias” cache mechanism

## Summary (≤15 lines)

1. **Prior claim under review:** after `c26d719`, unframed `H(proof‖commit‖asset‖script)` lets primer `71c93d43` insert a cache entry identical to the one later looked up for `f24a:1`, so invalid P1 is not re-verified.  
2. **Independently reproduced:** byte constructions `P1=P0‖C0‖G‖6a‖43` and `S0=6a‖43‖C1‖G‖6a`; post-fix preimages **byte-identical**; pre-fix keys **differ**; primer crypto **TRUE**; attack crypto **FALSE**; `erase=true` only marks reclaimable.  
3. **Falsified / corrected:** intervals claiming bytes past P1 length 4234; treating this as a SHA-256 collision (it is **identical preimage**); calling full-block “ACCEPT” from rangeproof-checker alone; assuming `elements-23.3.3` contains the fix (**it does not**).  
4. **Now confirmed (mechanism):** unframed post-fix keying + explicit-asset → serialized generator + this specific primer/attack pair ⇒ **same cache entry** (lab).  
5. **Still historically unknown:** whether any 4050336 acceptor ran post-`c26d719` code with the primer in-process cache; first acceptor; production versions.

---

## Verdicts (four levels — do not merge)

| Layer | Verdict |
|---|---|
| **1. Cryptographic mechanism** (identical unframed post-fix preimages; primer verifies; attack does not) | **CONFIRMED** |
| **2. Cache mechanism** (Set after primer; Get hit on attack; erase≠delete) | **CONFIRMED** (at `CachingRangeProofChecker` fidelity) |
| **3. Consensus / validation path** (full `ConnectBlock` / `ActivateBestChain` of 4050336) | **STRONGLY SUPPORTED** for the rangeproof locus; **UNPROVEN** as full-node reproduction |
| **4. Historical incident causality** | **UNPROVEN** (and **in tension** with tagged `elements-23.3.3` still being pre-fix) |

---

## I — Exact `c26d719c29` code (independent)

**Commit:** `c26d719c29a40da280a825b25657e9c3d8bc7d99`  
AuthorDate: 2026-08-03 · CommitDate: 2026-09-01  
Message: `fix: range proof cache bind to asset and scriptpubkey`  
Diff artifact: `artifacts/c26d719c29.diff` / live tree `elements/src/script/sigcache.cpp`.

### BEFORE

```cpp
void SignatureCache::ComputeEntryRangeProof(uint256& entry,
    const std::vector<unsigned char>& proof,
    const std::vector<unsigned char>& commitment) const {
  CSHA256 hasher = m_salted_hasher_range_proof;
  hasher.Write(proof.data(), proof.size())
        .Write(commitment.data(), commitment.size())
        .Finalize(entry.begin());
}
```

### AFTER (current `elements` HEAD / post-fix)

```cpp
// elements/src/script/sigcache.cpp:58-60
void SignatureCache::ComputeEntryRangeProof(..., asset_commitment, scriptPubKey) const {
  CSHA256 hasher = m_salted_hasher_range_proof;
  hasher.Write(proof.data(), proof.size())
        .Write(commitment.data(), commitment.size())
        .Write(asset_commitment.data(), asset_commitment.size())
        .Write(scriptPubKey.data(), scriptPubKey.size())
        .Finalize(entry.begin());
}
```

**Field order (PROVEN):** `proof` → `value_commitment` → `asset_commitment` → `scriptPubKey` bytes.  
**Framing (PROVEN):** **none** — no lengths, tags, or separators.

Call site still: `CachingRangeProofChecker::VerifyRangeProof` → `ComputeEntryRangeProof` then `Get(entry, !store)` (`sigcache.cpp:134-136`).

Backports `6253d7e103`, `212c43f475`: same functional change (verified in local diffs).

**Tagged `elements-23.3.3`:** still **PRE-FIX** API (`ComputeEntryRangeProof(proof, commitment)` only).  
`git merge-base --is-ancestor c26d719c29 elements-23.3.3` → **false**.

---

## II — Unframed concatenation ⇒ aliasing

**PROVEN:** post-fix hasher concatenates field bytes raw.

Therefore any two quadruplets with the same concatenated byte string yield the **same** salted hash for any salt (structural identity, not a SHA-256 collision).

Measured: `1000/1000` random salts give equal `H(salt‖preimage)` when preimages are identical (`final/data/28_key_reconstruction.json`).

---

## III — What `asset_commitment` actually is

`confidential_validation.cpp` (~372–388):

```cpp
std::vector<unsigned char> vchAssetCommitment = asset.vchCommitment;
...
if (asset.IsExplicit()) {
  secp256k1_generator_generate(..., &gen, asset.GetAsset().begin());
  secp256k1_generator_serialize(..., &vchAssetCommitment[0], &gen);
}
QueueCheck(..., new CRangeCheck(..., vchAssetCommitment, tx.vout[i].scriptPubKey, ...));
```

**PROVEN:** for **explicit** L-BTC, the cache/crypto asset field is **serialized generator**, not wire `0x01‖asset_id`.

Independent check: `secp256k1_generator_generate+serialize(explicit L-BTC)` equals `G` embedded in 71c9 OP_RETURN (`final/scripts/28_gen_check.c` → `equal 1`).

Prefixes: wire explicit `0x01…`; serialized generator `0x0a…` (or `0x0b`).

---

## IV–VII — Byte reconstruction (attack & primer)

Extracted from local hex (raw-derived):

| Field | Bytes | Notes |
|---|---:|---|
| P0 | 4166 | 71c9 out0 RP |
| C0 | 33 | 71c9 out0 value commitment |
| P1 | 4234 | f24a out1 RP |
| C1 | 33 | f24a out1 value commitment |
| S1 | 1 | `6a` |
| S0 | 69 | `6a43‖C1‖G‖6a` |
| G | 33 | from S0[35:68]; = generator ser. of L-BTC |

### Constructions — **PROVEN** byte-identical

```text
P1 == P0 || C0 || G || 0x6a || 0x43
S0 == 0x6a || 0x43 || C1 || G || 0x6a
S1 == 0x6a
```

Offsets into P1 using **`[start,end)`**:

| Interval | Content | Match |
|---|---|---|
| `[0,4166)` | P0 | **PROVEN** |
| `[4166,4199)` | C0 | **PROVEN** |
| `[4199,4232)` | G | **PROVEN** |
| `[4232,4234)` | `6a 43` | **PROVEN** |
| `[4234, …)` | — | **DNE** — any claim of bytes 4234–4300 inside P1 is **REFUTED** |

### Post-fix preimages

```text
attack = P1 || C1 || G || S1
       = P0 || C0 || G || 6a || 43 || C1 || G || 6a

primer = P0 || C0 || G || S0
       = P0 || C0 || G || 6a || 43 || C1 || G || 6a
```

**PROVEN:** `attack_preimage == primer_preimage` (length **4301**), **before** SHA-256.  
`SHA256(preimage)=82b0b8ccf8c743171f2bc8d80bb9982a81cfca583e8dfbe65563cf095e99c01a`

**REFUTED:** “SHA-256 collision of different inputs”.  
**PROVEN:** identical inputs.

If wire asset `0x01‖id` were hashed instead of `G`, preimages **differ** (`wire_asset_preimages_equal: false`) — so generator substitution is **load-bearing**.

### Pre-fix

`SHA256(P0‖C0) ≠ SHA256(P1‖C1)` — **PROVEN** (fingerprints `47b9fde7…` vs `86bc93c9…`).

---

## VIII–IX — Pre vs post inversion (core thesis)

| Mode | Primer key vs attack key | Status |
|---|---|---|
| PRE-FIX (`proof‖commit` only) | **≠** | **PROVEN** |
| POST-FIX unframed (`+asset_ser+script`) | **=** | **PROVEN** |
| POST-FIX with length framing (hypothetical) | **≠** | **MEASURED** (NODE D) |

Salt independence: **1000/1000** — structural.

---

## X — Native crypto

Using workspace Elements secp objects (`build/secp256k1.o`):

| Case | Result | Status |
|---|---|---|
| `verify(P0,C0,G,S0)` | **TRUE** | **MEASURED** |
| `verify(P1,C1,G,S1)` | **FALSE** | **MEASURED** |

OP_RETURN / `IsUnspendable`: `script[0]==OP_RETURN` → unspendable (**PROVEN** `script.h`); allows `min_value==0` policy pass on primer (**PROVEN** policy predicate in `sigcache.cpp`).

Log: `final/logs/28_red_team_reproduction.log`

---

## XI–XIII — Real cache semantics

### CuckooCache `erase=true`

Upstream `cuckoocache.h` documents and implements: `contains(e, erase=true)` sets **allow_erase** / GC flag but **still returns true**; lazy discard on later insert pressure.

**PROVEN from code.** Lab replay: after Connect-style `Get(erase)`, slot `reclaimable=1` and `present=1`; attack still **HIT**.

### Replay scenarios (`final/scripts/28_cache_replay.c`)

| Node | Setup | f24a:1 rangeproof checker |
|---|---|---|
| **A** | POST + mempool Set(primer) + Connect Get(erase) + attack | **ACCEPT (hit)** |
| **B** | POST cold (Connect primer without Set) + attack | **REJECT** |
| **C** | PRE + primed | **REJECT** |
| **D** | length-framed keying | collision **broken** |

**CRITICAL LABELING (PROVEN limitation):** “ACCEPT” here means **`CachingRangeProofChecker` returned true for out1 only** — **not** full `ConnectBlock` / `ActivateBestChain` of block 4050336.

---

## XIV–XV — What was / wasn’t reproduced

**Reproduced:** rangeproof cache alias path under Elements-faithful keying + verify + Get/Set/erase marking.  

**Not reproduced:** end-to-end `elementsd` processing of blocks 4050335–4050336, P2P, signing, or tip activation.

---

## XVI — Tally / inflation

Prior Mission 13 (not re-opened here as new crypto): Pedersen tally **holds iff C1 included**; opening of C1 **UNKNOWN**.  
Economic numeric amount of C1: **UNKNOWN**.

---

## XVII — On-chain chain / peg-outs

Mission 22 artifacts remain authoritative for continuity 4050335→4050349 and peg-outs A+B = `3998.66973280` BTC — **not re-litigated**; treated as **previously PROVEN** in-repo. This red-team focuses on the cache claim.

---

## XVIII–XX — Historical claims

| Claim | Verdict |
|---|---|
| Mechanism *can* make a post-fix node accept invalid f24a:1 RP given primed cache | **CONFIRMED** (checker-level) |
| Nodes that accepted 4050336 ran `c26d719` | **UNPROVEN** |
| `elements-23.3.3` includes the fix | **REFUTED** (tag still pre-fix) |
| Bridge “patched” ⇒ acceptors were post-fix | **REFUTED as implication** |
| Primer was in acceptor mempool | **UNKNOWN** |
| First acceptor identity | **UNKNOWN** |

Chronology (local git):

| Event | Date |
|---|---|
| `c26d719` author | 2026-08-03 |
| `c26d719` commit | 2026-09-01 |
| `212c43f` on 23.3.x | 2026-09-03 |
| `elements-23.3.3` tag tip date | 2026-04-10 (release **without** fix) |
| Incident | 2026-09-06 |

**INFERENCE risk in the reviewed report:** treating post-fix as the production consensus codepath without deployment evidence.

---

## XXI — Alternatives still open

Even with a confirmed alias mechanism:

- Pre-fix nodes + some other skip (`fScriptChecks=false`, non-stock binary)  
- Post-fix + this alias (requires primer in cache)  
- Unknown hybrid  

Mission 27 correctly showed **P1 never verifies**; that **falsifies** “prime exact `(P1,C1)` by verifying P1”. It does **not** falsify this **post-fix preimage-alias** path (primer verifies **P0/C0/S0**). Mission 27’s blanket “CACHE MECHANISM REFUTED AS INCIDENT CAUSE” is therefore **too strong** relative to this distinct mechanism — **corrected here**.

---

## XXII — Checklist

| Item | Status |
|---|---|
| Post-fix code identified | **PASS** |
| Keying reproduced | **PASS** |
| Generator used by cache confirmed | **PASS** |
| P0/P1/S0 constructions byte-exact | **PASS** |
| Preimages identical before hash | **PASS** |
| Post keys equal / pre keys differ | **PASS** |
| Salt independence | **PASS** (1000) |
| Primer TRUE / attack FALSE | **PASS** |
| Set / Hit / erase semantics | **PASS** (checker + cuckoo docs) |
| Full 4050335→4050336 `elementsd` | **FAIL / NOT DONE** |
| Historical post-fix deployment | **FAIL / UNPROVEN** |
| Peg-outs / tally | prior missions; C1 amount **UNKNOWN** |

---

## XXIII — Final four verdicts (repeat)

1. **Cryptographic mechanism:** **CONFIRMED**  
2. **Cache mechanism:** **CONFIRMED** (rangeproof checker fidelity)  
3. **Consensus/validation path:** **STRONGLY SUPPORTED** at RP locus; full-node **UNPROVEN**  
4. **Historical incident causality:** **UNPROVEN**

---

## Artifacts

| Path | Content |
|---|---|
| `final/28_red_team_review.md` | this report |
| `final/data/28_byte_identity.json` | P1/S0 offsets |
| `final/data/28_key_reconstruction.json` | preimages / salts / code notes |
| `final/data/28_crypto_verification.json` | verify results |
| `final/data/28_cache_replay.json` | NODE A–D summary |
| `final/data/28_historical_evidence.json` | UNKNOWN / 23.3.3 |
| `final/logs/28_red_team_reproduction.log` | harness output |
| `final/scripts/28_cache_replay.c` / `28_gen_check.c` | reproduction sources |

---

## Bottom line

The reviewed mechanism is **not** wishful: the **bytes and Elements post-fix keying really do alias**. That is a serious, independently confirmed technical finding.

It is **not** yet a closed historical proof that this is how 4050336 entered any production chainstate. Tagged **23.3.3 is still pre-fix**, so causality requires evidence of **which binary** acceptors ran and whether the primer sat in their **mempool-primed** cache.
