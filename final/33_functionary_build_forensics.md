# Mission 33 — Liquid functionary build & deployment forensics

**Date:** 2026-09-07
**Prior:** Mission 32 established post-fix **source** on public Elements branches before 4050336; deployment remained UNKNOWN.
**Goal:** Reconstruct, from **public** artifacts only, the Elements / `sidechaind` commit that functionaries could have run on 2026-09-06.

---

## Dependency graph (FACT)

```text
Blockstream/liquid-functionary  (Rust: blocksigner, watchman, HSM tools)
        │
        │  JSON-RPC only (sidechaind_rpc_url / user / pass)
        ▼
   sidechaind  ≈  Elements Core (elementsd)     ← OPERATOR-SUPPLIED, NOT BUILT HERE
        │
        │  (version / commit NOT pinned in public functionary repo)
        ▼
   CachingRangeProofChecker / sigcache.cpp      ← lives ONLY in Elements C++ tree
```

Also:

```text
liquid-functionary
        └── crates.io  elements = 0.24.1   (Rust serialization library)
                       ≠ Elements Core consensus cache
```

**There is no public arrow** `liquid-functionary commit → Elements SHA → sidechaind binary`.

---

## PART 1–4 — `liquid-functionary` complete

| Item | Finding | Class |
|---|---|---|
| Submodule / vendored Elements C++ | **Absent** | FACT |
| Dockerfile / `.github/workflows` | **Absent** | FACT |
| Commits in Aug–Sep 2026 | **None** | FACT |
| Last master tip | `d3ccee17…` **2024-03-22** (v2.4.0) | FACT |
| Branch `master-2.4.1` | `ee77c89f…` **2024-04-19** | FACT |
| GitHub `pushed_at` | **2024-06-11** | FACT |
| `c26d719` / `6253d7e` / `212c43f` / `ComputeEntryRangeProof` | **Not present** | FACT |
| Role vs Elements | RPC client: `getnewblockhex`, `testproposedblock`, `submitblock` | FACT |

Artifact: [`data/mission33_functionary_git.json`](data/mission33_functionary_git.json).

**Conclusion:** the public functionary repository **cannot** answer “which Elements commit was compiled into sidechaind.”

---

## PART 5–7 — `bitcoin-images` / Docker / CI

| Item | Finding |
|---|---|
| `elementsd` Dockerfile | Downloads **GitHub release** tarball `elements-${ELEMENTS_VERSION}` + GPG |
| Default `VER` | **23.3.3** (bump commit `26679ef` **2026-04-14**) |
| Aug–Sep 2026 elementsd changes | **None** |
| GitLab CI `build_elementsd` | **Manual**, on `elementsd/**` changes |
| Post-fix image before incident | **Not found** |
| Gitian README | Allows branch/commit — **capability**, no published post-fix functionary image found |
| Link to functionary deploy | **NOT ESTABLISHED** |

`liquidd-legacy` (v2.14.1.24 via `liquid-packages.blockstream.com`) is a **legacy** path; package index returned **404** this mission — not identified as 2026 functionary runtime.

Artifact: [`data/mission33_docker_artifacts.json`](data/mission33_docker_artifacts.json), copies under [`data/deployment_artifacts/`](data/deployment_artifacts/).

---

## PART 8–9 — Update process & bridge vs functionary

Help Center FAQ (updated **2026-08-06**): functionary = server + HSM; round-robin; peg management.
**Does not** document Elements version pinning, binary replacement, or emergency hotfix procedure.

| Role | Public software identity |
|---|---|
| Functionary blocksigner/watchman | `liquid-functionary` Rust + **external** sidechaind |
| Bridge nodes (“Bridge nodes are patched…”) | **Not identified** in public repos this mission |
| Shared Elements version? | **UNKNOWN** |

Do **not** infer that “bridge patched” ⇒ functionary `sidechaind` commit = `c26d719`.

---

## PART 10–11 — Timeline & PR #1599

| Date | Elements / deploy note |
|---|---|
| 2026-09-01 | `c26d719` on master |
| 2026-09-02/03 | `6253d7e` / `212c43f` on 23.x / 23.3.x |
| 2026-09-04 | Branch tips post-fix (Mission 32) |
| 2026-09-06 13:52–13:53 | 4050335 → 4050336 |
| 2026-09-06 | **No** public functionary / elementsd Docker change |
| 2026-09-06 17:21Z | PR #1599 merge (cleanup/cherry-pick packaging; patch **already** on 23.3.x) |

Plausible path `post-fix source → functionary build → deploy → 4050336`: **NOT ESTABLISHED**.

---

## PART 12 — Reconstructed builds (source matrix)

Full `elementsd` binaries were **not** rebuilt (no functionary-pinned commit to target).
Source classification via `git show` ([`data/reconstructed_builds/source_cache_matrix.json`](data/reconstructed_builds/source_cache_matrix.json)):

| Build | Cache key | Mission 28 alias |
|---|---|---|
| A `elements-23.3.3` | PRE-FIX | **NO** |
| B `23.3.x` @ pre-incident tip | POST-FIX unframed | **YES** |
| C `c26d719` | POST-FIX unframed | **YES** |
| D/E `6253d7e` / `212c43f` | POST-FIX unframed | **YES** |
| F `29.x` tip | PRE-FIX | **NO** |
| G `master` pre-incident | POST-FIX unframed | **YES** |
| E_functionary_pin | — | **N/A — unidentified** |

---

## PART 13–15 — Binary ID / primers / logs

| Search | Result |
|---|---|
| Historical functionary `sidechaind` SHA / build-id / Docker digest | **NOT FOUND** |
| Production `debug.log` / mempool dump for 71c9/2711/f24a | **NOT FOUND** |
| 71c9/2711 if runtime were post-fix | Still **historically plausible primers** (Levels A–C) — **not** demonstrated cache use |

---

## PART 16 — Evidence arrows

| Arrow | Status |
|---|---|
| SOURCE → BUILD (functionary sidechaind) | **UNKNOWN** |
| BUILD → BINARY | **UNKNOWN** |
| BINARY → DEPLOYMENT | **UNKNOWN** |
| DEPLOYMENT → NODE | **UNKNOWN** |
| NODE → MEMPOOL (71c9/2711) | **UNKNOWN** |
| MEMPOOL → CACHE | **UNKNOWN** |
| CACHE → f24a accept | lab **CONFIRMED** (Mission 28); historical **UNKNOWN** |
| f24a → 4050336 inclusion | on-chain **FACT**; first acceptor **UNKNOWN** |

---

## PART 17–18 — Falsification / equivalents

- Public default Docker/`23.3.3` = **PRE-FIX** — **compatible with** “many public nodes pre-fix,” **not proof** functionaries matched that.
- Public branches post-fix from Sep 3 — **compatible with** private deploy of post-fix, **not proof**.
- No public log **refutes** or **confirms** `71c9→cache→f24a`.
- Functional equivalents before incident: `c26d719`, `6253d7e`, `212c43f` (and branch tips containing them) — Mission 32.

---

## Evidence matrix

| Element | Evidence | Date | Source | Status |
|---|---|---|---|---|
| post-fix public source | branch tips | ≤2026-09-04 | Elements git | **CONFIRMED** |
| post-fix on 23.3.x | `212c43f` | 2026-09-03 | Elements git | **CONFIRMED** |
| functionary → Elements pin | — | — | liquid-functionary | **ABSENT / UNKNOWN** |
| functionary build of sidechaind | — | — | public CI | **NOT FOUND** |
| Docker post-fix pre-incident | — | — | bitcoin-images / Hub | **NOT FOUND** |
| production deployment of post-fix | — | — | — | **UNKNOWN** |
| 71c9 / 2711 exact alias | bins | Mission 30 | hex | **CONFIRMED** (crypto) |
| mempool / cache on acceptor | — | — | — | **UNKNOWN** |
| first acceptor | — | — | — | **UNKNOWN** |

---

## Deliverables

| Path | Role |
|---|---|
| [`33_functionary_build_forensics.md`](33_functionary_build_forensics.md) | this report |
| [`33_EXECUTIVE_SUMMARY.md`](33_EXECUTIVE_SUMMARY.md) | decisive findings |
| [`data/mission33_functionary_git.json`](data/mission33_functionary_git.json) | repo forensics |
| [`data/mission33_build_matrix.json`](data/mission33_build_matrix.json) | dependency + cache matrix |
| [`data/mission33_docker_artifacts.json`](data/mission33_docker_artifacts.json) | Docker/CI |
| [`data/mission33_deployment_evidence.json`](data/mission33_deployment_evidence.json) | arrows / roles |
| [`data/mission33_version_timeline.json`](data/mission33_version_timeline.json) | timeline |
| [`data/reconstructed_builds/source_cache_matrix.json`](data/reconstructed_builds/source_cache_matrix.json) | source classification |
| [`logs/33_functionary_forensics.log`](logs/33_functionary_forensics.log) | search log |

---

## POST-C26D719 CODE BEFORE INCIDENT

**CONFIRMED**

## FUNCTIONARY SOURCE COMMIT

**UNKNOWN**

## FUNCTIONARY BUILD

**UNKNOWN** *(Rust stack identified; `sidechaind`/Elements build not)*

## PRODUCTION DEPLOYMENT

**UNKNOWN**

## 71C9 / 2711 HISTORICAL PRIMING

**CANDIDATE ONLY**

## FIRST ACCEPTOR

**NOT IDENTIFIED**

## HISTORICAL CAUSALITY

**NOT DEMONSTRATED**

## FINAL ANSWER

> **Can the public build/deployment evidence establish that a Liquid production node capable of the Mission 28 cache exploit was running during the acceptance of 4050336?**

**NO**

The public `liquid-functionary` tree is a Rust RPC client to an external `sidechaind`; it does not pin, vendor, or build Elements Core, and it has no 2026 commits. Public `bitcoin-images/elementsd` still packages release tag **23.3.3** (pre-fix) with no post-fix image before the incident. Post-fix **source** on Elements branches is confirmed, but SOURCE→BUILD→BINARY→DEPLOYMENT for functionaries cannot be reconstructed from public artifacts. Therefore production capability during 4050336 remains unproven.
