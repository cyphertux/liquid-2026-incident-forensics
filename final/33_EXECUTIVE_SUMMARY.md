# Mission 33 — Executive summary

## Decisive findings

1. **`liquid-functionary` does not determine the Elements/`sidechaind` commit** — it is a Rust blocksigner/watchman talking to an external Elements Core over RPC; no submodule, no Dockerfile, no 2026 commits.
2. **Public Docker/`bitcoin-images` path packages release tags** (default **23.3.3** pre-fix); last elementsd bump **2026-04-14**; no post-fix functionary image found before the incident.
3. **Post-fix source before the incident remains CONFIRMED** (Mission 32) — that is SOURCE existence, not deployment.
4. **Every arrow from BUILD → DEPLOYMENT → NODE → CACHE is UNKNOWN** in public evidence.
5. **`71c9`/`2711` stay CANDIDATE ONLY**; first acceptor **NOT IDENTIFIED**; historical causality **NOT DEMONSTRATED**.

## Verdicts

| Axis | Result |
|---|---|
| POST-C26D719 CODE BEFORE INCIDENT | **CONFIRMED** |
| FUNCTIONARY SOURCE COMMIT | **UNKNOWN** |
| FUNCTIONARY BUILD | **UNKNOWN** |
| PRODUCTION DEPLOYMENT | **UNKNOWN** |
| 71C9 / 2711 HISTORICAL PRIMING | **CANDIDATE ONLY** |
| FIRST ACCEPTOR | **NOT IDENTIFIED** |
| HISTORICAL CAUSALITY | **NOT DEMONSTRATED** |

## Final answer

**NO** — public build/deployment evidence cannot establish that a Mission 28–capable production node was running during acceptance of 4050336.

Full report: [`33_functionary_build_forensics.md`](33_functionary_build_forensics.md).
