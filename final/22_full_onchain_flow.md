# 22 — Full on-chain flow: 4050335 → 4050349 → Bitcoin mainchain

**Scope:** UTXO / block / timeline reconstruction only.  
**ROOT CAUSE: NOT ADDRESSED BY THIS MISSION.**

---

## 1. Chain 4050335 → 4050349

Artifact: [`final/data/chain_4050335_4050349.json`](data/chain_4050335_4050349.json)

| HEIGHT | HASH | PREVHASH | TIMESTAMP | UTC | TX_COUNT | SOURCE |
|---:|---|---|---:|---|---:|---|
| 4050335 | `aad24e4fb64ca8adf4961667da87820cd48e553957ac64e75de7cdb298b5d66b` | `3b892680a538bdc4cef5b735176e541d79851cd1834fb32a071d86d01609ffe0` | 1788702730 | 2026-09-06 13:52:10 UTC | 5 | Blockstream Liquid API |
| 4050336 | `e1d9a2aae69e0fc3ca18f7f7f84e0615e92a5e3b5000d66c10c34043346da0d5` | `aad24e4fb64ca8adf4961667da87820cd48e553957ac64e75de7cdb298b5d66b` | 1788702790 | 2026-09-06 13:53:10 UTC | 7 | API |
| 4050337 | `c212cdcb6b2e68d4f56a7ddfee48bd3c02b2bfbb703b9f4506ac1597a40d38be` | `e1d9a2aae69e0fc3ca18f7f7f84e0615e92a5e3b5000d66c10c34043346da0d5` | 1788702850 | 2026-09-06 13:54:10 UTC | 6 | API |
| 4050338 | `0b998712cb557e04f1780e6285509fd3c6ed94344bdeb9ad9c8f819bfd232250` | `c212cdcb6b2e68d4f56a7ddfee48bd3c02b2bfbb703b9f4506ac1597a40d38be` | 1788702910 | 2026-09-06 13:55:10 UTC | 4 | API |
| 4050339 | `5ba1ea5defeebca7c4036c9242461be8cfd7e6de055156972c1c87d5b58e52cb` | `0b998712cb557e04f1780e6285509fd3c6ed94344bdeb9ad9c8f819bfd232250` | 1788702970 | 2026-09-06 13:56:10 UTC | 3 | API |
| 4050340 | `4a308cc2b35e675a8df9cd04e1748625c8200dd169ff40b522b2e10f2ee5526d` | `5ba1ea5defeebca7c4036c9242461be8cfd7e6de055156972c1c87d5b58e52cb` | 1788703030 | 2026-09-06 13:57:10 UTC | 2 | API |
| 4050341 | `37a4edbe432dd6bb676aa8657e01a7aaaeab1c01e7fc267d02c1dc025f7147fb` | `4a308cc2b35e675a8df9cd04e1748625c8200dd169ff40b522b2e10f2ee5526d` | 1788703090 | 2026-09-06 13:58:10 UTC | 4 | API |
| 4050342 | `90d1508b879a8f5ed832615785d653b0f2a6c06b485f1950665e93ea13e97aed` | `37a4edbe432dd6bb676aa8657e01a7aaaeab1c01e7fc267d02c1dc025f7147fb` | 1788703150 | 2026-09-06 13:59:10 UTC | 2 | API |
| 4050343 | `f7add9732519fd2737a1be40a86a96191db0ea126338cc934eb686834ddc98f8` | `90d1508b879a8f5ed832615785d653b0f2a6c06b485f1950665e93ea13e97aed` | 1788703210 | 2026-09-06 14:00:10 UTC | 6 | API |
| 4050344 | `3a4afa5b5a01119f6ecd8073374fb1418fa220c00b1ac0cd582d6adfc0be6c68` | `f7add9732519fd2737a1be40a86a96191db0ea126338cc934eb686834ddc98f8` | 1788703270 | 2026-09-06 14:01:10 UTC | 4 | API |
| 4050345 | `d5ebce62272c047f89fc98cc8ae54424e6d9e0d2290396b591dab74764cf6340` | `3a4afa5b5a01119f6ecd8073374fb1418fa220c00b1ac0cd582d6adfc0be6c68` | 1788703330 | 2026-09-06 14:02:10 UTC | 4 | API |
| 4050346 | `47c7d0c902746f15dc2651d1b29fdd2fd6b9ce658fbbb2bb88e5cefa8c87003a` | `d5ebce62272c047f89fc98cc8ae54424e6d9e0d2290396b591dab74764cf6340` | 1788703390 | 2026-09-06 14:03:10 UTC | 4 | API |
| 4050347 | `7ab19940d7f68503778959b67dee59c475fabb9b2d445f6ae515dac7923d9222` | `47c7d0c902746f15dc2651d1b29fdd2fd6b9ce658fbbb2bb88e5cefa8c87003a` | 1788703450 | 2026-09-06 14:04:10 UTC | 5 | API |
| 4050348 | `ca87fbece5fcfee99a00ee8920d68a35e1f8f2bc9c7b53b1b7e7f9c32aaa83fc` | `7ab19940d7f68503778959b67dee59c475fabb9b2d445f6ae515dac7923d9222` | 1788703510 | 2026-09-06 14:05:10 UTC | 3 | API |
| 4050349 | `90a8c1ae477530b1e50c30ab7dc709eacab5a68a8b3652c147441508b1244730` | `ca87fbece5fcfee99a00ee8920d68a35e1f8f2bc9c7b53b1b7e7f9c32aaa83fc` | 1788703570 | 2026-09-06 14:06:10 UTC | 6 | API |

**CHAIN CONTINUITY = CONFIRMED**  
(`prev(h) == hash(h-1)` for every height in the table).

---

## 2. Key transactions (locations)

| TX | Block | Index in block | Artifact |
|---|---:|---:|---|
| `f24a4b17…183f` | **4050336** | **1** | [`block_4050336_txids.json`](data/block_4050336_txids.json) |
| `46f117c9…b3d2` | **4050344** | **2** | [`block_4050344_txids.json`](data/block_4050344_txids.json) |
| `ce4caece…88f2` | **4050349** | **4** | [`block_4050349_txids.json`](data/block_4050349_txids.json) |
| `731f8fdf…7d8a` (correct id; not `…101eb…`) | **4050349** | **5** | same |
| `c6ea588a…2267` | **4050343** | (confirmed via tx status) | API tx |
| `3875a6d6…146b` | **4050337** | (confirmed via tx status) | API tx |
| `32892440…3814` | **4050348** | **1** | [`block_4050348_txids.json`](data/block_4050348_txids.json) |

Machine-readable graph: [`final/data/pegout_flow.json`](data/pegout_flow.json).

---

## 3. f24a outputs (independent branches)

| Out | Kind | Spent? | Spender | Block |
|---:|---|---|---|---:|
| 0 | confidential CT | **YES** | `3875a6d6…146b` vin0 | 4050337 |
| 1 | confidential OP_RETURN | **NO** | — | — |
| 2 | confidential CT | **YES** | `c6ea588a…2267` vin3 | 4050343 |
| 3 | explicit fee 58 | **NO** (fee) | — | — |

### Branch f24a:0
```
f24a:0
  → 3875a6d6…146b:0   (block 4050337)
  → c6ea588a…2267     as vin2
  → c6ea:0
  → 46f117… as vin11  (block 4050344)
```

### Branch f24a:1
```
f24a:1 → UNSPENT
```

### Branch f24a:2 (primary path to 46f117)
```
f24a:2
  → c6ea588a…2267 as vin3  (block 4050343)
  → c6ea:0
  → 46f117… as vin11       (block 4050344)
```

### Branch f24a:2 alternate (via c6ea:1 into ce4)
```
f24a:2
  → c6ea… as vin3
  → c6ea:1
  → 32892440…3814:0        (block 4050348)
  → ce4… as vin5           (block 4050349)
```

**FACT:** f24a:0 and f24a:2 **converge** at `c6ea` (distinct vins). Branches are listed separately above; convergence is explicit.

### Branch f24a:3
```
f24a:3 (fee) → not a spendable UTXO path
```

---

## 4. Transaction 46f117

| Field | Value |
|---|---|
| TXID | `46f117c990580501a5156937a8c8affda551a38b3c0b99d29eb8469ee6beb3d2` |
| Block | **4050344** / `3a4afa5b…6c68` |
| Timestamp | 1788703270 |
| Inputs | **13** |
| Outputs | **3** |
| out0 | **PEG-OUT = CONFIRMED** — value **265138358** sats = **2.65138358** L-BTC → BTC `bc1qkxwva32eh7mgezq5kladncd3n5wtcjmslh98my` |
| out1 | confidential change → Liquid `ex1qamnwmwueelwdywjkx7pxxzg9ur3znm6jq6r84v` |
| out2 | fee **225** |
| f24a-related input | vin**11** = `c6ea588a…2267:0` |

Other 12 inputs: **not** claimed as f24a descendants in this mission (no full ancestry proven for each).

Source: [`data/46f117c990580501a5156937a8c8affda551a38b3c0b99d29eb8469ee6beb3d2.bs.json`](../data/46f117c990580501a5156937a8c8affda551a38b3c0b99d29eb8469ee6beb3d2.bs.json) (copied under `final/data/`).

---

## 5. 46f117 → ce4

**DIRECT UTXO link = CONFIRMED**

```
46f117…b3d2:1
  → ce4caece…88f2 vin[4]
```

Spending block: **4050349**.

---

## 6. Transaction ce4

| Field | Value |
|---|---|
| TXID | `ce4caece413cd9d444ce7ed9f54e5b328b3da5e4af301aff59a3571f76e988f2` |
| Block | **4050349** / `90a8c1ae…4730` |
| Timestamp | 1788703570 |
| Inputs | **6** |
| Outputs | **3** |
| out0 | confidential change → `ex1qsg0xx2plutvahv8atmd6qxxua6utjxm4t4khey` |
| out1 | **PEG-OUT = CONFIRMED** — **399601834922** sats = **3996.01834922** L-BTC → BTC `bc1qgslsydz56d0ed6827hdemfmk5w2f6ldyc6wt7p` |
| out2 | fee **177** |

### ce4 inputs

| i | Outpoint | Source block (if known) | Relation to f24a |
|---:|---|---|---|
| 0 | `bda29202…5015:0` | UNKNOWN | **UNKNOWN** (not demonstrated) |
| 1 | `b81408f4…69a1:1` | UNKNOWN | **UNKNOWN** |
| 2 | `b55e10cd…c2c3:1` | 4050345 | **UNKNOWN** |
| 3 | `6269cfc7…41c4:0` | UNKNOWN | **UNKNOWN** |
| 4 | `46f117…b3d2:1` | 4050344 | **CONFIRMED** path includes f24a via c6ea→46f117 |
| 5 | `32892440…3814:0` | 4050348 | **CONFIRMED** path includes f24a via c6ea:1 |

**Do not assume all six inputs come from f24a.**

---

## 7. Change path ce4:0 → 731f

```
ce4:0  (ex1qsg0xx2plutvahv8atmd6qxxua6utjxm4t4khey)
  → 731f8fdf0428c8ef5f2dc48d9347e458b101be2430f22ca6e5f376fd42e47d8a vin[1]
```

Block: **4050349** (same block as ce4).

**Note:** The txid in the mission brief used `…101eb…`; the live outspend id is `…101be…`.

`731f` itself is an **additional Liquid peg-out** (not mere opaque change):

| Field | Value |
|---|---|
| Peg-out value | **399601658** sats = **3.99601658** L-BTC |
| BTC destination | `bc1qk3cvk5599nydy8zwavlxaduke54pgf4vl0ckru` |

This is **UTXO-linked** to ce4:0. It is **not** assumed to be the “main” economic exit; it is recorded because it is on-chain and matches a Bitcoin federation output (below).

---

## 8. Two primary peg-out branches (+ third found)

### BRANCH A
```
46f117…
  └── peg-out 2.65138358 L-BTC → bc1qkxwva…
```

### BRANCH B
```
ce4…
  └── peg-out 3996.01834922 L-BTC → bc1qgslsy…
```

### Sum A+B
```
2.65138358
+ 3996.01834922
= 3998.66973280 BTC
= 399866973280 sats
```

### BRANCH C (additional, scanned)
```
731f… (spends ce4:0)
  └── peg-out 3.99601658 L-BTC → bc1qk3cvk…
```

---

## 9. Liquid → Bitcoin `8db751…`

| Field | Value |
|---|---|
| TXID | `8db751a650ae2f12006b7e8c69a75e4df360e8afd6b9e05ae0b9fa6458a7b140` |
| BTC height | 965783 |
| BTC block_time | 1788704936 → **2026-09-06 14:28:56 UTC** |
| vin_count | 83 |
| fee | 34097 sats |

Relevant outputs ([`final/data/8db751_btc.json`](data/8db751_btc.json)):

| Value (sats) | Address | Matches Liquid peg-out |
|---:|---|---|
| 399601834922 | `bc1qgslsydz56d0ed6827hdemfmk5w2f6ldyc6wt7p` | ce4 out1 (**exact**) |
| 265138358 | `bc1qkxwva32eh7mgezq5kladncd3n5wtcjmslh98my` | 46f117 out0 (**exact**) |
| 399601658 | `bc1qk3cvk5599nydy8zwavlxaduke54pgf4vl0ckru` | 731f peg-out (**exact**) |

```
Liquid peg-out A (46f117)  ──amount+address──►  Bitcoin out
Liquid peg-out B (ce4)     ──amount+address──►  Bitcoin out
Liquid peg-out C (731f)    ──amount+address──►  Bitcoin out
```

**Link class:**

- **A. CRYPTOGRAPHIC / UTXO (Liquid-internal):** CONFIRMED as documented above.  
- **Liquid → Bitcoin UTXO spend link:** **not available** from public Liquid/BTC explorers in the usual outpoint sense (peg-out is a burn + federation payout).  
- **Structural correspondence:** CONFIRMED by identical satoshi amounts and identical BTC scriptPubKey addresses between Liquid `pegout` fields and Bitcoin outputs.  
- Timing: Liquid peg-outs at 14:01–14:06 UTC; Bitcoin tx at 14:28:56 UTC — consistent ordering, **not** used alone as proof.

Other BTC outputs (repeated ~1.677e8 sat payments, etc.) are **not** identified with Liquid peg-outs in this mission.

---

## 10. Link-type discipline

| Claim | Type |
|---|---|
| `f24a:2 → c6ea → 46f117` | **A — UTXO CONFIRMED** |
| `46f117:1 → ce4 vin4` | **A — UTXO CONFIRMED** |
| `c6ea:1 → 3289 → ce4 vin5` | **A — UTXO CONFIRMED** |
| Peg-out amounts/addresses match BTC outs | **A-structural (amount+script) CONFIRMED**; not a Liquid outpoint spend |
| “Same economic event / theft narrative” | **B — INTERPRETIVE** (not asserted here) |

---

## 11. Timeline

| TIME UTC | BLOCK | TX | EVENT | AMOUNT | RELATION |
|---|---:|---|---|---|---|
| 2026-09-06 13:53:10 | 4050336 | f24a | confirmed | — | creates outs 0–3 |
| 2026-09-06 13:54:10 | 4050337 | 3875 | spends f24a:0 | conf. | UTXO |
| 2026-09-06 14:00:10 | 4050343 | c6ea | spends f24a:2 (+3875:0, others) | conf. | UTXO |
| 2026-09-06 14:01:10 | 4050344 | 46f117 | peg-out A | 2.65138358 | vin11←c6ea:0 |
| 2026-09-06 14:05:10 | 4050348 | 3289 | spends c6ea:1 | conf. | UTXO |
| 2026-09-06 14:06:10 | 4050349 | ce4 | peg-out B | 3996.01834922 | vin4←46f117:1; vin5←3289:0 |
| 2026-09-06 14:06:10 | 4050349 | 731f | peg-out C (from ce4:0) | 3.99601658 | UTXO |
| 2026-09-06 14:28:56 | BTC 965783 | 8db751 | federation payout | matches A+B+C | amount+address |

---

## 12. Blocks 4050344 → 4050349 (roles)

| BLOCK | TXID | ROLE | INPUT RELATION | OUTPUT RELATION | AMOUNT | CONFIDENCE |
|---:|---|---|---|---|---|---|
| 4050344 | `46f117…` | peg-out A + change | vin11←c6ea:0 (f24a path) | out0 peg-out; out1 change | 2.65138358 | HIGH |
| 4050345 | `b55e10cd…` | later ce4 vin2 prevout | UNKNOWN vs f24a | — | — | present in block |
| 4050348 | `32892440…` | bridge c6ea:1 → ce4 | ←c6ea:1 | →ce4 vin5 | conf. | HIGH |
| 4050349 | `ce4…` | peg-out B | vin4←46f117:1; vin5←3289 | out1 peg-out; out0 change | 3996.01834922 | HIGH |
| 4050349 | `731f…` | peg-out C | vin1←ce4:0 | peg-out | 3.99601658 | HIGH |

Other txs in these blocks: listed in `block_*_txids.json`; not all individually attributed.

---

## 13. Peg-outs found (4050336–4050349)

| TXID | BLOCK | AMOUNT (BTC) | DESTINATION | RELATION TO f24a | CONFIDENCE |
|---|---:|---:|---|---|---|
| `46f117…b3d2` | 4050344 | 2.65138358 | `bc1qkxwva…` | UTXO via c6ea | HIGH |
| `ce4cae…88f2` | 4050349 | 3996.01834922 | `bc1qgslsy…` | UTXO via 46f117:1 (+ other vins) | HIGH |
| `731f8f…7d8a` | 4050349 | 3.99601658 | `bc1qk3cvk…` | UTXO via ce4:0 (ce4 has f24a-linked vins) | HIGH |

**Scan result (CONFIRMED):** exhaustive check of txs in **4050344–4050349** (plus 34 txs covering **4050336–4050343**) found **only these three** explicit `pegout` outputs — zero additional peg-outs, zero fetch failures ([Scan Liquid txs for pegouts](cff0f65e-45db-455a-a37d-c4ce0e5629ca)).

---

## 14. Amount check

| Quantity | sats | BTC |
|---|---:|---:|
| A | 265138358 | 2.65138358 |
| B | 399601834922 | 3996.01834922 |
| A+B | 399866973280 | **3998.66973280** |
| C (731f) | 399601658 | 3.99601658 |
| A+B+C | 400266574938 | 4002.66574938 |

Bitcoin `8db751` contains outs equal to **A, B, and C** exactly.  
Remaining BTC outs / fee **34097** are **not** labeled as Liquid peg-out counterparts here.

Difference A+B vs “~4000 BTC” media round numbers: **not interpreted**.

---

## 16. ASCII graph (confirmed links only)

```
f24a (4050336)
 │
 ├── :0 ──► 3875 (4050337) :0 ──► c6ea (4050343) vin2 ──┐
 │                                                       │
 ├── :1 UNSPENT                                          │
 │                                                       ▼
 ├── :2 ──────────────────────────────► c6ea vin3 ──► c6ea:0 ──► 46f117 vin11 (4050344)
 │                                           │                      │
 │                                           └─► c6ea:1             ├── peg-out 2.65138358 ──► BTC 8db751 (match)
 │                                                 │                │
 │                                                 ▼                └── :1 change
 │                                           3289 (4050348)                │
 │                                                 │                       ▼
 │                                                 └──────────► ce4 vin5   ce4 vin4 (4050349)
 │                                                                       │
 └── :3 fee                                                              ├── peg-out 3996.01834922 ──► BTC 8db751 (match)
                                                                         │
                                                                         └── :0 change ──► 731f (4050349)
                                                                                              └── peg-out 3.99601658 ──► BTC 8db751 (match)
```

---

## 17. Exact path f24a:2 → 46f117:1 → ce4

```
f24a4b179b5cc7e88b25a763911f7cbdf2bf45d1d1b5ab611e94461cef0a183f:2
  → c6ea588ac26f5838b6acbb2a444a33b325bbfeb39bf16dfe27b47215ffd72267:0
  → 46f117c990580501a5156937a8c8affda551a38b3c0b99d29eb8469ee6beb3d2:1
  → ce4caece413cd9d444ce7ed9f54e5b328b3da5e4af301aff59a3571f76e988f2  (vin 4)
```

(Intermediates: only `c6ea` between f24a:2 and 46f117 for this path.)

---

## 18. Result table

| # | Item | Answer | Status |
|---|---|---|---|
| A | 4050335→4050349 continuity | Yes | **CONFIRMED** |
| B | f24a block | 4050336 (index 1) | **CONFIRMED** |
| C | 46f117 block | 4050344 (index 2) | **CONFIRMED** |
| D | ce4 block | 4050349 (index 4) | **CONFIRMED** |
| E | f24a economic descendants | :0 and :2 → c6ea → 46f117 / ce4 paths; :1 unspent | **CONFIRMED** (UTXO) |
| F | 2.65138358 peg-out | 46f117 out0 | **CONFIRMED** |
| G | 3996.01834922 peg-out | ce4 out1 | **CONFIRMED** |
| H | 46f117→ce4 | :1 → ce4 vin4 | **CONFIRMED** |
| I | Liquid peg-outs → BTC 8db751 | amount+address match for A,B,(C) | **CONFIRMED** structural; Liquid→BTC outpoint **UNKNOWN** |
| J | total peg-out A+B | 3998.66973280 BTC | **CONFIRMED** |
| K | other peg-outs found | Only A/B/C in 4050336–4050349; no further peg-outs | **CONFIRMED** |
| L | missing UTXO links | full ancestry of ce4 vins 0–3 | **UNKNOWN** / incomplete |

---

## 19. Causality warning

**ON-CHAIN FACT:**  
These transactions/outputs are linked by the UTXO relationships documented above (and by exact amount+address correspondence between Liquid `pegout` fields and Bitcoin `8db751` outputs).

**ECONOMIC INTERPRETATION:**  
This appears consistent with a multi-step Liquid consolidation ending in federation BTC payouts totaling the matched peg-out amounts — **stated only as consistency, not as motive or exploit narrative.**

**ROOT CAUSE:**  
**NOT ADDRESSED BY THIS MISSION.**

---

## Artifacts

- `final/data/chain_4050335_4050349.json`
- `final/data/pegout_flow.json`
- `final/data/block_4050335.json` … `block_4050349.json`
- `final/data/block_4050344_txids.json` … (selected)
- `final/data/8db751_btc.json`
- `final/data/46f117….bs.json`, `final/data/ce4….bs.json`
