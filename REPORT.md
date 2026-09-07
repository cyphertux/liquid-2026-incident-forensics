# Liquid 2026-09-06 — Independent Falsifiable Reproduction Report

Date of analysis: 2026-09-07  
Workspace artifacts: repository root (relative paths under this tree)

**Note on sources:** No local TX files were present in the workspace despite the request to use “fichiers fournis”. Raw hex/JSON were fetched independently from `https://blockstream.info/liquid/api/` (and Bitcoin mainnet API). Checksums are in `artifacts/checksums.txt`.

---

## 1. Executive verdict

The **cache-key incompleteness bug is real** and was independently reproduced (historical code + artificial exploit).  
The **attack transaction’s rangeproof is cryptographically INVALID** under its on-chain L-BTC + `OP_RETURN` context.  
A **confirmed priming context that makes the same `(proof, commitment)` verify as TRUE** was **not found** among related on-chain transactions; therefore the full “primed node accepts / unprimed rejects this exact proof” chain for `f24a…` remains **partially unproven**.

**Overall:** mechanism of consensus split via cache is experimentally demonstrated in miniature; linking that mechanism to this specific on-chain proof as the root cause is **not fully closed**.

---

## 2. What we independently reproduced

| Claim | Result | Tag |
|---|---|---|
| `CachingRangeProofChecker` existed historically | YES | [VERIFIED BY SOURCE] |
| Pre-fix cache key = `SHA256(proof \|\| value_commitment)` only | YES | [VERIFIED BY SOURCE] |
| Commit `c26d719c29` exists and adds `asset_commitment` + `scriptPubKey` to key | YES | [VERIFIED BY SOURCE] |
| Cherry-picks `6253d7e103`, `212c43f475` exist | YES | [VERIFIED BY SOURCE] |
| No official release tag before 2026-09-06 contains the fix | YES (`elements-23.3.3` still vulnerable) | [VERIFIED BY SOURCE] |
| TX `f24a…` in block `4050336` / hash `e1d9…a0d5` | YES | [VERIFIED BY RAW TRANSACTION] |
| Output 1 commitment matches (full 33-byte form) | YES | [VERIFIED BY RAW TRANSACTION] |
| Rangeproof SHA256 = `6619fa29ce0967ae93baefef0bc6233972b05249a9861a96695a106bbcc098b2` | YES | [VERIFIED BY RAW TRANSACTION] |
| Native `secp256k1_rangeproof_verify` → **FALSE** on real context | YES | [VERIFIED BY CODE EXECUTION] |
| Artificial old-cache exploit: A→TRUE then B→TRUE by cache hit | YES | [VERIFIED BY CODE EXECUTION] |
| Artificial fixed-cache: A→TRUE then B→FALSE | YES | [VERIFIED BY CODE EXECUTION] |
| Peg-out amounts 2.65138358 + 3996.01834922 = 3998.66973280 BTC | YES | [VERIFIED BY RAW TRANSACTION] |
| Mainnet payout `8db751a650…b140` carries both amounts | YES | [VERIFIED BY RAW TRANSACTION] |

---

## 3. What we independently falsified / failed to reproduce

| Claim | Result | Tag |
|---|---|---|
| User-truncated commitment `…5b1d8` (missing nibbles) appears on wire | **FALSE** — actual is `…5b1d01d8` | [VERIFIED BY RAW TRANSACTION] |
| Attack `(P1,C1)` verifies under priming-related generators/scripts | **FALSE** for all tested plausible contexts (0 hits) | [VERIFIED BY CODE EXECUTION] |
| Priming TXs `71c93d43…` / `27114710…` contain the same rangeproof bytes as attack | **FALSE** — they embed **C1 in script**, different proofs | [VERIFIED BY RAW TRANSACTION] |
| Local “fichiers fournis” for TX hex | **NON REPRODUIT** (absent; replaced by explorer fetch) | [NOT VERIFIED] |
| Cleartext / negative value of C1 under L-BTC | **NON REPRODUIT** (needs blinding factor) | [NOT VERIFIED] |
| Historical mempool priming of `(P1,C1)` | **NON REPRODUIT** | [NOT VERIFIED] |

---

## 4. Historical Elements code

Process-global cache (`src/script/sigcache.cpp`):

```cpp
namespace {
    static SignatureCache rangeProofCache;
    static SignatureCache surjectionProofCache;
}
```

Verification path (same before/after fix for crypto; only cache key changed):

```cpp
bool CachingRangeProofChecker::VerifyRangeProof(...) {
    uint256 entry;
    rangeProofCache.ComputeEntryRangeProof(...);
    if (rangeProofCache.Get(entry, !store)) {
        return true;  // unconditional TRUE on cache hit
    }
    // parse commitment + generator, then:
    if (!secp256k1_rangeproof_verify(..., scriptPubKey ..., &tag)) return false;
    if (min_value == 0 && !scriptPubKey.IsUnspendable()) return false;
    if (store) rangeProofCache.Set(entry);
    return true;
}
```

Crypto inputs actually required: **proof, value commitment, asset generator, scriptPubKey (extra_commit), and unspendable/min_value policy**.

---

## 5. Exact cache key before fix

From parent of `c26d719c29`:

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

**Missing from key but used in verify:** asset generator (`vchAssetCommitment` / `tag`) and `scriptPubKey`.

Also present in release `elements-23.3.3` (2026-04-13) — last official release before the incident.

---

## 6. Exact cache key after fix

Commit `c26d719c29a40da280a825b25657e9c3d8bc7d99`:

```cpp
void SignatureCache::ComputeEntryRangeProof(uint256& entry,
    const std::vector<unsigned char>& proof,
    const std::vector<unsigned char>& commitment,
    const std::vector<unsigned char>& asset_commitment,
    const CScript& scriptPubKey) const {
    CSHA256 hasher = m_salted_hasher_range_proof;
    hasher.Write(proof.data(), proof.size())
          .Write(commitment.data(), commitment.size())
          .Write(asset_commitment.data(), asset_commitment.size())
          .Write(scriptPubKey.data(), scriptPubKey.size())
          .Finalize(entry.begin());
}
```

**Why this blocks the miniature attack:** contexts that differ only in generator/script no longer share a cache entry, so an invalid context cannot inherit a prior TRUE.

**Commit metadata:**
- AuthorDate: 2026-08-03; CommitDate: 2026-09-01
- Parent: `b0aba619b6…`
- Merged via PR #1592 → `31e8f27f4b` (2026-09-01)
- Cherry-pick `6253d7e103…` (PR #1595 elements-23.x), CommitDate 2026-09-02
- Cherry-pick `212c43f475…` from 6253d7e (PR #1599 23.3.x), CommitDate 2026-09-03
- `git tag --contains c26d719c29` → **empty** as of clone date (no release tag yet)

Diffs saved under `artifacts/*.diff`.

---

## 7. Cryptographic verification of P1/C1

### Extraction from `f24a4b179b5cc7e88b25a763911f7cbdf2bf45d1d1b5ab611e94461cef0a183f`

| Field | Value |
|---|---|
| vin / vout | 1 / 4 |
| Output index | **1** |
| Value commitment | `086f5d67160fc4b477954fb09ef321e5b589d7a07740a1a6df494ed2335b1d01d8` |
| Asset (explicit L-BTC wire) | `016d521c38…79026f` |
| Asset id (display) | `6f0279e9ed041c3d710a9f57d0c02928416460c4b722ae3457a11eec381c526d` |
| Derived L-BTC generator | `0a0a488de4899d0ae757f6cf8368663184d164106111ed9eaecf510e35282ddc6d` |
| scriptPubKey | `6a` (OP_RETURN) |
| nonce | `00` |
| Rangeproof length | **4234** bytes |
| Rangeproof SHA256 | **`6619fa29ce0967ae93baefef0bc6233972b05249a9861a96695a106bbcc098b2`** |
| Commitment SHA256 | `1a5afca31358014d3f0b0e29755aa8fbd009fcbc4f8258d2773f0a0d7408acce` |
| Surjection proof | empty (len 0; expected for explicit asset) |

### Native verification (Elements bundled secp256k1-zkp)

```
secp256k1_rangeproof_verify(L-BTC generator, extra_commit=0x6a) => FALSE
```

Sanity checks on same binary:
- `f24a` out0 under its real context → **TRUE**
- `f24a` out2 under its real context → **TRUE**

`secp256k1_rangeproof_info(P1)`: `exp=0 mantissa=52 min=0 max=2^52-1` (proof decodes as a structure, but does not verify against this commitment/context).

---

## 8. Search for valid priming context

Confirmed candidate priming txs (same address, height 4050335):

- `71c93d4339fe8328981a2ec0dd23808d1ff104dc4c4cbcfc5c06fb2bb622f411`
- `271147100a94f6337b6c3db39b30c92d5b97ed91597307b6f721f73a15187ec5`

Both have out0 script:

```
6a43 || <C1 33 bytes> || <L-BTC generator 33 bytes> || 6a
```

Their out0 rangeproofs **verify TRUE** under L-BTC + that long script (`min_value=0`, allowed because unspendable).  
They **do not** contain attack rangeproof `P1`. Commitment `C1` appears only as **script data**, not as their value commitment.

Brute-plausible context search for `(P1,C1)` across L-BTC / priming generators / scripts: **TOTAL_HITS=0**.

**Conclusion:** on-chain priming of the exact cache entry `(P1,C1)` is **NON REPRODUIT**. Priming txs are correlated and embed `C1`, but that alone does not satisfy the stated `(same proof, same commitment)` priming hypothesis.

---

## 9. Minimal cache exploit reproduction

Artifact: `tests/cache_repro.c`, log: `logs/cache_repro_run.log`

Constructed:
- Context A: valid proof for value under generator GA + script `OP_TRUE`
- Context B: **same proof + same commitment**, generator GB + script `OP_RETURN`
- Crypto baseline: A=TRUE, B=FALSE
- OLD keys for A and B: **identical**
- NEW keys for A and B: **distinct**

Results:

| Mode | Order | Outcome |
|---|---|---|
| OLD | A then B | B returns **TRUE via CACHE_HIT** |
| NEW | A then B | B returns **FALSE** (miss + crypto fail) |
| OLD | B then A | B FALSE (not cached); A TRUE |
| NEW | B then A | B FALSE; A TRUE |

**Cache property:** only successful verifications are inserted; failed B does not poison the cache. Exploit direction is **valid→invalid**, not the reverse.

---

## 10. Primed vs unprimed behavior

Same artificial contexts, separate process invocations:

| Process | Result for B |
|---|---|
| Unprimed OLD (`verify(B)` only) | FALSE |
| Primed OLD (`verify(A)` then `verify(B)`) | TRUE (cache hit) |
| Primed NEW (`verify(A)` then `verify(B)`) | FALSE |

This is exactly the consensus-split *mechanism* (process-local / node-local global `rangeProofCache`).  
Mapping onto historical Liquid nodes for **this** `P1` still requires a missing priming witness for `(P1,C1)`.

---

## 11. On-chain correlation

| Item | Observation | Tag |
|---|---|---|
| `f24a…` | confirmed height **4050336**, block `e1d9a2aa…a0d5` | [VERIFIED BY RAW TRANSACTION] |
| `46f117…` | peg-out **265138358 sat = 2.65138358 BTC** to `bc1qkxwva32eh7mgezq5kladncd3n5wtcjmslh98my` | [VERIFIED BY RAW TRANSACTION] |
| `ce4cae…` | peg-out **399601834922 sat = 3996.01834922 BTC** to `bc1qgslsydz56d0ed6827hdemfmk5w2f6ldyc6wt7p` | [VERIFIED BY RAW TRANSACTION] |
| Sum | **399866973280 sat = 3998.66973280 BTC** | [VERIFIED BY RAW TRANSACTION] |
| Mainnet `8db751a650ae2f12006b7e8c69a75e4df360e8afd6b9e05ae0b9fa6458a7b140` | pays **both** amounts (block 965783) | [VERIFIED BY RAW TRANSACTION] |

Causal link “invalid rangeproof cache bypass → these peg-outs” is **[INFERENCE]** from timing/flow, not proven by this reproduction alone.

### Commitment algebra (step 8)

Pedersen: `C = v·G_asset + r·H`.  
Changing `G_asset` changes which `v` (if any) is consistent with a fixed `(C,r)`.  
We **did not** recover `v` or `r` for `C1` under L-BTC. Claim of “enormous/negative value” is **[NOT VERIFIED]** / **[HYPOTHESIS]**.

---

## 12. What remains unproven

1. A concrete prior verification event where `(P1,C1)` verified TRUE under some `(G*, script*)`.  
2. That Liquid validating nodes that accepted block 4050336 were cache-primed with that entry.  
3. That nodes rejecting the block lacked that entry (vs another consensus divergence).  
4. Numerical opening of `C1` under L-BTC.  
5. Completeness of the inflation path (pedersen tally + this output) end-to-end in a full Elements node.

---

## 13. Final confidence scores

| Component | Score | Rationale |
|---|---:|---|
| Cache bug | **98/100** | Exact historical code + miniature exploit + fix diff |
| Invalid proof (real context) | **97/100** | Native zkp verify FALSE; verifier sanity-checked on sibling outs |
| Exploit transaction (`f24a`) | **85/100** | On-chain fields match; invalid proof confirmed; inflation causality not fully closed |
| Priming | **35/100** | Correlated priming txs found, but **not** same `(P,C)` valid context |
| Consensus split explanation | **70/100** | Mechanism demonstrated; binding to this incident’s node split incomplete |
| Complete root cause | **55/100** | Strong partial proof; missing priming witness for `P1` |

---

## Artifact index

```
repro/
  data/           # raw hex/json extractions
  tests/          # verify_rangeproof.c, cache_repro.c, search_contexts.c, ...
  build/          # binaries + secp object files
  logs/           # compile/run logs
  artifacts/      # commit diffs, checksums, sigcache excerpts
  elements/       # full git clone
  secp256k1-zkp/  # upstream clone (Elements submodule used for verify)
```

Re-run cache demo:

```bash
./build/cache_repro
./build/cache_repro unprimed_old
./build/cache_repro primed_old
./build/cache_repro primed_new
```

Re-run attack proof check:

```bash
./build/verify_one data/out1_rangeproof.hex data/out1_commitment.hex data/out1_asset.hex data/out1_script.hex
```
