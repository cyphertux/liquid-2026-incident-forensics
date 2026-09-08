#!/usr/bin/env bash
# Stub: produce primer.hex + alias_block.hex for Mission 40.
# Currently FAILS CLOSED — documents what remains once post-fix elementsd exists.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$ROOT/mission40/results"
mkdir -p "$OUT"
cat > "$OUT/TX_BLOCK_BUILDER_TODO.md" << 'EOF'
# Remaining work for Mission 40 (blocked on post-fix binary / confidential tx craft)

## Goal
Produce:
- `mission40/results/primer.hex` — raw tx whose validation SETS cache for (P0,C0,G,S0)
- `mission40/results/alias_block.hex` — block whose ConnectBlock looks up (P1,C1,G,S1)

## Non-negotiables
- Exact fixture bytes from `mission40/fixtures/{P0,C0,S0,P1,C1,S1}.hex`
- G derived as in Mission 28 (`mission40/expected/fixture_check.json`)
- Primer enters via normal validation (`sendrawtransaction`), not cache poke
- Alias block via `submitblock` / template path → real ConnectBlock

## Why not done in Mission 39
Offline construction of Elements confidential txs that:
1) embed arbitrary rangeproof bytes,
2) satisfy Pedersen balance + surjection,
3) spend valid regtest UTXOs,
requires either a running post-fix wallet stack or a bespoke tx builder linked to Elements libs.

## Suggested Mission 40 approach
1. Build+instrument elementsd (`mission40_build.sh` + `mission40_instrument.sh`).
2. Mine regtest mature coinbases; create blinded outputs via wallet.
3. If wallet cannot attach *exact* Mission 28 proofs, implement a rawtx editor that replaces
   rangeproof/value/script fields then re-signs / re-blinds companions — **document any field changes**.
4. Export hex files; re-run `mission40_run.sh` with `MISSION40_PRIMER_TX` and `alias_block.hex`.
5. Run `mission40_verify_D.sh`.

## Alternate (weaker) path
Generate a *new* algebraic alias pair on regtest (same absorb construction) if exact historical
bytes cannot be balanced. Still valid for mechanism D; weaker for f24a identity comparison.
EOF
echo "STUB: see $OUT/TX_BLOCK_BUILDER_TODO.md" >&2
exit 1
