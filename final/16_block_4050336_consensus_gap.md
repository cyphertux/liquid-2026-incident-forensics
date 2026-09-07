# 16 — Block 4050336 reconstruction & first consensus gap

## One-sentence answer

Under stock Elements Liquid code with an empty rangeproof cache, **ConnectBlock must reject f24a at out1’s rangeproof**; yet block `e1d9a2aa…a0d5` is **federation-signed (~11/15)** and appears **`in_best_chain` on Blockstream’s API**. The mechanism that reconciles those two facts is **not identified**. Cache poisoning of `(P1,C1)` remains **historically unsupported**.

---

## Absolute labeling used below

CONFIRMED / PROBABLE / PLAUSIBLE / UNCONFIRMED / REFUTED

---

## 1. Exact validation call graph (historical Elements)

### Accept / header path (before UTXO connect)

| Step | File | Function | What it checks | Skip conditions |
|---|---|---|---|---|
| Header proof | `src/block_proof.cpp` `CheckProof` (~55–67) | Dynafed: `GenericVerifyScript(signblockscript, m_signblock_witness, …)` | Federation signatures authorize **header** | `g_signed_blocks==false` → PoW instead |
| Challenge continuity | `ContextualCheckBlockHeader` `validation.cpp` ~4822+ | height, time, dynafed, checkpoints | `-reindex-chainstate` skips this function’s invocation path (comment ~4820) |
| AcceptBlock | `validation.cpp` | stores block if `CheckBlock` OK | | |

### CheckBlock (`validation.cpp` ~4564–4626)

Parameters: `(block, state, consensusParams, fCheckPOW, fCheckMerkleRoot)`

| Check | f24a / 4050336 |
|---|---|
| `block.fChecked` early return | skip if already checked |
| `CheckBlockHeader` → `CheckProof` | **PASS** (11 DER sigs + witness script) — CONFIRMED from `data/block.json` |
| Merkle root | **PASS** expected (`f9a463b6…104b`) — not rehashed here; CONFIRMED as published |
| Size / coinbase uniqueness | **PASS** (coinbase = `b36f…`; f24a index 1) |
| `CheckTransaction` each tx | **PASS** for f24a (`tx_check.cpp` — no CT proofs) |
| Legacy sigops | expected PASS |

**Does not call:** `VerifyAmounts`, rangeproofs, surjections, input scripts, UTXO spends.

### ContextualCheckBlock (`validation.cpp` ~4897+)

Witness commitment / finality / bip rules — **no** `VerifyAmounts`.

### ConnectBlock (`validation.cpp` ~2780+)

| Step | Params / flags | f24a |
|---|---|---|
| Re-`CheckBlock` | `!fJustCheck` | PASS |
| **`fScriptChecks`** | default `true`; see §2–3 | **true** on Liquid tip default |
| PAK `IsPAKValidTx` | enforce+dynafed | N/A for f24a (not pegout) |
| `CheckTxInputs(..., cacheStore=fJustCheck, fScriptChecks)` | connect: `cacheStore=false` | enters VerifyAmounts |
| `CheckInputScripts` | if `fScriptChecks` | would run if amounts passed |
| `UpdateCoins` | only if checks pass | |

### CheckTxInputs → VerifyAmounts (**proof that fScriptChecks gates amounts**)

```250:252:elements/src/consensus/tx_verify.cpp
        if (fScriptChecks && !VerifyAmounts(spent_inputs, tx, pvChecks, cacheStore)) {
            return state.Invalid(TxValidationResult::TX_CONSENSUS, "bad-txns-in-ne-out", "value in != value out");
        }
```

**CONFIRMED:** `VerifyAmounts` runs **if and only if** `fScriptChecks` is true (Elements mode). It is **not** unconditional.

### VerifyAmounts (`confidential_validation.cpp` ~148+)

Order when `pvChecks == nullptr` (immediate):

1. Parse input/output commitments  
2. **`CBalanceCheck`** → tally — f24a **PASS**  
3. For each confidential vout: **`CRangeCheck`** → `CachingRangeProofChecker::VerifyRangeProof`  
   - out0 **PASS**  
   - out1 **FAIL** ← first failing amount check  
   - out2 not reached in serial mode  
4. Surjections (not reached if out1 already failed serially)

With parallel `pvChecks`, checks are queued then executed by `CCheckQueue`; failure of out1 still fails the block.

### CachingRangeProofChecker (`sigcache.cpp` ~131–170)

1. Compute cache key → `Get(entry, !store)`  
2. On miss: `secp256k1_rangeproof_verify`  
3. Policy `min_value==0 && !IsUnspendable`  
4. `Set` only if `store`

---

## 2. Complete skip conditions for VerifyAmounts

| Condition | Can skip VerifyAmounts while “accepting” tip? | Verdict |
|---|---|---|
| `fScriptChecks == false` via **assumevalid** | Only for ancestors of configured assumevalid under work/time rules | Liquid default assumevalid **empty** → **cannot explain tip 4050336** |
| `g_con_elementsmode == false` | Would skip CT path entirely | Liquid has elements mode **true** |
| Cache **hit** | Does **not** skip the function; makes `VerifyRangeProof` return true without crypto | See §4 |
| `CheckBlock` / header-only / indexer | Can display a signed block without ConnectBlock | **PLAUSIBLE** for explorers; **not** full-node active-chain advance |
| `-reindex-chainstate` | Skips `ContextualCheckBlockHeader` path note; ConnectBlock still runs amounts when connecting | does not skip VerifyAmounts |
| Checkpoints | Reject old forks; don’t skip amounts on new tip | no |
| Deployment / softfork flags | Affect script flags, not the `fScriptChecks && VerifyAmounts` gate | no |
| `fJustCheck` | Only flips `cacheStore`; still runs VerifyAmounts | no |
| Genesis / snapshot / IBD assumevalid | Could skip deep history | **not** tip after empty assumevalid |

### 3. assumevalid — Liquid

```1335:1335:elements/src/kernel/chainparams.cpp
        consensus.defaultAssumeValid = uint256();
```

User may set `-assumevalid=<hash>`. For connecting **4050336 as tip** with default config:

**assumevalid cannot explain this failure.**

(If someone set assumevalid to a descendant of 4050336 *and* was still catching up under the 2-week equivalent-time rule, amounts could be skipped — **UNCONFIRMED** that any production Liquid node did so for this tip.)

---

## 4. Cache — final exclusion for historical f24a

| Question | Answer |
|---|---|
| Can cache bypass a crypto FAIL? | **Yes** — `Get` hit returns true before `secp256k1_rangeproof_verify` |
| Key (pre-c26d) | salted `SHA256(proof \|\| value_commitment)` |
| Required priming | Prior **successful** verify of **exact** `(P1, C1)` with `store=true` |
| Found on-chain? | **No** — P1 only in f24a; 71c/271 cannot prime | 

**Historical acceptance of f24a via cache: REFUTED** as demonstrated mechanism (theoretical path remains).  
Do **not** treat cache as the explanation of 4050336.

---

## 5. CheckBlock PASS vs ConnectBlock FAIL — normal?

**Yes — by design.**

```
CheckBlock
  checks: header proof, merkle, size, coinbase shape, CheckTransaction (no CT)
ConnectBlock additional:
  UTXO availability, VerifyAmounts (tally+RP+surjection), input scripts, BIP68, PAK (pegouts), fees accumulate
```

f24a: CheckBlock **PASS**; ConnectBlock amounts **FAIL** on out1 — **CONFIRMED** by code + native verify.

---

## 6–7. Signed block 4050336

| Field | Value | Status |
|---|---|---|
| Height | 4050336 | CONFIRMED |
| Hash | `e1d9a2aae69e0fc3ca18f7f7f84e0615e92a5e3b5000d66c10c34043346da0d5` | CONFIRMED |
| Prev | `aad24e4f…d66b` (4050335) | CONFIRMED |
| Merkle | `f9a463b6…104b` | CONFIRMED (published) |
| Timestamp | 1788702790 | CONFIRMED |
| tx_count | 7 | CONFIRMED |
| f24a position | **index 1** (after coinbase) | CONFIRMED |
| signblockscript | `00207f1a37f651f3cdae…e6be74` (P2WSH) | CONFIRMED |
| witness | 13 stack items: empty + **11 ECDSA sigs** + 513 B redeem script (`5b21…` = OP_15 …) | CONFIRMED |
| Status API | `in_best_chain: true`, next `c212cdcb…` | CONFIRMED (Blockstream API snapshot in `data/block_status.json`) |

**What signatures prove:** the federation authorized **this block header** (dynafed signblock challenge).  

**What they do not prove:** that every transaction passes `VerifyAmounts`.  

**Order:** `CheckProof` runs at **header/CheckBlock** time — **before** ConnectBlock amounts.  

A compromised or buggy producer **could** assemble an invalid-tx block that still gathers signatures if signers sign headers without fully validating txs — **PLAUSIBLE**, **not CONFIRMED** as what happened.  
**Signed ≠ compromised.**

---

## 8. H6 discrimination

| Variant | Status |
|---|---|
| A. Invalid tx deliberately produced | PLAUSIBLE (P1 uniquely fails) |
| B. Tx valid on some special node | UNCONFIRMED (no context A) |
| C. Valid under different version | UNCONFIRMED |
| D. Validation bug | PLAUSIBLE category; no concrete bug shown that skips failed verify |
| E. Producer compromise | UNCONFIRMED — signatures alone insufficient |

---

## 9–10. Versions & Liquid params

| Item | Finding |
|---|---|
| Producer Elements version | **UNCONFIRMED** (not in block) |
| Public releases through 23.3.3 | vulnerable cache key; **still reject** f24a on empty cache |
| c26d719 / cherry-picks | CommitDates 2026-09-01…09-03; **also reject** empty cache |
| Liquid consensus params | signed blocks, empty defaultAssumeValid, elements mode | CONFIRMED |
| Can two nodes diverge on same block bytes via chainparams alone? | Only if **different** params/flags/builds/cache/assumevalid — not if identical stock Liquid | 

---

## 11. “Fork” observation

| Claim | Status |
|---|---|
| Blockstream tip includes 4050336 | CONFIRMED via API `in_best_chain` |
| “mempool.space Liquid stuck” | **UNCONFIRMED** in this workspace (no captured tip dump) |
| Competing block hash at 4050336 | **UNCONFIRMED** |
| Same merkle/tx list on published block | CONFIRMED for the known block |

Do **not** convert explorer disagreement into a proven consensus-split root cause without node logs.

---

## 12. Local reconstruction

Available under `data/` / `final/data/`:

- Header fields + dynafed ext + signblock witness (`block.json`)  
- All 7 tx hexes  
- RP scan (`final/data/block_4050336_tx_rp_scan.json`)

| Validation | Needs full node? |
|---|---|
| Header / merkle / signblock script structure | No (partial; full script verify needs GenericVerifyScript) |
| Per-tx CheckTransaction | No (structural) |
| Rangeproof / tally / surjection | No — native secp (**done**) |
| ConnectBlock UTXO / scripts / active chain | **Yes** — **NON REPRODUIT** |

---

## 13–14. f24a control matrix & first FAIL

| Control | Result |
|---|---|
| CheckTransaction | PASS |
| Fee explicit 58 | PASS |
| Pedersen tally | PASS |
| Rangeproof out0 | PASS |
| **Rangeproof out1** | **FAIL** ← **first FAIL** in VerifyAmounts |
| Rangeproof out2 | PASS (isolated); NOT REACHED if serial abort after out1 |
| Surjection out2 | PASS (isolated) |
| Asset explicit outs | PASS |
| CheckInputScripts | NOT REACHED (amounts fail first) |
| ConnectBlock (honest empty cache) | FAIL |
| CheckBlock | PASS |

No earlier consensus fail found before out1 RP inside amount verification.

---

## 15. Block without f24a (conceptual)

Removing f24a ⇒ new merkle root ⇒ new block identity ⇒ **existing 11 signatures invalid**.  
Shows f24a is **committed** by the signed header, not a relay artifact.

---

## 16. Minimal equivalent transaction

Any tx with: confidential out + invalid RP + tally balanced by that commitment reproduces the **same ConnectBlock reject**.  
Artificial cache A→B still shows OLD key false-positive — **not** historical f24a.

---

## 17. All other txs in 4050336

Native scan: **only f24a:1 fails**. Every other confidential RP in the block **PASS**.

| txid prefix | role | RP anomalies |
|---|---|---|
| b36f… | coinbase | explicit only |
| **f24a…** | | **out1 FAIL only** |
| 090162… | | PASS |
| c652… | 64-in consolidation | PASS |
| efa5… | | PASS |
| 5707… | 14 confidential outs | PASS |
| 2817… | | PASS |

**CONFIRMED:** f24a out1 is the **unique** RP anomaly in the block.

---

## 18. Timeline (ordering only — no forced causality)

```
4050333  0fbde521… creates f24a’s prevout
4050335  271147… / 71c93d… embed C1 bytes in OP_RETURN scripts (NOT P1)
4050336  block e1d9… includes f24a @ index 1  [signed, 11 sigs]
4050337  3875… spends f24a:0
4050343  c6ea… spends f24a:2
4050344  46f117… peg-out 2.65138358
4050349  ce4cae… peg-out 3996.01834922
BTC      8db751… pays both
```

---

## 19. Second-bug search (bypass)

Searched code paths: no OP_RETURN exemption from `CRangeCheck`; fee outputs skip RP only when **explicit**; witness parsing still feeds out1 proof into verify.  
**No concrete second bug that accepts failed verify found.** Status: **UNCONFIRMED**.

---

## 20. H7

H7 only if **two independent** mechanisms are **necessary**.  

Minimal gap is **one**: “why did VerifyAmounts succeed for someone / why is block in a best chain?”  
Subsequent peg-outs need only **valid spendable outs** (out2 path) after that acceptance.  

→ Prefer **single unexplained acceptance**, not mandatory dual-bug H7. H7 stays a **bucket** for unknowns, score moderated.

---

## 21. Functionary compromise

| Evidence | Status |
|---|---|
| 11 signatures on 4050336 | CONFIRMED |
| Abnormal signing pattern | UNCONFIRMED |
| PAK violation on f24a | N/A (not pegout) |
| Key misuse | UNCONFIRMED |

**Signed block ≠ compromised functionary.**

---

## 22. Central question — how can 4050336 exist if ConnectBlock(f24a)=FAIL?

| Code | Meaning | Status for this incident |
|---|---|---|
| **A** | validation bypassed | No stock bypass for tip; REFUTED as default-path explanation |
| **B** | cache/prior state | Theoretical yes; historical priming **REFUTED** / unsupported |
| **C** | different version | UNCONFIRMED |
| **D** | different configuration | UNCONFIRMED (assumevalid tip: cannot explain default) |
| **E** | validation bug | PLAUSIBLE category; instance **UNCONFIRMED** |
| **F** | functionary/producer compromised | UNCONFIRMED |
| **G** | error in our reproduction | PROBABLE low: multi-verify FAIL; still possible unknown context |
| **H** | other (e.g. indexer ≠ full ConnectBlock; incomplete public fork data) | PLAUSIBLE |

**Answer: unknown.** Best-supported framing:  
**CONFIRMED paradox** between (honest empty-cache ConnectBlock reject) and (signed block + explorer best-chain).  
Mechanism ∈ {B,C,E,G,H} **not singled out**.

---

## 23. Updated scores /100

| H | FOR | AGAINST | Missing | Score |
|---|---|---|---|---:|
| **H1** cache | Bug real; artificial A→B | No A; no P1 preimage; unique fail still needs accept path | Any mempool log with (P1,C1) | **15** |
| **H2** other CT/RP | Unique bad RP in block; Sep CT commits | No accepting bug identified | Concrete patch | **36** |
| **H3** accounting | C1 in tally | No surplus; tally OK | blinds | **12** |
| **H4** consensus/version | CheckBlock≠Connect; possible node divergence | Same reject if same binary+empty cache | Node version inventory | **44** |
| **H5** other Elements | Near-date commits | None shown to skip failed RP | full audit | **30** |
| **H6** functionary/producer | **11 sigs CONFIRMED**; can sign without amounts | ≠ compromise; no intent proof | signer validation policy | **55** |
| **H7** combination | Residual bucket | Not shown that **two** bugs are required | — | **40** |

**Most consistent (not confirmed):** **H6-weighted unexplained inclusion** — a federation-signed block committed f24a; honest amount verification fails; **acceptance mechanism unknown**. Cache not selected.

---

## 25. Minimal fact/bug set

To cover observations **without elegant fiction**:

1. **CONFIRMED:** f24a out1 RP fails under L-BTC+`6a`; unique RP fail in 4050336.  
2. **CONFIRMED:** Stock ConnectBlock + empty cache must reject f24a.  
3. **CONFIRMED:** 4050336 is federation-signed (~11 sigs) and listed in Blockstream best chain.  
4. **CONFIRMED:** Peg-outs spend **f24a:2** (valid RP), not f24a:1.  
5. **REFUTED (historical):** cache priming of `(P1,C1)` as demonstrated cause.  
6. **UNCONFIRMED:** the single bridge between (2) and (3).

**No single named mechanism currently satisfies all five simultaneously.**  

Minimal set =  
`{unique invalid RP in signed block}` + `{unexplained best-chain acceptance}` + `{normal spends of valid outs → peg-outs}`  

Not: `{cache priming}` + `{Pedersen inflation}` + `{functionary key compromise}`.
