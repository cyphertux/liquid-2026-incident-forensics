# 07 — Consensus split

## Mechanism that *can* split nodes (FACT — CODE + EXECUTION)

Given identical block bytes containing a tx with `(P,C)` that is:
- crypto-valid under context A
- crypto-invalid under context B (e.g. L-BTC)

Then under **pre-fix** Elements:

| Node state | Validation of B |
|---|---|
| Previously verified A (primed) | ACCEPT via cache hit |
| Never saw A / restarted | REJECT via crypto |

Under **post-fix** Elements: both REJECT B (different keys).

This was executed in miniature (`elements_cache_harness`).

## Historical Liquid split for block 4050336 (STATUS)

| Claim | Status |
|---|---|
| Block 4050336 exists with f24a | CONFIRMED on-chain |
| f24a out1 rangeproof invalid under real context | CONFIRMED by crypto execution |
| Some Liquid nodes accepted / some rejected that block due to cache priming of P1/C1 | **UNCONFIRMED** — requires missing context A + dual-node experiment |
| Dual elementsd ACCEPT vs REJECT on same block bytes | **NON REPRODUIT** (no full nodes) |

## Consensus-critical path (FACT from source)

```
AcceptBlock / ConnectBlock
  → CheckTransaction / confidential checks
    → confidential_validation.cpp queues CRangeCheck
      → CachingRangeProofChecker::VerifyRangeProof
        → cache hit? return true
        → else secp256k1_rangeproof_verify + policy
```

A cache false-positive changes the **consensus validation result** of a transaction/block for that process.
