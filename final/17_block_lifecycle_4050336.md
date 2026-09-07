# 17 — Lifecycle of block 4050336 & first accepting node

## Verdict (Mission 23)

**B — Le chemin standard ne peut pas accepter le bloc** (Elements stock, cache vide, tip Liquid).

Complement: **C** remains true for *full-node* reproduction (no `elementsd` run here), but the **code path analysis is sufficient** to show stock acceptance is impossible without a special state/tooling deviation.

**Le paradoxe 4050336 n’est pas expliqué.**  
Signed ≠ functionary compromised.

---

## 1. Call graph — ProcessNewBlock → best chain

```
P2P / submitblock / RPC
  ChainstateManager::ProcessNewBlock(block)          validation.cpp ~5246
    LOCK cs_main
    CheckBlock(block)                                ~4564
      CheckBlockHeader → CheckProof (signblock)      block_proof.cpp
      CheckMerkleRoot
      CheckTransaction × N                           tx_check.cpp  [NO VerifyAmounts]
    if OK:
      AcceptBlock(block, …)                          ~5144
        AcceptBlockHeader                            ~4964
          ContextualCheckBlockHeader                 ~4822
          CheckProof already implied via header path
        Write block to disk / update index
    NotifyHeaderTip
    ActiveChainstate().ActivateBestChain(state, block)  ~3971
      ActivateBestChainStep                          ~3843
        ConnectTip                                   ~3650
          ConnectBlock(block, …, fJustCheck=false)   ~2780
            fScriptChecks = true | assumevalid gate  ~2851
            CheckTxInputs(..., cacheStore=false, fScriptChecks)
              if (fScriptChecks) VerifyAmounts(...)  tx_verify.cpp:250
                CBalanceCheck / CRangeCheck / CSurjectionCheck
            CheckInputScripts if fScriptChecks
            UpdateCoins → tip advances if all OK
```

| Function | Fail modes relevant to f24a |
|---|---|
| CheckBlock | Would **PASS** f24a |
| AcceptBlockHeader | Signatures / height / time — expected **PASS** for published header |
| ConnectBlock / VerifyAmounts | **FAIL** on out1 RP (empty cache) |
| ActivateBestChain | Tip **does not** advance if ConnectBlock fails |

**CONFIRMED:** becoming `best chain` requires successful `ConnectBlock`, not merely signatures.

---

## 2. Block construction (Liquid / Elements)

| API / path | File | Validates amounts before emit? |
|---|---|---|
| `BlockAssembler::CreateNewBlock` | `node/miner.cpp` ~130 | Pulls txs from **mempool only** (`addPackageTxs`). If `options.test_block_validity` (**default `true`**, `miner.h:182`) → calls `TestBlockValidity` → `ConnectBlock(..., fJustCheck=true)` → **VerifyAmounts YES** |
| `getnewblockhex` | `rpc/mining.cpp` ~1151 | Uses CreateNewBlock with defaults → **TestBlockValidity ON** |
| `getblocktemplate` paths | mining.cpp | Various; several call `TestBlockValidity` |
| `wallet signblock` | `wallet/rpc/elements.cpp` ~56 | Docstring: *“checking that it would be accepted first”* → **`testBlockValidity` before sign** (~108) |
| `combineblocksigs` | mining.cpp ~1297 | Merges sigs; **`CheckProof` only** for `complete` — **NO TestBlockValidity** |
| `testproposedblock` | mining.cpp ~1613 | Explicit `TestBlockValidity` |
| `submitblock` | mining.cpp ~1058 | `ProcessNewBlock` → full ConnectBlock |

### Does the producer verify VerifyAmounts before signing?

| Path | Answer |
|---|---|
| Stock `getnewblockhex` + default assembler | **YES** (via TestBlockValidity), if that path is used |
| Stock wallet `signblock` | **YES** |
| Stock `combineblocksigs` alone | **NO** amount validation |
| Production HSM / custom federation tooling | **UNKNOWN** (not in this repo) |

Liquid public docs (docs.liquid.net): functionaries sign **after validating contents** — **policy claim**, not verified against HSM firmware here.

---

## 3–4. Mempool → block vs direct inject

### Must a tx pass mempool?

**No (consensus).** A block may contain any tx bytes; ConnectBlock validates independently.

**Yes (stock template path).** `CreateNewBlock` only selects mempool entries.

### Mempool entry for f24a (empty cache)

`AcceptToMemoryPool` → `CheckTxInputs(..., cacheStore=true, fScriptChecks=true)` → `VerifyAmounts` → out1 **FAIL**.

**CONFIRMED:**

> f24a devait soit être validée avec un état spécial, soit être injectée directement dans un bloc (hors CreateNewBlock mempool), soit provenir d’un nœud/outil différent.

### Direct inclusion — CRITICAL

**YES, conceptually possible:** craft block hex containing f24a without mempool, obtain ≥11 signatures via a path that **skips** `TestBlockValidity` (e.g. raw `combineblocksigs` / custom signer), broadcast.  

Independent full nodes still must `ConnectBlock` → should **REJECT** unless they also have a special state.

---

## 5. Federation (roles only — no persons)

| Item | Source | Status |
|---|---|---|
| ~15 functionaries; blocksigners + watchmen | docs.liquid.net / Blockstream help | CONFIRMED (public docs) |
| Quorum **≥ 11 of 15** (~2/3) | same | CONFIRMED |
| Round-robin propose ~60s; others validate & sign | same | CONFIRMED as documented process |
| HSMs hold keys | Blockstream help | CONFIRMED as described |
| Who proposed 4050336 | — | **UNKNOWN** |
| Exact Elements binary on HSMs 2026-09-06 | — | **UNKNOWN** |

---

## 6. Signatures on 4050336

| Property | Value | Status |
|---|---|---|
| Script type | P2WSH `00207f1a37f6…e6be74` | CONFIRMED |
| Witness | empty + **11** DER ECDSA + redeem (`OP_15` / 0x5b …) | CONFIRMED |
| Authenticates | Block header challenge (dynafed signblock) | CONFIRMED (`CheckProof` / `GenericVerifyScript`) |
| Guarantees | Quorum authorized **this header** | CONFIRMED |
| Does **not** guarantee | Every tx passed VerifyAmounts on every signer | CONFIRMED (separate code path) |

**Can majority signatures exist without identical transactional validation?**  
**YES (code):** signing APIs can omit `TestBlockValidity`; HSMs may differ.  
**Docs say they validate** — **UNCONFIRMED** that all 11 did for 4050336.

---

## 7. Producer vs observer

| Dimension | Producer (template/sign) | Observer (ProcessNewBlock) |
|---|---|---|
| Mempool contents | Local | May differ |
| Rangeproof cache | Process-local RAM | Independent |
| `test_block_validity` on assemble | Default true | N/A |
| Must ConnectBlock to advance tip | If they submit & activate | Yes |
| Version / patches / flags | **UNKNOWN** | **UNKNOWN** |
| Custom HSM firmware | Possible | N/A |

---

## 8. Exact version around 2026-09-06

| Evidence | Result |
|---|---|
| Latest GitHub release tag then | `elements-23.3.3` (2026-04-13) — **no** c26d fix |
| Fix commits | Author 2026-08-03; CommitDates 2026-09-01…09-03; PR #1599 same day as incident |
| Functionary / Blockstream production version | **UNKNOWN** |

Do not invent a version.

---

## 9. Validation matrix (stock empty cache)

| version/config | CheckBlock | VerifyAmounts | ConnectBlock / tip |
|---|---|---|---|
| 23.3.3 default Liquid | PASS | FAIL out1 | FAIL |
| +c26d719 / 6253 / 212c empty cache | PASS | FAIL out1 | FAIL |
| assumevalid tip default | PASS | runs (`fScriptChecks=true`) | FAIL |
| hypothetical assumevalid skip (non-default tip catchup) | PASS | **SKIPPED** | may connect without amounts — **not default tip** |
| OLD cache + mythical (P1,C1) prime | PASS | PASS (hit) | PASS — **historically unsupported** |

---

## 10. assumevalid — definitive

| Question | Answer |
|---|---|
| Can it skip VerifyAmounts? | **YES** — sets `fScriptChecks=false` → skips gate at `tx_verify.cpp:250` |
| Where? | `ConnectBlock` `validation.cpp` ~2851–2877 |
| Conditions | Non-null AssumedValidBlock; block ancestor of that hash & best header; work threshold; equivalent time ≤ 2 weeks |
| Liquid default | `defaultAssumeValid = uint256()` **empty** |
| Tip 4050336 default | **CANNOT EXPLAIN** |

---

## 11. fScriptChecks — exact

| Item | Fact |
|---|---|
| Defined | Local `bool fScriptChecks = true` in `ConnectBlock` |
| Modified | Only by assumevalid block (~2852–2877) |
| Gates | `VerifyAmounts` **and** `CheckInputScripts` |
| Name vs reality | Despite the name, in Elements mode it **also** gates confidential amount checks — **CONFIRMED** by `tx_verify.cpp:250` |
| Tip default Liquid | `true` |
| IBD / reindex | May be `false` under assumevalid catchup |
| Signed blocks | No special exemption |

---

## 12. Signed ≠ validated

```
CheckProof(signblock)  →  authorizes header
ConnectBlock/VerifyAmounts  →  consensus UTXO transition
```

Signatures **can** exist on a block later rejected by an independent validator — **CONFIRMED** as a code-level possibility (`combineblocksigs` without TestBlockValidity). Whether that happened for 4050336: **UNKNOWN**.

---

## 13. Scenario DIRECT-BLOCK (conceptual)

```
craft block with f24a
→ skip TestBlockValidity
→ combineblocksigs / custom sign (≥11)
→ submitblock → ProcessNewBlock
→ CheckBlock PASS
→ ConnectBlock → VerifyAmounts FAIL
→ tip does NOT advance on honest node
```

Failure locus: **ConnectBlock / out1 RP**.

---

## 14. Scenario MEMPOOL

```
f24a → AcceptToMemoryPool → VerifyAmounts FAIL → never in mempool
→ CreateNewBlock cannot select it
```

If forced into template anyway + `test_block_validity=true`:

```
TestBlockValidity → ConnectBlock(fJustCheck) → FAIL → CreateNewBlock throws
```

**CONFIRMED (code):**

> même binaire + cache vide : impossible de construire honnêtement 4050336 avec f24a via mempool+getnewblockhex/signblock stock.

---

## 15. Consequence — how did the producer obtain the block?

| Option | Status |
|---|---|
| A version différente | UNKNOWN |
| B configuration différente | UNKNOWN |
| C validation bypass | PLAUSIBLE (non-TestBlockValidity signing) |
| D bug producteur | PLAUSIBLE category |
| E transaction injectée hors mempool | PLAUSIBLE / CRITICAL |
| F compromise producteur | UNCONFIRMED (≠ mere signature) |
| G autre / our incomplete model | PLAUSIBLE (C) |

---

## 16. Fork data

| Height | Hash (Blockstream) | Status |
|---|---|---|
| 4050335 | `aad24e4f…d66b` | CONFIRMED |
| 4050336 | `e1d9a2aa…a0d5` | CONFIRMED; `in_best_chain` |
| 4050337 | `c212cdcb…38be` | CONFIRMED as `next_best` |

Competing branch / mempool.space tip: **UNCONFIRMED** in this workspace (no captured alternate tips).

---

## 17. First node that accepted 4050336 as best chain

**UNKNOWN.**  
Earliest public artifact here: Blockstream Liquid API snapshot showing `in_best_chain: true` — not a first-seen timestamp of ConnectBlock success.

---

## 18. Signature sequence vs validation

| Order | Stock Elements QA (`signblock`) | `combineblocksigs` | Documented federation |
|---|---|---|---|
| Validate then sign | YES | NO amounts check | Claims validate then sign |
| Sign then network ConnectBlock | submitter still faces ConnectBlock | same | same |

**Potentially the key:** if production signing omitted full `TestBlockValidity`, signatures can accumulate on an amounts-invalid block; **observers who ConnectBlock correctly should still refuse tip advance** — deepening the paradox that Blockstream shows best-chain.

---

## 19. Commits 2026-08-20 → 2026-09-07 (touched paths)

| Commit | Change | Relevance to accept f24a |
|---|---|---|
| c26d719c29 | Rangeproof cache key +asset+script | Fix poisoning; empty-cache still rejects |
| 779e71f545 | Mempool reject confidential-asset pegouts | Not f24a |
| c7ec63c583 | Dynafed header height always validated | Unrelated to RP |
| 4ddaefc8cc / #1593 | RPC/blind errors on bad RP | Wallet/RPC |
| (broader Aug: sighash+RP, blindpsbt, secp bump) | Construction / signing defaults | No ConnectBlock skip found |

---

## 20. H6 discipline

We only have: **≥11 actors signed a header our independent amount validation rejects.**  

That is **not** proof of compromise, collusion, or malice. It is compatible with bug, config, custom firmware, incomplete validation, or unknown version.

---

## 21. Scores (delta from Mission 16)

| H | Score | Delta reason |
|---|---:|---|
| H1 cache | **12** | Still no priming; construction path also blocked |
| H2 other CT/RP | **35** | Unchanged category |
| H3 accounting | **12** | — |
| H4 version/config | **50** | ↑ direct-inject + non-validating combineblocksigs + UNKNOWN prod version |
| H5 other Elements | **32** | Slight ↑ for tooling gaps |
| H6 producer/signing process | **52** | ↑ process can sign without TestBlockValidity; **not** “compromised” |
| H7 combination | **38** | Single gap still dominates |

---

## 22. Minimal conditions for best-chain despite honest ConnectBlock FAIL

| # | Condition | Status |
|---|---|---|
| 1 | Block bytes include f24a and valid federation quorum signatures | **CONFIRMED** |
| 2 | At least one path produced/signed the block **without** successful stock `TestBlockValidity`/`VerifyAmounts` (or with a non-reproduced special state) | **PROBABLE** (required by contradiction) |
| 3 | Something caused an indexed “best chain” view to advance despite our ConnectBlock reject model — e.g. nodes that connected successfully, or explorer ≠ full ConnectBlock semantics | **UNKNOWN** which |
| 4 | Historical `(P1,C1)` cache prime | **REFUTED** as demonstrated |
| 5 | Default assumevalid tip skip | **REFUTED** |

---

## 23. Final

**B:** standard Elements Liquid path (mempool → getnewblockhex → signblock → submit → ConnectBlock) **cannot** honestly produce and activate 4050336 with f24a on empty cache.

**Paradox: NOT explained.**

Next investigative priority (not executed here): capture **functionary/HSM validation policy** and **whether any full node log shows ConnectBlock success or reject** for `e1d9a2aa…`.
