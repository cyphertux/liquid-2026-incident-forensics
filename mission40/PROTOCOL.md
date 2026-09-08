# Mission 40 protocol (execute after post-fix build)

## 0. Preconditions
```bash
./scripts/mission40_verify_fixtures.py
./scripts/mission40_build.sh          # needs deps
./scripts/mission40_instrument.sh     # then rebuild
# provenance: mission40/build_provenance.json
```

## 1. Start identical nodes
`./scripts/mission40_run.sh` starts A/B; exits 2 until tx/block artifacts exist.

## 2. Produce artifacts (Mission 40 implementation work)
Write:
- `mission40/results/primer.hex` — raw tx validated on Node A via `sendrawtransaction` that causes CACHE SET for primer context `(P0,C0,G,S0)`.
- `mission40/results/alias_block.hex` — block containing alias output context `(P1,C1,G,S1)` for ConnectBlock.

Constraints:
- Do **not** poke the cache directly.
- Prefer RPC `sendrawtransaction` / `submitblock` / `getblocktemplate`+`submitblock`.
- Same `alias_block.hex` to A and B.

## 3. Hot/cold
- Node A: primer first (process kept alive) → submit alias block.
- Node B: never saw primer → same alias block.
- Expect logs: A HIT / B MISS (if instrumented).

## 4. Restart test
```bash
# after primer on A:
elements-cli -datadir=mission40/nodeA stop
# restart A without primer
# submit alias → expect MISS/REJECT (process-local cache)
```

## 5. Framed control
```bash
git checkout --detach $RECOMMENDED_COMMIT
./scripts/mission40_apply_framed.sh
# rebuild into mission40/build-framed/
# repeat hot/cold → expect no bypass
```

## 6. Verify
```bash
./scripts/mission40_verify_D.sh mission40/results
# Look for POTENTIAL D — REVIEW REQUIRED only if checklist green
```
