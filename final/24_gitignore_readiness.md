# Mission 24 — `.gitignore` / Git publication hygiene

**Date:** 2026-09-07  
**Scope:** First-commit readiness via `.gitignore` only.  
**Not in scope:** Scientific conclusions, LICENSE creation, commit, push, root-cause work.

---

## Verdict

```
PUBLICATION STATUS: GO
```

`git add .` stages the public forensic pack and excludes clones, dependencies, builds, and host binaries **without** `git add -f`. No private keys, `.env`, or environment-specific binaries are staged. Scientific wording was not modified.

---

## 1. `.gitignore` before / after

### Before (Mission 23, abbreviated)

```
elements/
secp256k1-zkp/
tools/node_modules/
build/
*.o
final/scripts/{batch_context_search,cache_repro,elements_cache_harness,verify_one,verify_rangeproof}
.DS_Store / Thumbs.db / *.swp / .vscode/ / .idea/
.env / .env.*
__pycache__/ / .pytest_cache/ / .venv/ / venv/
```

### After (this mission)

Added / strengthened:

| Rule | Purpose |
|---|---|
| `node_modules/` | any nested npm trees |
| `dist/` `target/` | common build output dirs |
| `*.a` `*.so` `*.dylib` `*.dll` `*.elf` `*.exe` | compiled artifacts |
| Extra `final/scripts/*` binary names | future/local harness builds without extension |
| `!.env.example` | allow env template if added later |
| `*.tmp` `*.temp` `*.swo` `*~` `*.code-workspace` | editor / temp noise |
| `*.py[cod]` `$py.class` `.mypy_cache/` | Python noise |
| `*.log` + `!logs/*.log` + `!final/logs/*.log` | ignore stray logs; **keep** evidence logs |
| `*.bak` `*.orig` `.cache/` | misc local |

**Explicitly not ignored:** `final/`, `data/`, `artifacts/`, `tests/`, `*.md`, `*.json`, `*.hex`, `*.csv`, evidence `*.log` under `logs/` and `final/logs/`.

---

## 2. Directories / paths excluded

| Path | Reason |
|---|---|
| `elements/` (~299MB) | upstream clone |
| `secp256k1-zkp/` (~11MB) | upstream clone |
| `tools/node_modules/` | npm dependencies |
| `build/` (~14MB) | local compile outputs + ELFs |
| `final/scripts/{cache_repro,elements_cache_harness,batch_context_search,verify_one,…}` | host ELF binaries |
| `.env` / `.env.*` | credentials (none present; rule prophylactic) |

---

## 3. Important paths kept trackable

| Path | Role |
|---|---|
| `README.md` | public entry / UNKNOWN / publication safety |
| `REPORT.md` | earlier notes |
| `.gitignore` | this hygiene |
| `final/01`–`23_*.md` | mission reports |
| `final/data/*` | chain, pegouts, blocks, checksums |
| `final/hex/*` | extracted proofs |
| `final/logs/*.log` | harness evidence outputs |
| `final/patches/*` | cache-fix diffs / excerpts |
| `final/scripts/*.c` | reproduction sources |
| `tests/*.c` | same sources |
| `data/*` | broader on-chain captures |
| `artifacts/*` | checksums / sigcache excerpts |
| `logs/*.log` | additional harness logs |
| `tools/package.json` + `package-lock.json` | JS tooling pins (no `node_modules`) |

---

## 4. Simulation performed

```text
git init          # empty repo at repro/
git add .         # no -f
git status
git diff --cached --stat
```

**Result:** **312 files** staged (includes this report), ~13k insertions (as of this audit).

### `git status --ignored` (abbreviated)

```text
!! build/
!! elements/
!! final/scripts/batch_context_search
!! final/scripts/cache_repro
!! final/scripts/elements_cache_harness
!! final/scripts/verify_one
!! secp256k1-zkp/
!! tools/node_modules/
```

### Mandatory staged checks

| Check | Result |
|---|---|
| `elements/` staged | **No** |
| `secp256k1-zkp/` staged | **No** |
| `node_modules/` staged | **No** |
| `build/` staged | **No** |
| ELF / compile binaries staged | **No** (`file` scan of index empty) |
| `.env` staged | **No** (none present; rule active) |
| Private key / API token blobs | **No** (pattern hits only in audit docs describing scans) |
| Personal host path / live hostname in staged env files | **No** (`build_env.json` remains redacted) |

### Evidence logs

After `git add .`, `final/logs/*.log` and `logs/*.log` **are staged**.  
Note: `git check-ignore -v` may print the un-ignore rule (`!…/*.log`); disposition after `git add .` confirms they remain public.

---

## 5. Suspect / false-positive notes

| Item | Assessment |
|---|---|
| Blanket `*.json` / `*.md` / `data/` | **Not used** (would be false positives) |
| Blanket `*.log` | Softened with `!logs/*.log` and `!final/logs/*.log` so evidence is kept |
| Mentions of `BEGIN PRIVATE KEY` / `/home/maxime` inside `final/23_publication_readiness.md` | Documentation of prior audit/redaction — **not** live secrets |
| Empty `logs/*.err` / empty log stubs | Harmless; optional cleanup later, not blocking |
| Nested `.git` inside `elements/` / `secp256k1-zkp/` | Irrelevant while those trees stay ignored |

---

## 6. Remaining operator notes (non-blocking)

1. Repo is **initialized** and currently **staged** for a human `git commit` (not performed here).  
2. Default branch name from `git init` is `master`; rename to `main` if desired before GitHub push.  
3. `LICENSE` still **ABSENT** — do not invent (Mission 23).  
4. Clone Elements / secp and `npm ci` in `tools/` locally when rebuilding harnesses.  
5. Do not use `git add -f` on ignored paths.

---

## 7. Scientific content

**Unchanged.** No README thesis edits in this mission. Distinctions FACT / REPRODUCED / INFERENCE / HYPOTHESIS / UNKNOWN preserved as previously published.

---

## Final status

```
PUBLICATION STATUS: GO
```

Ready for:

```bash
git diff --cached
git commit
```

(No push performed.)
