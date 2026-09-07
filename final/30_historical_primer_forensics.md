# Mission 30 — Forensic identification of the historical cache primer

**Date:** 2026-09-07
**Builds on:** Mission 28 (post-`c26d719` unframed preimage aliasing)
**Does not reuse:** Mission 27 / 25 old-key `H(proof‖commitment)` priming model as the test criterion.

## Objective (answered at end)

> Can we identify an actual on-chain primer whose exact post-`c26d719c29` cache preimage aliases the cache preimage of `f24a:1`, and can we establish enough chronology/runtime evidence to connect that primer to the acceptance of block `4050336`?

---

## 1. Attack-side target (raw artifacts)

TX: `f24a4b179b5cc7e88b25a763911f7cbdf2bf45d1d1b5ab611e94461cef0a183f` · **vout 1** · height **4050336**

| Field | Source | Bytes / value |
|---|---|---|
| **P1** | `final/hex/out1_rangeproof.hex` | 4234 · SHA256 `6619fa29…c098b2` |
| **C1** | `final/hex/out1_commitment.hex` | `086f5d67160fc4b477954fb09ef321e5b589d7a07740a1a6df494ed2335b1d01d8` |
| **Wire asset** | `final/hex/out1_asset.hex` | explicit L-BTC `01‖…` |
| **S1** | `final/hex/out1_script.hex` | `6a` (OP_RETURN) |
| **G (cache/crypto)** | serialized generator (not wire) | `0a0a488de4899d0ae757f6cf8368663184d164106111ed9eaecf510e35282ddc6d` |

**FACT:** for explicit assets, `confidential_validation.cpp` overwrites the asset buffer with `secp256k1_generator_serialize(generator_generate(asset_id))` before `CRangeCheck` / cache (Mission 28).

---

## 2. Exact post-`c26d719` keying (source, not paraphrase)

From `SignatureCache::ComputeEntryRangeProof` after `c26d719c29`:

```cpp
CSHA256 hasher = m_salted_hasher_range_proof;  // salt already in hasher state
hasher.Write(proof.data(), proof.size())
      .Write(commitment.data(), commitment.size())
      .Write(asset_commitment.data(), asset_commitment.size())
      .Write(scriptPubKey.data(), scriptPubKey.size())
      .Finalize(entry.begin());
```

| Property | Value |
|---|---|
| Field order | `proof → value_commitment → asset_commitment → scriptPubKey` |
| Framing | **none** (no lengths / tags / separators) |
| Hash | **single** `CSHA256::Finalize` (not double-SHA256) |
| Salt | process-local; absorbed before field writes |
| Asset bytes (explicit) | **serialized generator**, not wire `0x01‖asset_id` |
| Cache | process-global `rangeProofCache` (`CuckooCache`); hit ⇒ unconditional TRUE |

**Unsalted field preimage compared in this mission:**

```text
preimage = proof || value_commitment || G_ser || scriptPubKey
```

Identical preimages ⇒ identical salted keys for any salt (Mission 28: 1000/1000).
**Not** a SHA-256 collision: identical inputs.

Target dump: [`final/data/f24a_target_cache_preimage.bin`](data/f24a_target_cache_preimage.bin)
`len=4301` · `SHA256(preimage)=82b0b8ccf8c743171f2bc8d80bb9982a81cfca583e8dfbe65563cf095e99c01a`

---

## 3–5. Candidate enumeration and exact alias search

Scanner: [`final/scripts/mission30_postfix_preimage_scan.py`](scripts/mission30_postfix_preimage_scan.py)
Machine table: [`final/data/mission30_primer_candidates.json`](data/mission30_primer_candidates.json)

Evaluated (**72** rows):

- **68** Q0-family outs (4049384–4050246) from Mission 26 curated list
- **`71c93d43…` out0 + out1**
- **`27114710…` out0 + out1**

Primary test: `candidate_preimage == f24a_out1_preimage` (byte-for-byte).

### Compact results

| Candidate tx | vout | proof len | C | G | S | preimage len | exact alias with f24a |
|---|---:|---:|---|---|---|---:|---|
| `71c93d4339fe8328…f411` | 0 | 4166 | C0 | G | `6a43‖C1‖G‖6a` (69 B) | **4301** | **YES** |
| `271147100a94f633…7ec5` | 0 | 4166 | C0 | G | same as 71c9 | **4301** | **YES** |
| `71c93d43…` / `27114710…` | 1 | 4174 | (sibling) | G | (other) | ≠4301 match | **NO** |
| Q0 ×68 (all identical tuple) | 1 | 4166 | CQ | G | `6a0100` | 4235 | **NO** |

Bins:

- [`71c93d43_vout0_cache_preimage.bin`](data/71c93d43_vout0_cache_preimage.bin) — `cmp` identical to target
- [`27114710_vout0_cache_preimage.bin`](data/27114710_vout0_cache_preimage.bin) — `cmp` identical to target
- [`Q0_family_cache_preimage.bin`](data/Q0_family_cache_preimage.bin) — **differs** (`SHA256=15812b45…`)

### Why 71c9 / 2711 alias (measured, not assumed)

```text
P1 = P0 || C0 || G || 6a || 43
S0 = 6a || 43 || C1 || G || 6a
S1 = 6a

attack = P1 || C1 || G || S1
       = P0 || C0 || G || 6a || 43 || C1 || G || 6a

primer = P0 || C0 || G || S0
       = P0 || C0 || G || 6a || 43 || C1 || G || 6a
```

**FACT:** byte-identical 4301-byte preimages **before** SHA-256.

Mission 25/27 correctly **REFUTED** them as primers under **old** `H(P‖C)`.
Under **post-fix unframed** keying they **are** exact aliases. Different keying model → different verdict.

---

## 6. The 68 identical rangeproofs (post-fix test)

| Question | Answer |
|---|---|
| Are `(P,C,G,S)` identical across the 68? | **YES** — Q0, CQ, G, `S=6a0100` |
| Identical post-fix preimages among themselves? | **YES** |
| Equal to f24a:1 preimage? | **NO** — LCP with attack preimage = **2** bytes; lengths 4235 vs 4301 |
| Q0 == P1[:4166]? | **NO** (Q0 SHA256 `8cdcb808…` ≠ P0 `0e018e2b…`) |
| Script continuation / boundary trick into attack preimage? | **NO** — `6a0100` ≠ required `6a43‖C1‖G‖6a` continuation |
| Variants among the 68? | **None** in `(P,C,asset,script)` for this family |

**68-proof theory as historical priming of f24a via post-fix aliasing:** **REFUTED**.

---

## 7. Alternate primer search

Within curated window + local hex + Mission 26 scan products:

- Full **P1** as a standalone verifying field before f24a: **not found** (Mission 27; reconfirmed absence of other aliases in this candidate set).
- Only exact post-fix aliases found: **71c9 out0** and **2711 out0**.
- Sibling outs of those txs: **do not** alias.
- No other candidate in the 72-row table produces the 4301-byte target.

Broader claim “no other primer exists on Liquid ever”: **not asserted** — search is bounded to the incident window artifacts already collected. Within that bound, aliases = {71c9, 2711}.

---

## 8. Chronology

```text
2026-09-05 22:01:10Z   first Q0 @ 4049384          [ON-CHAIN]
        … 68× Q0 …
~2026-09-06 12:23Z     last Q0 @ 4050246           [ON-CHAIN]
2026-09-06 13:52:10Z   4050335: 2711 + 71c9 (P0)   [ON-CHAIN]  ← exact aliases
        ↓
   mempool opportunity on some node?               [UNPROVEN]
        ↓
2026-09-06 13:53:10Z   4050336: f24a               [ON-CHAIN]
        ↓
4050337…4050349 + peg-outs                         [ON-CHAIN]
```

Mined presence of 71c9/2711 **before** f24a is **FACT**.
**Does not prove** any functionary had them in mempool/cache.

---

## 9. Lab reconfirm (Nodes A–D)

Re-ran [`final/scripts/28_cache_replay.c`](scripts/28_cache_replay.c) → [`final/logs/30_cache_replay_reconfirm.log`](logs/30_cache_replay_reconfirm.log):

| Node | Result for f24a:1 RP checker |
|---|---|
| **A** post-fix + primer Set | **ACCEPT (cache hit)** |
| **B** post-fix cold | **REJECT** |
| **C** pre-fix primed | **REJECT** |
| **D** length-framed keying | alias **broken** |

Scope: `CachingRangeProofChecker` only — **not** full `ConnectBlock`.

---

## 10. Production-version paradox

| Fact | Status |
|---|---|
| `c26d719c29` CommitDate **2026-09-01** | CODE |
| Backports `6253d7e103` (09-02), `212c43f475` (09-03) | CODE |
| `git tag --contains c26d719c29` → **empty** (local clone) | CODE |
| `elements-23.3.3` is **not** an ancestor of the fix | CODE |
| Public commentary: SideSwap / explorers on pre-fix releases; `@liquidexplorer` allegedly **23.3.2** and **rejected** | PUBLIC REPORTING (not a primary node log) |
| Whether **accepting** runtimes ran post-fix from `master` / backport / private build | **UNKNOWN** |

**Tension (do not collapse):**

- Public “cache bug” narratives often describe **pre-fix** incomplete keying (`proof‖commit` only).
- The **71c9 → f24a** alias requires **post-fix unframed** keying (Mission 28/30).
- Under **pre-fix**, 71c9/2711 **do not** alias f24a:1 (Mission 25) — Node C REJECT.

So: identifying 71c9/2711 as Level-A/C primers does **not** by itself prove production used this path.

---

## 11. First accepting runtime

Prior Mission 21 + this pass: **no** public primary artifact (RPC dump, `debug.log`, version pin, dual-node ACCEPT/REJECT with configs) identifying the first runtime that `ConnectBlock`’d / activated 4050336.

Explorer divergence reports exist in public discussion; they are **not** treated here as ConnectBlock proofs.

```
FIRST ACCEPTOR: UNKNOWN
FIRST ACCEPTANCE MECHANISM: UNKNOWN
```

---

## 12. Evidence levels (do not collapse)

| Level | Meaning | 71c9 / 2711 out0 | Q0 ×68 |
|---|---|---|---|
| **A** Cryptographic identity | Exact preimage equality | **CONFIRMED** | **REFUTED** |
| **B** Functional exploit | Lab Set→Hit | **CONFIRMED** | N/A for f24a |
| **C** Historical candidate | On-chain before f24a | **CONFIRMED** | on-chain but wrong preimage |
| **D** Historical cache priming | Entry on accepting node | **UNPROVEN** | **REFUTED** as f24a primer |
| **E** Historical causality | Caused acceptance of f24a | **UNPROVEN** | **REFUTED** via this path |

---

## 13. Does the 68-proof theory survive?

**REFUTED** as the post-fix historical priming mechanism for `f24a:1`.

They remain interesting as a repeated dry-run/spam pattern; they do **not** produce the required concatenation.

---

## Artifacts

| Path | Role |
|---|---|
| `final/data/mission30_primer_candidates.json` | Full candidate table + chronology + verdict |
| `final/data/f24a_target_cache_preimage.bin` | Attack preimage (4301 B) |
| `final/data/71c93d43_vout0_cache_preimage.bin` | Alias primer bin |
| `final/data/27114710_vout0_cache_preimage.bin` | Alias primer bin |
| `final/data/Q0_family_cache_preimage.bin` | Non-alias family bin |
| `final/logs/30_cache_replay_reconfirm.log` | Nodes A–D reconfirm |
| `final/scripts/mission30_postfix_preimage_scan.py` | Scanner |

Independent check:

```bash
cmp final/data/f24a_target_cache_preimage.bin final/data/71c93d43_vout0_cache_preimage.bin
cmp final/data/f24a_target_cache_preimage.bin final/data/27114710_vout0_cache_preimage.bin
# both silent exit 0
```

---

## CONFIRMED

- Exact post-`c26d719` unframed field order and generator substitution for explicit assets.
- f24a:1 post-fix preimage length **4301**, SHA256 `82b0b8cc…`.
- **`71c93d43…:0` and `27114710…:0` produce byte-identical preimages** to f24a:1 (Level **A**), mined at **4050335** (Level **C**).
- Lab Set→Hit / cold reject / pre-fix reject / framed break (Level **B**).
- All 68 Q0 tuples share one post-fix preimage that **≠** f24a:1.

## STRONGLY SUPPORTED

- Among scanned incident-window candidates, 71c9/2711 out0 are the **only** exact post-fix aliases of f24a:1.
- Chronology is compatible with a mempool-priming opportunity (~60s) **if** an acceptor ran post-fix code with those txs in-process.
- Public reports that some explorers on **pre-fix** releases diverged / rejected are consistent with Node B/C lab behavior — **not** proof of acceptor identity.

## REFUTED

- The **68 Q0** proofs as post-fix primers of f24a:1.
- Calling the 71c9/f24a equality a **SHA-256 collision**.
- “`elements-23.3.3` contains `c26d719`”.
- Using Mission 27’s old-key non-collision to dismiss 71c9/2711 under **post-fix** keying.
- Collapsing Level C → Level D/E (on-chain existence ⇒ cache on acceptor ⇒ caused acceptance).

## STILL UNKNOWN

- Whether accepting production nodes ran post-`c26d719` keying on 2026-09-06.
- Whether 71c9/2711 entered those nodes’ mempools/caches.
- First accepting runtime / version / config.
- Full historical `ConnectBlock(4050336)` via this path.
- Alternate acceptance mechanisms (`fScriptChecks`, other bugs) if acceptors were pre-fix.
- Numeric opening of C1.

---

**Did Mission 30 identify the historical cache primer used to accept `f24a`?**

**NO** — Mission 30 identified the only scanned on-chain transactions (`71c93d43…:0`, `27114710…:0`) whose post-fix cache preimages are byte-identical to `f24a:1` (Levels A–C), but did **not** establish that any accepting runtime was primed by them or that this path caused acceptance of block 4050336 (Levels D–E remain unproven).
