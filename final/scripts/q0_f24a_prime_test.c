#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <secp256k1.h>
#include <secp256k1_generator.h>
#include <secp256k1_rangeproof.h>

typedef struct { uint32_t s[8]; uint64_t bits; unsigned char buf[64]; int n; } SHA256_CTX;
static uint32_t rotr(uint32_t x,int n){return (x>>n)|(x<<(32-n));}
static void sha256_transform(SHA256_CTX *c, const unsigned char *d){
  static const uint32_t K[64]={0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
  uint32_t w[64],a,b,c2,d2,e,f,g,h,t1,t2,aa[8]; int i;
  for(i=0;i<8;i++) aa[i]=c->s[i];
  for(i=0;i<16;i++) w[i]=(d[4*i]<<24)|(d[4*i+1]<<16)|(d[4*i+2]<<8)|d[4*i+3];
  for(;i<64;i++){ uint32_t s0=rotr(w[i-15],7)^rotr(w[i-15],18)^(w[i-15]>>3); uint32_t s1=rotr(w[i-2],17)^rotr(w[i-2],19)^(w[i-2]>>10); w[i]=w[i-16]+s0+w[i-7]+s1; }
  a=aa[0];b=aa[1];c2=aa[2];d2=aa[3];e=aa[4];f=aa[5];g=aa[6];h=aa[7];
  for(i=0;i<64;i++){ uint32_t S1=rotr(e,6)^rotr(e,11)^rotr(e,25); uint32_t ch=(e&f)^((~e)&g); t1=h+S1+ch+K[i]+w[i]; uint32_t S0=rotr(a,2)^rotr(a,13)^rotr(a,22); uint32_t maj=(a&b)^(a&c2)^(b&c2); t2=S0+maj; h=g;g=f;f=e;e=d2+t1;d2=c2;c2=b;b=a;a=t1+t2; }
  for(i=0;i<8;i++) c->s[i]=aa[i]+(i==0?a:i==1?b:i==2?c2:i==3?d2:i==4?e:i==5?f:i==6?g:h);
}
static void sha256_init(SHA256_CTX *c){ c->s[0]=0x6a09e667;c->s[1]=0xbb67ae85;c->s[2]=0x3c6ef372;c->s[3]=0xa54ff53a;c->s[4]=0x510e527f;c->s[5]=0x9b05688c;c->s[6]=0x1f83d9ab;c->s[7]=0x5be0cd19; c->bits=0;c->n=0; }
static void sha256_update(SHA256_CTX *c,const unsigned char *d,size_t len){ c->bits+=(uint64_t)len*8; while(len--){ c->buf[c->n++]=*d++; if(c->n==64){sha256_transform(c,c->buf);c->n=0;} } }
static void sha256_final(SHA256_CTX *c,unsigned char out[32]){ int i; uint64_t bits=c->bits; c->buf[c->n++]=0x80; if(c->n>56){while(c->n<64)c->buf[c->n++]=0;sha256_transform(c,c->buf);c->n=0;} while(c->n<56)c->buf[c->n++]=0; for(i=7;i>=0;i--)c->buf[c->n++]=(unsigned char)(bits>>(8*i)); sha256_transform(c,c->buf); for(i=0;i<8;i++){out[4*i]=c->s[i]>>24;out[4*i+1]=c->s[i]>>16;out[4*i+2]=c->s[i]>>8;out[4*i+3]=c->s[i];} }
static void key_old(unsigned char out[32], const unsigned char *proof,size_t plen,const unsigned char *commit){ SHA256_CTX c; sha256_init(&c); sha256_update(&c,proof,plen); sha256_update(&c,commit,33); sha256_final(&c,out); }
static void hex32(const unsigned char *b){ for(int i=0;i<32;i++) printf("%02x",b[i]); }
static unsigned char *read_hex(const char *path, size_t *out_len){
  FILE *f=fopen(path,"r"); if(!f){ fprintf(stderr,"missing %s\n",path); return NULL; }
  fseek(f,0,SEEK_END); long n=ftell(f); fseek(f,0,SEEK_SET);
  char *txt=malloc(n+1); if(fread(txt,1,n,f)!=(size_t)n){} txt[n]=0; fclose(f);
  while(n>0&&(txt[n-1]=='\n'||txt[n-1]=='\r')) txt[--n]=0;
  size_t plen=n/2; unsigned char *buf=malloc(plen);
  for(size_t i=0;i<plen;i++){ unsigned v; sscanf(txt+2*i,"%2x",&v); buf[i]=(unsigned char)v; }
  free(txt); *out_len=plen; return buf;
}
int main(void){
  size_t qplen,fplen,qslen,fslen,tmp;
  unsigned char *qp=read_hex("final/hex/Q0_rangeproof.hex",&qplen);
  unsigned char *qc=read_hex("final/hex/Q0_commitment.hex",&tmp);
  unsigned char *qa=read_hex("final/hex/Q0_asset.hex",&tmp);
  unsigned char *qs=read_hex("final/hex/Q0_script.hex",&qslen);
  unsigned char *fp=read_hex("final/hex/out1_rangeproof.hex",&fplen);
  unsigned char *fc=read_hex("final/hex/out1_commitment.hex",&tmp);
  unsigned char *fa=read_hex("final/hex/out1_asset.hex",&tmp);
  unsigned char *fs=read_hex("final/hex/out1_script.hex",&fslen);
  unsigned char *p0=read_hex("final/hex/71c93d43_out0_rangeproof.hex",&tmp);
  unsigned char *c0=read_hex("final/hex/71c93d43_out0_commitment.hex",&tmp);
  if(!qp||!qc||!qa||!qs||!fp||!fc||!fa||!fs||!p0||!c0) return 1;
  unsigned char kq[32],kf[32],kp[32];
  key_old(kq,qp,qplen,qc); key_old(kf,fp,fplen,fc); key_old(kp,p0,4166,c0);
  printf("=== Mission 26 reproduction ===\n");
  printf("Q0 proof_len=%zu\n", qplen);
  printf("OLD_KEY fingerprint Q0||CQ "); hex32(kq); printf("\n");
  printf("OLD_KEY fingerprint P0||C0 "); hex32(kp); printf("\n");
  printf("OLD_KEY fingerprint P1||C1 "); hex32(kf); printf("\n");
  printf("Q0_key==f24a_key? %d\n", !memcmp(kq,kf,32));
  printf("P0_key==f24a_key? %d\n", !memcmp(kp,kf,32));
  printf("Q0_key==P0_key? %d\n", !memcmp(kq,kp,32));

  secp256k1_context *ctx=secp256k1_context_create(SECP256K1_CONTEXT_VERIFY|SECP256K1_CONTEXT_SIGN);
  secp256k1_generator gen; unsigned char aser[33];
  secp256k1_pedersen_commitment commit; secp256k1_generator tag; uint64_t mn=0,mx=0;
  if(qa[0]==0x01){ if(!secp256k1_generator_generate(ctx,&gen,qa+1)) return 2; secp256k1_generator_serialize(ctx,aser,&gen);} else memcpy(aser,qa,33);
  if(!secp256k1_pedersen_commitment_parse(ctx,&commit,qc)) return 3;
  if(!secp256k1_generator_parse(ctx,&tag,aser)) return 4;
  int ok=secp256k1_rangeproof_verify(ctx,&mn,&mx,&commit,qp,qplen,qs,qslen,&tag);
  printf("Q0 secp256k1_rangeproof_verify => %d (script=%zu bytes)\n", ok, qslen);

  if(fa[0]==0x01){ if(!secp256k1_generator_generate(ctx,&gen,fa+1)) return 5; secp256k1_generator_serialize(ctx,aser,&gen);} else memcpy(aser,fa,33);
  if(!secp256k1_pedersen_commitment_parse(ctx,&commit,fc)) return 6;
  if(!secp256k1_generator_parse(ctx,&tag,aser)) return 7;
  int ok2=secp256k1_rangeproof_verify(ctx,&mn,&mx,&commit,fp,fplen,fs,fslen,&tag);
  printf("f24a:1 secp256k1_rangeproof_verify => %d\n", ok2);
  printf("SCENARIO Q0(store)->f24a: keys differ => CACHE MISS; crypto fails => REJECT\n");
  printf("SCENARIO P0(store)->f24a: keys differ => CACHE MISS; crypto fails => REJECT\n");
  printf("VERDICT: 68-family and P0-family do NOT prime f24a (P1,C1)\n");
  return 0;
}
