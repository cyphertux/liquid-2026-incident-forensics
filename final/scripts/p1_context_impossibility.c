/* Test whether any simple/related context makes P1+C1 verify; and whether trailing junk dooms P1. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <secp256k1.h>
#include <secp256k1_generator.h>
#include <secp256k1_rangeproof.h>

static unsigned char *read_hex(const char *path, size_t *out_len){
  FILE *f=fopen(path,"r"); if(!f) return NULL;
  fseek(f,0,SEEK_END); long n=ftell(f); fseek(f,0,SEEK_SET);
  char *txt=malloc(n+1); fread(txt,1,n,f); txt[n]=0; fclose(f);
  while(n>0&&(txt[n-1]==10||txt[n-1]==13)) txt[--n]=0;
  size_t L=n/2; unsigned char *b=malloc(L);
  for(size_t i=0;i<L;i++){ unsigned v; sscanf(txt+2*i,"%2x",&v); b[i]=v; }
  free(txt); *out_len=L; return b;
}
static void asset_ser(secp256k1_context *ctx, const unsigned char *asset, unsigned char out[33]){
  if(asset[0]==0x01){ secp256k1_generator g; secp256k1_generator_generate(ctx,&g,asset+1); secp256k1_generator_serialize(ctx,out,&g); }
  else memcpy(out,asset,33);
}
static int ver(secp256k1_context *ctx, const unsigned char *proof, size_t plen, const unsigned char *commit,
               const unsigned char *aser, const unsigned char *script, size_t slen, const char *label){
  secp256k1_pedersen_commitment c; secp256k1_generator t; uint64_t mn=0,mx=0;
  if(!secp256k1_pedersen_commitment_parse(ctx,&c,commit)){ printf("%s parse_commit fail\n",label); return 0; }
  if(!secp256k1_generator_parse(ctx,&t,aser)){ printf("%s parse_gen fail\n",label); return 0; }
  int ok=secp256k1_rangeproof_verify(ctx,&mn,&mx,&c,proof,plen, slen?script:NULL, slen, &t);
  printf("%s => %d (plen=%zu slen=%zu)\n", label, ok, plen, slen);
  return ok;
}
int main(void){
  size_t p1l,p0l,c1l,c0l,al,sl,s0l,ql;
  unsigned char *P1=read_hex("final/hex/out1_rangeproof.hex",&p1l);
  unsigned char *P0=read_hex("final/hex/71c93d43_out0_rangeproof.hex",&p0l);
  unsigned char *C1=read_hex("final/hex/out1_commitment.hex",&c1l);
  unsigned char *C0=read_hex("final/hex/71c93d43_out0_commitment.hex",&c0l);
  unsigned char *A=read_hex("final/hex/out1_asset.hex",&al);
  unsigned char *S=read_hex("final/hex/out1_script.hex",&sl); /* 6a */
  unsigned char *S0=read_hex("final/hex/71c93d43_out0_script.hex",&s0l);
  unsigned char *Q0=read_hex("final/hex/Q0_rangeproof.hex",&ql);
  unsigned char *CQ=read_hex("final/hex/Q0_commitment.hex",&c0l);
  unsigned char *SQ=read_hex("final/hex/Q0_script.hex",&sl);
  secp256k1_context *ctx=secp256k1_context_create(SECP256K1_CONTEXT_VERIFY|SECP256K1_CONTEXT_SIGN);
  unsigned char aser[33]; asset_ser(ctx,A,aser);
  unsigned char empty=0;
  unsigned char s51=0x51;
  printf("=== P1/C1 contexts ===\n");
  ver(ctx,P1,p1l,C1,aser,S,1,"P1+C1+L+OP_RETURN");
  ver(ctx,P1,p1l,C1,aser,S0,s0l,"P1+C1+L+71c9script");
  ver(ctx,P1,p1l,C1,aser,&empty,0,"P1+C1+L+empty");
  ver(ctx,P1,p1l,C1,aser,&s51,1,"P1+C1+L+OP_TRUE");
  unsigned char sq[]={0x6a,0x01,0x00}; ver(ctx,P1,p1l,C1,aser,sq,3,"P1+C1+L+6a0100");
  printf("=== truncated / related ===\n");
  ver(ctx,P1,4166,C1,aser,S,1,"P1[:4166]+C1+L+6a");
  ver(ctx,P0,p0l,C1,aser,S,1,"P0+C1+L+6a");
  ver(ctx,P0,p0l,C1,aser,S0,s0l,"P0+C1+L+71c9script");
  ver(ctx,P0,p0l,C0,aser,S0,s0l,"P0+C0+L+71c9script (known good)");
  ver(ctx,P1,p1l,C0,aser,S0,s0l,"P1+C0+L+71c9script");
  ver(ctx,P1,4166,C0,aser,S0,s0l,"P1[:4166]=P0 +C0 +71c9script");
  /* GEN as asset? */
  unsigned char *GEN=S0+2+33; /* skip 6a43 C1 */
  ver(ctx,P1,p1l,C1,GEN,S,1,"P1+C1+GEN_as_asset+6a");
  ver(ctx,P0,p0l,C0,GEN,S0,s0l,"P0+C0+GEN_as_asset+71c9script");
  printf("=== Q0 sanity ===\n");
  unsigned char aserQ[33]; asset_ser(ctx,A,aserQ);
  unsigned char *A2=read_hex("final/hex/Q0_asset.hex",&al);
  asset_ser(ctx,A2,aserQ);
  unsigned char *SQb=read_hex("final/hex/Q0_script.hex",&sl);
  ver(ctx,Q0,ql,CQ,aserQ,SQb,sl,"Q0+CQ+real");
  return 0;
}
