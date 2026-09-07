# Mission 23 — Publication Readiness Audit

**Date:** 2026-09-07  
**Scope:** Pre-push review of the forensic research repository (content, secrets, scientific coherence, README accuracy).  
**Not in scope:** Application security of a deployed service; resolving the Liquid incident root cause.

---

## Final decision

```
PUBLICATION STATUS: GO WITH CHANGES
```

**Rationale:** No private keys, API tokens, RPC credentials, or other concrete secret material were found in the curated tree. The public README was updated to Mission 22 evidence and does not over-claim root cause / first acceptor. Remaining items are **hygiene and process** (omit host clones/binaries from the first public commit; optional license; human review of vulnerability-reproduction detail), not NO-GO blockers.

Unresolved forensic questions (root cause, first acceptor) are **not** publication defects.

---

## 1. Repository inventory

Working tree audited under `repro/` (~327 tracked-relevant files excluding ignored upstream clones). **Not a git repository** at audit time (`fatal: not a git repository`).

### Classification summary

| PATH | TYPE | PURPOSE | PUBLICATION STATUS |
|---|---|---|---|
| `README.md` | MD | Public entry / current conclusions | **KEEP** (updated this mission) |
| `REPORT.md` | MD | Earlier consolidated notes | **KEEP** (path redacted; README supersedes emphasis) |
| `.gitignore` | config | Exclude clones/binaries/env | **KEEP** (strengthened) |
| `LICENSE` | — | — | **ABSENT** |
| `final/01`–`21_*.md` | MD | Historical investigation reports | **KEEP** (historical; may overstate cache emphasis relative to M19–22) |
| `final/22_full_onchain_flow.md` | MD | Chain / UTXO / peg-outs | **KEEP** |
| `final/23_publication_readiness.md` | MD | This audit | **KEEP** |
| `final/data/*.json` | JSON | Blocks, scans, pegout_flow, signers | **KEEP** |
| `final/data/build_env.json` | JSON | Build host metadata | **KEEP / REDACT** (hostname redacted) |
| `final/hex/*` | hex | f24a / related proofs | **KEEP** |
| `final/logs/*` | logs | Harness outputs | **KEEP** |
| `final/patches/*` | diff/cpp | Cache-key fix excerpts | **KEEP** |
| `final/scripts/*.c` | C | Reproduction sources | **KEEP** |
| `final/scripts/{cache_repro,elements_cache_harness,batch_context_search,verify_one}` | ELF ~1.3MB | Host-built harnesses | **DELETE** from public commit (`.gitignore`; rebuild) |
| `tests/*.c` | C | Same harnesses | **KEEP** |
| `data/*` | hex/json | Broader raw captures | **KEEP** (public chain data) |
| `artifacts/*` | diff/txt | Checksums / sigcache excerpts | **KEEP** |
| `logs/*` | logs | Additional runs | **KEEP** |
| `build/*` | ELF/obj | Local builds | **DELETE** / omit (`.gitignore`) |
| `tools/package.json` + lock | JS | liquidjs helpers | **KEEP** |
| `tools/node_modules/` | deps | npm tree | **MOVE TO PRIVATE** / omit (`.gitignore`) |
| `elements/` (~299MB) | git clone | Elements source for builds | **MOVE TO PRIVATE** / omit; document clone URL |
| `secp256k1-zkp/` (~11MB) | git clone | Optional crypto tree | **MOVE TO PRIVATE** / omit |
| `.env` / `.DS_Store` / swap | — | — | **not present** / ignore |

### Representative KEEP evidence (blockchain)

All `final/data/block_405033*.json`, `chain_4050335_4050349.json`, `pegout_flow.json`, `signblock_pubkey_matches.json`, federation pubkey lists, and tx hex under `final/hex/` and `data/` are **public forensic data**, not secrets.

---

## 2. Secret scan

Patterns searched (curated tree; excluding `elements/`, `secp256k1-zkp/`, `tools/node_modules/`, `build/`):

- `BEGIN PRIVATE KEY` / RSA / OPENSSH
- `AKIA…`, `ghp_`, `github_pat_`, `xoxb-` / `xoxp-`
- `Bearer …`, `password=`, `secret=`, `token=`, `api_key=`, `private_key=`
- `Authorization:`, `Cookie:`, `rpcuser`/`rpcpassword` with live values
- mnemonic / seed-phrase style dumps

**Result:** no matches indicating live credentials in curated sources.

Hits in ignored upstream trees (`elements/doc` mentioning `rpcpassword` as documentation) are **not** repository secrets.

Federation Liquid public keys, txids, block hashes, addresses, rangeproofs, commitments: **not classified as secrets**.

`SECRET_SCAN = PASS` (curated pack).

---

## 3. Git history scan

| Item | Result |
|---|---|
| `git status` / `git log` | **N/A** — no `.git` in `repro/` or parent `test/` |
| `SECRET_IN_GIT_HISTORY` | **N/A** (no history yet) |

**Recommendation:** initialize git **after** `.gitignore` is in place; never force-add `build/`, `elements/`, `secp256k1-zkp/`, `tools/node_modules/`, or prebuilt ELFs. After first remote push, re-scan history if any accidental adds occur. Do **not** rewrite history unless a secret is later found.

---

## 4. Private data scan

| Element | Classification | Notes |
|---|---|---|
| Hostname in `build_env.json` (local VM hostname) | **PRIVATE** → redacted | Was operational host fingerprint |
| Absolute path under local home directory (`…/repro/...`) | **AMBIGUOUS** → redacted/relativized | Revealed local username/layout |
| `psgreco@gmail.com` in `final/logs/code_history_probe.log` | **PUBLIC** | Git author on Elements history probe |
| Named public developers / GitHub accounts in upstream docs | **PUBLIC** | Not treated as private |
| Personal emails/phones/home addresses of investigators | **not found** | |

---

## 5. Infrastructure data scan

| Element | Class |
|---|---|
| Blockstream Liquid explorer API URLs | **PUBLIC ARCHITECTURE** |
| Liquid federation 11-of-15 / functionary overview (Help Center) | **PUBLIC ARCHITECTURE** |
| Local hostname (pre-redaction) | **PRIVATE OPERATIONAL** → redacted |
| RPC endpoints with credentials | **not found** |
| Internal deployment configs / HSM material | **not found** |

---

## 6. Blockchain data classification

| Data | Status |
|---|---|
| Txids, block hashes/heights, addresses, pubkeys, scripts, signatures, rangeproofs, commitments, UTXO graphs, peg-out amounts | **EXPECTED FORENSIC CONTENT** |
| Hidden secrets inside hex/JSON | **none identified** |
| Mission 22 peg-outs A/B/C and A+B = `3998.66973280` BTC | **supported** by `final/data/pegout_flow.json` |

---

## 7. README consistency audit

| CLAIM | EVIDENCE | STATUS | ACTION |
|---|---|---|---|
| f24a exists / in 4050336 | `final/02`, hex, block JSON | **OK** | keep |
| Rangeproof fails independent verify | `final/03`, logs | **OK** | keep |
| Stock path rejects empty cache | `final/05`, harness logs | **OK** | keep |
| Cache vuln + lab exploit | `final/01`, `05`, patches | **OK** | keep; not historical causation |
| 11/15 signatures | fed pubkeys + match JSON | **OK** | keep |
| 4050337.prev = 4050336 | chain JSON | **OK** | keep |
| Continuity through 4050349 | `chain_4050335_4050349.json` | **OK** | README updated |
| Three peg-outs; A+B only | `pegout_flow.json` | **OK** | README updated |
| UTXO graph Mission 22 | `22_full_onchain_flow.md` | **OK** | README updated |
| First acceptor / mechanism | Missions 19–21 | **UNKNOWN** | README states UNKNOWN |
| Root cause | — | **UNRESOLVED** | README states unresolved |
| Timestamp 14:13 UTC | was inconsistent | **FIXED** → **13:53:10 UTC** | corrected |
| “Two peg-outs totaling 3998…” | stale vs M22 | **FIXED** | three peg-outs; A+B called out |

Causality language in README: distinguishes on-chain vs causal; lab vs historical. **Pass.**

---

## 8. Stale claim audit

Historical reports (`final/01`–`11` especially) discuss cache exploitability and “root cause” scoring without later acceptor boundary language. Missions 19–21/22 **do not delete** those reports.

| Stale theme | Where | Treatment |
|---|---|---|
| Cache as incomplete root-cause score | `11_final_verdict.md` | KEEP; README supersedes |
| Consensus split | discussed as unconfirmed | KEEP; README: not demonstrated |
| Explorer-only / artifact doubts (if any early notes) | earlier missions | KEEP as history |
| Chain stop at 4050343 | pre-M22 README | **FIXED** in README |

---

## 9. Reproduction command audit

| Command area | File exists | Plausible | Notes |
|---|---|---|---|
| `verify_rangeproof` build/run | `tests/verify_rangeproof.c` | yes | default data dir now relative `"data"`; needs Elements secp objects; **partial** (not full ConnectBlock) |
| `elements_cache_harness` | `tests/elements_cache_harness.c` | yes | lab A→B; matches logged expectations |
| Chain continuity python one-liner | `final/data/chain_4050335_4050349.json` | yes | verified `True` this audit |
| Full `elementsd` ConnectBlock | — | **incomplete** | README correctly says not completed |

`cache_repro.c` uses local `system("./build/cache_repro …")` — **SAFE FOR PUBLIC REPOSITORY** (local subprocess only).

All curated harnesses: **SAFE FOR PUBLIC REPOSITORY** (crypto verification / local search; no remote mutation).

---

## 10. External link audit

| URL | Check | Verdict |
|---|---|---|
| https://github.com/ElementsProject/elements | fetched OK | **OK** |
| https://github.com/ElementsProject/elements/releases/tag/elements-23.3.3 | primary source | **OK** (known release tag; pattern consistent) |
| https://github.com/ElementsProject/secp256k1-zkp | related project | **OK** |
| https://docs.liquid.net/ | docs | **OK** (relevant) |
| https://elementsproject.org/ | project site | **OK** |
| https://help.blockstream.com/liquid-network/faqs/what-is-a-liquid-network-functionary | fetched OK | **OK** |
| `https://blockstream.info/liquid/api/` | explorer API used for captures | **OK** (base API; not fabricated) |

No fabricated URLs found in README.

---

## 11. Large file audit

| FILE / DIR | SIZE | PURPOSE | RECOMMENDATION |
|---|---|---|---|
| `elements/` | ~299MB | Upstream clone | **Omit** from public push; clone from GitHub |
| `tools/node_modules/` | large | npm deps | **Omit**; `npm ci` from lockfile |
| `build/*.o` / harness ELFs | ~1.0–1.3MB each | Host builds | **Omit**; rebuild |
| `final/scripts/*` ELFs | ~1.3MB each | Same | **Omit** (gitignored) |
| `secp256k1-zkp/` | ~11MB | Optional clone | **Omit** |
| Largest hex (`f24a…hex`) | ~26KB | Evidence | **KEEP** (no LFS needed) |
| Block JSON under `final/data/` | small–moderate | Evidence | **KEEP** |

Git LFS: **not required** for curated evidence if clones/binaries are omitted.

---

## 12. License status

```
LICENSE = ABSENT
```

Do **not** invent a license. Recommend human choice (e.g. CC-BY / MIT / research disclaimer) before or shortly after publication.

---

## 13. Recommended changes

### Applied during this audit

1. Redacted `final/data/build_env.json` hostname.  
2. Relativized default data path in `tests/verify_rangeproof.c` and `final/scripts/verify_rangeproof.c`.  
3. Softened absolute path in `REPORT.md`.  
4. Strengthened `.gitignore` (clones, `build/`, harness binaries, env/editor junk).  
5. Updated root `README.md` for Mission 22 (chain→4050349, three peg-outs, UTXO graph, Publication Safety, UNKNOWN acceptor, status banner).  
6. Updated `final/README.md` pointers to M22–23.  
7. Corrected block timestamp display to **13:53:10 UTC**.

### Remaining before / at push (operator)

1. `git init` only with `.gitignore` respected; do not `git add -f` binaries/clones.  
2. Optionally delete local ELFs under `final/scripts/` and `build/` from the release tarball working copy (already ignored).  
3. Choose a `LICENSE` deliberately.  
4. **Human review** of vulnerability reproduction detail (below).  
5. Re-run a secret scan after the first commit.

---

## 14. Responsible disclosure / vulnerability content

| Topic | Classification |
|---|---|
| Incomplete rangeproof cache key (historical Elements) | **ALREADY PUBLIC** (upstream fix commits `c26d719` / related; discussed in investigation) |
| Laboratory A→B false-positive reproduction | **Research / reproduction of known class of bug** — flag for **human review** before loud “exploit how-to” marketing; technical content is appropriate for a forensic repo if framed as lab-only |
| Production exploitation of cache in the incident | **NOT CLAIMED** (correct) |

Do **not** auto-remove harnesses or patches.

---

## Scripts safety table

| Script | Objective | Credentials | Remote mutate | Verdict |
|---|---|---|---|---|
| `verify_rangeproof.c` | Verify P1 under L-BTC context | no | no | **SAFE** |
| `elements_cache_harness.c` | Lab old/new cache key behavior | no | no | **SAFE** |
| `cache_repro.c` | Lab priming demo | no | no | **SAFE** |
| `batch_context_search.c` / `search_contexts.c` | Search verifying contexts | no | no | **SAFE** |
| `pedersen_tally_f24a.c` / `c1_constraint_probe.c` | Balance / constraints | no | no | **SAFE** |
| `verify_one.c` / `rp_info.c` | Single-proof helpers | no | no | **SAFE** |

---

## README path existence (post-update)

All markdown link targets in `README.md` resolve **except** this file prior to write; after write: **PATH EXISTS = YES** for `final/23_publication_readiness.md` and Mission 22 artifacts referenced.

---

## Publication Safety statement (for operators)

The repository contains public blockchain forensic data and reproducibility artifacts. No credentials or private keys were intentionally included. Public chain data is not treated as secret. Identified sensitive operational fingerprints (hostname, absolute home paths) were redacted. This audit is **not** a claim of “security audited” or “100% safe.”

---

## Final status block (mirrors README)

```
Investigation status: ONGOING / ROOT CAUSE UNRESOLVED
FIRST ACCEPTOR: UNKNOWN
FIRST ACCEPTANCE MECHANISM: UNKNOWN
FIRST UNPROVEN LINK: the first runtime that placed block 4050336
                     into an active chainstate.
```

**No causal narrative is justified beyond this point.**

---

## GO / NO-GO checklist

| Criterion | Met? |
|---|---|
| No private key / credential found | **Yes** |
| No material unsupported causal claim in README | **Yes** (after update) |
| Mission 22 reflected | **Yes** |
| Historical reports preserved | **Yes** |
| Remaining hygiene actions | **Yes** → **GO WITH CHANGES** |

```
PUBLICATION STATUS: GO WITH CHANGES
```
