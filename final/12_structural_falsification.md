# 12 — Structural falsification of the cache→f24a hypothesis

## Executive structural finding

The cache bug is real and process/mempool-wide. Accepting f24a via that bug **structurally requires** a prior successful `VerifyRangeProof(P1, C1, …)` with `store=true` (typically mempool), because the pre-fix key is exactly `H(salt || P1 || C1)`.

**No such prior call is evidenced on-chain:** P1 bytes appear **only** inside f24a in all scanned material (local + blocks 4050320–4050339 and 4050330–4050336). Transactions `71c93d43…` / `27114710…` are therefore classified **NOT PRIMING OF P1/C1**.

Separately, f24a **is** on the UTXO path to the peg-outs via **out2** (valid proof), while the invalid **out1** is an unspendable OP_RETURN that can still affect the Pedersen tally if its rangeproof were wrongly accepted.

---

## 1. Reverse engineering — exact pre-fix objects

| Item | Exact location |
|---|---|
| Class | `CachingRangeProofChecker` |
| Header | `src/script/sigcache.h` |
| Impl | `src/script/sigcache.cpp` |
| Constructor | `CachingRangeProofChecker(bool storeIn) { store = storeIn; }` |
| Verify | `bool VerifyRangeProof(proof, valueCommitment, assetCommitment, scriptPubKey, ctx)` |
| Lookup | `rangeProofCache.Get(entry, !store)` |
| Insert | `rangeProofCache.Set(entry)` only if `store` and crypto+policy succeed |
| Key (pre-`c26d719`) | salted SHA256 over **`proof \|\| value_commitment` only** |
| Backing store | anonymous-namespace `static SignatureCache rangeProofCache` |
| Create/init | `InitRangeproofCache` from `src/init.cpp` (AppInit) |
| Destroy | process exit (no explicit destroy; static lifetime) |
| Duration | **RAM, process lifetime**; restart clears |

### Call graph (FACT)

```
Mempool accept (AcceptToMemoryPool / PreChecks path)
  validation.cpp ~1100: CheckTxInputs(..., cacheStore=true, ...)
    consensus/tx_verify.cpp: VerifyAmounts(..., cacheStore)
      confidential_validation.cpp: for each confidential vout
        QueueCheck(..., new CRangeCheck(val, vtxoutwit[i].vchRangeproof, assetCommitment, scriptPubKey, store_result))
          CRangeCheck::operator()
            CachingRangeProofChecker(store).VerifyRangeProof(...)
              ComputeEntryRangeProof → Get → [miss] secp256k1_rangeproof_verify → [ok] Set

ConnectBlock (actual connect, fJustCheck=false)
  validation.cpp ~3041: fCacheResults = fJustCheck  // false
    CheckTxInputs(..., cacheStore=false, ...)
      same VerifyAmounts path
        VerifyRangeProof with store=false
          Get(entry, erase=true)  // consults cache; may erase on hit
          on miss: crypto; on success: does NOT Set
```

Issuance path also calls `CRangeCheck` with **empty script** and issuance rangeproof bytes (`VerifyIssuanceAmount`) — still requires those exact proof+commitment bytes to insert a key.

---

## 2. `secp256k1_rangeproof_verify` arguments

Signature (Elements secp header):

```
int secp256k1_rangeproof_verify(
  const secp256k1_context *ctx,
  uint64_t *min_value,          // OUT
  uint64_t *max_value,          // OUT
  const secp256k1_pedersen_commitment *commit,  // from value commitment
  const unsigned char *proof, size_t plen,
  const unsigned char *extra_commit, size_t extra_commit_len,  // scriptPubKey bytes
  const secp256k1_generator *gen  // from asset commitment / derived generator
);
```

| Argument | Influences crypto? | OLD cache key? | NEW cache key? |
|---|---|---|---|
| `ctx` | setup only | no | no |
| `commit` (value commitment) | YES | YES | YES |
| `proof` / `plen` | YES | YES | YES |
| `extra_commit` (= scriptPubKey) | YES | **NO** | YES |
| `gen` (asset generator) | YES | **NO** | YES |
| `min_value`/`max_value` | outputs; Elements then applies policy `min==0 && !IsUnspendable` | no | no |
| Output `nNonce` field | **not passed** to verify | no | no |

---

## 3. Exact prerequisite to insert `H(P1||C1)`

**FACT:** Pre-fix key ignores generator and script. Therefore any successful verify of **exactly** the byte strings `(P1, C1)` inserts the same entry, regardless of context.

**FACT:** The only production call sites pass:
- `rangeproof = tx.witness.vtxoutwit[i].vchRangeproof` (or issuance witness proofs)
- `value commitment = tx.vout[i].nValue.vchCommitment` (or issuance amount commitment)

Therefore a priming transaction **must present P1 as that output’s rangeproof field and C1 as that output’s value commitment** (or the issuance analogues). Embedding C1 inside an OP_RETURN **script** does not call `VerifyRangeProof(P1,C1)`.

**FACT — store flag:** Insertion requires `store=true`. Mempool validation uses `true`. Actual block connect uses `false` (consult only; `Get(..., erase=true)`).

So historical priming of f24a via cache implies: some prior **mempool** (or other store=true) validation of a tx carrying **raw P1+C1**.

---

## 4. Cache scope — definitive

| Property | Verdict |
|---|---|
| Transaction-local only? | **NO** |
| Mempool-wide / process-wide? | **YES** — `static SignatureCache rangeProofCache` |
| Shared mempool ↔ block validation? | **YES** — block connect consults same static cache |
| Persist to disk? | **NO** |
| Insert on block connect? | **NO** (`store=false`) |
| Erase on block-connect lookup? | **YES** (`Get(entry, !store)` ⇒ erase when store=false) |

**Classification: D/E — process-wide, shared across validations; primed primarily by mempool path.**

---

## 5. Indirect priming

Can X insert `H(P1||C1)` without containing raw P1?  
**NO** under the Elements call graph above, unless X still passes those exact vectors into `VerifyRangeProof` (no other serializer invents them).

Therefore: **X must contain P1 bytes as a rangeproof argument and C1 as the commitment argument.**

---

## 6. Classification of 4050335 candidates

| tx | Contains C1? | Contains P1? | Can call Verify(P1,C1)? | Classification |
|---|---|---|---|---|
| `71c93d43…f411` | in OP_RETURN script only | **NO** | **NO** | **NOT PRIMING OF P1/C1** |
| `27114710…7ec5` | in OP_RETURN script only | **NO** | **NO** | **NOT PRIMING OF P1/C1** |

Their own out0 proofs verify under L-BTC+long script, with **different** commitments — inserts `H(P'||C') ≠ H(P1||C1)`.

---

## 7. Exhaustive P1 search (accessible material)

| Scope | P1 present? |
|---|---|
| Local workspace hex/json | only f24a / derived out1 artifacts |
| Blocks 4050330–4050336 | **only f24a** |
| Blocks 4050320–4050329, 4050337–4050339 | **no P1** |

**FACT:** No pre-f24a on-chain occurrence of P1 found in the accessible window.

⇒ Historical mempool-only priming remains **possible in theory** but **UNCONFIRMED** (no mempool archive).

---

## 8. Acceptance matrix for f24a’s critical check (out1)

| Situation | Native crypto out1 | OLD checker return | Notes |
|---|---|---|---|
| cache empty + f24a | FALSE | **FALSE / REJECT** | executed |
| cache primed with true context A for (P1,C1) + f24a | FALSE | **TRUE / ACCEPT** | artificial A→B only; **no historical A** |
| primed with wrong context (other proof) | — | no effect on H(P1\|\|C1) | key mismatch |
| primed with P1 + other commitment | — | no effect | key mismatch |
| primed with other proof + C1 | — | no effect | key mismatch |
| NEW/fixed cache + any priming of A + f24a | FALSE | **FALSE** | different keys | 
| store=false block connect, empty cache | FALSE | FALSE | executed path semantics |

**Minimal condition for OLD cache to accept out1:** prior successful verify of **exact (P1,C1)** with store=true.

---

## 9–10. Artificial / mathematical context A for P1

Rangeproof verification binds the Fiat–Shamir statement to `(commit, generator, extra_commit)`.  
Given fixed P1 and C1, a satisfying `(G*, S*)` is essentially the **original proving statement** (or a soundness break).

- Related-asset/script search: **0/128**
- No constructive derivation of G* without prover secrets
- Claiming “must exist” without evidence is forbidden

**Operational conclusion:** For incident analysis, treat **“P1 has a usable alternate context A” as UNCONFIRMED / not evidenced**.  
If none exists, the scenario “valid A → poison B for this P1” **cannot explain f24a**.

---

## 11. All confidential outs of f24a

| out | commitment | proof SHA256 | native verify | role |
|---|---|---|---|---|
| 0 | `08360f95…03f8` | `3fe0fb01…1321` | **TRUE** | spendable; spent by `3875a6d6…` |
| **1** | **C1** `086f5d…01d8` | **P1** `6619fa29…98b2` | **FALSE** | OP_RETURN; **unspent**; tally participant if accepted |
| 2 | `08dc50cf…8d35` | `a33fe028…aa93` | **TRUE** | spendable; **feeds peg-out chain** |
| 3 | explicit fee 58 | none | n/a | fee |

Cache exploit, if any for this tx, concerns **out1 / P1 only** among these proofs.

---

## 12. Reconstructed validation of f24a (without full node)

Must pass (among Elements confidential checks):
1. `HasValidFee`
2. `VerifyAmounts` → Pedersen tally over inputs/outputs **including C1**
3. Rangeproof checks for outs 0,1,2
4. Surjection checks where asset blinded (out2)
5. Scripts / BIP68 / standardness (mempool)

**FACT:** Out1 rangeproof fails native verify under real context → with empty cache, `VerifyAmounts` fails → tx consensus-invalid.  
**Other checks:** not fully re-executed in-process; no evidence they independently fail once amounts validate.  
**INFERENCE:** the rangeproof on out1 is the clear blocker we can demonstrate.

---

## 13. Is f24a the primary exploit tx?

| Reading | Status |
|---|---|
| A. Primary exploit vehicle (invalid out1 enables bad tally → inflated spendable value in out0/out2) | **PLAUSIBLE** structurally; **not proven** without showing tally inflation numerically |
| B. Result of already-corrupt prior state | **UNCONFIRMED** — no earlier P1 |
| C. Marker/test only | **UNLIKELY** — sits on peg-out UTXO path via out2 |
| D. Normal tx with one bad output | compatible with A |

**IMPORTANT FACT — UTXO link (new):**

```
f24a:2  →  c6ea588a… (vin3)
c6ea588a:0  →  46f117… (vin11)
46f117:1  →  ce4cae…
ce4cae peg-out + 46f117 peg-out  →  BTC 8db751…
```

f24a:**1** (bad proof) is **not** spent into peg-outs.  
Economic exit uses f24a:**2** (valid proof) plus many other 46f117 inputs.

So f24a can still matter if out1’s commitment **distorts the confidential balance**, allowing out2 (or others) to carry more L-BTC than inputs justify — that is the classic CT inflation pattern. **Opening blinds to quantify that inflation: NOT DONE.**

---

## 14–15. Peg-outs & consensus split

- Peg-out linkage amounts: **CONFIRMED**
- Causal “cache bug ⇒ these BTC”: **UNCONFIRMED**
- Dual-node split on 4050336: **UNCONFIRMED / NON REPRODUIT**

Code-level split mechanism (primed mempool vs clean/IBD) remains **PLAUSIBLE** given `store=true` mempool vs `store=false` connect semantics.

---

## 16. Other bugs / secp PRs

- PR #369 surjection nonce: not shown tied to f24a out1 (explicit asset, empty surjection)
- PR #370 rangeproof nonce API warning: prover hygiene; does not make verify return TRUE for P1 under L-BTC
- No alternate consensus bug demonstrated for this tx

---

## 17. Verdict table (falsification pass)

| | Question | STATUS |
|---|---|---|
| A | Cache bug exists? | **CONFIRMED** |
| B | Exploitable (A→B)? | **CONFIRMED** (artificial) |
| C | P1 invalid under L-BTC? | **CONFIRMED** |
| D | Context A for P1/C1 exists? | **UNCONFIRMED** (0 evidence; 128 misses) |
| E | Can make f24a accept via cache? | **PLAUSIBLE iff D**; else **structurally blocked** |
| F | f24a likely used this bug? | **UNCONFIRMED** (needs D + mempool priming) |
| G | Explains 4050336 split? | **UNCONFIRMED** |
| H | Explains peg-outs? | **PLAUSIBLE linkage via out2**; causal CT inflation **UNCONFIRMED** |
| I | Functionaries must be compromised? | **NOT REQUIRED** in the poisoned-consensus scenario (**PLAUSIBLE**) |
| J | Root cause of 6 Sep incident? | **UNCONFIRMED** |

---

## 18. Category choice

**3. "Vulnerability demonstrated, causal link to incident unproven"**

Rationale: mechanism confirmed; historical priming of `(P1,C1)` not found and structurally must include raw P1 before f24a; no full-node accept/reject reproduction; P1 absent from accessible pre-f24a chain data.

### Mandatory negatives (scientific successes)

1. **71c93d43 / 27114710 cannot prime P1/C1** — classified NOT PRIMING.  
2. **P1 not found before f24a** in accessible blocks/files.  
3. **Empty cache rejects f24a out1** — executed.  
4. **Without a real context A, artificial priming of f24a’s out1 cannot be performed with this P1** — only with a different proof constructed for a known A.
