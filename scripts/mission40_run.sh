#!/usr/bin/env bash
# Mission 40 run orchestrator — requires post-fix elementsd from mission40_build.sh
# Does NOT declare D; emits POTENTIAL D — REVIEW REQUIRED only if checklist passes.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck source=/dev/null
source "$ROOT/mission40/COMMIT.txt"

log() { printf '[mission40_run] %s\n' "$*"; }
die() { printf '[mission40_run] ERROR: %s\n' "$*" >&2; exit 1; }

"$ROOT/scripts/mission40_verify_fixtures.py" \
  || die "fixture check failed"

PROV="$ROOT/mission40/build_provenance.json"
[[ -f "$PROV" ]] || die "missing $PROV — run scripts/mission40_build.sh first"
BIN="$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["binary"])' "$PROV")"
COMMIT="$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["commit"])' "$PROV")"
[[ -x "$BIN" ]] || die "binary not executable: $BIN"
[[ "$COMMIT" == "$RECOMMENDED_COMMIT" ]] || die "provenance commit mismatch"
"$BIN" -version 2>&1 | grep -q "$REJECT_IF_VERSION_CONTAINS" \
  && die "refusing pre-fix 23.3.3 binary"

CLI="${BIN%d}cli"
[[ -x "$CLI" ]] || CLI="$(dirname "$BIN")/elements-cli"
[[ -x "$CLI" ]] || die "elements-cli not found next to elementsd"

# Apply instrumentation if not present (requires rebuild — warn)
if ! grep -q 'MISSION40 CACHE' "${ELEMENTS_DIR:-$ROOT/elements}/src/script/sigcache.cpp" 2>/dev/null; then
  log "WARNING: source not instrumented. Prefer: scripts/mission40_instrument.sh && rebuild"
fi

DATAA="$ROOT/mission40/nodeA"
DATAB="$ROOT/mission40/nodeB"
mkdir -p "$DATAA" "$DATAB"
# shellcheck source=/dev/null
source "$DATAA/ports.env"
RPC_A=$RPC_PORT; P2P_A=$P2P_PORT
# shellcheck source=/dev/null
source "$DATAB/ports.env"
RPC_B=$RPC_PORT; P2P_B=$P2P_PORT

rpc() {
  local datadir=$1; shift
  "$CLI" -datadir="$datadir" -chain=regtest \
    -rpcuser=mission40 -rpcpassword=mission40-lab-only-not-a-secret "$@"
}

start_node() {
  local datadir=$1 rpc=$2 p2p=$3
  "$BIN" -datadir="$datadir" -conf="$datadir/elements.conf" -chain=regtest -daemon \
    -port="$p2p" -rpcport="$rpc" \
    -rpcuser=mission40 -rpcpassword=mission40-lab-only-not-a-secret \
    -listen=0 -discover=0 -dnsseed=0 -fixedseeds=0
}

stop_node() {
  local datadir=$1
  rpc "$datadir" stop >/dev/null 2>&1 || true
}

wait_rpc() {
  local datadir=$1
  for _ in $(seq 1 60); do
    rpc "$datadir" getblockchaininfo >/dev/null 2>&1 && return 0
    sleep 0.5
  done
  return 1
}

log "Stopping any prior lab nodes..."
stop_node "$DATAA"; stop_node "$DATAB"; sleep 1

# Fresh chainstate for reproducibility of cold start (optional wipe)
if [[ "${MISSION40_WIPE:-1}" == "1" ]]; then
  rm -rf "$DATAA/regtest" "$DATAB/regtest"
fi

log "Starting Node A (will be HOT) and Node B (COLD) — same binary"
start_node "$DATAA" "$RPC_A" "$P2P_A"
start_node "$DATAB" "$RPC_B" "$P2P_B"
wait_rpc "$DATAA" || die "Node A RPC timeout"
wait_rpc "$DATAB" || die "Node B RPC timeout"

HA="$(rpc "$DATAA" getblockchaininfo | python3 -c 'import json,sys; print(json.load(sys.stdin)["bestblockhash"])')"
HB="$(rpc "$DATAB" getblockchaininfo | python3 -c 'import json,sys; print(json.load(sys.stdin)["bestblockhash"])')"
[[ "$HA" == "$HB" ]] || die "genesis/tip mismatch A=$HA B=$HB"
log "Shared tip/genesis hash: $HA"

OUT_DIR="$ROOT/mission40/results"
mkdir -p "$OUT_DIR"
RESULTS="$OUT_DIR/run_$(date -u +%Y%m%dT%H%M%SZ).json"

cat > "$OUT_DIR/BLOCK_INJECTION_STATUS.txt" << 'EOF'
STATUS: PENDING_IMPLEMENTATION_IN_MISSION_40

Mission 39 prepared fixtures + node infra + D verifier.
Injection of Mission 28 primer/alias as *valid regtest confidential
transactions* into ProcessNewBlock is NOT completed here because:

1) P0/P1/C0/C1/S0/S1 are output-field fragments from Liquid mainnet txs;
2) Full txs 71c9 / f24a are not valid on regtest (chain params, assets, UTXO);
3) Building synthetic confidential txs that embed *exactly* those proof bytes
   requires a post-fix wallet/rawtx pipeline + valid prevouts (needs binary).

Mission 40 must implement ONE of:
  A) reconstruct minimal valid confidential txs carrying exact fixture bytes; OR
  B) document why exact-byte injection is impossible and use a new pair with
     the SAME algebraic alias property generated on regtest (weaker for f24a
     comparison, still valid for D on mechanism).

Until a block hex is written to mission40/results/alias_block.hex and submitted
via submitblock / getblocktemplate path, D cannot be evaluated.
EOF

# Placeholder hooks for Mission 40 implementers
PRIMER_HEX="${MISSION40_PRIMER_TX:-}"
ALIAS_BLOCK="${MISSION40_ALIAS_BLOCK:-$OUT_DIR/alias_block.hex}"

if [[ -z "$PRIMER_HEX" || ! -f "$ALIAS_BLOCK" ]]; then
  log "No primer tx / alias block artifacts yet — writing skeleton results and exiting 2"
  python3 - "$RESULTS" "$BIN" "$COMMIT" "$HA" << 'PY'
import json, sys, pathlib
path, binary, commit, tip = sys.argv[1:5]
json.dump({
  "status": "INFRA_READY_WAITING_FOR_TX_BLOCK_ARTIFACTS",
  "binary": binary,
  "commit": commit,
  "shared_tip": tip,
  "d_evaluated": False,
  "message": "Nodes started identically; Mission 40 must supply primer tx + alias block hex",
}, open(path, "w"), indent=2)
print("wrote", path)
PY
  stop_node "$DATAA"; stop_node "$DATAB"
  exit 2
fi

# --- If artifacts exist, run cold/hot protocol ---
log "Priming Node A via sendrawtransaction (normal validation path)"
rpc "$DATAA" sendrawtransaction "$(cat "$PRIMER_HEX")" | tee "$OUT_DIR/primer_txid_A.txt"

log "Submitting same alias block to A (hot) and B (cold)"
BLOCK_HEX="$(tr -d '\n' < "$ALIAS_BLOCK")"
RA="$(rpc "$DATAA" submitblock "$BLOCK_HEX" || true)"
RB="$(rpc "$DATAB" submitblock "$BLOCK_HEX" || true)"
echo "submitblock_A=$RA" | tee "$OUT_DIR/submit_A.txt"
echo "submitblock_B=$RB" | tee "$OUT_DIR/submit_B.txt"

# Capture tips
TA="$(rpc "$DATAA" getbestblockhash || true)"
TB="$(rpc "$DATAB" getbestblockhash || true)"
BH="$(sha256sum "$ALIAS_BLOCK" | awk '{print $1}')"

# Extract HIT/MISS from debug logs if instrumented
rg 'MISSION40 CACHE' "$DATAA/regtest/debug.log" > "$OUT_DIR/cache_A.log" || true
rg 'MISSION40 CACHE' "$DATAB/regtest/debug.log" > "$OUT_DIR/cache_B.log" || true

python3 - "$RESULTS" << PY
import json
json.dump({
  "status": "SUBMITTED",
  "submitblock_A": """$RA""",
  "submitblock_B": """$RB""",
  "tip_A": """$TA""",
  "tip_B": """$TB""",
  "alias_block_file_sha256": """$BH""",
  "d_evaluated": False,
  "note": "Run scripts/mission40_verify_D.sh on this results dir",
}, open("$RESULTS", "w"), indent=2)
PY

"$ROOT/scripts/mission40_verify_D.sh" "$OUT_DIR" || true

stop_node "$DATAA"; stop_node "$DATAB"
log "Done. Review $OUT_DIR — human must decide D."
