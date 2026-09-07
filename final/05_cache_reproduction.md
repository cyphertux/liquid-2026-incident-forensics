# 05 — Cache reproduction (Elements-faithful control flow)

## What was executed (FACT)

Harness: `final/scripts/elements_cache_harness.c`  
Binary: `final/scripts/elements_cache_harness`  
Log: `final/logs/elements_cache_harness_run.log`

Control flow mirrors `CachingRangeProofChecker::VerifyRangeProof`:
1. compute cache key (OLD or NEW)
2. on hit → return TRUE (no crypto)
3. else `secp256k1_rangeproof_verify`
4. policy min_value/unspendable
5. on success + store → insert key

(Real Elements also salts the hasher with a process nonce; omitted identically for both modes — relative key equality across contexts is the property under test.)

## Artificial contexts (FACT — CODE EXECUTION)

| Mode | Order | A | B |
|---|---|---|---|
| OLD | A→B | ret=1 crypto=1 | **ret=1 hit=1 crypto=-1 (FALSE POSITIVE)** |
| NEW | A→B | ret=1 crypto=1 | ret=0 crypto=0 |
| OLD | B→A | B ret=0 | A ret=1 |
| NEW | B→A | B ret=0 | A ret=1 |

Also earlier toy harness `cache_repro` confirms same directional property.

## Historical f24a on clean cache (FACT)

```
clean cache verify f24a out1: ret=0 hit=0 crypto=0
```

for both OLD and NEW keying — as expected without priming.

## Cannot run historical priming of P1/C1 (FACT)

No known context A that makes crypto(P1,C1)=TRUE was found (see 04).  
Therefore Tests “prime then accept f24a” with **this** proof are **NON REPRODUIT**.
