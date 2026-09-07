# 01 — Exact historical Elements cache code

## FACT — Commit inventory

| Commit | Role | AuthorDate | CommitDate | Contained in |
|---|---|---|---|---|
| `c26d719c29a40da280a825b25657e9c3d8bc7d99` | original fix on master | 2026-08-03 | 2026-09-01 | `master` (merged PR #1592 → `31e8f27f4b`) |
| `6253d7e103655ec015097de1505b5f3785ff6447` | backport | 2026-08-03 | 2026-09-02 | PR #1595 → `elements-23.x` (merged 2026-09-03) |
| `212c43f475fc202b5b9e6dbb1f1c616e1a06a6f7` | cherry-pick of 6253d7e | 2026-08-03 | 2026-09-03 | PR #1599 → `elements-23.3.x` (merged **2026-09-06**) |

Parent of `c26d719c29`: `b0aba619b6ff2846617b285274a954e0404137f5`  
Files touched by original fix: `src/script/sigcache.cpp`, `src/script/sigcache.h`  
Function: `SignatureCache::ComputeEntryRangeProof` + call site in `CachingRangeProofChecker::VerifyRangeProof`

Diffs: `final/patches/c26d719c29.diff`, `6253d7e103.diff`, `212c43f475.diff`

## FACT — Cache key before fix (also in all releases ≤ elements-23.3.3)

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

## FACT — Cache key after fix

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

## FACT — Behavior before/after

On cache hit, Elements returns `true` **without** re-running `secp256k1_rangeproof_verify`.  
Before fix, two calls with same `(proof, value_commitment)` but different `(asset generator, scriptPubKey)` share one key.  
After fix, they do not.

Failed verifications are **not** inserted (`Set` only after successful crypto + policy checks).

## FACT — Release table (fix present?)

| version/tag | contains fix? | commit | tag date (approx) |
|---|---|---|---|
| elements-22.1.1 | NO | — | 2023-03 |
| elements-23.2.7 | NO | — | 2025-04 |
| elements-23.3.0 | NO | — | 2025-05 |
| elements-23.3.1 | NO | — | 2025-11 |
| elements-23.3.2 | NO | — | 2026-02 |
| **elements-23.3.3** | **NO** | — | **2026-04-13** (latest official before incident) |
| master @ 2026-09-04 | YES | c26d719c29 | unreleased tip |
| elements-23.x after PR #1595 | YES | 6253d7e103 | branch, not a release tag at analysis time |
| elements-23.3.x after PR #1599 | YES | 212c43f475 | merged 2026-09-06 (incident day); no new release tag observed |

`git tag --contains c26d719c29` → empty at clone time.

## FACT — Cache location

- Process-global `static SignatureCache rangeProofCache` in `src/script/sigcache.cpp`
- Initialized via `InitRangeproofCache` from `src/init.cpp`
- **RAM only** (not persisted to disk / DB). Restart clears it.
