#!/usr/bin/env python3
"""Verify Mission 28/36/37 fixture algebraic properties. Fail closed."""
from __future__ import annotations
import hashlib, json, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FIX = ROOT / "mission40" / "fixtures"

def hx(name: str) -> bytes:
    t = (FIX / name).read_text().strip().replace("\n", "")
    return bytes.fromhex(t)

def sha256(*parts: bytes) -> bytes:
    h = hashlib.sha256()
    for p in parts:
        h.update(p)
    return h.digest()

def key_unframed(P, C, A, S, salt=b"") -> bytes:
    return sha256(salt, P, C, A, S)

def key_framed(P, C, A, S, salt=b"") -> bytes:
    def enc(b: bytes) -> bytes:
        return len(b).to_bytes(8, "little") + b
    return sha256(salt, enc(P), enc(C), enc(A), enc(S))

def main() -> int:
    P0, C0, S0 = hx("P0.hex"), hx("C0.hex"), hx("S0.hex")
    P1, C1, S1 = hx("P1.hex"), hx("C1.hex"), hx("S1.hex")
    A_wire = hx("A_wire.hex")

    assert len(C0) == 33 and len(C1) == 33, "commitments must be 33 bytes"
    assert S1 == b"\x6a", f"S1 expected OP_RETURN byte, got {S1.hex()}"
    assert P1.startswith(P0), "P1 must start with P0"
    assert P1[len(P0):len(P0)+33] == C0, "P1 mid C0 mismatch"
    G = P1[len(P0)+33:len(P0)+66]
    assert len(G) == 33
    assert P1[len(P0)+66:] == b"\x6a\x43", "P1 suffix != 6a43"
    assert S0 == b"\x6a\x43" + C1 + G + b"\x6a", "S0 construction mismatch"
    assert P1 == P0 + C0 + G + b"\x6a\x43"

    pre_A = P0 + C0 + G + S0
    pre_B = P1 + C1 + G + S1
    assert pre_A == pre_B, "preimage_A != preimage_B"
    assert (P0, C0, G, S0) != (P1, C1, G, S1)

    kA = key_unframed(P0, C0, G, S0)
    kB = key_unframed(P1, C1, G, S1)
    assert kA == kB, "unframed keys differ"
    fA = key_framed(P0, C0, G, S0)
    fB = key_framed(P1, C1, G, S1)
    assert fA != fB, "framed keys unexpectedly equal"

    # G should match serialized generator of explicit asset id (A_wire[1:])
    # (optional soft check — full secp verify is in C harness)
    out = {
        "ok": True,
        "P0_len": len(P0),
        "P1_len": len(P1),
        "S0_len": len(S0),
        "S1_len": len(S1),
        "G_hex": G.hex(),
        "preimage_len": len(pre_A),
        "preimage_sha256": hashlib.sha256(pre_A).hexdigest(),
        "unframed_key_sha256": kA.hex(),
        "framed_key_A": fA.hex(),
        "framed_key_B": fB.hex(),
        "A_wire_hex": A_wire.hex(),
        "constructions": {
            "P1": "P0||C0||G||6a||43",
            "S0": "6a||43||C1||G||6a",
        },
    }
    dest = ROOT / "mission40" / "expected" / "fixture_check.json"
    dest.parent.mkdir(parents=True, exist_ok=True)
    dest.write_text(json.dumps(out, indent=2))
    print(json.dumps(out, indent=2))
    print("FIXTURE CHECK OK", file=sys.stderr)
    return 0

if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as e:
        print(f"FIXTURE CHECK FAILED: {e}", file=sys.stderr)
        raise SystemExit(1)
