#!/usr/bin/env bash
# Audit automatic D conditions. Never prints "D PROVEN".
# Usage: scripts/mission40_verify_D.sh [results_dir]
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck source=/dev/null
source "$ROOT/mission40/COMMIT.txt"
RES="${1:-$ROOT/mission40/results}"
REPORT="$RES/D_checklist.txt"

pass=0; fail=0; na=0
check() {
  local id=$1; shift
  local ok=$1; shift
  local msg="$*"
  if [[ "$ok" == "1" ]]; then
    echo "[x] $id — $msg" | tee -a "$REPORT"
    pass=$((pass+1))
  elif [[ "$ok" == "N" ]]; then
    echo "[ ] $id — $msg (NOT AVAILABLE)" | tee -a "$REPORT"
    na=$((na+1))
  else
    echo "[ ] $id — $msg" | tee -a "$REPORT"
    fail=$((fail+1))
  fi
}

mkdir -p "$RES"
: > "$REPORT"
echo "Mission 40 D checklist — $(date -u +%Y-%m-%dT%H:%M:%SZ)" | tee -a "$REPORT"
echo "results dir: $RES" | tee -a "$REPORT"
echo | tee -a "$REPORT"

PROV="$ROOT/mission40/build_provenance.json"
if [[ -f "$PROV" ]]; then
  COMMIT="$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["commit"])' "$PROV")"
  VER="$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1])).get("version_banner",""))' "$PROV")"
  [[ "$COMMIT" == "$RECOMMENDED_COMMIT" ]]; c1=$?
  check "exact_commit" "$([[ $c1 -eq 0 ]] && echo 1 || echo 0)" "commit=$COMMIT expected=$RECOMMENDED_COMMIT"
  echo "$VER" | grep -q "$REJECT_IF_VERSION_CONTAINS"; pre=$?
  check "post_fix_binary" "$([[ $pre -ne 0 ]] && echo 1 || echo 0)" "version banner must not be 23.3.3 pre-fix"
else
  check "exact_commit" N "build_provenance.json missing"
  check "post_fix_binary" N "build_provenance.json missing"
fi

# Fixture algebraic properties
if "$ROOT/scripts/mission40_verify_fixtures.py" >/dev/null; then
  check "fixtures_preimage_alias" 1 "preimage_A==preimage_B and framed keys differ"
else
  check "fixtures_preimage_alias" 0 "fixture verification failed"
fi

# Runtime evidence files
[[ -f "$RES/cache_A.log" ]]; hasA=$?
[[ -f "$RES/cache_B.log" ]]; hasB=$?
check "cache_logs_present" "$([[ $hasA -eq 0 && $hasB -eq 0 ]] && echo 1 || echo 0)" "cache_A.log and cache_B.log"

HIT_A=0; MISS_B=0; SKIP_A=0; CRYPTO_B=0
if [[ -f "$RES/cache_A.log" ]]; then
  grep -q 'CACHE HIT' "$RES/cache_A.log" && HIT_A=1
  grep -q 'crypto=SKIPPED' "$RES/cache_A.log" && SKIP_A=1
fi
if [[ -f "$RES/cache_B.log" ]]; then
  grep -q 'CACHE MISS' "$RES/cache_B.log" && MISS_B=1
  grep -q 'CRYPTO FALSE\|native crypto' "$RES/cache_B.log" && CRYPTO_B=1
fi
check "cache_HIT_on_A" "$HIT_A" "Node A must log CACHE HIT on alias"
check "cache_MISS_on_B" "$MISS_B" "Node B must log CACHE MISS on alias"
check "crypto_skipped_on_A" "$SKIP_A" "Node A must skip native crypto"
check "crypto_executed_on_B" "$CRYPTO_B" "Node B must execute native crypto (MISS path)"

# Tips / submitblock
TA=""; TB=""
if [[ -f "$RES/run_"*.json ]]; then
  LATEST="$(ls -1t "$RES"/run_*.json | head -1)"
  TA="$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1])).get("tip_A",""))' "$LATEST")"
  TB="$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1])).get("tip_B",""))' "$LATEST")"
fi
check "same_block_attempted" "$([[ -f "$RES/alias_block.hex" ]] && echo 1 || echo 0)" "alias_block.hex present"
if [[ -n "$TA" && -n "$TB" && "$TA" != "$TB" ]]; then
  check "final_tips_differ" 1 "tip_A=$TA tip_B=$TB"
  check "A_ACCEPT_B_REJECT_heuristic" 1 "tips differ after same submitblock (heuristic — human review)"
else
  check "final_tips_differ" 0 "tips missing or equal (A=$TA B=$TB)"
  check "A_ACCEPT_B_REJECT_heuristic" 0 "cannot claim divergent ConnectBlock verdict yet"
fi

check "same_config" 1 "nodeA/B elements.conf consensus fields identical by construction (ports differ)"
check "hot_cold_only_intended" N "human must confirm no other conf/UTXO drift"

echo | tee -a "$REPORT"
echo "PASS=$pass FAIL=$fail NA=$na" | tee -a "$REPORT"

if [[ $fail -eq 0 && $na -eq 0 && $HIT_A -eq 1 && $MISS_B -eq 1 ]]; then
  echo | tee -a "$REPORT"
  echo "POTENTIAL D — REVIEW REQUIRED" | tee -a "$REPORT"
  echo "(Do NOT auto-declare D PROVEN. Confirm ProcessNewBlock/ConnectBlock path and no later equalizing checks.)" | tee -a "$REPORT"
  exit 0
fi

echo | tee -a "$REPORT"
echo "D NON DÉMONTRÉ — checklist incomplete or failed" | tee -a "$REPORT"
exit 1
