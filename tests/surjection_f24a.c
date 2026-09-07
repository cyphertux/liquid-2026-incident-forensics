#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <secp256k1.h>
#include <secp256k1_generator.h>
#include <secp256k1_surjectionproof.h>
static int hx(const char *hex, unsigned char *out, size_t n){
  for(size_t i=0;i<n;i++){ unsigned v; sscanf(hex+2*i,"%2x",&v); out[i]=v; } return 1;
}
int main(void){
  secp256k1_context *ctx=secp256k1_context_create(SECP256K1_CONTEXT_VERIFY);
  unsigned char in_asset[33], out2_asset[33];
  hx("0abc38a76bc56788e46f7c911f7863aa4926e2718fd3823166fe5f7bd66f1278df", in_asset, 33);
  hx("0b0957be4cbd0d1cc30d2742988f84f931f4f47d4f8b81e40dc1db42cabf4aae7b", out2_asset, 33);
  FILE *f=fopen("data/f24a_out2_surjection.hex","r");
  char buf[512]; if(fscanf(f,"%511s", buf)!=1) return 2; fclose(f);
  size_t slen=strlen(buf)/2; unsigned char *sproof=malloc(slen);
  for(size_t i=0;i<slen;i++){ unsigned v; sscanf(buf+2*i,"%2x",&v); sproof[i]=v; }
  secp256k1_generator tags[1], gen;
  if(!secp256k1_generator_parse(ctx,&tags[0],in_asset)) return 3;
  if(!secp256k1_generator_parse(ctx,&gen,out2_asset)) return 4;
  secp256k1_surjectionproof proof;
  if(!secp256k1_surjectionproof_parse(ctx,&proof,sproof,slen)){ puts("surj parse fail"); return 5; }
  int ok=secp256k1_surjectionproof_verify(ctx,&proof,tags,1,&gen);
  printf("surjection_verify out2 vs [in_asset] => %d (slen=%zu)\n", ok, slen);
  free(sproof); secp256k1_context_destroy(ctx); return ok?0:1;
}
