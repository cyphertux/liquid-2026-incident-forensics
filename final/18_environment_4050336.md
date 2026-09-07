# 18 — Environment that accepted / published 4050336

## Separation first (Mission 25)

| Event | Meaning | What we know |
|---|---|---|
| **EVENT A** | 4050336 constructed & quorum-signed | **CONFIRMED** (block bytes + 11-of-15 witness) |
| **EVENT B** | 4050336 appears as **best chain** (Blockstream API) | **CONFIRMED** as observation; **mechanism UNKNOWN** |

A and B need not share the same mechanism.

**Rule:** signed ≠ validated; Blockstream best-chain ≠ all nodes accepted; possible ≠ probable.

---

## 1. Who produces Liquid blocks?

| Fact | Status |
|---|---|
| Strong Federation: ~15 functionaries; blocksigners propose round-robin ~60s; ≥11 must sign | CONFIRMED (docs.liquid.net / Blockstream help) |
| Propose → others validate contents (documented policy) → combine signatures → broadcast | CONFIRMED as **documentation**, not as verified HSM firmware |
| Identity of the round’s proposer for height 4050336 | **UNKNOWN** |
| Personal names | not assigned (no public proof tying this block’s DER sigs to named operators) |

---

## 2. Who signed 4050336? (parse only)

| Field | Value |
|---|---|
| `signblockscript` | P2WSH `00207f1a37f651f3cdae…e6be74` |
| Redeem (last witness item) | `OP_11` + **15×** compressed pubkeys + `OP_15` + `OP_CHECKMULTISIG` |
| Quorum | **11-of-15** |
| Witness layout | `[OP_0 empty][11 DER ECDSA sigs][redeem 513 B]` |
| Authenticates | Header / dynafed signblock challenge (`CheckProof` → `GenericVerifyScript`) |
| Which 11 of 15 keys | **UNKNOWN** without per-sig matching (not done); pubkeys listed in `final/data/block_4050336_fed_pubkeys.txt` if written |

**CONFIRMED:** valid quorum signature ≠ each signer ran successful `ConnectBlock`.

---

## 3. Signature process (Elements code)

| Step | Command / path | Amount validation mandatory? |
|---|---|---|
| **A Construction** | `getnewblockhex` → `CreateNewBlock` (mempool packages) | **YES** if `test_block_validity` (default **true**) → `TestBlockValidity` → `ConnectBlock(fJustCheck)` → `VerifyAmounts` |
| **B Validation** | `testproposedblock` / wallet `signblock` pre-check | **YES** (`TestBlockValidity`) |
| **C Signature** | wallet `signblock` | After B; signing itself only signs header |
| **D Combine** | `combineblocksigs` | **NO** amount check; only assembles witness + `CheckProof` for `complete` |
| **E Broadcast** | `submitblock` → `ProcessNewBlock` | Receiver: **YES** full `ConnectBlock` to advance tip |

Production HSM path: **UNKNOWN** whether it mirrors A–B or skips to C–D.

---

## 4. `test_block_validity`

| Question | Answer |
|---|---|
| Default | `true` (`miner.h:182`) |
| Used by `getnewblockhex`? | **YES** (CreateNewBlock default) |
| CLI to disable? | **No** — `ApplyArgsManOptions` does not expose it; only fuzz/bench set `false` |
| wallet `signblock` | Uses `testBlockValidity` **independently** (always, not this flag) |
| `combineblocksigs` | Does not use it |
| Historically disabled on Liquid prod? | **UNKNOWN** — do not conclude |

---

## 5. Direct path to signblock

| Path | Usable in stock Elements? | Production Liquid? |
|---|---|---|
| Arbitrary txs → handcrafted block hex → `combineblocksigs` | **YES** (no TestBlockValidity) | **UNKNOWN** if used |
| Arbitrary → `getnewblockhex` without mempool | **NO** (assembler only mempool) | — |
| Mempool → `getnewblockhex` → `signblock` with f24a empty cache | **NO** (fails TestBlockValidity / mempool) | — |

---

## 6. Roles A–D

| Role | version | config | flags | cache | assumevalid | mode |
|---|---|---|---|---|---|---|
| A builder | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN |
| B signer(s) | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN |
| C receiver | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN |
| D best-chain indexer (Blockstream API) | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN | UNKNOWN |

---

## 7. Versions 2026-08-20 → 2026-09-07 (public)

| Source | Finding |
|---|---|
| GitHub Elements releases | Latest tag `elements-23.3.3` (2026-04); **no** newer release tag before incident |
| Cache fix | `c26d719c29` CommitDate 2026-09-01; cherry-picks 09-02/09-03; PR #1599 same calendar day as incident |
| Functionary / Blockstream deploy notes for exact binary | **UNKNOWN** |
| Press (SideSwap / Liquid X) | Claim Elements software bug / PAK not compromised — **hearsay for our forensic bar**; not version pins |

---

## 8. Commit table (effect on f24a accept)

| Commit | File(s) | Change | Effect on f24a | Proof |
|---|---|---|---|---|
| c26d719c29 | sigcache.cpp | Cache key +asset+script | Empty cache still REJECT | native verify + code |
| 6253d7e103 / 212c43f475 | sigcache.cpp | Same cherry-pick | Same | git |
| 779e71f545 | pak/validation | Mempool confidential pegout | Not f24a | diff |
| c7ec63c583 | validation/chain | Dynafed height | Unrelated | diff |
| 4ddaefc8cc | blind/RPC | Error handling | Unrelated to ConnectBlock | diff |
| No commit found | — | Skip `VerifyAmounts` on tip | **None identified** | grep call sites |

---

## 9. Exhaustive VerifyAmounts skips

| Mechanism | Can explain tip 4050336 default Liquid? |
|---|---|
| `fScriptChecks==false` (assumevalid) | **NO** (default empty assumevalid) — CANNOT EXPLAIN tip |
| `g_con_elementsmode==false` | **NO** (Liquid true) |
| Rangeproof **cache hit** | Theoretical; historical P1 prime **unsupported** — excluded per mission |
| `block.fChecked` | Only skips re-`CheckBlock`, **not** amounts |
| Checkpoints | Don’t skip amounts on new tip |
| IBD/reindex/snapshot | Not tip activation story without evidence |
| Genesis | N/A |
| Trusted/special signed-block exemption | **Not in code** |

---

## 10–11. BlockIndex / caches

| Level | Meaning | Raised when |
|---|---|---|
| TREE | headers OK | AcceptBlockHeader |
| TRANSACTIONS | txs present / CheckBlock-class | Have data |
| CHAIN / SCRIPTS | economic + scripts | **After successful ConnectBlock** (`RaiseValidity(BLOCK_VALID_SCRIPTS)` ~3172) |

**No** BlockIndex state marks a block “fully valid” without having passed ConnectBlock amounts/scripts once.

Caches distinguished:

| Cache | Can fake ConnectBlock success? |
|---|---|
| Rangeproof cache | Only if hit on (P1,C1) — excluded historically |
| Signature / script caches | Not a substitute for VerifyAmounts |
| `block.fChecked` | CheckBlock only |
| BlockIndex VALID_* | Consequence of ConnectBlock, not a bypass |

---

## 12. CheckBlock PASS → “valid” without ConnectBlock?

**Aucun bypass identifié** for advancing **active chain tip**.  

AcceptBlock may **store** the block (HAVE_DATA) after CheckBlock; that is **not** best-chain activation.

---

## 13. AcceptBlock vs ActivateBestChain

```
AcceptBlock     → disk + index entry (“accepted” as known block)
ActivateBestChain → ConnectTip → ConnectBlock → tip / best chain
```

| Phrase | When true |
|---|---|
| Block accepted (stored) | After AcceptBlock success |
| Block = best chain | After ConnectBlock success + chain selection |

Explorer `in_best_chain: true` claims the second — **how their node got there is UNKNOWN**.

---

## 14. First accepting node

**UNKNOWN.**

---

## 15–18. Explorers / two chains / concurrent block

| Query | Result |
|---|---|
| Live tip compare Blockstream vs mempool.space Liquid | **UNCONFIRMED** (API reset this session) |
| Local snapshot | Blockstream: 4050336 `e1d9…` `in_best_chain`, next `c212…` |
| Competing block at 4050336 | **UNKNOWN** / not found in local artifacts |
| First divergent height | **UNKNOWN** (cannot assume 4050336 without alternate tip data) |

Do not infer fork solely from a stuck explorer UI.

---

## 19. Producer compromised?

Necessary for **confirmed compromise** (none met):

- intentional invalid construction **and**
- conscious signing of known-invalid amounts **and**
- exclusion of bug/config/tooling error **and**
- positive evidence of malice  

**H6 = producer/process issue category, NOT confirmed compromise.**

---

## 20. H4 version/config → ACCEPT/REJECT

| Candidate | f24a VerifyAmounts | 4050336 tip | Tested? |
|---|---|---|---|
| 23.3.3 empty cache | REJECT | REJECT | YES (native+code) |
| +c26d empty cache | REJECT | REJECT | YES |
| assumevalid tip default | REJECT | REJECT | YES (code) |
| assumevalid skip (non-default) | SKIP amounts | may ACCEPT | code-only; not shown used |
| `test_block_validity=false` assemble | N/A build | still REJECT on honest Connect | code-only |
| Prod HSM firmware | UNKNOWN | UNKNOWN | **impossible here** |

---

## 21. H5 — other Elements bug (P1 FALSE but ACCEPT)

| Candidate | Code support? |
|---|---|
| Skip RP on OP_RETURN | **NO** — unspendable skip is only for **explicit 0**; confidential still `CRangeCheck` |
| Skip fee / index off-by-one | **NO** — out1 is confidential non-fee |
| Witness parse drop out1 proof | Would fail “missing proof” not accept |
| Asset-specific skip | **NO** |

**No H5 instance retained.**

---

## 22. H2 — verify FALSE but VerifyAmounts TRUE?

Same process call: `CRangeCheck` returns error if `VerifyRangeProof` false (unless cache hit).  
Queue completion fails the block.  

**No bug found** mapping `secp256k1_rangeproof_verify==0` → `VerifyAmounts==true` without cache/assumevalid.

---

## 23. Minimal model (matches 4050336 shape)

```
handcrafted/injected block containing CT out with invalid RP
→ CheckBlock PASS
→ quorum sign via path without TestBlockValidity (e.g. combineblocksigs / custom)
→ broadcast
→ honest ConnectBlock FAIL
```

Compared to 4050336: structure matches EVENT A possibility; EVENT B still requires an extra unexplained accept.

---

## 24. Minimum for best-chain appearance

Demonstrated steps only:

1. **CONFIRMED:** block commits f24a + valid 11-of-15 header signatures (EVENT A).  
2. **PROBABLE (by contradiction only):** construction/signing did not follow stock mempool+`TestBlockValidity` success — else empty-cache stock cannot emit the template. (*“Probable” here = logical necessity under stock-code premises, not a witnessed producer log.*)  
3. **UNKNOWN:** environment that executed successful `ConnectBlock` / or non-full-node indexing that still labels `in_best_chain`.  
4. **UNKNOWN:** whether other full nodes rejected (fork) or also accepted.

Do **not** fill 3–4 with story.

---

## 26. Scores

| H | Evidence | Against | Unknown | /100 |
|---|---|---|---|---:|
| H1 cache | Bug real | No historical P1 prime | — | **10** |
| H2 CT/RP | Unique bad RP | No verify→accept bug | — | **28** |
| H3 accounting | — | No surplus | — | **10** |
| H4 version/config | Prod version unknown; skip flags exist off-default | Default tip cannot | Prod binary | **48** |
| H5 other Elements | — | OP_RETURN skip refuted | — | **22** |
| H6 producer/process | combineblocksigs w/o amounts; federation process | ≠ compromise | HSM policy | **50** |
| H7 combination | A≠B | Not proven two bugs | — | **35** |

---

## 27. Conclusion A–F

| # | Question | Answer |
|---|---|---|
| **A** | How was 4050336 constructed? | **UNKNOWN** (stock honest path impossible; direct inject **possible**) |
| **B** | How was it signed? | **CONFIRMED** 11-of-15 P2WSH CHECKMULTISIG; **UNKNOWN** whether signers ran TestBlockValidity |
| **C** | How was it accepted (ConnectBlock)? | **UNKNOWN** |
| **D** | Why honest independent validation rejects? | **CONFIRMED** — out1 `rangeproof_verify` FALSE under L-BTC+`6a` |
| **E** | Why Blockstream shows best-chain? | **UNKNOWN** |
| **F** | What produces a fork? | **UNKNOWN** (alternate tip not evidenced here) |

### Environment identification result

**Not identified.**  

Closest **code-level** statement: any environment that both (1) obtained quorum signatures and (2) advanced a tip including f24a must diverge from **stock empty-cache ConnectBlock**, via a factor in {non-default config, non-stock tooling, unreproduced validation state, indexer semantics ≠ ConnectBlock, or error in our model} — **none selected**.
