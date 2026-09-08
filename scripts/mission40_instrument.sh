#!/usr/bin/env bash
# Apply Mission 40 instrumentation to Elements working tree (lab logging only).
# Idempotent-ish: refuses if already instrumented.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ELEMENTS_DIR="${ELEMENTS_DIR:-$ROOT/elements}"
SIG="$ELEMENTS_DIR/src/script/sigcache.cpp"
[[ -f "$SIG" ]] || { echo "missing $SIG"; exit 1; }
if grep -q 'MISSION40 CACHE' "$SIG"; then
  echo "Instrumentation already present in $SIG"
  exit 0
fi

python3 - "$SIG" << 'PY'
import pathlib, sys
path = pathlib.Path(sys.argv[1])
text = path.read_text()
needle = "if (rangeProofCache.Get(entry, !store)) {\n        return true;\n    }"
if needle not in text:
    # try alternate brace style
    sys.exit("Could not locate Get(entry) block for instrumentation")
insert_before_get = '''    // MISSION40 instrumentation (lab)
    LogPrintf("MISSION40 CACHE GET key=%s proof_len=%zu script_len=%zu store=%d\\n",
              entry.ToString().substr(0, 16), vchRangeProof.size(), scriptPubKey.size(), (int)store);

'''
hit = '''    if (rangeProofCache.Get(entry, !store)) {
        LogPrintf("MISSION40 CACHE HIT key=%s crypto=SKIPPED erase=%d\\n",
                  entry.ToString().substr(0, 16), (int)(!store));
        return true;
    }

    LogPrintf("MISSION40 CACHE MISS key=%s -> native crypto\\n", entry.ToString().substr(0, 16));
'''
text2 = text.replace(
    "if (rangeProofCache.Get(entry, !store)) {\n        return true;\n    }",
    hit,
    1,
)
# SET log
text2 = text2.replace(
    "    if (store) {\n        rangeProofCache.Set(entry);\n    }",
    "    if (store) {\n        rangeProofCache.Set(entry);\n"
    "        LogPrintf(\"MISSION40 CACHE SET key=%s crypto=TRUE\\n\", entry.ToString().substr(0, 16));\n"
    "    }",
    1,
)
# Init log
text2 = text2.replace(
    'LogPrintf("Using %zu MiB out of %zu Mib requested for rangeproof cache, able to store %zu elements\\n",\n'
    '            approx_size_bytes >> 20, max_size_bytes >> 20, num_elems);',
    'LogPrintf("Using %zu MiB out of %zu Mib requested for rangeproof cache, able to store %zu elements\\n",\n'
    '            approx_size_bytes >> 20, max_size_bytes >> 20, num_elems);\n'
    '    LogPrintf("MISSION40 CACHE CREATE rangeProofCache elems=%zu\\n", num_elems);',
    1,
)
# crypto false
old_crypto = (
    "    if (!secp256k1_rangeproof_verify(secp256k1_ctx_verify_amounts, &min_value, &max_value, &commit, "
    "vchRangeProof.data(), vchRangeProof.size(), scriptPubKey.size() ? &scriptPubKey.front() : nullptr, "
    "scriptPubKey.size(), &tag)) {\n        return false;\n    }"
)
new_crypto = (
    "    if (!secp256k1_rangeproof_verify(secp256k1_ctx_verify_amounts, &min_value, &max_value, &commit, "
    "vchRangeProof.data(), vchRangeProof.size(), scriptPubKey.size() ? &scriptPubKey.front() : nullptr, "
    "scriptPubKey.size(), &tag)) {\n"
    "        LogPrintf(\"MISSION40 CRYPTO FALSE key=%s\\n\", entry.ToString().substr(0, 16));\n"
    "        return false;\n    }"
)
if old_crypto in text2:
    text2 = text2.replace(old_crypto, new_crypto, 1)
path.write_text(text2)
print("Instrumented", path)
PY

echo "OK: instrumentation applied to $SIG"
echo "Rebuild elementsd after this (scripts/mission40_build.sh or make -C elements)."
