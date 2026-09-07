#!/usr/bin/env python3
"""Scan Liquid blocks for exact P0 rangeproof occurrences."""
from __future__ import annotations

import hashlib
import json
import os
import random
import sys
import time
import urllib.error
import urllib.request
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path
from threading import Lock

BASES = [
    "https://blockstream.info/liquid/api",
    "https://liquid.network/api",
]
CACHE = Path(__file__).resolve().parent / "scan_cache"
CACHE.mkdir(parents=True, exist_ok=True)
OUT = Path(__file__).resolve().parent / "68_rangeproof_occurrences.json"
PROGRESS = CACHE / "progress.json"

P0_HEX = Path(
    "final/hex/71c93d43_out0_rangeproof.hex"
).read_text().strip()
assert len(P0_HEX) == 8332
P0_PREFIX = P0_HEX[:64]
P0_SHA = hashlib.sha256(bytes.fromhex(P0_HEX)).hexdigest()
C0 = Path(
    "final/hex/71c93d43_out0_commitment.hex"
).read_text().strip()

# Also look for other 4166-byte proofs via length marker fd4601 (LE compact size 0x0146 = 326)
# Elements uses compactSize; 4166 = 0x1046 -> fd 46 10
PROOF_LEN_MARKER = "fd4610"  # little-endian compact size for 4166

lock = Lock()
stats = {"api_errors": [], "requests": 0, "retries": 0}
base_idx = 0


def log(msg: str) -> None:
    print(msg, flush=True)


def http_get(path: str, timeout: float = 60.0) -> str:
    global base_idx
    last_err: Exception | None = None
    for attempt in range(12):
        base = BASES[base_idx % len(BASES)]
        url = f"{base}{path}"
        try:
            with lock:
                stats["requests"] += 1
            req = urllib.request.Request(
                url,
                headers={"User-Agent": "liquid-forensic-scan/1.0", "Accept": "*/*"},
            )
            with urllib.request.urlopen(req, timeout=timeout) as resp:
                data = resp.read().decode("utf-8", errors="replace")
            return data
        except Exception as e:  # noqa: BLE001
            last_err = e
            with lock:
                stats["retries"] += 1
            # rotate base on failure
            base_idx += 1
            sleep = min(30.0, (0.4 * (2 ** min(attempt, 6))) + random.random())
            if attempt >= 3:
                with lock:
                    stats["api_errors"].append(
                        {"url": url, "error": str(e), "attempt": attempt + 1}
                    )
                log(f"  retry {attempt+1}/12 {path}: {e}")
            time.sleep(sleep)
    raise RuntimeError(f"GET failed after retries: {path}: {last_err}")


def cache_read(name: str) -> str | None:
    p = CACHE / name
    if p.exists():
        return p.read_text()
    return None


def cache_write(name: str, text: str) -> None:
    p = CACHE / name
    p.write_text(text)


def get_block_hash(height: int) -> str:
    name = f"h_{height}.hash"
    cached = cache_read(name)
    if cached:
        return cached.strip()
    h = http_get(f"/block-height/{height}").strip()
    cache_write(name, h + "\n")
    return h


def get_txids(block_hash: str) -> list[str]:
    name = f"b_{block_hash}.txids"
    cached = cache_read(name)
    if cached:
        return json.loads(cached)
    raw = http_get(f"/block/{block_hash}/txids")
    txids = json.loads(raw)
    cache_write(name, json.dumps(txids))
    return txids


def get_block_header(block_hash: str) -> dict:
    name = f"b_{block_hash}.header.json"
    cached = cache_read(name)
    if cached:
        return json.loads(cached)
    raw = http_get(f"/block/{block_hash}")
    data = json.loads(raw)
    # keep slim
    slim = {
        "id": data.get("id"),
        "height": data.get("height"),
        "timestamp": data.get("timestamp"),
        "tx_count": data.get("tx_count"),
    }
    cache_write(name, json.dumps(slim))
    return slim


def get_tx_hex(txid: str) -> str:
    name = f"tx_{txid}.hex"
    cached = cache_read(name)
    if cached:
        return cached.strip()
    hx = http_get(f"/tx/{txid}/hex", timeout=90.0).strip()
    cache_write(name, hx + "\n")
    return hx


def analyze_tx(txid: str, height: int, block_hash: str, timestamp) -> list[dict]:
    hx = get_tx_hex(txid).lower()
    occs: list[dict] = []

    # Exact P0 search
    start = 0
    while True:
        idx = hx.find(P0_PREFIX, start)
        if idx < 0:
            break
        chunk = hx[idx : idx + 8332]
        if len(chunk) == 8332:
            proof_bytes = bytes.fromhex(chunk)
            proof_sha = hashlib.sha256(proof_bytes).hexdigest()
            identical = chunk == P0_HEX
            c0_present = C0.lower() in hx
            # try to find commitment immediately before proof region (heuristic)
            # value commitment is 33 bytes = 66 hex, often preceded by asset commitment
            vc = None
            if identical or proof_sha == P0_SHA:
                # look backwards for C0 specifically
                if c0_present:
                    vc = C0
            occs.append(
                {
                    "txid": txid,
                    "height": height,
                    "block_hash": block_hash,
                    "timestamp_if_known": timestamp,
                    "proof_sha256": proof_sha,
                    "proof_len": 4166,
                    "value_commitment_if_known": vc,
                    "hex_offset": idx,
                    "byte_identical_to_p0": identical,
                    "c0_in_same_tx": c0_present,
                    "notes": "exact P0 prefix match; full 4166 verified"
                    if identical
                    else "prefix matched but body differs",
                }
            )
        start = idx + 2

    # Also flag other 4166-byte proofs via compact-size marker (may false-positive)
    # Only if no exact P0 found, still record distinct fd4610.. proofs of length 4166
    if not occs:
        # search for length marker; proof follows marker
        pos = 0
        while True:
            m = hx.find(PROOF_LEN_MARKER, pos)
            if m < 0:
                break
            proof_start = m + len(PROOF_LEN_MARKER)
            chunk = hx[proof_start : proof_start + 8332]
            if len(chunk) == 8332:
                try:
                    proof_bytes = bytes.fromhex(chunk)
                except ValueError:
                    pos = m + 2
                    continue
                if len(proof_bytes) == 4166:
                    proof_sha = hashlib.sha256(proof_bytes).hexdigest()
                    # skip if already would be P0 (shouldn't happen)
                    if proof_sha != P0_SHA:
                        # only record if it looks like a surjection/rangeproof start
                        # rangeproofs often start with 0x40...
                        if chunk.startswith("40") or chunk.startswith("60"):
                            occs.append(
                                {
                                    "txid": txid,
                                    "height": height,
                                    "block_hash": block_hash,
                                    "timestamp_if_known": timestamp,
                                    "proof_sha256": proof_sha,
                                    "proof_len": 4166,
                                    "value_commitment_if_known": C0
                                    if C0.lower() in hx
                                    else None,
                                    "hex_offset": proof_start,
                                    "byte_identical_to_p0": False,
                                    "c0_in_same_tx": C0.lower() in hx,
                                    "notes": "other 4166-byte proof candidate (fd4610 marker)",
                                }
                            )
            pos = m + 2
    return occs


def load_progress() -> dict:
    if PROGRESS.exists():
        return json.loads(PROGRESS.read_text())
    return {"done_heights": [], "occurrences": [], "other_4166": []}


def save_progress(prog: dict) -> None:
    PROGRESS.write_text(json.dumps(prog))


def scan_heights(heights: list[int], prog: dict, workers: int = 8) -> None:
    done = set(prog["done_heights"])
    todo = [h for h in heights if h not in done]
    log(f"Scanning {len(todo)} heights ({len(done)} already done) with {workers} workers")

    def scan_one(height: int) -> tuple[int, list[dict], list[dict]]:
        block_hash = get_block_hash(height)
        header = get_block_header(block_hash)
        ts = header.get("timestamp")
        txids = get_txids(block_hash)
        found = []
        other = []
        for txid in txids:
            # coinbase rarely has confidential proofs; still scan
            try:
                occs = analyze_tx(txid, height, block_hash, ts)
            except Exception as e:  # noqa: BLE001
                log(f"  ERROR tx {txid} @ {height}: {e}")
                with lock:
                    stats["api_errors"].append(
                        {"txid": txid, "height": height, "error": str(e)}
                    )
                continue
            for o in occs:
                if o.get("byte_identical_to_p0") or o["proof_sha256"] == P0_SHA:
                    found.append(o)
                else:
                    other.append(o)
        return height, found, other

    # Process in batches to save progress frequently
    batch_size = 20
    for i in range(0, len(todo), batch_size):
        batch = todo[i : i + batch_size]
        with ThreadPoolExecutor(max_workers=workers) as ex:
            futs = {ex.submit(scan_one, h): h for h in batch}
            for fut in as_completed(futs):
                height, found, other = fut.result()
                with lock:
                    prog["done_heights"].append(height)
                    prog["occurrences"].extend(found)
                    prog["other_4166"].extend(other)
                if found:
                    log(
                        f"HIT height={height} n={len(found)} "
                        f"txids={[x['txid'][:16] for x in found]}"
                    )
                else:
                    log(f"ok {height} (no P0)")
        save_progress(prog)
        log(
            f"progress {len(prog['done_heights'])}/{len(done)+len(todo)} "
            f"hits={len(prog['occurrences'])} other4166={len(prog['other_4166'])}"
        )


def write_final(prog: dict, window: str, method: str) -> None:
    # Deduplicate occurrences by (txid, hex_offset)
    seen = set()
    occs = []
    for o in prog["occurrences"]:
        key = (o["txid"], o.get("hex_offset"))
        if key in seen:
            continue
        seen.add(key)
        # slim output fields
        occs.append(
            {
                "txid": o["txid"],
                "height": o["height"],
                "block_hash": o["block_hash"],
                "timestamp_if_known": o.get("timestamp_if_known"),
                "proof_sha256": o["proof_sha256"],
                "proof_len": o["proof_len"],
                "value_commitment_if_known": o.get("value_commitment_if_known"),
                "notes": o.get("notes", ""),
            }
        )
    occs.sort(key=lambda x: (x["height"], x["txid"]))

    other = []
    seen_o = set()
    for o in prog.get("other_4166", []):
        key = (o["txid"], o.get("hex_offset"), o["proof_sha256"])
        if key in seen_o:
            continue
        seen_o.add(key)
        other.append(o)

    unique_hashes = sorted({o["proof_sha256"] for o in occs})
    result = {
        "scan_method": method,
        "window": window,
        "p0_sha256": P0_SHA,
        "p0_len": 4166,
        "p0_prefix": P0_PREFIX,
        "c0": C0,
        "occurrences": occs,
        "count": len(occs),
        "unique_proof_hashes": unique_hashes,
        "other_4166_candidates": other[:50],  # cap
        "other_4166_count": len(other),
        "heights_scanned": sorted(set(prog["done_heights"])),
        "heights_scanned_count": len(set(prog["done_heights"])),
        "api_error_count": len(stats["api_errors"]),
        "api_errors_sample": stats["api_errors"][-20:],
        "requests": stats["requests"],
        "retries": stats["retries"],
        "limitations": (
            "Searched raw tx hex for exact P0 bytes (prefix + full 8332 hex). "
            "Other 4166-byte candidates via compact-size marker fd4610 are heuristic "
            "and may include false positives; only P0 exact matches are authoritative. "
            "Did not parse full Elements witness structure for every output."
        ),
    }
    OUT.write_text(json.dumps(result, indent=2) + "\n")
    log(f"Wrote {OUT} count={result['count']}")


def main() -> None:
    mode = sys.argv[1] if len(sys.argv) > 1 else "priority"

    prog = load_progress()
    # Normalize
    prog.setdefault("done_heights", [])
    prog.setdefault("occurrences", [])
    prog.setdefault("other_4166", [])

    if mode == "sample":
        # every 10th across 4049384–4050246
        heights = list(range(4049384, 4050247, 10))
        method = "every-10th sample 4049384–4050246 + known checks"
        window = "4049384-4050246 sample step=10"
    elif mode == "priority":
        # (c) full 4050200–4050336 and 4049384–4049500
        heights = list(range(4050200, 4050337)) + list(range(4049384, 4049501))
        method = "priority full: 4050200–4050336 and 4049384–4049500"
        window = "4050200-4050336 + 4049384-4049500"
    elif mode == "full_claim":
        heights = list(range(4049384, 4050247))
        method = "full scan claimed Bitquery window 4049384–4050246"
        window = "4049384-4050246"
    elif mode == "full_extended":
        heights = list(range(4049384, 4050337))
        method = "full scan 4049384–4050336"
        window = "4049384-4050336"
    elif mode == "dense_hits":
        # denser around any known hits
        centers = sorted({o["height"] for o in prog["occurrences"]})
        heights = []
        for c in centers:
            heights.extend(range(max(4049384, c - 50), min(4050336, c + 50) + 1))
        heights = sorted(set(heights))
        method = "dense ±50 around prior hits"
        window = f"around hits {centers}"
    elif mode == "finalize":
        write_final(
            prog,
            window="see heights_scanned",
            method="merged from progress cache",
        )
        return
    else:
        # custom: start-end
        a, b = mode.split("-")
        heights = list(range(int(a), int(b) + 1))
        method = f"full scan {a}-{b}"
        window = f"{a}-{b}"

    # Always ensure known block is included
    if 4050335 not in heights:
        heights.append(4050335)
    heights = sorted(set(heights))

    scan_heights(heights, prog, workers=6)
    write_final(prog, window=window, method=method)

    # Summary
    occs = prog["occurrences"]
    p0 = [o for o in occs if o.get("byte_identical_to_p0") or o["proof_sha256"] == P0_SHA]
    if p0:
        hs = sorted(o["height"] for o in p0)
        log(f"SUMMARY P0 count={len(p0)} first={hs[0]} last={hs[-1]}")
    else:
        log("SUMMARY P0 count=0")


if __name__ == "__main__":
    main()
