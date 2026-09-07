/* Minimal falsifiable reproduction of Elements CachingRangeProofChecker key bug.
 *
 * Demonstrates:
 *   Context A: (proof P, commitment C, generator GA, script SA) -> crypto TRUE
 *   Context B: (same P, same C, generator GB, script SB)         -> crypto FALSE
 *
 * Old cache key = H(P || C)           -> after verify(A), verify(B) returns TRUE (cache hit)
 * New cache key = H(P || C || G || S) -> after verify(A), verify(B) returns FALSE (miss + crypto fail)
 *
 * Also runs primed-vs-unprimed process modes via argv.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <secp256k1.h>
#include <secp256k1_generator.h>
#include <secp256k1_rangeproof.h>

/* Simple single-entry cache for demonstration (not Elements' cuckoo cache). */
typedef struct {
    int occupied;
    unsigned char key[32];
    int value; /* always 1 when occupied */
} toy_cache;

static void sha256_simple(const unsigned char *data, size_t len, unsigned char out[32]);

/* Minimal SHA256 (public domain compact impl) */
#include <stddef.h>
typedef struct { uint32_t s[8]; uint64_t bits; unsigned char buf[64]; int n; } SHA256_CTX;
static uint32_t rotr(uint32_t x,int n){return (x>>n)|(x<<(32-n));}
static void sha256_transform(SHA256_CTX *c, const unsigned char *d){
  static const uint32_t K[64]={
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
  uint32_t w[64],a,b,c2,d2,e,f,g,h,t1,t2; int i;
  for(i=0;i<16;i++){ w[i]=(d[4*i]<<24)|(d[4*i+1]<<16)|(d[4*i+2]<<8)|d[4*i+3]; }
  for(;i<64;i++){ uint32_t s0=rotr(w[i-15],7)^rotr(w[i-15],18)^(w[i-15]>>3); uint32_t s1=rotr(w[i-2],17)^rotr(w[i-2],19)^(w[i-2]>>10); w[i]=w[i-16]+s0+w[i-7]+s1; }
  a=c->s[0];b=c->s[1];c2=c->s[2];d2=c->s[3];e=c->s[4];f=c->s[5];g=c->s[6];h=c->s[7];
  for(i=0;i<64;i++){ uint32_t S1=rotr(e,6)^rotr(e,11)^rotr(e,25); uint32_t ch=(e&f)^((~e)&g); t1=h+S1+ch+K[i]+w[i]; uint32_t S0=rotr(a,2)^rotr(a,13)^rotr(a,22); uint32_t maj=(a&b)^(a&c2)^(b&c2); t2=S0+maj; h=g;g=f;f=e;e=d2+t1;d2=c2;c2=b;b=a;a=t1+t2; }
  c->s[0]+=a;c->s[1]+=b;c->s[2]+=c2;c->s[3]+=d2;c->s[4]+=e;c->s[5]+=f;c->s[6]+=g;c->s[7]+=h;
}
static void sha256_init(SHA256_CTX *c){ c->s[0]=0x6a09e667;c->s[1]=0xbb67ae85;c->s[2]=0x3c6ef372;c->s[3]=0xa54ff53a;c->s[4]=0x510e527f;c->s[5]=0x9b05688c;c->s[6]=0x1f83d9ab;c->s[7]=0x5be0cd19; c->bits=0; c->n=0; }
static void sha256_update(SHA256_CTX *c, const unsigned char *d, size_t len){
  c->bits += (uint64_t)len*8;
  while(len--){ c->buf[c->n++]=*d++; if(c->n==64){ sha256_transform(c,c->buf); c->n=0; } }
}
static void sha256_final(SHA256_CTX *c, unsigned char out[32]){
  int i; uint64_t bits=c->bits; c->buf[c->n++]=0x80;
  if(c->n>56){ while(c->n<64) c->buf[c->n++]=0; sha256_transform(c,c->buf); c->n=0; }
  while(c->n<56) c->buf[c->n++]=0;
  for(i=7;i>=0;i--) c->buf[c->n++]=(unsigned char)(bits>>(8*i));
  sha256_transform(c,c->buf);
  for(i=0;i<8;i++){ out[4*i]=c->s[i]>>24; out[4*i+1]=c->s[i]>>16; out[4*i+2]=c->s[i]>>8; out[4*i+3]=c->s[i]; }
}
static void sha256_simple(const unsigned char *data, size_t len, unsigned char out[32]){ SHA256_CTX c; sha256_init(&c); sha256_update(&c,data,len); sha256_final(&c,out); }

enum cache_mode { CACHE_OLD = 0, CACHE_NEW = 1 };

static void compute_key_old(unsigned char key[32],
                            const unsigned char *proof, size_t plen,
                            const unsigned char *commit33) {
  /* Matches historical Elements: hasher.Write(proof).Write(commitment) into salted SHA256.
   * We omit process salt (irrelevant for collision across contexts). */
  SHA256_CTX c; sha256_init(&c);
  sha256_update(&c, proof, plen);
  sha256_update(&c, commit33, 33);
  sha256_final(&c, key);
}

static void compute_key_new(unsigned char key[32],
                            const unsigned char *proof, size_t plen,
                            const unsigned char *commit33,
                            const unsigned char *asset33,
                            const unsigned char *script, size_t slen) {
  SHA256_CTX c; sha256_init(&c);
  sha256_update(&c, proof, plen);
  sha256_update(&c, commit33, 33);
  sha256_update(&c, asset33, 33);
  if (slen) sha256_update(&c, script, slen);
  sha256_final(&c, key);
}

static int cache_get(toy_cache *cache, const unsigned char key[32]) {
  if (!cache->occupied) return 0;
  return memcmp(cache->key, key, 32) == 0;
}
static void cache_set(toy_cache *cache, const unsigned char key[32]) {
  memcpy(cache->key, key, 32);
  cache->occupied = 1;
  cache->value = 1;
}

typedef struct {
  unsigned char proof[5200];
  size_t plen;
  unsigned char commit[33];
  unsigned char asset_ser[33]; /* serialized generator used as cache asset field */
  unsigned char script[128];
  size_t slen;
  secp256k1_generator gen;
  secp256k1_pedersen_commitment ped;
} ctx_pack;

static int crypto_verify(secp256k1_context *secp, const ctx_pack *x) {
  uint64_t minv=0, maxv=0;
  return secp256k1_rangeproof_verify(secp, &minv, &maxv, &x->ped,
                                     x->proof, x->plen,
                                     x->slen ? x->script : NULL, x->slen,
                                     &x->gen) == 1;
}

static int cached_verify(secp256k1_context *secp, toy_cache *cache, enum cache_mode mode,
                         const ctx_pack *x, int store, const char *label, int *cache_hit_out) {
  unsigned char key[32];
  if (mode == CACHE_OLD) compute_key_old(key, x->proof, x->plen, x->commit);
  else compute_key_new(key, x->proof, x->plen, x->commit, x->asset_ser, x->script, x->slen);

  if (cache_get(cache, key)) {
    printf("%s: CACHE_HIT -> TRUE (mode=%s)\n", label, mode==CACHE_OLD?"OLD":"NEW");
    if (cache_hit_out) *cache_hit_out = 1;
    return 1;
  }
  if (cache_hit_out) *cache_hit_out = 0;
  int ok = crypto_verify(secp, x);
  printf("%s: crypto=%d cache_miss (mode=%s)\n", label, ok, mode==CACHE_OLD?"OLD":"NEW");
  if (ok && store) cache_set(cache, key);
  return ok;
}

static int build_context_A(secp256k1_context *secp, ctx_pack *A, ctx_pack *B) {
  unsigned char assetA[32], assetB[32], blind[32], nonce[32];
  memset(assetA, 0x11, 32); memset(assetB, 0x22, 32);
  memset(blind, 0x33, 32); memset(nonce, 0x44, 32);
  uint64_t value = 123456789ULL;

  if (!secp256k1_generator_generate(secp, &A->gen, assetA)) return 0;
  if (!secp256k1_generator_serialize(secp, A->asset_ser, &A->gen)) return 0;
  if (!secp256k1_pedersen_commit(secp, &A->ped, blind, value, &A->gen)) return 0;
  if (!secp256k1_pedersen_commitment_serialize(secp, A->commit, &A->ped)) return 0;

  A->script[0] = 0x51; /* OP_TRUE */
  A->slen = 1;
  A->plen = sizeof(A->proof);
  if (!secp256k1_rangeproof_sign(secp, A->proof, &A->plen, /*min*/1, &A->ped, blind, nonce,
                                 /*exp*/0, /*min_bits*/32, value,
                                 NULL, 0, A->script, A->slen, &A->gen)) {
    fprintf(stderr, "rangeproof_sign failed\n");
    return 0;
  }

  /* Context B: same proof+commitment, different generator + script */
  if (!secp256k1_generator_generate(secp, &B->gen, assetB)) return 0;
  if (!secp256k1_generator_serialize(secp, B->asset_ser, &B->gen)) return 0;
  memcpy(B->proof, A->proof, A->plen); B->plen = A->plen;
  memcpy(B->commit, A->commit, 33);
  memcpy(&B->ped, &A->ped, sizeof(A->ped));
  B->script[0] = 0x6a; /* OP_RETURN */
  B->slen = 1;
  return 1;
}

static void hex32(const unsigned char *p){ for(int i=0;i<32;i++) printf("%02x", p[i]); }

static int run_sequence(enum cache_mode mode, int order_BA, const char *title) {
  printf("\n======== %s ========\n", title);
  secp256k1_context *secp = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY | SECP256K1_CONTEXT_SIGN);
  ctx_pack A, B; memset(&A,0,sizeof A); memset(&B,0,sizeof B);
  if (!build_context_A(secp, &A, &B)) { fprintf(stderr, "build failed\n"); return 2; }

  int cryptoA = crypto_verify(secp, &A);
  int cryptoB = crypto_verify(secp, &B);
  printf("baseline crypto A=%d B=%d (expect 1 and 0)\n", cryptoA, cryptoB);
  if (!(cryptoA == 1 && cryptoB == 0)) {
    fprintf(stderr, "FATAL: artificial contexts did not separate TRUE/FALSE\n");
    return 3;
  }

  unsigned char keyA_old[32], keyB_old[32], keyA_new[32], keyB_new[32];
  compute_key_old(keyA_old, A.proof, A.plen, A.commit);
  compute_key_old(keyB_old, B.proof, B.plen, B.commit);
  compute_key_new(keyA_new, A.proof, A.plen, A.commit, A.asset_ser, A.script, A.slen);
  compute_key_new(keyB_new, B.proof, B.plen, B.commit, B.asset_ser, B.script, B.slen);
  printf("OLD keys equal? %d (expect 1)\n", memcmp(keyA_old, keyB_old, 32)==0);
  printf("NEW keys equal? %d (expect 0)\n", memcmp(keyA_new, keyB_new, 32)==0);
  printf("OLD key A="); hex32(keyA_old); printf("\n");
  printf("NEW key A="); hex32(keyA_new); printf("\n");
  printf("NEW key B="); hex32(keyB_new); printf("\n");

  toy_cache cache; memset(&cache, 0, sizeof cache);
  int hit1=0, hit2=0, r1, r2;
  if (!order_BA) {
    r1 = cached_verify(secp, &cache, mode, &A, 1, "verify(A)", &hit1);
    r2 = cached_verify(secp, &cache, mode, &B, 1, "verify(B)", &hit2);
  } else {
    r1 = cached_verify(secp, &cache, mode, &B, 1, "verify(B)", &hit1);
    r2 = cached_verify(secp, &cache, mode, &A, 1, "verify(A)", &hit2);
  }
  printf("results: first=%d (hit=%d) second=%d (hit=%d)\n", r1, hit1, r2, hit2);
  secp256k1_context_destroy(secp);
  return 0;
}

int main(int argc, char **argv) {
  const char *mode = argc > 1 ? argv[1] : "all";
  if (strcmp(mode, "unprimed_old") == 0) {
    /* Process that only sees B under OLD cache */
    secp256k1_context *secp = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY | SECP256K1_CONTEXT_SIGN);
    ctx_pack A,B; memset(&A,0,sizeof A); memset(&B,0,sizeof B);
    build_context_A(secp,&A,&B);
    toy_cache cache; memset(&cache,0,sizeof cache);
    int hit=0; int r=cached_verify(secp,&cache,CACHE_OLD,&B,1,"UNPRIMED verify(B)",&hit);
    printf("UNPRIMED_OLD_RESULT=%d hit=%d\n", r, hit);
    return r; /* expect 0 */
  }
  if (strcmp(mode, "primed_old") == 0) {
    secp256k1_context *secp = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY | SECP256K1_CONTEXT_SIGN);
    ctx_pack A,B; memset(&A,0,sizeof A); memset(&B,0,sizeof B);
    build_context_A(secp,&A,&B);
    toy_cache cache; memset(&cache,0,sizeof cache);
    int hit=0;
    cached_verify(secp,&cache,CACHE_OLD,&A,1,"PRIMED verify(A)",&hit);
    int r=cached_verify(secp,&cache,CACHE_OLD,&B,1,"PRIMED verify(B)",&hit);
    printf("PRIMED_OLD_RESULT=%d hit=%d\n", r, hit);
    return r ? 0 : 1; /* expect success (bug) */
  }
  if (strcmp(mode, "primed_new") == 0) {
    secp256k1_context *secp = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY | SECP256K1_CONTEXT_SIGN);
    ctx_pack A,B; memset(&A,0,sizeof A); memset(&B,0,sizeof B);
    build_context_A(secp,&A,&B);
    toy_cache cache; memset(&cache,0,sizeof cache);
    int hit=0;
    cached_verify(secp,&cache,CACHE_NEW,&A,1,"PRIMED_NEW verify(A)",&hit);
    int r=cached_verify(secp,&cache,CACHE_NEW,&B,1,"PRIMED_NEW verify(B)",&hit);
    printf("PRIMED_NEW_RESULT=%d hit=%d\n", r, hit);
    return r; /* expect 0 */
  }

  run_sequence(CACHE_OLD, 0, "OLD cache: A then B (exploit path)");
  run_sequence(CACHE_NEW, 0, "NEW cache: A then B (fixed path)");
  run_sequence(CACHE_OLD, 1, "OLD cache: B then A (inverse)");
  run_sequence(CACHE_NEW, 1, "NEW cache: B then A (inverse)");

  printf("\n======== primed vs unprimed subprocesses ========\n");
  fflush(stdout);
  int u = system("./build/cache_repro unprimed_old");
  int p = system("./build/cache_repro primed_old");
  int n = system("./build/cache_repro primed_new");
  printf("subprocess exit codes: unprimed_old=%d primed_old=%d primed_new=%d\n", u, p, n);
  printf("Interpretation: unprimed rejects B (nonzero), primed_old accepts B (zero), primed_new rejects B (nonzero)\n");
  return 0;
}
