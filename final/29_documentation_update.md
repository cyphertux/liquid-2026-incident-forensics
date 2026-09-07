# Mission 29 — Integrate Mission 28 counter-expertise into the public dossier

**Type:** Documentary only — no new exploit hypothesis, no invented historical proof.
**Inputs:** [`27_exact_p1c1_priming.md`](27_exact_p1c1_priming.md), [`28_red_team_review.md`](28_red_team_review.md), associated `data/28_*.json`, `p0_p1_byte_comparison.json`, `68_rangeproof_*.json`, `cache_priming_candidates.json`.

---

## Changes made

| File | Action |
|---|---|
| [`../README.md`](../README.md) | Rewrote current narrative: Executive Summary, Status Matrix, Timeline (2026-09-01/02/03/06), Technical Mechanism + ASCII diagram, Revision of Mission 27, Remaining Historical Link, hypotheses / ruled-out / not-proven, reading order 24–29, reproduction B2, Current Conclusion / Verdict |
| [`../REPORT.md`](../REPORT.md) | Appended **Revision after Mission 28** (preserved §§1–13) |
| [`README.md`](README.md) | Pointed pack index at Missions 27–29 |
| This file | Mission 29 deliverable |

No new experimental harnesses or historical claims were added.

---

## Obsolete formulations corrected

| Old (or over-strong) wording | Correction |
|---|---|
| “SHA-256 collision” | **REFUTED** as description → **preimage aliasing** (identical 4301-byte preimage before SHA-256) |
| “elements-23.3.3 contains the fix” | **REFUTED** — tag remains **pre-`c26d719`** |
| “4050336 was accepted” (from checker alone) | RP-validation **locus** reproducible; full historical `ConnectBlock` **UNPROVEN** |
| “cache refuted as incident cause” (Mission 27 overreach) | Mechanism **CONFIRMED**; historical causality **UNPROVEN** |
| Pre-fix `71c9` primes `H(P1‖C1)` | Remains **REFUTED** (Missions 12/25) |
| ~68 Q0 proofs prime f24a | Remains **REFUTED** (Mission 26) |
| “P1 must be Verify→Set to exploit” | **REFUTED** — alias path uses valid primer |
| “confirmed root cause” / “definitively caused” | **Not used**; root cause unresolved |

---

## New / restated verdicts

### CONFIRMED

- Post-fix unframed keying vulnerability (non-injective concatenation).
- Primer and f24a:1 → identical 4301-byte post-fix cache preimages **before** SHA-256.
- Primer verifies **TRUE**; f24a:1 verifies **FALSE**.
- Cache Set → Hit at `CachingRangeProofChecker`; salt-independent; `erase=true` does not immediately remove entry; framing breaks alias.
- Primed post-fix node can pass the **rangeproof locus**; cold post-fix / pre-fix primed reject.

### STRONGLY SUPPORTED

- Mechanism matches the behavior needed for an invalid rangeproof to skip crypto at that locus; coherent with f24a / 4050336 anomaly **as a candidate**.

### UNPROVEN

- Production nodes ran post-fix at incident time.
- Primer reached accepting mempools / cache.
- Full historical `ConnectBlock` via this path.
- First acceptor; exact production config / functionary versions.
- Numeric amount in C1.
- Cache mechanism **caused** the incident.

### REFUTED

- ~68 Q0 as direct f24a primers; pre-fix `71c9` priming of `H(P1‖C1)`; “SHA-256 collision”; “23.3.3 contains the fix”; “P1 must be pre-validated for the exploit.”

---

## References — Missions 27 and 28

| Mission | Role after update |
|---|---|
| **27** | Kept: falsifies honest Verify→Set of exact `(P1,C1)`. Over-broad “cache refuted as cause” **superseded**. |
| **28** | Authoritative for lab mechanism: byte identity, crypto TRUE/FALSE, Set→Hit, framing, erase behavior. Historical causality **UNPROVEN**. |

Artifacts: `final/data/28_key_reconstruction.json`, `28_byte_identity.json`, `28_cache_replay.json`, `28_crypto_verification.json`, `28_historical_evidence.json`, `logs/28_red_team_reproduction.log`, `scripts/28_cache_replay.c`.

---

## Exact unknowns (remaining)

1. Functionary / bridge-node version inventory at 2026-09-06.
2. Mempool acceptance of `71c93d43` / `27114710` on those nodes.
3. `ProcessNewBlock` / `ActivateBestChain` / cache-state evidence.
4. Identity of the first accepting runtime.
5. End-to-end historical `ConnectBlock(4050336)` under the primed post-fix path.

---

## Next scientific question

> Were the nodes that accepted block 4050336 running the post-fix keying from `c26d719c29`, and had the `71c9`/`2711` primers already placed the colliding cache entry before the block arrived?

This is an **operational / historical** question, not a cryptographic one.

---

## Publication safety

Documentary pass only: no secrets, private keys, credentials, tokens, internal hostnames, or production logs were added.

---

## Validation notes

- `git diff --check` expected clean after edits.
- Dangerous phrases (`confirmed root cause`, `definitively caused`, `the functionaries ran`, `4050336 was accepted because`, affirmative “SHA-256 collision”) must not appear as claims in updated public docs.
