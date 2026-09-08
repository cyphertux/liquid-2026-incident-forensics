#!/usr/bin/env bash
# Apply framed (length-prefixed) ComputeEntryRangeProof for Mission 40 control build.
# LAB ONLY — not for consensus deployment.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ELEMENTS_DIR="${ELEMENTS_DIR:-$ROOT/elements}"
SIG="$ELEMENTS_DIR/src/script/sigcache.cpp"
[[ -f "$SIG" ]] || { echo "missing $SIG"; exit 1; }
if grep -q 'MISSION40_FRAMED' "$SIG"; then
  echo "Framed patch already present"
  exit 0
fi

python3 - "$SIG" << 'PY'
import pathlib, sys, re
path = pathlib.Path(sys.argv[1])
text = path.read_text()
# Replace body of ComputeEntryRangeProof Write chain
pat = re.compile(
    r'(void SignatureCache::ComputeEntryRangeProof\([^\)]*\)[^{]*\{\s*'
    r'CSHA256 hasher = m_salted_hasher_range_proof;\s*)'
    r'hasher\.Write\(proof\.data\(\), proof\.size\(\)\)'
    r'\.Write\(commitment\.data\(\), commitment\.size\(\)\)'
    r'\.Write\(asset_commitment\.data\(\), asset_commitment\.size\(\)\)'
    r'\.Write\(scriptPubKey\.data\(\), scriptPubKey\.size\(\)\)'
    r'\.Finalize\(entry\.begin\(\)\);',
    re.M,
)
# Also match inline class method style (23.3.x)
pat2 = re.compile(
    r'(void ComputeEntryRangeProof\([^\)]*\)\s*\{\s*'
    r'CSHA256 hasher = m_salted_hasher_range_proof;\s*)'
    r'hasher\.Write\(proof\.data\(\), proof\.size\(\)\)'
    r'\.Write\(commitment\.data\(\), commitment\.size\(\)\)'
    r'\.Write\(asset_commitment\.data\(\), asset_commitment\.size\(\)\)'
    r'\.Write\(scriptPubKey\.data\(\), scriptPubKey\.size\(\)\)'
    r'\.Finalize\(entry\.begin\(\)\);',
    re.M,
)
framed = r'''\1// MISSION40_FRAMED length-prefix each field (uint64 LE)
    auto wlen = [&](const unsigned char* p, size_t n) {
        unsigned char L[8];
        for (int i = 0; i < 8; ++i) L[i] = (unsigned char)((n >> (8 * i)) & 0xff);
        hasher.Write(L, 8);
        if (n) hasher.Write(p, n);
    };
    wlen(proof.data(), proof.size());
    wlen(commitment.data(), commitment.size());
    wlen(asset_commitment.data(), asset_commitment.size());
    wlen(scriptPubKey.data(), scriptPubKey.size());
    hasher.Finalize(entry.begin());'''
new, n = pat.subn(framed, text, count=1)
if n == 0:
    new, n = pat2.subn(framed, text, count=1)
if n != 1:
    sys.exit(f"failed to patch ComputeEntryRangeProof (matches={n})")
path.write_text(new)
print("Framed patch applied", path)
PY
echo "OK: framed control patch applied — rebuild required"
