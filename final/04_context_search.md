# 04 — Search for context A where P1+C1 verifies TRUE

## Goal

Find any `(asset_generator, script/extra_commit)` such that:

`secp256k1_rangeproof_verify(P1, C1, gen, script) == TRUE`

while the real f24a context is FALSE.

## FACT — On-chain appearance of P1 / C1 (blocks 4050330–4050336)

Scan log: `final/data/block_scan_4050330_4050336.json`

| height | txid | C1 as value commitment | C1 in script | P1 bytes present |
|---|---|---|---|---|
| 4050335 | `27114710…7ec5` | no | **yes (OP_RETURN)** | **no** |
| 4050335 | `71c93d43…f411` | no | **yes (OP_RETURN)** | **no** |
| 4050336 | `f24a…183f` | **yes (vout1)** | no | **yes** |

P1 appears **only** in f24a in this window.

## FACT — Batch context search

- Candidate contexts: **128** unique `(asset, script)` pairs from address `ex1q7kgx4…` history + all block 4050335 txs + empty-script variants per asset  
- File: `final/data/candidate_contexts.json`  
- Runner: `final/scripts/batch_context_search.c`  
- Log: `final/logs/batch_context_search.log`  

**SUMMARY: tested=128 hits=0**

## FACT — Implication for priming txs 71c93d43 / 27114710

Their out0 scripts are:

```
6a 43 <C1 33 bytes> <L-BTC generator 33 bytes> 6a
```

Their out0 **value commitments differ from C1**, and their rangeproofs differ from P1.  
Those rangeproofs **do** verify TRUE under L-BTC + that long script.  

They therefore **cannot** insert cache key `H(P1 || C1)` under the historical keying function, because the proof bytes are not P1.

## Verdict for Part 6

**NON REPRODUIT:** no context A found for `(P1, C1)` among all systematically tested related on-chain contexts.

**INFERENCE (not fact):** either (i) priming occurred off-chain / mempool-only / unpublished, (ii) context A uses a generator/script outside the related set, or (iii) the incident’s acceptance path for f24a is not explained by priming of this exact `(P1,C1)` pair.
