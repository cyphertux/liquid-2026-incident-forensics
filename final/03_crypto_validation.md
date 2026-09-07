# 03 — Independent cryptographic validation

## Environment (FACT)

- arch: `aarch64`
- gcc: Ubuntu 15.2.0
- library: Elements bundled `src/secp256k1` compiled manually with  
  `-DENABLE_MODULE_RANGEPROOF=1 -DENABLE_MODULE_GENERATOR=1 -DENABLE_MODULE_SURJECTIONPROOF=1`  
  objects in `build/secp256k1.o` etc.
- API: `secp256k1_rangeproof_verify` (exact Elements path)

## Parameters used by Elements (FACT)

From `CachingRangeProofChecker::VerifyRangeProof` / `confidential_validation.cpp`:

1. parse value commitment  
2. parse **asset generator** (for explicit assets: `secp256k1_generator_generate(asset_id)` then serialize)  
3. `secp256k1_rangeproof_verify(ctx, &min, &max, &commit, proof, plen, scriptPubKey, scriptPubKey.size(), &generator)`  
4. policy: reject if `min_value==0 && !scriptPubKey.IsUnspendable()`

`nonce` of the confidential output is **not** an argument to `rangeproof_verify`.

## Argument / cache table (FACT)

| argument | influences verify? | in OLD cache key? | in NEW cache key? |
|---|---|---|---|
| rangeproof bytes | YES | YES | YES |
| value commitment | YES | YES | YES |
| asset generator (serialized) | YES | **NO** | YES |
| scriptPubKey as `extra_commit` | YES | **NO** | YES |
| output nonce field | NO (for verify) | NO | NO |
| min/max (outputs of verify) | policy uses min | NO | NO |
| process salt | keying only | YES (implicit) | YES (implicit) |

## Results (FACT — CODE EXECUTION)

| Test | Result |
|---|---|
| P1+C1+L-BTC generator+script `6a` | **FALSE** |
| P1+C1+L-BTC+empty script | FALSE |
| P1+C1+L-BTC+P2WPKH | FALSE |
| f24a out0 under its real context | **TRUE** (sanity) |
| f24a out2 under its real context | **TRUE** (sanity) |
| Elements-faithful clean cache path on f24a out1 | **ret=0** (reject) |

Logs: `final/logs/verify_rangeproof_out.log`, `elements_cache_harness_run.log`  
`rangeproof_info(P1)`: exp=0 mantissa=52 min=0 max=2^52−1 (structure decodes; does not imply validity for this commit/context).
