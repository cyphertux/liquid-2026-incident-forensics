/* Independent rangeproof verification harness for Liquid incident reproduction.
 * Links against Elements' bundled secp256k1-zkp (rangeproof + generator modules).
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <secp256k1.h>
#include <secp256k1_generator.h>
#include <secp256k1_rangeproof.h>

static int hex_nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static unsigned char *hex_decode(const char *hex, size_t *out_len) {
    size_t n = strlen(hex);
    if (n % 2) return NULL;
    *out_len = n / 2;
    unsigned char *buf = malloc(*out_len);
    if (!buf) return NULL;
    for (size_t i = 0; i < *out_len; i++) {
        int a = hex_nibble(hex[2 * i]);
        int b = hex_nibble(hex[2 * i + 1]);
        if (a < 0 || b < 0) { free(buf); return NULL; }
        buf[i] = (unsigned char)((a << 4) | b);
    }
    return buf;
}

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc((size_t)n + 1);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, (size_t)n, f) != (size_t)n) { free(buf); fclose(f); return NULL; }
    buf[n] = 0;
    fclose(f);
    /* strip trailing whitespace/newlines */
    while (n > 0 && (buf[n-1] == '\n' || buf[n-1] == '\r' || buf[n-1] == ' ')) buf[--n] = 0;
    return buf;
}

static void print_hex(const char *label, const unsigned char *p, size_t n) {
    printf("%s (%zu): ", label, n);
    for (size_t i = 0; i < n; i++) printf("%02x", p[i]);
    printf("\n");
}

/* Mimic Elements: for explicit asset, generator = secp256k1_generator_generate(asset_id)
 * asset_id is the 32 bytes after the 0x01 prefix, in the same byte order as CAsset (wire order).
 */
static int make_generator(const secp256k1_context *ctx,
                          secp256k1_generator *gen,
                          unsigned char gen_ser[33],
                          const unsigned char *asset_or_gen, size_t asset_len,
                          int force_explicit_as_id) {
    if (asset_len == 33 && (asset_or_gen[0] == 0x0a || asset_or_gen[0] == 0x0b) && !force_explicit_as_id) {
        if (!secp256k1_generator_parse(ctx, gen, asset_or_gen)) return 0;
        memcpy(gen_ser, asset_or_gen, 33);
        return 1;
    }
    if (asset_len == 33 && asset_or_gen[0] == 0x01) {
        if (!secp256k1_generator_generate(ctx, gen, asset_or_gen + 1)) return 0;
        if (!secp256k1_generator_serialize(ctx, gen_ser, gen)) return 0;
        return 1;
    }
    if (asset_len == 32) {
        if (!secp256k1_generator_generate(ctx, gen, asset_or_gen)) return 0;
        if (!secp256k1_generator_serialize(ctx, gen_ser, gen)) return 0;
        return 1;
    }
    if (asset_len == 33) {
        /* treat whole buffer as serialized generator attempt */
        if (!secp256k1_generator_parse(ctx, gen, asset_or_gen)) return 0;
        memcpy(gen_ser, asset_or_gen, 33);
        return 1;
    }
    return 0;
}

static int verify_one(const secp256k1_context *ctx,
                      const unsigned char *proof, size_t proof_len,
                      const unsigned char *commit33,
                      const unsigned char *asset_or_gen, size_t asset_len,
                      const unsigned char *extra, size_t extra_len,
                      const char *label) {
    secp256k1_pedersen_commitment commit;
    secp256k1_generator gen;
    unsigned char gen_ser[33];
    uint64_t min_v = 0, max_v = 0;

    printf("\n=== %s ===\n", label);
    print_hex("commit", commit33, 33);
    print_hex("asset/gen in", asset_or_gen, asset_len);
    print_hex("extra_commit(script)", extra, extra_len);
    printf("proof_len=%zu\n", proof_len);

    if (!secp256k1_pedersen_commitment_parse(ctx, &commit, commit33)) {
        printf("RESULT: FALSE (commitment parse failed)\n");
        return 0;
    }
    if (!make_generator(ctx, &gen, gen_ser, asset_or_gen, asset_len, 0)) {
        printf("RESULT: FALSE (generator parse/generate failed)\n");
        return 0;
    }
    print_hex("generator_ser", gen_ser, 33);

    int ok = secp256k1_rangeproof_verify(ctx, &min_v, &max_v, &commit,
                                         proof, proof_len,
                                         extra_len ? extra : NULL, extra_len,
                                         &gen);
    printf("secp256k1_rangeproof_verify => %d\n", ok);
    if (ok) {
        printf("min_value=%llu max_value=%llu\n",
               (unsigned long long)min_v, (unsigned long long)max_v);
        printf("RESULT: TRUE\n");
    } else {
        printf("RESULT: FALSE\n");
    }
    return ok;
}

int main(int argc, char **argv) {
    const char *base = argc > 1 ? argv[1] : "data";
    char path[1024];

    snprintf(path, sizeof(path), "%s/out1_rangeproof.hex", base);
    char *proof_hex = read_file(path);
    snprintf(path, sizeof(path), "%s/out1_commitment.hex", base);
    char *commit_hex = read_file(path);
    snprintf(path, sizeof(path), "%s/out1_asset.hex", base);
    char *asset_hex = read_file(path);
    snprintf(path, sizeof(path), "%s/out1_script.hex", base);
    char *script_hex = read_file(path);

    if (!proof_hex || !commit_hex || !asset_hex || !script_hex) {
        fprintf(stderr, "failed reading extracted hex files under %s\n", base);
        return 2;
    }

    size_t proof_len, commit_len, asset_len, script_len;
    unsigned char *proof = hex_decode(proof_hex, &proof_len);
    unsigned char *commit = hex_decode(commit_hex, &commit_len);
    unsigned char *asset = hex_decode(asset_hex, &asset_len);
    unsigned char *script = hex_decode(script_hex, &script_len);
    if (!proof || !commit || !asset || !script || commit_len != 33) {
        fprintf(stderr, "hex decode failed\n");
        return 2;
    }

    secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY | SECP256K1_CONTEXT_SIGN);
    if (!ctx) return 3;

    /* Context B: real L-BTC explicit asset + OP_RETURN script (as in f24a out1) */
    int real = verify_one(ctx, proof, proof_len, commit, asset, asset_len, script, script_len,
                          "REAL context: L-BTC explicit asset generator + script OP_RETURN (0x6a)");

    /* Empty script */
    int empty_script = verify_one(ctx, proof, proof_len, commit, asset, asset_len, NULL, 0,
                                  "ALT: same L-BTC generator, empty extra_commit");

    /* Spendable P2WPKH script from same tx out0 */
    const char *spk_hex = "0014f5906a8572cf826a9d737ce1bc6a19a066868630";
    size_t spk_len;
    unsigned char *spk = hex_decode(spk_hex, &spk_len);
    int spendable = verify_one(ctx, proof, proof_len, commit, asset, asset_len, spk, spk_len,
                               "ALT: same L-BTC generator, spendable P2WPKH script");

    /* H = secp256k1 default generator (G's twin used historically) — try asset commitment from out2 */
    const char *blind_asset = "0b0957be4cbd0d1cc30d2742988f84f931f4f47d4f8b81e40dc1db42cabf4aae7b";
    size_t ba_len;
    unsigned char *ba = hex_decode(blind_asset, &ba_len);
    int other_asset = verify_one(ctx, proof, proof_len, commit, ba, ba_len, script, script_len,
                                 "ALT: out2 blinded asset generator + OP_RETURN");

    int other_asset_spk = verify_one(ctx, proof, proof_len, commit, ba, ba_len, spk, spk_len,
                                     "ALT: out2 blinded asset generator + P2WPKH");

    /* Try generator from prevout asset commitments (input's parent) */
    const char *prev_assets[] = {
        "0abc38a76bc56788e46f7c911f7863aa4926e2718fd3823166fe5f7bd66f1278df",
        "0b6df45dfc3e96bd5c49479f7ea74fc07726d81b984844405ffbec327b4b521896",
        "0b18d9aa24a701e2b27bd81c7f4fc1a537ccb59a81e55c566faa3c7dc6aaab3785",
        "0bd0e7e6d5e5c0d1531bea2eec3aeb55134131d53356d2f934444268cf08592a6b",
        NULL
    };
    for (int i = 0; prev_assets[i]; i++) {
        size_t l; unsigned char *a = hex_decode(prev_assets[i], &l);
        char label[128];
        snprintf(label, sizeof(label), "ALT: prevout assetcommitment[%d] + OP_RETURN", i);
        verify_one(ctx, proof, proof_len, commit, a, l, script, script_len, label);
        snprintf(label, sizeof(label), "ALT: prevout assetcommitment[%d] + P2WPKH", i);
        verify_one(ctx, proof, proof_len, commit, a, l, spk, spk_len, label);
        snprintf(label, sizeof(label), "ALT: prevout assetcommitment[%d] + empty script", i);
        verify_one(ctx, proof, proof_len, commit, a, l, NULL, 0, label);
        free(a);
    }

    printf("\n======== SUMMARY ========\n");
    printf("real_LBTC_OP_RETURN=%d\n", real);
    printf("LBTC_empty_script=%d\n", empty_script);
    printf("LBTC_P2WPKH=%d\n", spendable);
    printf("out2_asset_OP_RETURN=%d\n", other_asset);
    printf("out2_asset_P2WPKH=%d\n", other_asset_spk);

    secp256k1_context_destroy(ctx);
    free(proof); free(commit); free(asset); free(script); free(spk); free(ba);
    free(proof_hex); free(commit_hex); free(asset_hex); free(script_hex);
    return real ? 0 : 1;
}
