#!/usr/bin/env python3
"""Mission 39.1 — independent verification of EXACT Mission 28 artifacts.

Fails explicitly if any required property is not satisfied.
Does not declare D. Does not require post-fix elementsd.
"""
from __future__ import annotations

import hashlib
import json
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
M40 = ROOT / "mission40"
FIX = M40 / "fixtures"


class Fail(Exception):
    pass


def hx(path: Path) -> bytes:
    text = path.read_text().strip().replace("\n", "").replace(" ", "")
    return bytes.fromhex(text)


def check(cond: bool, fail_msg: str) -> None:
    if not cond:
        raise Fail(fail_msg)


def key_unframed(P: bytes, C: bytes, A: bytes, S: bytes, salt: bytes = b"") -> bytes:
    return hashlib.sha256(salt + P + C + A + S).digest()


def key_framed(P: bytes, C: bytes, A: bytes, S: bytes, salt: bytes = b"") -> bytes:
    h = hashlib.sha256()
    h.update(salt)
    for x in (P, C, A, S):
        h.update(struct.pack("<I", len(x)))
        h.update(x)
    return h.digest()


def main() -> int:
    results = []

    def step(name: str, fn):
        try:
            fn()
            results.append((name, True, None))
            print(f"[PASS] {name}")
        except Fail as e:
            results.append((name, False, str(e)))
            print(f"[FAIL] {name}: {e}")
        except Exception as e:  # noqa: BLE001
            results.append((name, False, repr(e)))
            print(f"[FAIL] {name}: {e!r}")

    P0, C0, S0 = hx(FIX / "P0.hex"), hx(FIX / "C0.hex"), hx(FIX / "S0.hex")
    P1, C1, S1 = hx(FIX / "P1.hex"), hx(FIX / "C1.hex"), hx(FIX / "S1.hex")
    G = hx(FIX / "G.hex") if (FIX / "G.hex").exists() else S0[35:68]

    primer = hx(M40 / "primer.hex")
    alias = hx(M40 / "alias.hex")
    primer_block = hx(M40 / "primer_block.hex")
    alias_block = hx(M40 / "alias_block.hex")

    def t_primer_decodable():
        check(len(primer) > 100, "primer too short")
        check(primer[:4] == b"\x02\x00\x00\x00", "primer version not 2")

    def t_alias_decodable():
        check(len(alias) > 100, "alias too short")
        check(alias[:4] == b"\x02\x00\x00\x00", "alias version not 2")

    def t_deterministic():
        check(primer == hx(FIX / "tx_71c9.hex"), "primer != fixtures/tx_71c9.hex")
        check(alias == hx(FIX / "tx_f24a.hex"), "alias != fixtures/tx_f24a.hex")

    def t_P0():
        check(primer.find(P0) >= 0, "P0 not in primer.hex")
        check(len(P0) == 4166, f"P0 len {len(P0)}")

    def t_C0():
        check(primer.find(C0) >= 0, "C0 not in primer.hex")
        check(len(C0) == 33, "C0 size")

    def t_G():
        check(G == S0[35:68], "G != S0[35:68]")
        check(primer.find(G) >= 0, "G not in primer")
        check(alias.find(G) >= 0, "G not in alias")
        check(G[0] in (0x0A, 0x0B), "G not serialized generator prefix")

    def t_S0():
        check(S0 == b"\x6a\x43" + C1 + G + b"\x6a", "S0 construction mismatch")
        check(primer.find(S0) >= 0, "S0 not in primer")

    def t_P1():
        check(P1 == P0 + C0 + G + b"\x6a\x43", "P1 construction mismatch")
        check(alias.find(P1) >= 0, "P1 not in alias")

    def t_C1():
        check(alias.find(C1) >= 0, "C1 not in alias")
        check(len(C1) == 33, "C1 size")

    def t_S1():
        check(S1 == b"\x6a", "S1 must be OP_RETURN")
        pat = C1 + b"\x00\x01\x6a"
        check(alias.find(pat) >= 0, "S1 not located as script after C1||nonce0||len")

    def t_preimage():
        a = P0 + C0 + G + S0
        b = P1 + C1 + G + S1
        check(a == b, "preimage A != preimage B (byte-exact)")
        check(len(a) == 4301, f"preimage len {len(a)}")
        check(
            hashlib.sha256(a).hexdigest()
            == "82b0b8ccf8c743171f2bc8d80bb9982a81cfca583e8dfbe65563cf095e99c01a",
            "preimage sha256 mismatch",
        )

    def t_keys_equal():
        ka, kb = key_unframed(P0, C0, G, S0), key_unframed(P1, C1, G, S1)
        check(ka == kb, "unframed key A != key B")

    def t_framed_differ():
        fa, fb = key_framed(P0, C0, G, S0), key_framed(P1, C1, G, S1)
        check(fa != fb, "framed keys unexpectedly equal")

    def t_primer_crypto():
        bin_path = ROOT / "build" / "m391_crypto"
        check(bin_path.exists(), "build/m391_crypto missing — compile first")
        r = subprocess.run(
            [str(bin_path), str(FIX / "P0.hex"), str(FIX / "C0.hex"), str(FIX / "G.hex"), str(FIX / "S0.hex"), "primer"],
            capture_output=True,
            text=True,
        )
        check(r.returncode == 0, f"primer crypto not TRUE: {r.stdout} {r.stderr}")

    def t_alias_crypto_cold():
        bin_path = ROOT / "build" / "m391_crypto"
        r = subprocess.run(
            [str(bin_path), str(FIX / "P1.hex"), str(FIX / "C1.hex"), str(FIX / "G.hex"), str(FIX / "S1.hex"), "alias"],
            capture_output=True,
            text=True,
        )
        check(r.returncode != 0, f"alias crypto unexpectedly TRUE: {r.stdout}")

    def t_manifests():
        for name in ("primer_manifest.json", "alias_manifest.json", "primer.json", "alias.json"):
            p = M40 / name
            check(p.exists(), f"missing {name}")
            json.loads(p.read_text())
        man = json.loads((M40 / "primer_manifest.json").read_text())
        check(man.get("native_crypto_expected") is True, "primer_manifest crypto flag")
        aman = json.loads((M40 / "alias_manifest.json").read_text())
        check(aman.get("native_crypto_expected") is False, "alias_manifest crypto flag")
        check(aman.get("preimage", {}).get("byte_identical_to_primer") is True, "alias preimage flag")

    def t_blocks_contain_txs():
        check(primer in primer_block, "primer.tx not inside primer_block.hex")
        check(alias in alias_block, "alias.tx not inside alias_block.hex")
        pb = json.loads((M40 / "primer_block.json").read_text())
        ab = json.loads((M40 / "alias_block.json").read_text())
        check(pb.get("NOT_REGTEST") is True, "primer_block must be marked NOT_REGTEST")
        check(ab.get("NOT_REGTEST") is True, "alias_block must be marked NOT_REGTEST")

    def t_mapping():
        m = json.loads((ROOT / "final/data/39_1_fixture_mapping.json").read_text())
        names = {f["Mission28_field"] for f in m["fields"]}
        for req in ("P0", "C0", "G", "S0", "P1", "C1", "S1"):
            check(req in names, f"mapping missing {req}")

    def t_f24a_untouched():
        ftx = hx(M40 / "f24a" / "tx.hex")
        check(ftx == alias, "f24a/tx.hex mutated vs alias.hex")

    for name, fn in [
        ("primer.hex décodable", t_primer_decodable),
        ("alias.hex décodable", t_alias_decodable),
        ("transactions déterministes", t_deterministic),
        ("P0 exact", t_P0),
        ("C0 exact", t_C0),
        ("G exact", t_G),
        ("S0 exact", t_S0),
        ("P1 exact", t_P1),
        ("C1 exact", t_C1),
        ("S1 exact", t_S1),
        ("preimage A == preimage B", t_preimage),
        ("key A == key B", t_keys_equal),
        ("framed key A != framed key B", t_framed_differ),
        ("primer crypto valide", t_primer_crypto),
        ("alias crypto invalide à froid", t_alias_crypto_cold),
        ("artefacts cohérents avec manifests", t_manifests),
        ("blocks contain txs + NOT_REGTEST marked", t_blocks_contain_txs),
        ("fixture mapping present", t_mapping),
        ("f24a unmodified", t_f24a_untouched),
    ]:
        step(name, fn)

    failed = [n for n, ok, _ in results if not ok]
    print("\n======== SUMMARY ========")
    for n, ok, err in results:
        print(f"{'PASS' if ok else 'FAIL':4}  {n}" + (f"  ({err})" if err else ""))
    if failed:
        print(f"\nVERDICT: VERIFY_FAILED ({len(failed)} checks)")
        return 1
    print("\nVERDICT: VERIFY_OK (EXACT artifacts)")
    print("NOTE: Mission 39.1 overall readiness remains NOT_READY for regtest ConnectBlock")
    print("D: not declared")
    return 0


if __name__ == "__main__":
    sys.exit(main())
