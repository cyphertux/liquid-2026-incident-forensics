# Mission 31 — Historical Runtime & Cache Attribution

**Date:** 2026-09-07
**Depends on:** Missions 28–30 (mechanism + on-chain alias candidates)
**Nature:** Historical forensics — not a new exploit exercise.

## Question

Can public evidence establish that `71c93d43…:0` and/or `27114710…:0` primed a **post-`c26d719`** cache on a **production** Liquid node that then accepted `f24a` and enabled inclusion of block **4050336**?

---

## PART I — Software state (primary Git)

### Timeline (FACT from local `elements/` + `gh api`)

| When | Event | Post-`c26d719`? |
|---|---|---|
| 2026-04-13 | `elements-23.3.3` tagged | **No** |
| 2026-04-14 | Docker `blockstream/elementsd:latest` / `:23.3.3` | **No** |
| 2026-08-03 | `c26d719c29` **AuthorDate** (Byron Hambly, Blockstream) | authored |
| 2026-09-01 11:39 +02 | `c26d719c29` **CommitDate** | on master lineage |
| 2026-09-01 14:38 UTC | PR **#1592** merged (“blind/blindpsbt fixes”; LLM-scan bundle) | **Yes** (master) |
| 2026-09-02 | `6253d7e103` → 23.x path; PR **#1595** | branch yes |
| 2026-09-03 | `212c43f475` on 23.3.x cherry-pick path | branch yes |
| **2026-09-06 13:52–13:54 UTC** | **4050335 → 4050336 → 4050337** (incident window) | production **UNKNOWN** |
| 2026-09-06 **17:21 UTC** | PR **#1599** merge into 23.3.x | **after** incident |
| Mission 31 check | Docker Hub `latest` still **23.3.3** (2026-04-14) | **No** public post-fix image |

Artifacts: [`data/mission31_timeline.json`](data/mission31_timeline.json), [`logs/31_forensic_search.log`](logs/31_forensic_search.log).

`git tag --contains c26d719c29` → **empty**.
`c26d719` **not** ancestor of `elements-23.3.3`.

### What `c26d719c29` changed (re-derived from source)

Files: `src/script/sigcache.cpp`, `src/script/sigcache.h` only (+6/−4 lines in `.cpp`).

| | |
|---|---|
| **Before** | `ComputeEntryRangeProof(proof, commitment)` → `H(salt‖proof‖commit)` |
| **After** | `ComputeEntryRangeProof(proof, commitment, asset_commitment, scriptPubKey)` → same writes **unframed** |
| **Call site** | `CachingRangeProofChecker::VerifyRangeProof` passes asset + script into key |

Parent: `b0aba619b6…`.
PR #1592 body: *“Fixes a number of small issues picked up during LLM scans”* — **FACT** (gh).
ACK: `tomt1664 ACK c26d719c29 tested locally` — **FACT** (merge message).
No public security advisory / CVE narrative in the PR body.

### Production path search

| Search | Result |
|---|---|
| Elements tags containing fix | **None** |
| Docker Hub post-fix tag | **Not found** (latest = 23.3.3) |
| Historical Docker commit-hash tags | **Exist** (e.g. `23.2.2-e215b73c1f11`) — shows **capability** to ship non-release builds, **not** that functionaries ran `c26d719` |
| Functionary Docker/manifests for 2026-09-06 | **Not found** |
| Public `getnetworkinfo` / version dump for acceptor | **Not found** (Mission 21 reconfirmed) |
| Liquid/Blockstream infra repos with pinned post-fix deploy | **Not found** in this search |

**Did Liquid routinely deploy unreleased Elements commits?**
Public Docker history shows **occasional** commit-tagged images.
**Routine production policy for functionaries:** **UNKNOWN** — no primary evidence found; do not infer from general practice.

---

## PART II — Primers `71c9` / `2711`

### Byte identity (Mission 30 bins re-checked)

```text
cmp f24a_target_cache_preimage.bin 71c93d43_vout0_cache_preimage.bin  → identical
cmp f24a_target_cache_preimage.bin 27114710_vout0_cache_preimage.bin  → identical
71c9 and 2711 out0 share identical (P,C,S); G shared
len = 4301; SHA256(preimage) = 82b0b8cc…
```

**FACT** Level **A**. Lab verify primer TRUE / attack FALSE — Level **B** ([`data/28_crypto_verification.json`](data/28_crypto_verification.json)).
Mined at **4050335** (~60s before f24a) — Level **C**.

### Mempool eligibility vs presence

| Question | Evidence |
|---|---|
| Valid / mined? | **FACT** — in block 4050335 |
| Could enter some mempool before f24a? | **Temporally compatible** (HYPOTHESIS) |
| Broadcast logs? | **UNKNOWN** |
| Functionary saw it? | **UNKNOWN** |
| Populated cache? | **UNKNOWN** |

**On-chain existence ≠ mempool presence on the accepting node.**

### Attacker chronology (bounded)

```text
Q0 ×68 (4049384–4050246)     [REFUTED as post-fix f24a primers]
        ↓
4050335: 2711 + 71c9 (P0)    [exact post-fix aliases — Level A–C]
        ↓
4050336: f24a
        ↓
4050337… → peg-outs (Mission 22)
```

Full wallet graph beyond curated pack: **not newly reconstructed** here; Mission 26/22/30 remain authoritative for on-chain relationships. No public mempool dump upgrades Level C → D.

---

## PART III — Cache lifetime (source)

| Property | Source fact |
|---|---|
| Process-global `rangeProofCache` | `sigcache.cpp` |
| Default budget | `DEFAULT_VALIDATION_CACHE_BYTES/4` = **8 MiB** |
| `Get(erase=true)` | marks reclaimable; **not** immediate delete |
| Eviction | lazy insert / epoch aging (`cuckoocache.h`) |
| Restart / reindex | clears in-memory cache |

**60s gap:** survival is **temporally plausible** under modest insert pressure — **INFERENCE**, not historical priming proof.

---

## PART IV — First acceptor & “split”

| Observation | Class |
|---|---|
| Blockstream Liquid API tip lineage includes 4050336→4050349 | **FACT** (Mission 22) |
| 11/15 federation signatures on 4050336 | **FACT** |
| SIGNATURE ≠ ConnectBlock validation | **FACT** (code + Mission 21) |
| Named Node A ACCEPT / Node B REJECT with versions | **UNKNOWN** (no primary logs) |
| Public claim `@liquidexplorer` on 23.3.2 rejected | **PUBLIC_REPORTING** only |
| Explorer HTML ≠ acceptance proof | **FACT** |

**FIRST ACCEPTOR: not identified.**

---

## PART V — Production paradox (H1–H6)

| ID | Hypothesis | Status after search |
|---|---|---|
| **H1** | Functionaries ran private/unreleased post-`c26d719` | **UNKNOWN** |
| **H2** | Cherry-pick into production branch deployed | **UNKNOWN** (cherry-picks exist; deploy unknown; 23.3.x PR merge **after** incident) |
| **H3** | Bridge nodes ≠ public Elements tags | **UNKNOWN** |
| **H4** | Mixed versions across nodes | **Compatible with public reports**; **not proven** |
| **H5** | Different mechanism than Mission 28 path | **OPEN** |
| **H6** | Mission 28 real but unrelated to Sep 6 | **OPEN** |

**Critical tension (do not collapse):**

- Mission 28 path for `71c9→f24a` requires **post-fix unframed** keying.
- Public tagged/Docker artifacts point to **pre-fix** `23.3.3`.
- Under **pre-fix**, `71c9` does **not** alias f24a (Mission 25/30 Node C).
- Therefore: either acceptors ran non-tag post-fix code (**unproven**), or acceptance used **another** path (**open**).

### Fix provenance

- Bundled in LLM-scan PR #1592, not a standalone security release note.
- Whitehat (PUBLIC_REPORTING): *“Please fix the vulnerability first… As of the latest commit, there is risk on-chain…”* — consistent with either “network not patched” or “master still unsafe”; **encrypted** technical details not public.
- No public post-incident advisory in this search naming the **exact** production commit that was vulnerable / patched.

---

## PART VI — Attribution scoring

| Level | Meaning | `71c9`/`2711` |
|---|---|---|
| A Exact alias | **CONFIRMED** |
| B Valid primer | **CONFIRMED** |
| C Temporal | **CONFIRMED** |
| D Mempool on acceptor | **UNKNOWN** |
| E Cache on acceptor | **UNKNOWN** |
| F Same node accepted f24a | **UNKNOWN** |
| G Causal chain to 4050336 | **UNKNOWN** |

**Highest level actually reached: C.**

---

## PART VII — Falsification attempts

| Test | Result |
|---|---|
| Primers verify? | **YES** (lab) |
| Alias without post-fix? | **NO** — pre-fix REJECT |
| f24a without priming (post-fix cold)? | **REJECT** |
| 68 Q0 better primer? | **REFUTED** |
| Cache must survive 60s? | Plausible, **not proven** historically |
| Production binary contains post-fix? | **UNKNOWN** — if **no**, Mission 28 path **fails** as historical explanation |

Necessary historical condition “acceptor ran post-`c26d719`” is **not established**. That does **not** refute the lab mechanism; it **blocks** historical attribution.

---

## PART VIII — Alternate explanations (kept open)

Not ruled out by this mission: `fScriptChecks`/assumevalid misuse, other CT bugs, producer `combineblocksigs` without full validation + divergent acceptors, custom federation software, manual handling.
Mission 21 already: stock paths exist that **produce** signed invalid blocks without proving acceptance.

---

## Evidence recoverability

After systematic search of:

- local Elements git + GitHub PRs/tags
- Docker Hub `blockstream/elementsd`
- prior Missions 19–22 / 28–30 artifacts
- public incident reporting

**Not recovered (and not present in this repository):**

- functionary/bridge version inventory for 2026-09-06
- mempool / `debug.log` proving primer→cache on an acceptor
- dual-node ACCEPT/REJECT with configs
- official statement of the production commit at incident time

**Conclusion:** the Level D–G gap is **not currently crossable from public evidence**. That is a **FACT about the evidence set**, not a claim that the event did not involve this path.

---

## Deliverables

| File | Role |
|---|---|
| [`31_historical_runtime_attribution.md`](31_historical_runtime_attribution.md) | this report |
| [`31_EXECUTIVE_SUMMARY.md`](31_EXECUTIVE_SUMMARY.md) | decisive findings only |
| [`data/mission31_runtime_matrix.json`](data/mission31_runtime_matrix.json) | software/hypotheses matrix |
| [`data/mission31_primer_evidence.json`](data/mission31_primer_evidence.json) | Levels A–G |
| [`data/mission31_timeline.json`](data/mission31_timeline.json) | chronology |
| [`logs/31_forensic_search.log`](logs/31_forensic_search.log) | search log |

---

## HISTORICAL PRIMER

**CANDIDATE ONLY**

## PRODUCTION RUNTIME

**MIXED/UNKNOWN**

## HISTORICAL CAUSALITY

**NOT DEMONSTRATED**

## FIRST ACCEPTOR

**Not identified** (`UNKNOWN`).

## FINAL ANSWER

> **Can the public evidence currently establish that `71c9/2711` primed a post-`c26d719` cache on a production Liquid node which then accepted `f24a` and caused or enabled the inclusion of `4050336`?**

**NO**

Public evidence reaches only Levels A–C (exact alias, valid primer, mined ~60s before `f24a`). Levels D–G (mempool/cache on an acceptor, same-node acceptance, causal chain) have no primary artifacts. Tagged/Docker production surfaces remain pre-`c26d719`, while the Mission 28 path requires post-fix keying—an unresolved paradox. Without recoverable operational evidence, historical attribution cannot be established.
