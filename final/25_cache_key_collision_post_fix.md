# 25 — The acceptance path: a cache-key collision against the *post-fix* key

## Executive summary (≤15 lines)

1. The open question of `15`/`16`/`21` — how f24a passed `ConnectBlock` — has a mechanical answer.
2. It is cache priming, but **not** of `(P1,C1)` under the historical key. Chapter `04`'s negative is correct.
3. The pre-fix key `H(salt‖proof‖commitment)` has two fields, one fixed at 33 B, so it is **injective**.
4. `c26d719c29` appends `asset_commitment‖scriptPubKey` with **no length prefix and no domain separator**. Four fields, two variable: the encoding stops being injective.
5. `P1 == P0 ‖ C0 ‖ G ‖ 0x6a ‖ 0x43` where `(P0,C0)` are `71c93d43:0`'s proof and value commitment — **CONFIRMED BY EXECUTION**.
6. `S0 == 0x6a ‖ 0x43 ‖ C1 ‖ G ‖ 0x6a` — the primer's OP_RETURN script is the tail of the attack's preimage — **CONFIRMED BY EXECUTION**.
7. Both post-fix preimages are the **same 4301 bytes**, split at different field boundaries → same cache entry — **CONFIRMED BY EXECUTION**.
8. Under the pre-fix key the preimages are 4199 B vs 4267 B → no collision — **CONFIRMED BY EXECUTION**.
9. `vchAssetCommitment` entering the key is the **serialized generator**, not the wire asset field (`confidential_validation.cpp:379-383`). All three outputs are explicit L-BTC, so the same `G` enters all three keys.
10. Replay over unmodified upstream `cuckoocache.h`: post-fix + primer in mempool → **ACCEPT**; post-fix cold → REJECT; pre-fix → REJECT.
11. Collision is independent of the per-process salt: 2000/2000 — **CONFIRMED BY EXECUTION**.
12. The direction of the split is therefore **inverted** relative to the working hypothesis of `15`–`21`.

**Therefore:** the cache bug that admitted f24a is not the one `elements-23.3.3` shipped. It is the one
`c26d719c29` introduced. Chapter `04`'s "priming NON REPRODUIT" stands as written; only the inference
drawn from it (that priming is not the acceptance path) does not.

---

## 1. Why the earlier search could not find it

`04` searched for a context making `(P1,C1)` verify TRUE, and for an on-chain output carrying `P1` as its
proof. Both searches are sound and both return zero. They are also the wrong searches.

A cache hit does not require the same *proof*. It requires the same *key*. Under the pre-fix key the two
coincide, because the key is a function of `(proof, commitment)` alone and the commitment is fixed-length.
Under the post-fix key they no longer coincide.

| Question | Answer | Chapter |
|---|---|---|
| Is there a context where `(P1,C1)` verifies TRUE? | No | `04` — stands |
| Do `71c93d43` / `27114710` contain `P1`? | No | `04` — stands |
| Can they insert `H(P1‖C1)` under the **historical** key? | No | `04` — stands |
| Can they insert the **same entry as f24a:1** under the **post-fix** key? | **Yes** | this chapter |

---

## 2. The pivot: which bytes enter the key

`vchAssetCommitment` handed to `CachingRangeProofChecker::VerifyRangeProof` is not `nAsset.vchCommitment`.
For an explicit asset, `VerifyAmounts` overwrites it in place with the serialized generator:

```cpp
// src/confidential_validation.cpp:379-383 (master)
if (asset.IsExplicit()) {
    int ret = secp256k1_generator_generate(ctx, &gen, asset.GetAsset().begin());
    assert(ret != 0);
    secp256k1_generator_serialize(ctx, &vchAssetCommitment[0], &gen);
}
QueueCheck(checks, new CRangeCheck(&val, ptxoutwit->vchRangeproof,
                                   vchAssetCommitment, tx.vout[i].scriptPubKey, store_result));
```

This is structurally forced: `VerifyRangeProof` then calls `secp256k1_generator_parse`, which accepts only
the `0x0a` / `0x0b` prefixes. An explicit asset in wire form (`01 ‖ assetid`) would not parse.

| Field | f24a:1, 71c93d43:0, 27114710:0 |
|---|---|
| wire asset | `016d521c38ec1ea15734ae22b7c46064412829c0d0579f0a713d1c04ede979026f` |
| **entering the key** | `0a0a488de4899d0ae757f6cf8368663184d164106111ed9eaecf510e35282ddc6d` |

All three carry the same explicit L-BTC asset, so `G` is identical across them. **FACT**, re-derived
natively in `scripts/gt_crypto.c`.

---

## 3. Structure of the two objects

```
P1  =  P0 ‖ C0 ‖ G ‖ 0x6a ‖ 0x43           4166 + 33 + 33 + 1 + 1  =  4234 B
S0  =  0x6a ‖ 0x43 ‖ C1 ‖ G ‖ 0x6a         OP_RETURN PUSH67 <C1 ‖ G ‖ 0x6a>  =  69 B
```

The 4234-byte "rangeproof" of f24a:1 is not a proof. It is `71c93d43:0`'s cache preimage extended by 35
bytes. The 69-byte OP_RETURN script of the primer is not a data carrier. It is the tail of f24a:1's
preimage. Each object carries exactly the bytes the other is missing.

This also explains the header anomaly recorded in `15 §9`: `P1` opens `4033…` with no `min_value`, length
4234, against `6033…` / 4174 for out0 and out2. It opens that way because bytes `[0:4166]` are `P0`.

---

## 4. The alignment

Both outputs hand the hasher the same 4301 bytes.

| offset | content | f24a:1 reads it as | 71c93d43:0 reads it as |
|---|---|---|---|
| `[0:4166]` | `P0`, primer proof | proof | proof |
| `[4166:4199]` | `C0`, primer value commitment | proof | value_commitment |
| `[4199:4232]` | `G`, L-BTC generator | proof | asset_commitment |
| `[4232:4234]` | `6a 43`, OP_RETURN PUSH67 | proof | scriptPubKey |
| `[4234:4267]` | `C1`, attack value commitment | value_commitment | scriptPubKey |
| `[4267:4300]` | `G`, L-BTC generator | asset_commitment | scriptPubKey |
| `[4300:4301]` | `6a`, OP_RETURN | scriptPubKey | scriptPubKey |

The hinge is at offset 4234. SHA-256 sees only the string, never the split.

| key | primer | attack | result |
|---|---|---|---|
| pre-fix `H(salt‖proof‖commitment)` | 4199 B | 4267 B | different lengths, **no collision** |
| **post-fix** `H(salt‖proof‖commitment‖asset‖script)` | 4301 B | 4301 B | **identical preimages, same entry** |

---

## 5. Execution

`scripts/gt_crypto.c` — native secp256k1-zkp, exits 0 iff all four assertions hold.
Log: `logs/gt_crypto_run.log`.

```
derived generator f24a:1           0a0a488de4899d0ae757f6cf8368663184d164106111ed9eaecf510e35282ddc6d
derived generator 71c93d43:0       0a0a488de4899d0ae757f6cf8368663184d164106111ed9eaecf510e35282ddc6d
G found inside attack proof P1:    YES (offset 4199)
G found inside primer script S0:   YES (offset 35)
P1 == P0 || C0 || G || 0x6a || 0x43 : TRUE
S0 == 0x6a || 0x43 || C1 || G || 0x6a : TRUE
primer 71c93d43:0  (C0,P0,script=69B,G) -> TRUE    min=0 max=4503599627370495
attack f24a:1      (C1,P1,script=1B ,G) -> FALSE
VERDICT: structural=1 script=1 primer_true=1 attack_false=1
```

`scripts/cache_split.cpp` — replica of `VerifyRangeProof` over **unmodified upstream `cuckoocache.h`**,
real crypto underneath, SHA-256 self-tested against a NIST vector on every run.
Log: `logs/cache_split_run.log`.

```
=== PRE-FIX  key    primer 4199 B  f462ee33…  |  attack 4267 B  8a11f3f7…   SAME: no
=== POST-FIX key    primer 4301 B  8026a5bf…  |  attack 4301 B  8026a5bf…   SAME: YES
=== FRAMED   key    primer 4317 B  b266607b…  |  attack 4317 B  f87c8b86…   SAME: no
post-fix collision over 2000 independent per-process salts : 2000/2000
```

The salt is random per process. The collision is a preimage identity, not a hash accident.

---

## 6. Validation-path replay

```
---- node A  post-fix build, up since < 4050335, primer transited its mempool
  ATMP    4050335 primer  store=true   -> TRUE   crypto TRUE, entry stored
  ATMP    4050336 attack  store=true   -> TRUE   CACHE HIT (crypto skipped)
  CONNECT 4050335 primer  store=false  -> TRUE   CACHE HIT (crypto skipped)
  CONNECT 4050336 attack  store=false  -> TRUE   CACHE HIT (crypto skipped)
  => block 4050336 : ACCEPTED

---- node B  post-fix build, cold start / IBD, primer only seen inside a block
  CONNECT 4050336 attack  store=false  -> FALSE  crypto FALSE   => REJECTED

---- node C  pre-fix build (elements-23.3.3), primer transited its mempool
  ATMP    4050336 attack  store=true   -> FALSE  crypto FALSE   => REJECTED

---- node D  proposed length-framed key, primer transited its mempool
  ATMP    4050336 attack  store=true   -> FALSE  crypto FALSE   => REJECTED
```

This is the shape of the split described in `16 §22`: one block, one byte string, two verdicts, decided by
the process's memory history rather than by consensus rules.

### 6.1 Why the entry survives 4050335's connect

`store=true` happens only in `AcceptToMemoryPool`. `ConnectBlock` passes `store=false`, hence
`Get(entry, erase=true)`, and never inserts. The entry nonetheless survives:

```cpp
// src/cuckoocache.h — upstream
inline bool contains(const Element& e, const bool erase) const {
    std::array<uint32_t, 8> locs = compute_hashes(e);
    for (const uint32_t loc : locs)
        if (table[loc] == e) {
            if (erase) allow_erase(loc);   // bit_set only; element stays in table
            return true;
        }
    return false;
}
```

`allow_erase` marks the slot reclaimable; the value stays matchable until a later insert overwrites it.
Node A shows this empirically.

### 6.2 Consequence for the acceptor question of `21`

`21` returns FIRST ACCEPTOR = UNKNOWN, and that stands: no node logs were obtained here either. What this
chapter adds is a **necessary condition** on whoever accepted:

> any node that connected 4050336 was running a post-fix rangeproof cache key **and** had the 4050335
> primer in its mempool.

That is falsifiable from a version inventory or a single `debug.log`. It is **INFERENCE**, not FACT, until
one is produced.

---

## 7. Remediation, tested

Length-frame each field. Implemented as `FRAMED` in `scripts/cache_split.cpp` and exercised as node D:

```cpp
static void WriteLen(CSHA256& h, size_t n) {
    unsigned char le[4] = {(unsigned char)(n),       (unsigned char)(n >> 8),
                           (unsigned char)(n >> 16), (unsigned char)(n >> 24)};
    h.Write(le, 4);
}
// then, per field:  WriteLen(hasher, f.size()); hasher.Write(f.data(), f.size());
```

Preimages become 4317 B on both sides and hash to `b266607b…` vs `f87c8b86…`. The primed node rejects
f24a. BIP-340 style per-field tagged hashing is equivalent and preferable if the codebase already has it.

**Class note.** `ComputeEntrySurjectionProof(entry, hash, proof, commitment)` has one variable-length field
today (`proof`) between two fixed ones, so it has no split ambiguity yet. The moment a second variable
field is appended it becomes the rangeproof key. Framing it now costs nothing.

---

## 8. What this chapter does and does not claim

| Claim | Status |
|---|---|
| `P1` is structurally `P0‖C0‖G‖6a43` | **CONFIRMED BY EXECUTION** |
| `S0` is structurally `6a43‖C1‖G‖6a` | **CONFIRMED BY EXECUTION** |
| Post-fix preimages are byte-identical | **CONFIRMED BY EXECUTION** |
| Pre-fix preimages differ | **CONFIRMED BY EXECUTION** |
| Primer verifies TRUE, attack verifies FALSE, natively | **CONFIRMED BY EXECUTION** |
| A post-fix primed node accepts, others reject | **CONFIRMED BY EXECUTION** (replica, not `elementsd`) |
| The real acceptors ran a post-fix key | **INFERENCE** — needs a version inventory |
| The primer transited the real acceptors' mempools | **INFERENCE** — needs `debug.log` |
| Numerical value committed by `C1` | **NON REPRODUIT** — needs the blinding factor |
| Attacker intent | **INFERENCE** — a 4301-byte exact preimage match is not plausibly accidental, but intent is not a measurable |

Full `elementsd` accept/reject remains **NON REPRODUIT**, exactly as in `06` and `15 §5`. This chapter
reproduces the checker and its cache, not the chainstate.

### What would falsify it

- A post-fix `elementsd`, primed by 4050335, that still rejects 4050336.
- Evidence that the functionaries ran a pre-fix key on 2026-09-06. The collision would remain real but
  would not be the path that was used.
- A divergence between the `SignatureCache` replica here and Elements' real `CSHA256`. The NIST self-test
  on every run makes this unlikely but does not exclude it.

---

## 9. Reproduce

```bash
git clone --depth 1 https://github.com/ElementsProject/secp256k1-zkp.git zkp

gcc -O2 -DSECP256K1_STATIC -Izkp -Izkp/include -Izkp/src \
    -DENABLE_MODULE_GENERATOR=1 -DENABLE_MODULE_RANGEPROOF=1 \
    -DENABLE_MODULE_SURJECTIONPROOF=1 -DECMULT_WINDOW_SIZE=15 -DECMULT_GEN_PREC_BITS=4 \
    -c zkp/src/secp256k1.c zkp/src/precomputed_ecmult.c zkp/src/precomputed_ecmult_gen.c
ar rcs libsecp.a secp256k1.o precomputed_ecmult.o precomputed_ecmult_gen.o

gcc -O2 -DSECP256K1_STATIC -Izkp/include final/scripts/gt_crypto.c libsecp.a -o gt_crypto
./gt_crypto            # exits 0 iff all four assertions hold

# needs src/cuckoocache.h from ElementsProject/elements on the include path,
# plus final/scripts/shim/ for util/fastrange.h and the CSHA256 replica
g++ -std=c++17 -O2 -DSECP256K1_STATIC -I. -Ifinal/scripts/shim -Izkp/include \
    final/scripts/cache_split.cpp libsecp.a -o cache_split
./cache_split
```

Both read the hex under `data/`. `-DSECP256K1_STATIC` is required on MinGW, otherwise every secp symbol
comes back as an undefined `__imp_` import.
