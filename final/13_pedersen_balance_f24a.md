# 13 — Pedersen conservation analysis for f24a

## Summary

`secp256k1_pedersen_verify_tally` on the real commitments of `f24a` returns **TRUE** with out1 included and **FALSE** without it. Out1 is therefore **algebraically required** for conservation, despite being an unspendable OP_RETURN that is never spent later. Its cleartext value remains **non déterminable** (blind unknown). Numeric inflation is **not demonstrated**; structural dependence of the balance on an out-of-range-unproven commitment is **demonstrated**.

Log: `final/logs/pedersen_tally_f24a.log`  
Harness: `tests/pedersen_tally_f24a.c`

---

## 1–3. Commitments and generators (FACT)

### Input (1)

| | |
|---|---|
| prevout | `0fbde521…3b95:2` |
| value commitment | `09121a4e92717097b9562e2cf3cdbee843a55f819074f6c0e11a29c46874fcde3d` |
| asset generator (blinded) | `0abc38a76bc56788e46f7c911f7863aa4926e2718fd3823166fe5f7bd66f1278df` |
| issuance | none |

### Outputs (4)

| out | kind | value | asset / generator |
|---|---|---|---|
| 0 | confidential / explicit L-BTC asset | commit `08360f95ce0a63e76daab6a05616103ba7462a9af7db0697e9c8fa6a65aba103f8` | explicit L-BTC → gen `0a0a488de4899d0ae757f6cf8368663184d164106111ed9eaecf510e35282ddc6d` |
| **1** | confidential / explicit L-BTC / OP_RETURN | **C1** `086f5d67160fc4b477954fb09ef321e5b589d7a07740a1a6df494ed2335b1d01d8` | same L-BTC generator |
| 2 | confidential / blinded asset | commit `08dc50cfb1c9b0b512b446cd80d8bbe81f5ace540f59780d6fad773fa9204d8d35` | asset commit `0b0957be4cbd0d1cc30d2742988f84f931f4f47d4f8b81e40dc1db42cabf4aae7b` |
| 3 | **explicit fee** | **58** sats | explicit L-BTC (same gen); Elements builds commit with **zero blind** |

Derived fee commitment (executed):  
`09118f5365192ef32379842b3d9ea61fdd120f655bc7376de41826e7cba4030fe2`

No issuance pseudo-inputs.

---

## 4–5. Conservation equation as Elements implements it

From `VerifyAmounts` (`confidential_validation.cpp`):

1. Build list **In** = parsed/derived Pedersen commitments of all inputs (+ issuances).  
2. Build list **Out** = commitments of all outputs (explicit amounts → `pedersen_commit(v, blind=0, gen)`).  
3. **Value balance (EC):** `secp256k1_pedersen_verify_tally(In, Out)`  
   i.e. as curve points:  
   `Σ C_in = Σ C_out`  
4. **Separately:** rangeproof for every confidential value.  
5. **Separately:** surjection for every blinded asset.  
6. Fee is just another explicit output in the tally (here 58 L-BTC).

There is **no** separate integer “value balance” check for confidential amounts without opening. Asset balance for blinded assets is enforced via **surjection**, not via the tally alone.

### Separation

| Component | Role in f24a |
|---|---|
| Value balance (Pedersen tally) | 1 input commit vs out0+out1+out2+fee58 |
| Fee | explicit 58 under L-BTC gen, zero blind |
| Asset balance | out0/out1 explicit L-BTC; out2 blinded + surjection vs input gen |
| Explicit outputs | fee only (value); assets out0/out1/fee explicit |
| Confidential outputs | out0, out1, out2 (values) |

---

## 6. Is the equation satisfiable with present proofs? (FACT — CODE EXECUTION)

| Check | Result |
|---|---|
| Pedersen tally **with** C1 (out1) | **TRUE (1)** |
| Pedersen tally **without** C1 | **FALSE (0)** |
| Rangeproof out0 | TRUE |
| Rangeproof out1 (P1) under L-BTC+`6a` | **FALSE** |
| Rangeproof out2 | TRUE |
| Surjection out2 vs `[input asset gen]` | **TRUE (1)** (67 B proof) |

So: the **elliptic-curve conservation equation holds** for the published commitments.  
The **rangeproof suite does not** (out1 fails).  
`VerifyAmounts` requires **both** (plus surjections). With empty cache, f24a fails on out1’s rangeproof even though the tally is fine.

---

## 7. What out1 does to the equation (without inventing v)

**FACT:** Removing out1’s commitment from the RHS makes the tally fail.  
Therefore C1 is **not optional noise**: it is a necessary term in  
`C_in = C_out0 + C1 + C_out2 + C_fee58` (as group elements).

**NON DÉTERMINABLE:** the scalar `v` (or “negative / huge” interpretation) of C1 under L-BTC generator, because `C = v·G_asset + r·H` and `r` is unknown.  
We do **not** assign a numeric value to out1.

---

## 8–10. Dependence on out1 rangeproof

| Question | Answer |
|---|---|
| Does the **tally** depend on P1? | **No.** Tally uses only commitment points. |
| Does **tx validation** (`VerifyAmounts`) depend on out1 rangeproof? | **Yes.** Confidential outs must pass `CRangeCheck` / `CachingRangeProofChecker`. |
| Can an invalid rangeproof make an otherwise impossible conservation accept? | **No for the tally itself** — the tally already holds with these points. The rangeproof’s job is to ensure each confidential commit is an **in-range** opening under its generator. Skipping P1 does **not** invent a new tally; it **drops the in-range constraint** on a commit that is already in the tally. |
| Could that enable inflation? | **Structurally yes:** if C1 opens to a value outside `[0,2^64)` (or inconsistent with a positive L-BTC amount), spendable outs can still sum with it on the curve while representing more “effective” spendable value. **Magnitude: non déterminable.** |

---

## 11. Comparison of validation modes

| Mode | Tally | Out0 RP | Out1 RP | Out2 RP | Net `VerifyAmounts` |
|---|---|---|---|---|---|
| Correct crypto, empty cache | PASS | PASS | **FAIL** | PASS | **FAIL** |
| Correct crypto + OLD cache primed with (P1,C1) under some A | PASS | PASS | **cache TRUE** | PASS | **PASS** (if A existed) |
| Validation “without P1” (omit out1 RP check only) | PASS | PASS | skipped | PASS | would PASS crypto suite except policy — **not** how Elements works |
| Omit out1 from tally | **FAIL** | — | — | — | FAIL |

---

## Importance of out1 (given it is unspent)

| Role | Verdict |
|---|---|
| Own UTXO later spent into peg-outs | **No** (OP_RETURN, unspent) |
| Transaction validation (rangeproof required) | **Yes — blocker under honest verify** |
| Pedersen bilan | **Yes — necessary commitment term** |
| Elsewhere (marker only) | Insufficient: tally fails without it |

**Primary importance: bilan + validation, not downstream UTXO liquidity.**

---

## 12. Conclusion

**B. inflation seulement suspectée**

Justification:
- Conservation EC **holds** and **requires** C1 — FACT.  
- P1 **fails** under the claimed L-BTC context — FACT.  
- Accepting the tx despite that would remove the in-range guarantee on a balance-critical commit — structural inflation risk — FACT/INFERENCE.  
- **No numeric surplus** of L-BTC is computed — blinds unknown — **non déterminable**.  
- Therefore inflation is **not réellement démontrée (A)** and not **aucune (C)**; not merely “invalid for unrelated reasons (D)” because the tally path is coherent and the failing check is exactly the rangeproof tied to that balance term.
