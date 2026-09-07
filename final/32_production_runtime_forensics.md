# Mission 32 — Production runtime / fix deployment forensics

**Date:** 2026-09-07
**Goal:** Determine from **public** evidence whether any Liquid production node could have been running post-`c26d719` (or a functional equivalent) before/during block **4050336**.
**Not the goal:** Prove Mission 28; upgrade Level C primers to historical causality without new ops evidence.

---

## Central question

> What **exact** code ran on Liquid nodes that could have accepted 4050336?

---

## PART 1–2 — Elements history & patch signature

### Patch (LEVEL A)

| | |
|---|---|
| Commits (functionally equivalent) | `c26d719c29` (master), `6253d7e103` (23.x), `212c43f475` (23.3.x) |
| Files | `src/script/sigcache.cpp`, `src/script/sigcache.h` |
| Change | 2-arg → 4-arg `ComputeEntryRangeProof`; fields `proof‖commit‖asset‖script` **unframed** |
| Pickaxe | **Only these three** commits introduce `Write(asset_commitment)…Write(scriptPubKey)` in `ElementsProject/elements` |
| Tests in `c26d719` | **None** dedicated; bundled in PR #1592 (“LLM scans”) |

### Tags (LEVEL B)

`git tag --contains` for all three hashes → **empty**.
`elements-23.3.3`, `23.3.4rc1`, `29.4.1rc1` → **PRE-FIX**.

### Branch tips **at incident time** (2026-09-06 13:52 UTC) — LEVEL A/B

| Branch | Tip then | Post-fix unframed? |
|---|---|---|
| `master` | `c7e856fab1…` | **YES** (`c26d719`) |
| `elements-23.x` | `323fc71625…` | **YES** (`6253d7e103`, merged via PR #1595 on **2026-09-03**) |
| `elements-23.3.x` | `6c2c5626ba…` | **YES** (`212c43f475` dated **2026-09-03**; already ancestor of Sep 4 tip) |
| `elements-29.x` | `d7789ac2e8…` | **NO** (still 2-arg) |

### Correction to Mission 31 emphasis

**PR #1599** merge timestamp is **2026-09-06 17:21 UTC** (after the incident).
That does **not** mean the patch was absent from `elements-23.3.x` before the incident: `212c43f475` was already in that branch’s history from **2026-09-03**.
Equate carefully: *merge commit after incident* ≠ *patch absent before incident*.

Artifact: [`data/mission32_git_history.json`](data/mission32_git_history.json).

---

## PART 3–6 — Global search, Docker, Liquid stack

### GitHub

- `ComputeEntryRangeProof` / `CachingRangeProofChecker`: Elements + unrelated forks; drivechain fork still **PRE-FIX**.
- `Blockstream/liquid-functionary`: public; **no** `CachingRangeProofChecker`.
- No other org found hosting a Liquid-specific reimplementation of this cache key.

### Docker / `bitcoin-images` (LEVEL A for build method; not for production use)

- `Blockstream/bitcoin-images/elementsd`: downloads **GitHub release** tarball; default `VER=23.3.3`.
- Hub `latest` / `23.3.3` last pushed **2026-04-14** — **PRE-FIX**.
- README allows gitian build from `tag_or_commit` — **capability** to ship untagged builds.
- Historical tags with commit suffixes exist — **capability**, not Sep-6 post-fix proof.
- **No** post-`c26d719` Docker tag found in checked Hub listing.

Copied: [`data/deployment_artifacts/`](data/deployment_artifacts/).

### Liquid-specific patch stack

- Public functionary stack talks to **`sidechaind` (elementsd) via RPC**; Rust `elements = 0.24.1` is a library, **not** the C++ rangeproof cache.
- **No** public Liquid-only patch implementing `c26d719` without those hashes.
- **elementsd commit for functionaries: not pinned** in the public functionary repo.

---

## PART 7 — PR #1599

| | |
|---|---|
| Title | Cherry picks from 23.x into 23.3.x |
| Merged | **2026-09-06T17:21:03Z** (after ~13:53 incident) |
| Content | Includes `212c43f475` among other cherry-picks |
| Ancestry | `212c43f475` already reachable from `elements-23.3.x` tip dated **2026-09-04** |

---

## PART 8–9 — Incident communications

| Statement class | What it shows | What it does **not** show |
|---|---|---|
| Whitehat OP_RETURN: fix first; risk at latest commit; patch every node | Patching demanded | Exact commit / that Mission 28 path was the bug |
| Secondary “bridge nodes patched” | Post-incident remediation claimed | Hash of deployed binary |
| Analyst narrative (incomplete key = missing asset/script) | Focus on **PRE-FIX** incomplete keying | Does not identify Mission 28 post-fix framing alias as the historical bug |
| Report that an unpatched explorer rejected | Compatible with cold reject | Acceptor identity / version |

**“Patched” ≠ proof of `c26d719`.** Encrypted whitehat details to Blockstream are **not** public.

---

## PART 10–11 — Node / functionary versions

| Search | Result |
|---|---|
| `getnetworkinfo` for 4050336 acceptor | **NOT FOUND** |
| Functionary org → Elements version map | **NOT FOUND** |
| ELIP-203 (Jun 2026): upgrade to ≥23.3.1 | Public coordination — **PRE-dates** `c26d719`; does not imply post-fix |

Help Center: functionaries = server + HSM; operators = Federation members. **No** public per-operator Elements pin for 2026-09-06.

---

## PART 12 — Paradox matrix

| Runtime | Behavior | Mission 28 alias? | Existed pre-incident? | Deployed on Liquid? |
|---|---|---|---|---|
| 23.3.3 / Docker latest | PRE-FIX | **NO** | YES | Public default **strongly supported**; functionary use **UNKNOWN** |
| master / `c26d719` | POST unframed | **YES** | YES (from Sep 1) | **UNKNOWN** |
| 23.x / `6253d7e` | POST unframed | **YES** | YES (from Sep 3) | **UNKNOWN** |
| 23.3.x / `212c43f` | POST unframed | **YES** | YES (from Sep 3) | **UNKNOWN** |
| 29.x tip checked | PRE-FIX | **NO** | YES | **UNKNOWN** |
| Framed fix | PATCHED | **NO** | lab only | **UNKNOWN** |

**Do not confuse “code existed on public branches” with “code was deployed.”**

---

## PART 13 — Hypotheses

| ID | Status | Evidence | Counter |
|---|---|---|---|
| H1 private post-fix | UNKNOWN | — | no logs |
| H2 cherry-pick deployed | UNKNOWN | branches had fix by Sep 3–4 | no deploy artifact |
| H3 bridge ≠ functionary build | UNKNOWN | sidechaind unpinned | — |
| H4 mixed versions | Compatible with reports | reject vs tip stories | not primary dual logs |
| H5 other validation bug | OPEN | — | — |
| H6 Mission 28 unrelated | OPEN | public narrative = PRE-FIX bug | — |
| H7 config divergence | OPEN | — | — |
| H8 ConnectBlock bypass | OPEN | `combineblocksigs` path in code | — |

---

## PART 14–15 — First acceptor / cache theory discipline

First accepting runtime: **NOT IDENTIFIED** (Mission 21/31 stand).
`71c9`/`2711` = cryptographic candidates only — **not** demonstrated historical primers.

Evidence levels reached: **A–B only**. C–G **not** reached.

---

## Deliverables

| Path | Role |
|---|---|
| [`32_production_runtime_forensics.md`](32_production_runtime_forensics.md) | this report |
| [`32_EXECUTIVE_SUMMARY.md`](32_EXECUTIVE_SUMMARY.md) | decisive findings |
| [`data/mission32_git_history.json`](data/mission32_git_history.json) | branches/commits at incident |
| [`data/mission32_deployment_evidence.json`](data/mission32_deployment_evidence.json) | Docker/functionary/comms |
| [`data/mission32_runtime_matrix.json`](data/mission32_runtime_matrix.json) | matrices + verdicts |
| [`logs/32_forensic_search.log`](logs/32_forensic_search.log) | search log |
| [`data/deployment_artifacts/`](data/deployment_artifacts/) | copied Docker build files |

---

## POST-C26D719 PRODUCTION CODE

**UNKNOWN**

## LIQUID DEPLOYMENT PATH

**PARTIALLY IDENTIFIED**

## 71C9 / 2711 HISTORICAL USE

**CANDIDATE ONLY**

## FIRST ACCEPTOR

**NOT IDENTIFIED**

## HISTORICAL CAUSALITY

**NOT DEMONSTRATED**

## FINAL ANSWER

> **Did we find public evidence that a production Liquid node capable of the Mission 28 cache exploit was running before/during 4050336?**

**NO**

Public Git history shows that **post-fix unframed** keying (via `c26d719` / `6253d7e` / `212c43f`) existed on `master`, `elements-23.x`, and `elements-23.3.x` **before** the incident — so the software **could** have been built. Tagged releases and Docker `latest` remained **pre-fix** `23.3.3`. No public artifact pins functionary/bridge `sidechaind` to those commits on 2026-09-06. No node version dump identifies the first acceptor. Therefore production capability of the Mission 28 path is **possible but unproven**; historical deployment is **not established**.
