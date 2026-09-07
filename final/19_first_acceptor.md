# 19 — First acceptor of block 4050336

## Bottom line

**Who first accepted 4050336 into an active chain: UNKNOWN.**

**New FACT:** Blockstream’s published chain has a continuous parent link  
`4050336 (e1d9…) → 4050337 (c212…) → 4050338 (0b99…) → … → 4050343 (f7ad…)`.  
So 4050336 is not a lone explorer display: later **signed** blocks commit it as ancestor.

That still does **not** identify the first software environment that ran a successful `ConnectBlock` on f24a.

---

## Labels

FACT / INFERENCE / HYPOTHESIS / UNKNOWN / REFUTED

---

## 1. Environments / versions

| ENVIRONMENT | VERSION | COMMIT | CONFIG KNOWN? | SOURCE | CONFIDENCE |
|---|---|---|---|---|---|
| Public Elements release nearest pre-incident | `elements-23.3.3` | release tag 2026-04-13 | defaults in tree | GitHub releases | HIGH (tag exists) |
| Cache-fix on master | `c26d719c29` | CommitDate 2026-09-01 | N/A | git | HIGH |
| Functionary production binary 2026-09-06 | UNKNOWN | UNKNOWN | UNKNOWN | — | — |
| Blockstream.info Liquid backend | UNKNOWN | UNKNOWN | UNKNOWN | API only | — |
| mempool.space / liquid.network backends | UNKNOWN | UNKNOWN | UNKNOWN | API unreachable this mission | — |
| Docker/manifests for Liquid prod | UNKNOWN | — | — | not found here | — |

**FACT:** No public source in this workspace pins the exact Elements commit running on functionaries or explorers on 2026-09-06.  
**REFUTED:** Inferring production version from release calendar alone.

---

## 2. Block 4050336 reconstruction

| Field | Value | Tag |
|---|---|---|
| height | 4050336 | FACT (`final/data/block_4050336.json`) |
| hash | `e1d9a2aae69e0fc3ca18f7f7f84e0615e92a5e3b5000d66c10c34043346da0d5` | FACT |
| prev | `aad24e4fb64ca8adf4961667da87820cd48e553957ac64e75de7cdb298b5d66b` | FACT |
| merkle | `f9a463b6…104b` | FACT |
| tx_count | 7 | FACT |
| f24a position | index **1** | FACT (`block_txids.json`) |
| timestamp | 1788702790 | FACT |
| signblockscript | P2WSH `00207f1a37f6…e6be74` | FACT |
| redeem | `OP_11` + 15 pubs + `OP_15` + `OP_CHECKMULTISIG` | FACT |
| witness | 1 empty + **11** DER + redeem | FACT |
| Individual ECDSA verify of each of 11 | **not executed** this mission | UNKNOWN |
| Aggregate `CheckProof` | expected by Elements; not re-run in harness here | UNKNOWN (structure consistent) |

**FACT:** 11 signatures authorize the **header/signblock challenge**, not that each signer ran `TestBlockValidity` / `ConnectBlock`.  
**Code FACT:** wallet `signblock` *would* TestBlockValidity; `combineblocksigs` would not. Production path UNKNOWN.

---

## 3. Reception logs (2026-09-06 ~16:00–16:15 CEST)

| TIMESTAMP | NODE/ACTOR | EVENT | SOURCE | CONFIDENCE |
|---|---|---|---|---|
| — | — | ProcessNewBlock / ConnectBlock / reject logs | searched workspace + public web this session | **UNKNOWN** (none recovered) |
| block_time 1788702790 | explorer index | 4050336 indexed | Blockstream API JSON | FACT (API field; timezone mapping not re-derived here) |
| block_time 1788702850 | explorer index | 4050337 indexed (+60s) | Blockstream API | FACT |

No operator log line proving first ConnectBlock success.

---

## 4. Explorer comparison

| Explorer | 4050336 visible? | Hash match e1d9? | Chain after? | SOURCE | Tag |
|---|---|---|---|---|---|
| Blockstream.info/liquid | YES (`in_best_chain`) | YES | YES through ≥4050343 | local JSON + API walk | FACT |
| mempool.space/liquid | — | — | — | API reset / no capture | UNKNOWN |
| liquid.network | page fetch non-informative | — | — | WebFetch empty app shell | UNKNOWN |

**No FACT of explorer divergence** captured this mission.  
**REFUTED (for this dataset):** using “stuck UI” as proof of validation split.

---

## 5. Scenario “signed but rejected”

| Step | Possible in code? | Evidenced historically? |
|---|---|---|
| A produce block with f24a | YES (direct inject) | UNKNOWN how |
| B 11 signatures | YES | FACT signatures exist |
| C some nodes reject | YES (stock empty-cache ConnectBlock) | UNKNOWN if any did |
| D some accept | required for later tips | **INFERENCE** at least one tip-building path treated e1d9 as parent |
| E produce 4050337+ | YES | FACT chain exists |
| F Blockstream follows | YES as observation | FACT API shows that chain |

Trigger for D: **UNKNOWN**.

---

## 6. CRITICAL — does 4050337 descend from 4050336?

**FACT — YES.**

```
4050337.id  = c212cdcb6b2e68d4f56a7ddfee48bd3c02b2bfbb703b9f4506ac1597a40d38be
4050337.prev = e1d9a2aae69e0fc3ca18f7f7f84e0615e92a5e3b5000d66c10c34043346da0d5
```

Artifacts: `final/data/block_4050337.json`, `final/data/block_4050336.json`.

Also **FACT:** continuous ancestry through at least 4050343 on Blockstream:

```
4050336 e1d9 ← 4050337 c212 ← 4050338 0b99 ← 4050339 5ba1 ← 4050340 4a30
← 4050341 37a4 ← 4050342 90d1 ← 4050343 f7ad
```

**FACT:** `f24a:0` is spent in tx `3875…` confirmed in block `c212…` (height 4050337) — `final/data/f24a_descendant_probe.json`.  
**INFERENCE:** On that chain, UTXO created by f24a was available ⇒ 4050336 had been connected into the UTXO set used for that tip (or an equivalent state). This is stronger than “explorer HTML shows the block,” but still does not name the first accepting process.

Δt 4050336→4050337 = **60 seconds** (FACT timestamps).

4050337 also has 11 DER sigs + same signblockscript pattern (FACT).

---

## 7. Concurrent branch

| Item | Result |
|---|---|
| Alternate hash at height 4050336 | **UNKNOWN** / not found |
| Consensus split (two incompatible tips) | **NOT DECLARED** — insufficient evidence |

---

## 8. Producer identity

| Block | Proposer pubkey / operator | Tag |
|---|---|---|
| 4050336 | UNKNOWN which of 15 | — |
| 4050337 | UNKNOWN | — |
| Mapping DER sig ↔ pubkey index | not computed | UNKNOWN |

Federation pubkeys listed in `final/data/block_4050336_fed_pubkeys.txt` (FACT list; **no human names**).

---

## 9. Validation configs (lab vs production)

| Config | f24a stock result | Proven on Liquid 2026-09-06? |
|---|---|---|
| A empty cache | REJECT | production config UNKNOWN |
| B primed (P1,C1) | ACCEPT theoretical | historical prime **unsupported** / excluded |
| C assumevalid empty | REJECT (checks on) | Liquid default FACT in chainparams |
| D assumevalid set | may skip amounts | **not proven** used at tip |
| E fScriptChecks=true | REJECT | default tip |
| F fScriptChecks=false | SKIP VerifyAmounts | **not proven** at tip |

**REFUTED as production explanation without config proof:** treating lab ACCEPT paths as what happened.

---

## 10. Unexplored accept paths (matrix)

| CODE PATH | CONDITION | IN LIQUID TREE? | PROVEN ACTIVE 2026-09-06? | COULD ACCEPT f24a? | EVIDENCE |
|---|---|---|---|---|---|
| assumevalid → fScriptChecks=false | non-default | YES | UNKNOWN | YES | validation.cpp |
| rangeproof cache hit | prior store of (P1,C1) | YES | unsupported | YES | sigcache; priming search negative |
| Skip CRangeCheck for OP_RETURN confidential | — | NO | — | NO | confidential_validation.cpp |
| BlockIndex VALID_* without ConnectBlock | — | NO for SCRIPTS | — | NO | RaiseValidity after ConnectBlock |
| `block.fChecked` | re-CheckBlock only | YES | N/A | NO for amounts | CheckBlock |
| Custom HSM / non-Elements validator | off-tree | UNKNOWN | UNKNOWN | UNKNOWN | — |
| Indexer without ConnectBlock | off-tree | UNKNOWN | UNKNOWN | display ≠ tip | — |

---

## 11. Blockstream acceptance mechanism

**BLOCKSTREAM ACCEPTANCE MECHANISM = UNKNOWN.**

Known only: their Liquid API returns `in_best_chain: true` for e1d9 and serves a child chain.  
Unknown: full Elements node vs external index; version; config; first-seen time.

---

## 12. Divergence matrix (stock expectation)

```
f24a under stock empty-cache + fScriptChecks=true → REJECT  [FACT lab/code]
```

| Actor class | Observed behavior | Tag |
|---|---|---|
| Independent secp verify | REJECT out1 | FACT |
| Stock ConnectBlock model | REJECT | FACT (code) |
| Blockstream published tip lineage | continues after e1d9 | FACT |
| First accepting runtime | — | UNKNOWN |
| Producer of 4050337 | used e1d9 as prev | FACT (header) |

**First place reality diverges from stock empty-cache expectation:**  
somewhere between “header signed” and “UTXO tip advanced enough to mine/index 4050337+” — **location UNKNOWN**.

---

## 13. Verdict A–K

| # | Question | Answer | Label |
|---|---|---|---|
| **A** | Who constructed 4050336? | UNKNOWN | UNKNOWN |
| **B** | Who signed 4050336? | 11-of-15 federation keys (which subset UNKNOWN) | FACT quorum structure; UNKNOWN identities |
| **C** | Who first accepted 4050336? | **UNKNOWN** | UNKNOWN |
| **D** | Which software/version accepted it? | **UNKNOWN** | UNKNOWN |
| **E** | Did another node reject it? | UNKNOWN | UNKNOWN |
| **F** | Actual consensus split? | **NOT ESTABLISHED** | UNKNOWN / not claimed |
| **G** | Did 4050337 descend from 4050336? | **YES** | **CONFIRMED/FACT** |
| **H** | Is cache poisoning necessary? | **No proof it is** | REFUTED as necessary given current evidence |
| **I** | Is cache poisoning sufficient? | Sufficient in lab A→B; **not shown** historically for P1/C1 | PLAUSIBLE lab / unsupported hist |
| **J** | Is f24a causal to the incident? | Present on ancestry of peg-out path; **causality not proven** | FACT presence; UNKNOWN causality |
| **K** | Remains UNKNOWN? | First acceptor; versions; rejectors; split; Blockstream node semantics; signing pre-validation | — |

### Confidence board

| Claim | Rank |
|---|---|
| 4050337.prev = e1d9 | **CONFIRMED** |
| Chain continues ≥4050343 from e1d9 on Blockstream | **CONFIRMED** |
| Stock empty-cache rejects f24a | **CONFIRMED** |
| First acceptor identity/version | **UNKNOWN** |
| Cache explains acceptance | **REFUTED** as demonstrated historical mechanism |
| Functionaries “must have ConnectBlock’d” | **not assumed** (forbidden); only header quorum FACT |

---

## Artifacts

- `final/data/block_4050336.json`
- `final/data/block_4050337.json`
- `final/data/block_4050338.json` … `block_4050343.json` (parent walk)
- `final/data/f24a_descendant_probe.json`
- `final/data/block_4050336_fed_pubkeys.txt`
