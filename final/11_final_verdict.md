# 11 — Final verdict

## Executive summary (≤15 lines)

1. Pre-fix Elements rangeproof cache keyed only `proof||value_commitment` — **confirmed in source and releases through elements-23.3.3**.  
2. Fix `c26d719c29` (+ cherry-picks) adds asset commitment + scriptPubKey — **confirmed**; **no release tag** contained it before the incident.  
3. f24a out1 `(P1,C1)` has SHA256(P1)=`6619fa29…c098b2` and **fails** native `secp256k1_rangeproof_verify` under real L-BTC+OP_RETURN — **confirmed by execution**.  
4. Artificial A→B with Elements-faithful control flow yields OLD cache false-positive; NEW rejects — **confirmed**.  
5. Systematic search (128 contexts; blocks 4050330–4050336) found **no** context where `(P1,C1)` verifies TRUE; priming txs embed C1 in script but **not** P1 — **confirmed negative**.  
6. Full `elementsd` dual-node accept/reject of f24a/block 4050336 — **NON REPRODUIT** (tooling limits).  
7. Peg-outs 2.65138358 + 3996.01834922 = 3998.66973280 BTC and mainnet `8db751…` — **confirmed on-chain**, analyzed separately from root-cause proof.  

**Therefore:** the exploit *mechanism* is experimentally real; the claim that this mechanism, via priming of this exact `(P1,C1)`, is the root cause of the 6 Sep 2026 incident is **not yet fully demonstrated**.

## 1. Definitely demonstrated (FACT)

- Cache key incompleteness bug exists and is exploitable in miniature  
- P1 invalid in real L-BTC context  
- Clean-cache Elements-flow rejects f24a out1 proof  
- Fix commits/backports/release lag  
- Peg-out amount chain to BTC tx `8db751…`

## 2. Strongly probable

- A vulnerable Liquid node with empty cache would reject f24a’s critical rangeproof check  
- Process-local RAM cache can cause divergent validation results **if** priming of the same `(P,C)` occurred  
- Functionary HSMs need not be key-compromised for peg-outs once L-BTC state is consensus-accepted (**INFERENCE**)

## 3. Not demonstrated

- Context A for historical `(P1,C1)`  
- That 71c93d43/27114710 primed P1/C1  
- Full-node ACCEPT after priming / REJECT without  
- Dual-node consensus split on block 4050336  
- Exact committed value v for C1  
- Complete causal chain f24a → inflation amount → peg-outs

## Status table (Part 27)

| # | Proposition | STATUS | Proof basis |
|---|---|---|---|
| 1 | Cache bug exists | **CONFIRMED** | source + releases |
| 2 | Cache bug exploitable | **CONFIRMED** | harness execution |
| 3 | P1 invalid under L-BTC | **CONFIRMED** | secp verify |
| 4 | P1 valid in another context | **UNCONFIRMED** | 128 contexts, 0 hits |
| 5 | A→B false positive | **CONFIRMED** (artificial) | harness |
| 6 | f24a accepted after priming | **UNCONFIRMED** | missing A + no full node |
| 7 | f24a rejected without priming | **PROBABLE** | clean verify FALSE; full tx path incomplete |
| 8 | patched rejects f24a | **PROBABLE** | same; NEW key would not help without A |
| 9 | 4050336 contains/depends on f24a | **CONFIRMED** | explorer |
| 10 | 4050336 caused consensus split via this bug | **UNCONFIRMED** | |
| 11–13 | peg-out / BTC linkage | **CONFIRMED** | on-chain |
| 14 | functionaries need not be compromised | **PLAUSIBLE** | scenario analysis |
| 15 | cache bug is root cause of incident | **UNCONFIRMED** | mechanism yes; historical binding no |
| 16 | historical priming identified | **REFUTED** for claimed txs as P1/C1 primers; **UNCONFIRMED** globally | |

## Scores /100 (justified)

| Axis | Score | Why |
|---|---:|---|
| Cache vulnerability | **98** | Exact code + release evidence |
| Cryptographic invalidity | **97** | Native verify + sanity outs |
| Alternative-context validity | **15** | Explicitly searched; not found |
| Exploitability | **90** | Artificial A→B solid |
| f24a exploitation | **40** | Invalid proof yes; accept-via-cache no |
| Priming | **20** | Correlated txs; not cryptographic priming of P1 |
| Consensus split | **45** | Mechanism yes; incident split no |
| Peg-out linkage | **95** | Hard on-chain amounts/links |
| Functionary compromise not required | **70** | Logical, not empirically proven |
| **Overall root cause** | **48** | Strong mechanism, weak historical closure |

## Category (Part 18)

**3 — Vulnerability demonstrated, causal link to incident unproven**

See also `12_structural_falsification.md` (call graph, mempool vs block `store` flag, NOT PRIMING classification, UTXO path via out2).

## Closing sentence

**Le niveau de preuve actuel ne permet pas encore d'affirmer que le bug de cache est la cause racine de l'incident du 6 septembre 2026.**

Pourquoi: le mécanisme est démontré, mais (1) aucun contexte A pour `(P1,C1)` n’est établi, (2) P1 n’apparaît pas avant f24a dans le matériel accessible, (3) `71c93d43`/`27114710` ne peuvent pas primer `H(P1||C1)`, (4) le full-node ACCEPT/REJECT n’a pas été reproduit. Structurellement, sans priming mempool de la paire exacte `(P1,C1)`, le cache ne peut pas faire accepter l’out1 de f24a.
