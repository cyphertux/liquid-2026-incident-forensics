#!/usr/bin/env python3
"""Mission 30 — post-c26d719 unframed cache preimage scan vs f24a:1.

Exact keying (sigcache.cpp after c26d719c29):
  CSHA256 hasher = m_salted_hasher_range_proof;  // salt already absorbed
  hasher.Write(proof).Write(value_commitment).Write(asset_commitment).Write(scriptPubKey);

For explicit assets, confidential_validation.cpp replaces wire asset with
serialized generator before QueueCheck / cache.

This script compares the unsalted field preimage:
  preimage = proof || value_commitment || G_ser || scriptPubKey
Salt independence: identical preimages => identical keys for any salt (Mission 28).
"""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
HEX = ROOT / "final" / "hex"
DATA = ROOT / "final" / "data"
OUT_JSON = DATA / "mission30_primer_candidates.json"
OUT_TARGET_BIN = DATA / "f24a_target_cache_preimage.bin"


def read_hex_file(p: Path) -> bytes:
    raw = p.read_text().strip().replace("\n", "").replace(" ", "")
    return bytes.fromhex(raw)


def sha256(b: bytes) -> str:
    return hashlib.sha256(b).hexdigest()


def post_fix_preimage(proof: bytes, commit: bytes, gen: bytes, script: bytes) -> bytes:
    return proof + commit + gen + script


def row(
    *,
    txid: str,
    vout: int,
    height,
    timestamp,
    proof: bytes,
    commit: bytes,
    gen: bytes,
    script: bytes,
    target: bytes,
    family: str,
    notes: str,
    spent_unknown: bool = True,
) -> dict:
    pre = post_fix_preimage(proof, commit, gen, script)
    eq = pre == target
    return {
        "txid": txid,
        "vout": vout,
        "block_height": height,
        "timestamp": timestamp,
        "family": family,
        "proof_len": len(proof),
        "proof_sha256": sha256(proof),
        "value_commitment": commit.hex(),
        "generator": gen.hex(),
        "script": script.hex(),
        "script_len": len(script),
        "preimage_len": len(pre),
        "preimage_sha256": sha256(pre),
        "exact_alias_with_f24a_out1": eq,
        "position_vs_f24a": (
            "before"
            if height is not None and height < 4050336
            else ("same_block" if height == 4050336 else ("after" if height else "unknown"))
        ),
        "mined": height is not None,
        "mempool_presence_on_acceptor": "UNPROVEN",
        "spent": "UNKNOWN" if spent_unknown else None,
        "evidence_level": "A" if eq else "none",
        "notes": notes,
        "preimage_bin_path": None,
    }


def main() -> None:
    # --- Attack target from raw hex artifacts ---
    P1 = read_hex_file(HEX / "out1_rangeproof.hex")
    C1 = read_hex_file(HEX / "out1_commitment.hex")
    wire_asset = read_hex_file(HEX / "out1_asset.hex")
    S1 = read_hex_file(HEX / "out1_script.hex")

    # Serialized L-BTC generator (Mission 28 FACT; matches generator_generate+serialize)
    G = bytes.fromhex(
        "0a0a488de4899d0ae757f6cf8368663184d164106111ed9eaecf510e35282ddc6d"
    )

    assert len(P1) == 4234, len(P1)
    assert len(C1) == 33
    assert S1 == b"\x6a"
    assert wire_asset[0] == 0x01

    target = post_fix_preimage(P1, C1, G, S1)
    assert len(target) == 4301
    OUT_TARGET_BIN.write_bytes(target)

    candidates: list[dict] = []

    # --- P0 family: 71c9 / 2711 out0 (do not assume; measure) ---
    for prefix, txid, height, ts in [
        (
            "71c93d43",
            "71c93d4339fe8328981a2ec0dd23808d1ff104dc4c4cbcfc5c06fb2bb622f411",
            4050335,
            1788702730,
        ),
        (
            "27114710",
            "271147100a94f6337b6c3db39b30c92d5b97ed91597307b6f721f73a15187ec5",
            4050335,
            1788702730,
        ),
    ]:
        P0 = read_hex_file(HEX / f"{prefix}_out0_rangeproof.hex")
        C0 = read_hex_file(HEX / f"{prefix}_out0_commitment.hex")
        S0 = read_hex_file(HEX / f"{prefix}_out0_script.hex")
        r = row(
            txid=txid,
            vout=0,
            height=height,
            timestamp=ts,
            proof=P0,
            commit=C0,
            gen=G,
            script=S0,
            target=target,
            family="P0_71c9_2711",
            notes="standalone out0; OP_RETURN embeds C1||G",
        )
        if r["exact_alias_with_f24a_out1"]:
            bin_path = DATA / f"{txid[:8]}_vout0_cache_preimage.bin"
            bin_path.write_bytes(post_fix_preimage(P0, C0, G, S0))
            r["preimage_bin_path"] = str(bin_path.relative_to(ROOT))
            r["evidence_level"] = "A+C"  # crypto identity + on-chain before f24a
            r["segmentation"] = {
                "attack": "P1||C1||G||S1",
                "primer": "P0||C0||G||S0",
                "P1_equals_P0_C0_G_6a43": P1 == P0 + C0 + G + b"\x6a\x43",
                "S0_equals_6a43_C1_G_6a": S0 == b"\x6a\x43" + C1 + G + b"\x6a",
                "expanded_common": "P0||C0||G||6a||43||C1||G||6a",
            }
        candidates.append(r)

        # also out1 if present
        op1 = HEX / f"{prefix}_out1_rangeproof.hex"
        if op1.exists():
            P = read_hex_file(op1)
            C = read_hex_file(HEX / f"{prefix}_out1_commitment.hex")
            S = read_hex_file(HEX / f"{prefix}_out1_script.hex")
            candidates.append(
                row(
                    txid=txid,
                    vout=1,
                    height=height,
                    timestamp=ts,
                    proof=P,
                    commit=C,
                    gen=G,
                    script=S,
                    target=target,
                    family="P0_71c9_2711_sibling",
                    notes="sibling confidential out",
                )
            )

    # --- Q0 family (68 identical) from curated hex + JSON rows ---
    Q0 = read_hex_file(HEX / "Q0_rangeproof.hex")
    CQ = read_hex_file(HEX / "Q0_commitment.hex")
    SQ = read_hex_file(HEX / "Q0_script.hex")
    assert len(Q0) == 4166
    assert SQ == bytes.fromhex("6a0100")

    q0_keys = json.loads((DATA / "68_rangeproof_cache_keys.json").read_text())
    q0_rows = q0_keys["family_Q0"]["rows"]
    assert len(q0_rows) == 68

    # All 68 share same (P,C,G,S) for post-fix preimage — verify + emit one representative bin
    q0_pre = post_fix_preimage(Q0, CQ, G, SQ)
    q0_alias = q0_pre == target
    q0_bin = DATA / "Q0_family_cache_preimage.bin"
    q0_bin.write_bytes(q0_pre)

    # Boundary analysis vs attack preimage
    boundary = {
        "Q0_equals_P1_prefix_4166": Q0 == P1[:4166],
        "CQ_equals_C0": CQ.hex() == candidates[0]["value_commitment"] if candidates else None,
        "CQ_equals_C1": CQ == C1,
        "SQ_equals_S1": SQ == S1,
        "SQ_is_prefix_of_S0_style": False,
        "preimage_len_Q0": len(q0_pre),
        "preimage_len_attack": len(target),
        "lcp_bytes_with_attack_preimage": sum(
            1 for a, b in zip(q0_pre, target) if a == b
        )
        if False
        else None,
    }
    # compute LCP
    lcp = 0
    for a, b in zip(q0_pre, target):
        if a != b:
            break
        lcp += 1
    boundary["lcp_bytes_with_attack_preimage"] = lcp
    boundary["first_diff_offset"] = lcp if lcp < min(len(q0_pre), len(target)) else None

    for i, jr in enumerate(q0_rows):
        # confirm JSON fields match curated Q0 hex
        assert jr["value_commitment"] == CQ.hex()
        assert jr["script"] == SQ.hex()
        assert jr["proof_sha256"] == sha256(Q0)
        r = row(
            txid=jr["txid"],
            vout=jr["out_index"],
            height=jr["height"],
            timestamp=jr.get("timestamp"),
            proof=Q0,
            commit=CQ,
            gen=G,
            script=SQ,
            target=target,
            family="Q0_bitquery_68",
            notes="identical (Q0,CQ,G,S=6a0100) across all 68",
        )
        r["exact_alias_with_f24a_out1"] = q0_alias
        r["preimage_bin_path"] = str(q0_bin.relative_to(ROOT)) if i == 0 else None
        r["boundary_vs_attack"] = boundary if i == 0 else None
        candidates.append(r)

    # --- Alternate search: any other RP-bearing outs in local hex + scan for P0 prefix / P1 ---
    # Search scan_cache tx hex for P0 / attack-preimage fragments if cache present
    scan_dir = DATA / "scan_cache"
    alternate_hits = []
    p0_prefix = read_hex_file(HEX / "71c93d43_out0_rangeproof.hex")[:32].hex()
    p1_full_hex = P1.hex()
    target_hex = target.hex()
    q0_prefix = Q0[:32].hex()

    if scan_dir.is_dir():
        # Grep-like scan of .hex files for rare patterns
        for p in scan_dir.glob("*.hex"):
            try:
                text = p.read_text()
            except Exception:
                continue
            t = text.replace("\n", "").lower()
            hit = {
                "file": p.name,
                "contains_full_P1": p1_full_hex in t,
                "contains_P0_prefix32": p0_prefix in t,
                "contains_Q0_prefix32": q0_prefix in t,
                "contains_full_attack_preimage": target_hex in t,
            }
            if hit["contains_full_P1"] or hit["contains_full_attack_preimage"]:
                alternate_hits.append(hit)
            elif hit["contains_P0_prefix32"] and "71c93d43" not in p.name and "27114710" not in p.name and "f24a" not in p.name:
                # count only interesting; store limited
                if len(alternate_hits) < 200:
                    alternate_hits.append(hit)

    # Count P0-prefix appearances in scan_cache
    p0_hex_full = read_hex_file(HEX / "71c93d43_out0_rangeproof.hex").hex()
    p0_tx_files = []
    if scan_dir.is_dir():
        for p in scan_dir.glob("t_*.hex"):
            try:
                t = p.read_text().replace("\n", "").lower()
            except Exception:
                continue
            if p0_hex_full in t:
                p0_tx_files.append(p.name)

    aliases = [c for c in candidates if c["exact_alias_with_f24a_out1"]]
    non_aliases_sample = [c for c in candidates if not c["exact_alias_with_f24a_out1"]]

    # uniqueness of preimage hashes among candidates
    by_pre = {}
    for c in candidates:
        by_pre.setdefault(c["preimage_sha256"], []).append(f"{c['txid']}:{c['vout']}")

    out = {
        "mission": 30,
        "keying": {
            "commit": "c26d719c29a40da280a825b25657e9c3d8bc7d99",
            "function": "SignatureCache::ComputeEntryRangeProof",
            "write_order": ["proof", "value_commitment", "asset_commitment", "scriptPubKey"],
            "framing": "NONE",
            "hash": "single SHA256 over salted hasher then fields (CSHA256 Finalize once)",
            "salt": "process-local m_salted_hasher_range_proof; identical preimages => identical keys for any salt",
            "asset_field_for_explicit": "serialized_generator_not_wire_01_asset",
            "G_ser_hex": G.hex(),
            "wire_asset_f24a": wire_asset.hex(),
        },
        "f24a_target": {
            "txid": "f24a4b179b5cc7e88b25a763911f7cbdf2bf45d1d1b5ab611e94461cef0a183f",
            "vout": 1,
            "height": 4050336,
            "proof_len": len(P1),
            "proof_sha256": sha256(P1),
            "value_commitment": C1.hex(),
            "generator": G.hex(),
            "script": S1.hex(),
            "preimage_len": len(target),
            "preimage_sha256": sha256(target),
            "preimage_bin": str(OUT_TARGET_BIN.relative_to(ROOT)),
        },
        "summary": {
            "candidates_evaluated": len(candidates),
            "exact_aliases": len(aliases),
            "alias_txids": sorted({c["txid"] for c in aliases}),
            "Q0_family_aliases_f24a": q0_alias,
            "Q0_boundary": boundary,
            "unique_preimage_hashes": len(by_pre),
            "p0_full_proof_in_scan_cache_tx_files": p0_tx_files,
            "alternate_scan_hits_interesting": [
                h for h in alternate_hits if h.get("contains_full_P1") or h.get("contains_full_attack_preimage")
            ],
        },
        "candidates": candidates,
        "verdict_draft": {
            "on_chain_exact_alias_before_f24a": len(aliases) > 0,
            "historical_cache_priming_level_D": "UNPROVEN",
            "historical_causality_level_E": "UNPROVEN",
        },
    }

    OUT_JSON.write_text(json.dumps(out, indent=2) + "\n")
    print(json.dumps(out["summary"], indent=2))
    print("aliases:", out["summary"]["alias_txids"])
    print("wrote", OUT_JSON)
    print("wrote", OUT_TARGET_BIN, "len", len(target), "sha256", sha256(target))


if __name__ == "__main__":
    main()
