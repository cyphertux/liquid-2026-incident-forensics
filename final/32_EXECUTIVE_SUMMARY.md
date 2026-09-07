# Mission 32 — Executive summary

## Decisive findings

1. **Post-fix unframed cache keying existed on public branches before the incident** — `master` (`c26d719`), `elements-23.x` (`6253d7e`), and `elements-23.3.x` (`212c43f`, already in tip by 2026-09-04).
2. **Official tags and Docker `latest` did not** — still `23.3.3` pre-fix (Hub updated 2026-04-14).
3. **PR #1599 merge is after the incident**, but that does **not** mean `23.3.x` lacked the patch beforehand.
4. **Deployment path partially mapped** — public Docker builds from release tarballs; functionary repo does not pin `sidechaind` Elements commit.
5. **No public proof** any production Liquid node ran post-fix code on 2026-09-06; first acceptor still unidentified; `71c9`/`2711` remain Level A–C candidates only.

## Verdicts

| Axis | Result |
|---|---|
| POST-C26D719 PRODUCTION CODE | **UNKNOWN** |
| LIQUID DEPLOYMENT PATH | **PARTIALLY IDENTIFIED** |
| 71C9 / 2711 HISTORICAL USE | **CANDIDATE ONLY** |
| FIRST ACCEPTOR | **NOT IDENTIFIED** |
| HISTORICAL CAUSALITY | **NOT DEMONSTRATED** |

## Final answer

**NO** — no public evidence establishes that a production Liquid node capable of the Mission 28 exploit was running before/during 4050336.

Full report: [`32_production_runtime_forensics.md`](32_production_runtime_forensics.md).
