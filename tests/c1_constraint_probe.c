#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <secp256k1.h>
#include <secp256k1_generator.h>
#include <secp256k1_rangeproof.h>

static int hx(const char *hex, unsigned char *out, size_t n){
  for(size_t i=0;i<n;i++){ unsigned v; if(sscanf(hex+2*i,"%2x",&v)!=1) return 0; out[i]=(unsigned char)v; }
  return 1;
}

int main(void){
  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_NONE);
  unsigned char asset_id[32];
  /* wire explicit asset without 01 prefix is BE; Elements generate() uses internal id */
  hx("6d521c38ec1ea15734ae22b7c46064412829c0d0579f0a713d1c04ede979026f", asset_id, 32);
  secp256k1_generator gen;
  if(!secp256k1_generator_generate(ctx, &gen, asset_id)){ puts("gen fail"); return 1; }
  unsigned char gen_ser[33];
  secp256k1_generator_serialize(ctx, gen_ser, &gen);
  printf("G_L "); for(int i=0;i<33;i++) printf("%02x", gen_ser[i]); printf("\n");

  unsigned char c1raw[33];
  hx("086f5d67160fc4b477954fb09ef321e5b589d7a07740a1a6df494ed2335b1d01d8", c1raw, 33);
  secp256k1_pedersen_commitment C1, C;
  if(!secp256k1_pedersen_commitment_parse(ctx, &C1, c1raw)){ puts("parse C1 fail"); return 1; }

  unsigned char zero_blind[32] = {0};
  int hit0 = 0;
  for(uint64_t v=0; v<=100000; v++){
    if(!secp256k1_pedersen_commit(ctx, &C, zero_blind, v, &gen)) continue;
    unsigned char ser[33];
    secp256k1_pedersen_commitment_serialize(ctx, ser, &C);
    if(memcmp(ser, c1raw, 33)==0){ printf("HIT blind0 v=%llu\n", (unsigned long long)v); hit0=1; break; }
  }
  if(!hit0) puts("no_hit: C1 != commit(v,r=0,G_L) for v in [0,100000]");

  unsigned char cin[33], c0[33], c2[33];
  hx("09121a4e92717097b9562e2cf3cdbee843a55f819074f6c0e11a29c46874fcde3d", cin, 33);
  hx("08360f95ce0a63e76daab6a05616103ba7462a9af7db0697e9c8fa6a65aba103f8", c0, 33);
  hx("08dc50cfb1c9b0b512b446cd80d8bbe81f5ace540f59780d6fad773fa9204d8d35", c2, 33);
  secp256k1_pedersen_commitment In, Out0, Out2, Fee;
  if(!secp256k1_pedersen_commitment_parse(ctx, &In, cin)) return 2;
  if(!secp256k1_pedersen_commitment_parse(ctx, &Out0, c0)) return 2;
  if(!secp256k1_pedersen_commitment_parse(ctx, &Out2, c2)) return 2;
  if(!secp256k1_pedersen_commit(ctx, &Fee, zero_blind, 58, &gen)) return 2;

  const secp256k1_pedersen_commitment* pins[1] = {&In};
  const secp256k1_pedersen_commitment* pfull[4] = {&Out0, &C1, &Out2, &Fee};
  const secp256k1_pedersen_commitment* pwout1[3] = {&Out0, &Out2, &Fee};
  printf("tally_with_C1=%d tally_without_C1=%d\n",
    secp256k1_pedersen_verify_tally(ctx, pins, 1, pfull, 4),
    secp256k1_pedersen_verify_tally(ctx, pins, 1, pwout1, 3));

  FILE *f = fopen("data/out1_rangeproof.hex","r");
  if(!f){ puts("missing proof hex"); return 3; }
  char *hex = NULL; size_t cap=0; ssize_t n=getline(&hex,&cap,f); fclose(f);
  while(n>0 && (hex[n-1]=='\n'||hex[n-1]=='\r')) hex[--n]=0;
  size_t plen = strlen(hex)/2;
  unsigned char *proof = malloc(plen);
  for(size_t i=0;i<plen;i++){ unsigned v; sscanf(hex+2*i,"%2x",&v); proof[i]=v; }
  int exp=0, mantissa=0; uint64_t min_v=0, max_v=0;
  int info = secp256k1_rangeproof_info(ctx, &exp, &mantissa, &min_v, &max_v, proof, plen);
  printf("rangeproof_info ok=%d exp=%d mant=%d min=%llu max=%llu plen=%zu\n",
    info, exp, mantissa, (unsigned long long)min_v, (unsigned long long)max_v, plen);

  unsigned char script = 0x6a;
  min_v=0; max_v=0;
  int ver = secp256k1_rangeproof_verify(ctx, &min_v, &max_v, &C1, proof, plen, &script, 1, &gen);
  printf("rangeproof_verify L-BTC+OP_RETURN => %d\n", ver);

  puts("constraint_extractable_on_v1: NO (need DLOG or openings)");
  puts("sign_of_v1: non determinable");
  puts("in_range_v1: non determinable from tally; unproven by P1");
  free(proof); free(hex);
  secp256k1_context_destroy(ctx);
  return 0;
}
