/* Reconstruct Elements VerifyAmounts Pedersen tally for f24a (no rangeproofs). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <secp256k1.h>
#include <secp256k1_generator.h>
#include <secp256k1_rangeproof.h>

static int hx(const char *hex, unsigned char *out, size_t n){
  if(strlen(hex)!=2*n) return 0;
  for(size_t i=0;i<n;i++){ unsigned v; if(sscanf(hex+2*i,"%2x",&v)!=1) return 0; out[i]=(unsigned char)v; }
  return 1;
}

static void print_pt(const char *lab, const unsigned char *c33){
  printf("%s ", lab); for(int i=0;i<33;i++) printf("%02x", c33[i]); printf("\n");
}

int main(void){
  secp256k1_context *ctx=secp256k1_context_create(SECP256K1_CONTEXT_VERIFY|SECP256K1_CONTEXT_SIGN);

  /* Input prevout value commitment (blinded) */
  unsigned char in0[33];
  hx("09121a4e92717097b9562e2cf3cdbee843a55f819074f6c0e11a29c46874fcde3d", in0, 33);
  unsigned char in_asset[33];
  hx("0abc38a76bc56788e46f7c911f7863aa4926e2718fd3823166fe5f7bd66f1278df", in_asset, 33);

  /* Outputs */
  unsigned char out0[33], out1[33], out2[33];
  hx("08360f95ce0a63e76daab6a05616103ba7462a9af7db0697e9c8fa6a65aba103f8", out0, 33);
  hx("086f5d67160fc4b477954fb09ef321e5b589d7a07740a1a6df494ed2335b1d01d8", out1, 33);
  hx("08dc50cfb1c9b0b512b446cd80d8bbe81f5ace540f59780d6fad773fa9204d8d35", out2, 33);

  /* Explicit L-BTC asset id (wire order after 0x01) for fee */
  unsigned char lbtc_asset32[32];
  hx("6d521c38ec1ea15734ae22b7c46064412829c0d0579f0a713d1c04ede979026f", lbtc_asset32, 32);
  secp256k1_generator lbtc_gen;
  if(!secp256k1_generator_generate(ctx, &lbtc_gen, lbtc_asset32)){ puts("gen fail"); return 2; }
  unsigned char lbtc_gen_ser[33];
  secp256k1_generator_serialize(ctx, lbtc_gen_ser, &lbtc_gen);
  print_pt("L-BTC generator", lbtc_gen_ser);

  /* Fee = explicit 58 with zero blind under L-BTC generator */
  unsigned char zero_blind[32]={0};
  secp256k1_pedersen_commitment fee_commit;
  if(!secp256k1_pedersen_commit(ctx, &fee_commit, zero_blind, 58, &lbtc_gen)){ puts("fee commit fail"); return 2; }
  unsigned char fee_ser[33];
  secp256k1_pedersen_commitment_serialize(ctx, fee_ser, &fee_commit);
  print_pt("fee commitment (explicit 58)", fee_ser);

  secp256k1_pedersen_commitment cin0, cout0, cout1, cout2, cfee;
  if(!secp256k1_pedersen_commitment_parse(ctx,&cin0,in0)) {puts("in0 parse fail"); return 2;}
  if(!secp256k1_pedersen_commitment_parse(ctx,&cout0,out0)) {puts("out0 parse fail"); return 2;}
  if(!secp256k1_pedersen_commitment_parse(ctx,&cout1,out1)) {puts("out1 parse fail"); return 2;}
  if(!secp256k1_pedersen_commitment_parse(ctx,&cout2,out2)) {puts("out2 parse fail"); return 2;}
  cfee = fee_commit;

  /* Full tally as Elements: 1 in, 4 outs (0,1,2,fee) */
  {
    const secp256k1_pedersen_commitment *ins[1] = { &cin0 };
    const secp256k1_pedersen_commitment *outs[4] = { &cout0, &cout1, &cout2, &cfee };
    int ok = secp256k1_pedersen_verify_tally(ctx, ins, 1, outs, 4);
    printf("\nTALLY full (in0 vs out0+out1+out2+fee58) => %d\n", ok);
  }

  /* Without out1 */
  {
    const secp256k1_pedersen_commitment *ins[1] = { &cin0 };
    const secp256k1_pedersen_commitment *outs[3] = { &cout0, &cout2, &cfee };
    int ok = secp256k1_pedersen_verify_tally(ctx, ins, 1, outs, 3);
    printf("TALLY without out1 (in0 vs out0+out2+fee58) => %d\n", ok);
  }

  /* Only out1 vs nothing meaningful: in0 vs out1 alone */
  {
    const secp256k1_pedersen_commitment *ins[1] = { &cin0 };
    const secp256k1_pedersen_commitment *outs[1] = { &cout1 };
    int ok = secp256k1_pedersen_verify_tally(ctx, ins, 1, outs, 1);
    printf("TALLY in0 vs out1 alone => %d\n", ok);
  }

  /* out0+out2+fee vs in0+out1  (rearrange: if full balances, then in0 - out1 = out0+out2+fee) */
  {
    const secp256k1_pedersen_commitment *ins[2] = { &cin0, &cout1 }; /* treat out1 as negative by putting on opposite side... */
    /* pedersen_verify_tally: sum(ins) == sum(outs). So in0 == out0+out1+out2+fee means in0+(-out1) == out0+out2+fee.
       Library has no negation helper easily; instead check in0+X == outs without out1 is already done.
       If full OK and without-out1 FAIL, out1 is necessary for balance. */
  }

  /* Surjection-relevant generators */
  print_pt("input asset gen (blinded)", in_asset);
  print_pt("out0 value commit", out0);
  print_pt("out1 value commit C1", out1);
  print_pt("out2 value commit", out2);
  print_pt("out2 asset commit", (unsigned char*)"");
  unsigned char out2_asset[33];
  hx("0b0957be4cbd0d1cc30d2742988f84f931f4f47d4f8b81e40dc1db42cabf4aae7b", out2_asset, 33);
  print_pt("out2 asset gen", out2_asset);

  /* Parse generators validity */
  secp256k1_generator g_in, g_out2, g_lbtc;
  printf("\ngenerator_parse input asset => %d\n", secp256k1_generator_parse(ctx,&g_in,in_asset));
  printf("generator_parse out2 asset => %d\n", secp256k1_generator_parse(ctx,&g_out2,out2_asset));
  g_lbtc = lbtc_gen;
  printf("L-BTC gen already generated => 1\n");

  /* Compare: is out2 asset equal to input asset? */
  printf("out2_asset == in_asset? %d\n", memcmp(out2_asset, in_asset, 33)==0);
  printf("out2_asset == L-BTC gen? %d\n", memcmp(out2_asset, lbtc_gen_ser, 33)==0);

  secp256k1_context_destroy(ctx);
  return 0;
}
