#!/usr/bin/env bash
# Mission 40 — build post-fix elementsd (immutable commit).
# Does NOT declare D. Fails closed if pre-fix binary or wrong formula.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
# shellcheck source=/dev/null
source "$ROOT/mission40/COMMIT.txt"

ELEMENTS_DIR="${ELEMENTS_DIR:-$ROOT/elements}"
BUILD_PREFIX="${BUILD_PREFIX:-$ROOT/mission40/build}"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 2)}"

log() { printf '[mission40_build] %s\n' "$*"; }
die() { printf '[mission40_build] ERROR: %s\n' "$*" >&2; exit 1; }

log "ROOT=$ROOT"
log "RECOMMENDED_COMMIT=$RECOMMENDED_COMMIT"

# --- dependency checks (non-fatal report + fatal if critical missing) ---
missing=()
for c in git g++ make; do
  command -v "$c" >/dev/null || missing+=("$c")
done
for c in cmake autoconf libtool pkg-config; do
  command -v "$c" >/dev/null || missing+=("$c(optional-until-configure)")
done
if [[ ${#missing[@]} -gt 0 ]]; then
  log "Missing tools: ${missing[*]}"
fi
command -v git >/dev/null || die "git required"
command -v g++ >/dev/null || die "g++ required"
[[ -d "$ELEMENTS_DIR/.git" ]] || die "Elements repo not found at $ELEMENTS_DIR"

cd "$ELEMENTS_DIR"
git rev-parse --is-inside-work-tree >/dev/null
git cat-file -e "${RECOMMENDED_COMMIT}^{commit}" 2>/dev/null \
  || die "commit $RECOMMENDED_COMMIT not in local repo — fetch origin first"

log "Checking out $RECOMMENDED_COMMIT (detached HEAD)"
git checkout --detach "$RECOMMENDED_COMMIT"

# --- verify unframed formula ---
SIGCACHE="src/script/sigcache.cpp"
[[ -f "$SIGCACHE" ]] || die "missing $SIGCACHE"
if ! grep -F "$EXPECTED_WRITE_SNIPPET" "$SIGCACHE" >/dev/null; then
  # allow whitespace differences: check four Write fields in order
  if ! awk '
    /ComputeEntryRangeProof/ {infn=1}
    infn && /hasher.Write\(proof/ {
      line=$0
      if (line ~ /proof/ && line ~ /commitment/ && line ~ /asset_commitment/ && line ~ /scriptPubKey/ \
          && line !~ /to_bytes/ && line !~ /GetSizeOfCompactSize/ && line !~ /WriteBE/) {
        found=1
      }
    }
    END {exit found?0:1}
  ' "$SIGCACHE"; then
    die "sigcache.cpp does not match expected unframed 4-field Write formula"
  fi
fi
# reject length-prefix framing
if grep -n 'ComputeEntryRangeProof' -A6 "$SIGCACHE" | grep -E 'WriteBE64|GetSizeOfCompactSize|len\(P\)|to_bytes\(8'; then
  die "unexpected framing markers near ComputeEntryRangeProof"
fi
log "CACHE FORMULA VERIFIED: salt||proof||value||asset||script (unframed)"

# --- configure + build ---
mkdir -p "$BUILD_PREFIX"
if [[ ! -f ./configure && -f ./autogen.sh ]]; then
  command -v autoconf >/dev/null || die "autoconf required to run autogen.sh"
  ./autogen.sh
fi
if [[ -f ./configure ]]; then
  ./configure --prefix="$BUILD_PREFIX" --without-gui --disable-bench --disable-tests \
    || die "configure failed — install Elements build deps then retry"
  make -j"$JOBS" \
    || die "make failed"
  make install \
    || die "make install failed"
elif [[ -f ./CMakeLists.txt ]] && command -v cmake >/dev/null; then
  cmake -S . -B "$BUILD_PREFIX/cmake-build" -DCMAKE_INSTALL_PREFIX="$BUILD_PREFIX" \
    -DBUILD_GUI=OFF -DBUILD_BENCH=OFF \
    || die "cmake configure failed"
  cmake --build "$BUILD_PREFIX/cmake-build" -j"$JOBS" \
    || die "cmake build failed"
  cmake --install "$BUILD_PREFIX/cmake-build" \
    || die "cmake install failed"
else
  die "No configure/CMake build system ready. Install deps (see Elements docs) then re-run."
fi

BIN=""
for cand in "$BUILD_PREFIX/bin/elementsd" "$ELEMENTS_DIR/src/elementsd"; do
  [[ -x "$cand" ]] && BIN="$cand" && break
done
[[ -n "$BIN" ]] || die "elementsd binary not found after build"

VER_OUT="$("$BIN" -version 2>&1 | head -5 || true)"
printf '%s\n' "$VER_OUT"
echo "$VER_OUT" | grep -q "$REJECT_IF_VERSION_CONTAINS" \
  && die "Refusing pre-fix release marker ($REJECT_IF_VERSION_CONTAINS)"

SHA256_BIN="$(sha256sum "$BIN" | awk '{print $1}')"
COMMIT_NOW="$(git rev-parse HEAD)"
[[ "$COMMIT_NOW" == "$RECOMMENDED_COMMIT" ]] \
  || die "HEAD $COMMIT_NOW != recommended $RECOMMENDED_COMMIT"

# record provenance
cat > "$ROOT/mission40/build_provenance.json" << EOF
{
  "commit": "$COMMIT_NOW",
  "binary": "$BIN",
  "sha256": "$SHA256_BIN",
  "version_banner": $(python3 -c 'import json,sys; print(json.dumps(sys.argv[1]))' "$VER_OUT"),
  "cache_formula": "SHA256(salt||proof||value_commitment||asset_commitment||scriptPubKey) unframed",
  "built_at_utc": "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
}
EOF

cat << EOF

========================================
BUILD VERIFIED
COMMIT = $COMMIT_NOW
BINARY = $BIN
SHA256 = $SHA256_BIN
CACHE FORMULA = SHA256(salt || proof || value_commitment || asset_commitment || scriptPubKey) [UNFRAMED]
========================================

EOF
