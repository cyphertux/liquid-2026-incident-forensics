# 15 — Validation divergence: why is f24a in block 4050336?

## Scientific stance

Two tracks already **falsified** as explanations of the incident via f24a:
1. No context A making `(P1,C1)` cryptographically valid.
2. No Pedersen inflation demonstrated from public commitments.

This note **does not rehabilitate the cache→f24a story**.  
It asks: *given that honest `secp256k1_rangeproof_verify(P1,C1,G_L,OP_RETURN)=FALSE`, how can f24a appear in a signed Liquid block?*

**Valid outcome (allowed):** the cache bug is a real vulnerability but **probably does not explain f24a**.

---

## 1. Call graph — validation of f24a

```
AcceptBlock / ProcessNewBlock
  CheckBlock(block)                          # context-free
    CheckTransaction(tx)                     # tx_check.cpp
      vin/vout non-empty, size, explicit amounts, dup inputs
      *** does NOT call VerifyAmounts / rangeproofs ***
  ContextualCheckBlock(...)                  # bip34/witness/etc.
  ...
ConnectBlock(block, ..., fJustCheck)
  [optional] assumevalid → may set fScriptChecks=false
  for each non-coinbase tx:
    Consensus::CheckTxInputs(..., cacheStore=fJustCheck, fScriptChecks)
      HaveInputs / maturity / pegin
      HasValidFee
      if (fScriptChecks) VerifyAmounts(spent_inputs, tx, pvChecks, cacheStore)
        build Pedersen commits (in + out + fee)
        QueueCheck CBalanceCheck          → pedersen_verify_tally
        for each confidential vout:
          QueueCheck CRangeCheck          → CachingRangeProofChecker::VerifyRangeProof
            cache Get(key); on miss: secp256k1_rangeproof_verify
            policy: min_value==0 && !IsUnspendable → fail
        for each blinded asset:
          QueueCheck CSurjectionCheck
    SequenceLocks
    if (fScriptChecks) CheckInputScripts(...)  # ECDSA/Schnorr witnesses
  PAK (consensus): IsPAKValidTx when enforce+dynafed  # only explicit peg-asset pegouts
  control.Wait()  # parallel CCheck queue must succeed
```

Mempool (`AcceptToMemoryPool` / `PreChecks` path ~1100):

```
CheckTransaction
Contextual checks / standardness
CheckTxInputs(..., cacheStore=true, fScriptChecks=true)
  → VerifyAmounts(..., store_result=true)
PAK policy (post-779e71): HasConfidentialPegoutOutput → TX_NOT_STANDARD
```

### Predicted PASS/FAIL for f24a (honest crypto, empty cache)

| Step | Result |
|---|---|
| CheckTransaction | **PASS** |
| ContextualCheckTransaction / ContextualCheckBlock (structure) | **PASS** (expected; not re-executed here) |
| HasValidFee (58) | **PASS** |
| Pedersen tally | **PASS** |
| Rangeproof out0 | **PASS** |
| Rangeproof out1 (P1) | **FAIL** ← first crypto failure |
| Rangeproof out2 | **NOT REACHED** if checks are sequential in-thread; with parallel queue, queued then fails overall |
| Surjection out2 | **PASS** (executed in isolation) |
| Script CheckInputScripts (vin0 P2WPKH) | would **PASS** if reached (signature present; not the failing locus) |
| ConnectBlock overall | **FAIL** (`bad-txns-in-ne-out` / rangeproof) |
| Mempool accept | **FAIL** (same VerifyAmounts) |

---

## 2. First point of divergence

**Exact rejection locus (honest implementation):**

`confidential_validation.cpp` → `CRangeCheck::operator()` →  
`CachingRangeProofChecker::VerifyRangeProof` →  
`secp256k1_rangeproof_verify(... commit=C1, proof=P1, extra_commit=0x6a, gen=G_L) == 0`

**Is this called on the block path?**  
**YES**, whenever `fScriptChecks == true` inside `ConnectBlock` → `CheckTxInputs` → `VerifyAmounts`.

Liquid mainnet (`CLiquidV1Params`): `consensus.defaultAssumeValid = uint256()` (**empty**).  
So default tip sync does **not** skip script/amount checks via assumevalid.  
(User `-assumevalid=` could still skip deep history; **not** a default tip explanation.)

Therefore: a stock Elements/Liquid node connecting tip block 4050336 **should reject f24a at out1 rangeproof**.

---

## 3. Mempool vs ConnectBlock

| | Mempool | ConnectBlock (`fJustCheck=false`) |
|---|---|---|
| `fScriptChecks` | `true` | `true` (default Liquid) |
| `cacheStore` / `store` | **`true`** (insert on success) | **`false`** (consult; `Get(..., erase=true)`; no Set) |
| VerifyAmounts | yes | yes |
| PAK confidential pegout | policy reject (after 779e71) | **not** this policy; consensus PAK only explicit peg-asset |

### Could f24a be…

| Option | Verdict |
|---|---|
| **A.** mempool accept then mined | **Only if** VerifyAmounts passed in mempool (valid RP **or** cache hit). Empty-cache crypto → **no**. |
| **B.** mempool reject but block accept | **No** under same software: ConnectBlock re-runs VerifyAmounts. Both fail or both succeed (modulo cache state / assumevalid / different binaries). |
| **C.** prior cache state | **Structurally the only pure-Elements way** for invalid P1 to pass — but **historical priming of (P1,C1) not found** (falsified for claimed primers). |
| **D.** different mempool vs block treatment | **Only** `store` / erase differ; **not** whether crypto runs on miss. |

**CRITICAL:** CheckBlock **does not** verify rangeproofs. A block containing f24a can look “fine” until ConnectBlock. That enables **header/block relay vs connect divergence**, not silent accept on a verifying node.

---

## 4. Validation flag differences (summary)

| Flag / concern | Mempool | Block connect |
|---|---|---|
| cacheStore | true | false (`fCacheResults = fJustCheck`) |
| erase on cache hit | `!store` → false (keep) | `!store` → true (consume) |
| script flags | standard + consensus | consensus (`flags` from SoftForkDeployments) |
| rangeproof | always via VerifyAmounts if fScriptChecks | same |
| asset / surjection | same | same |
| PAK confidential asset pegout | policy (post-779e71) | consensus gap remains (see §6) |
| assumevalid skip | N/A | can set fScriptChecks=false (Liquid default: off) |

---

## 5. Version matrix (native crypto + code reading)

Full `elementsd` Accept/Connect **NON REPRODUIT** (no full node build). Native `secp256k1_rangeproof_verify` / tally **reproduced**.

| Version | Cache key | P1 crypto (L-BTC+`6a`) | Tally | Mempool (empty cache) | Block connect (empty cache) |
|---|---|---|---|---|---|
| Historical vulnerable (pre-c26d, e.g. **23.3.3**) | OLD: `H(P\|\|C)` | FAIL | PASS | **REJECT** | **REJECT** |
| **c26d719c29** (master, CommitDate 2026-09-01) | NEW: +asset+script | FAIL | PASS | **REJECT** | **REJECT** |
| **6253d7e103** (23.x cherry-pick) | NEW | FAIL | PASS | **REJECT** | **REJECT** |
| **212c43f475** (23.3.x cherry-pick, CommitDate 2026-09-03) | NEW | FAIL | PASS | **REJECT** | **REJECT** |

**Difference NEW vs OLD for f24a:** none on a clean cache. Both reject.  
NEW only changes cross-context false positives **if** a valid priming context existed (not found).

---

## 6. Other commits 2026-08-25 → 2026-09-07 (not assuming c26d is the cause)

| Commit | Date (commit) | Touches | Relevance to f24a accept |
|---|---|---|---|
| c26d719c29 | 2026-09-01 | rangeproof **cache key** | Fix for *poisoning*; does not make P1 valid |
| 4e5ca94f6b / #1593 | 2026-09-01 | blind/RPC errors on bad RP | Wallet/RPC hygiene; **not** consensus skip |
| 779e71f545 | 2026-09-01 | **PAK**: reject confidential-asset pegouts in **mempool** | Peg-out policy; **f24a is not a pegout**. Consensus still ignores confidential peg-asset in `IsPAKValidOutput` |
| 34be37f34d / 6f1a51310b / a7a88214f7 / blindpsbt series | 2026-09-01 | BlindProof / surjection hardening | Construction-time; not ConnectBlock RP verify |
| 1051a8959f (#1584) | 2026-08-27 | SIGHASH_ALL_WITH_RANGEPROOF defaults | Signing; not verification of P1 |
| c7ec63c583 | 2026-09-01 | dynafed header block_height | Unrelated to CT amounts |
| 55ca02913f | 2026-08-14 | secp256k1 subtree bump | **Unknown** whether behavioral change on rangeproof; **not tested** against P1 |

No commit in this window was shown to **skip** `CRangeCheck` on ConnectBlock.

---

## 7. Fork / block 4050336

| Fact | Value |
|---|---|
| Height 4050335 | `aad24e4f…d66b`, 5 txs, merkle `26ea7d08…167c` |
| Height 4050336 | `e1d9a2aa…a0d5`, **7 txs**, merkle `f9a463b6…104b`, prev = 4050335 |
| Contains f24a | **YES** (confirmed earlier) |
| Liquid consensus | **signed blocks** (`g_signed_blocks=true`, 11-of-15 style signblockscript) |

**Unknown (not reproduced here):** which functionary set signed 4050336; whether any public node log shows ConnectBlock reject; whether a competing block at 4050336 existed; Elements version on functionary hosts; identical secp builds.

Same block hash / merkle / tx list are properties of **that** published block. Divergence among nodes would be **accept vs reject of the same block**, not a different merkle — unless a competing tip existed (**non déterminable** here).

---

## 8. Matrix (condensed)

`version | mempool | block | rangeproof out1 | tally | result`

```
23.3.3 (vuln cache)     | REJECT* | REJECT* | FAIL | PASS | reject tx/block
+c26d719 / 6253 / 212c  | REJECT* | REJECT* | FAIL | PASS | reject tx/block
* empty cache, fScriptChecks=true, native verify
with OLD cache + mythical prime of (P1,C1) | ACCEPT | ACCEPT | cache hit | PASS | accept — NOT HISTORICALLY SHOWN
```

---

## 9. Why does P1 fail? (no forced cause)

| Hypothesis | Evidence | Status |
|---|---|---|
| Bad value (v1 out of range) | Compatible with failed verify; **not proven** | possible |
| Wrong generator | Fail under G_L; also fail under tested alts | context not found |
| Wrong script/extra_commit | Fail with `6a`, empty, P2WPKH | context not found |
| Wrong commitment | C1 is the wire commitment; info parses | unlikely “wrong C” |
| Corrupted proof bytes | `rangeproof_info` **OK**; structure present | not mere truncation |
| Valid format, false statement | info OK + verify FAIL | **consistent** with forged/mismatched proof |
| Other | — | open |

**Header anomaly (FACT):**  
out0/out2 RP prefix `6033000000000000…` (len 4174);  
out1 prefix `4033d8c5cf01ae0a…` (len **4234**, +60).  
Different encoding / message / exp path — **structural difference**, not a diagnosis of “negative value”.

---

## 10–11. Structure of out1 vs out0/out2 / normal OP_RETURN

| Field | out0 | **out1** | out2 |
|---|---|---|---|
| script | P2WPKH | **OP_RETURN `6a`** | P2WPKH (same hash160 as out0) |
| asset | explicit L-BTC | explicit L-BTC | blinded |
| value | confidential | confidential **C1** | confidential |
| nonce | **`00`** | **`00`** | 33-byte blinding pubkey |
| RP | 4174 B, valid | **4234 B, invalid** | 4174 B, valid |
| surjection | empty | empty | 67 B, valid |

Anomalies of out1: unspendable + confidential + null nonce + **unique RP shape** + verify FAIL.  
out0 also has null nonce (cannot ECDH-unblind from nonce alone) but **valid** RP.

Normal OP_RETURN+CT comparables: **not collected in this pass** (API reset); treat “is out1 only an invalid proof?” → **at minimum yes (invalid proof)**; additional structural oddities listed above; **not** proven to be a second bug class.

---

## 12. Is f24a the inflation origin?

| Role | Verdict |
|---|---|
| Creates numeric surplus L-BTC | **Not demonstrated** |
| Inserts commitment C1 into UTXO set / tally | **Yes** (OP_RETURN still in tx effects via VerifyAmounts) |
| Intermediate / prepares later spends | **out0** spent in `3875a6d6…` (h 4050337); **out2** spent in `c6ea588a…` (h 4050343) → peg-out path; **out1 unspent** |
| Necessary property for peg-out path | **Spendable outs (esp. out2)** with **valid** proofs; out1 not spent |

Do **not** treat out1 as the proven inflation source.

---

## 13. Graph before f24a

```
ab1bcdbace… (multiple outs)
  → 0fbde521… @ height 4050333 (7 in / 5 out; fee 159)
       vout2: confidential value 09121a4e… / asset 0abc38a7…  (same address family ex1q7kgx4…)
         → spent by f24a:0 @ 4050336
```

All relevant amounts confidential. Same P2WPKH script appears on f24a out0/out2.  
**No** cleartext creation anomaly recoverable without blinds. Recent (2–3 blocks before f24a).

---

## 14. Graph after f24a

```
f24a:0 → 3875a6d6… (vin0) @ 4050337
f24a:1 → UNSPENT (OP_RETURN)
f24a:2 → c6ea588a… (vin3) @ 4050343
           → … → 46f117… peg-out 2.65138358
                → ce4cae… peg-out 3996.01834922
                  → BTC 8db751… pays both
```

**Necessary for continuation:** acceptance of **f24a as a whole** into the UTXO set (so out0/out2 exist). That acceptance, under Elements, requires **passing VerifyAmounts**, including out1’s rangeproof check — even though out1 is never spent.

---

## 15. Alternative hypothesis test

> “Arbitrary balanced commitments + other validation defect”  
> rather than “rangeproof cache poisoning”?

| Prediction | Result |
|---|---|
| Tally can hold with weird C1 | **TRUE** (observed) |
| Honest RP verify still required | **TRUE** in code |
| Cache poisoning needed for accept | **Only known pure-cache path**; historically **unsupported** |
| Other defect skipping CRangeCheck | **Not found** in ConnectBlock path |
| CheckBlock-only trust / functionary assemble without connect semantics | **Plausible organizationally**; **not proven** |

**Hypothesis status:** open, not confirmed. Stronger than “priming we never found” only as a *category* (non-cache bypass), not as a named bug.

---

## 16. Explicit non-rehabilitation

**The cache bug is a real vulnerability but does not currently explain f24a.**  
Reasons: no A, no preimage of P1 before f24a, claimed primers cannot insert `H(P1||C1)`, clean-cache reject on all compared versions.

---

## 17. Hypothesis scores

| ID | Hypothesis | FOR | AGAINST | Unknown | /100 |
|---|---|---|---|---|---:|
| **H1** | Rangeproof cache poisoning | Bug real; A→B artificial; f24a has bad P1; store asymmetry | No A; P1 absent pre-f24a; 71c/271 not primers | Whether any mempool ever saw (P1,C1) | **22** |
| **H2** | Other CT/rangeproof bug | P1 fails unexplained; Sep 1 CT churn; RP header atypical | No bug shown that accepts failed verify | secp subtree behavior | **34** |
| **H3** | Asset/value accounting bug | C1 required in tally | Tally consistent; no surplus shown | blinds | **18** |
| **H4** | Consensus/version mismatch | CheckBlock≠ConnectBlock; signed-block federation; split *possible* | Same reject expected if same binary+empty cache | Who ran what build | **42** |
| **H5** | Other Elements vulnerability | Many near-incident commits | None shown to skip CRangeCheck | full audit | **28** |
| **H6** | Functionary/HSM / block production | f24a **is** in a signed block; producers can assemble txs | No HSM breach evidence; not “keys stolen” required for bad tx inclusion | functionary validation policy | **48** |
| **H7** | Combination | Fits fragmented evidence; H1 alone fails historically | Vague | weights of parts | **58** |

### MOST CONSISTENT (not confirmed)

**H7 — combination**, with dominant open weight on **H6/H4**:  
a signed block included a transaction that **fails honest amount verification**, while the **cache-poisoning narrative for this exact `(P1,C1)` remains historically unsupported**.  

The **most precise local fact** remains: first honest failure is **out1 rangeproof** on a path that **is** invoked in ConnectBlock — so any node that accepted 4050336 either did not run that check as we model it, ran it against different code/data, or obtained a true result we cannot reproduce (including an undiscovered cache prime).

---

## Bottom line

1. **First reject:** `VerifyRangeProof` on f24a out1 — **and that check is on the block-connect path**.  
2. **Mempool vs block:** both should reject without a cache hit; only `store` differs.  
3. **Cache bug:** real; **probable non-explanation of f24a** given current evidence.  
4. **Inflation via Pedersen:** not demonstrated.  
5. **Why the block exists anyway:** **non déterminable** among {undiscovered prime, other validation defect, functionary non-verification, version skew}; **most consistent umbrella = H7**.
