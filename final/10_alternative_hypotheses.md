# 10 — Alternative hypotheses

## secp256k1-zkp PR #369 / #370 / Elements #1582

| Item | What it is | Link to incident? |
|---|---|---|
| PR #369 | Surjection: bind genrand nonce to full statement | **C — not demonstrated as cause**. Different proof system than rangeproof cache. |
| PR #370 | Documents that rangeproof nonce must not be reused across differing `extra_commit`/message; **does not change verification** | **B — thematically related** to context-sensitivity of proofs, but **not** the cache-key bug and not shown to enable f24a acceptance |
| Elements #1582 | Bumps secp submodule | **C** unless a specific verification-behavior change is shown (not shown here) |

Conclusion: these PRs are **not** substitutes for the demonstrated cache-key incompleteness. They are recent crypto hygiene changes around **prover nonce** handling.

## Other alternatives considered

| Hypothesis | Status |
|---|---|
| Pure rangeproof math bug in secp (verify accepts invalid under L-BTC) | **REFUTED for P1 under L-BTC** — verify returns FALSE |
| Surjection bug for f24a out1 | **REFUTED** — explicit asset, empty surjection (and Elements rejects nonempty surjection on explicit assets) |
| Parser / wrong API usage | **REFUTED** — same binary validates sibling outs TRUE |
| HSM / functionary key compromise required | **UNCONFIRMED**; scenario analysis says **not required** if consensus falsely accepts state (**INFERENCE**) |
| Double-spend / wrong block hash | **REFUTED** for quoted identifiers — explorer confirms block/tx linkage |
| PSET/blinding tooling bug alone | **PLAUSIBLE as construction aid**, not shown as consensus acceptance mechanism |
| Different consensus bug entirely | **PLAUSIBLE** — cannot be excluded while context A for P1/C1 remains missing |

## Pedersen / “negative value” (Part 18–19)

`C = v·G_asset + r·H`.  
Without recovering `r`, **exact v is NOT DETERMINED**.  
We do **not** assert a negative or huge plaintext value for C1.

Why P+C can verify under GA and fail under GB (general crypto fact, demonstrated artificially):
rangeproof challenges bind to the generator and `extra_commit`; changing either changes the statement. A proof authored for statement A does not verify for statement B.  
**For historical P1 specifically, statement A was not found.**
