/* Scenario harness: 71c9 / 2711 priming vs f24a:1 under OLD Elements cache key.
 * OLD key = SHA256(proof || value_commitment)  [salt omitted; equality preserved]
 * Does NOT mutate any remote system.
 */
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

typedef struct { unsigned char keys[32][32]; int n; } cache_t;
static void key_old(unsigned char out[32], const unsigned char *proof,size_t plen,const unsigned char *commit){
  SHA256_CTX c; sha256_init(&c); sha256_update(&c,proof,plen); sha256_update(&c,commit,33); sha256_final(&c,out);
}
static void hex32(const unsigned char *b){ for(int i=0;i<32;i++) printf("%02x", b[i]); }
static int cache_has(cache_t *c, const unsigned char k[32]){ for(int i=0;i<c->n;i++) if(!memcmp(c->keys[i],k,32)) return 1; return 0; }
static void cache_add(cache_t *c, const unsigned char k[32]){ if(c->n<32 && !cache_has(c,k)) memcpy(c->keys[c->n++],k,32); }

static unsigned char *read_hex_file(const char *path, size_t *out_len){
  FILE *f=fopen(path,"r"); if(!f){ fprintf(stderr,"missing %s\n", path); return NULL; }
  fseek(f,0,SEEK_END); long n=ftell(f); fseek(f,0,SEEK_SET);
  char *txt=malloc(n+1); fread(txt,1,n,f); txt[n]=0; fclose(f);
  while(n>0 && (txt[n-1]=='\n'||txt[n-1]=='\r'||txt[n-1]==' ')) txt[--n]=0;
  size_t plen=n/2; unsigned char *buf=malloc(plen);
  for(size_t i=0;i<plen;i++){ unsigned v; if(sscanf(txt+2*i,"%2x",&v)!=1){ free(txt); free(buf); return NULL; } buf[i]=(unsigned char)v; }
  free(txt); *out_len=plen; return buf;
}

static void asset_to_generator_ser(secp256k1_context *ctx, const unsigned char *asset_raw, unsigned char out33[33]){
  if(asset_raw[0]==0x01){ secp256k1_generator gen; secp256k1_generator_generate(ctx,&gen,asset_raw+1); secp256k1_generator_serialize(ctx,out33,&gen); }
  else memcpy(out33, asset_raw, 33);
}

typedef struct {
  char name[32];
  unsigned char *proof; size_t plen;
  unsigned char commit[33];
  unsigned char asset_ser[33];
  unsigned char *script; size_t slen;
  unsigned char key[32];
} out_t;

static int load_out(secp256k1_context *ctx, out_t *o, const char *name,
  const char *proof_p, const char *commit_p, const char *asset_p, const char *script_p)
{
  size_t al=0,cl=0;
  unsigned char *asset_raw;
  memset(o,0,sizeof *o);
  snprintf(o->name,sizeof o->name,"%s", name);
  o->proof=read_hex_file(proof_p,&o->plen); if(!o->proof) return 0;
  unsigned char *c=read_hex_file(commit_p,&cl); if(!c||cl!=33) return 0; memcpy(o->commit,c,33); free(c);
  asset_raw=read_hex_file(asset_p,&al); if(!asset_raw||al!=33) return 0;
  asset_to_generator_ser(ctx,asset_raw,o->asset_ser); free(asset_raw);
  o->script=read_hex_file(script_p,&o->slen); if(!o->script) return 0;
  key_old(o->key, o->proof, o->plen, o->commit);
  return 1;
}

static int verify_one(secp256k1_context *ctx, cache_t *cache, out_t *o, int store, const char *label){
  int hit=0, crypto=-1, ret;
  printf("  [%s] key=", label); hex32(o->key); printf("\n");
  if(cache_has(cache, o->key)){
    hit=1; ret=1; crypto=-1;
    printf("  [%s] CACHE HIT -> ACCEPT (crypto skipped) store=%d\n", label, store);
  } else {
    secp256k1_pedersen_commitment commit;
    secp256k1_generator tag;
    uint64_t min_value=0, max_value=0;
    if(!secp256k1_pedersen_commitment_parse(ctx,&commit,o->commit)) { ret=0; crypto=0; }
    else if(!secp256k1_generator_parse(ctx,&tag,o->asset_ser)) { ret=0; crypto=0; }
    else {
      crypto = secp256k1_rangeproof_verify(ctx,&min_value,&max_value,&commit,o->proof,o->plen,
        o->slen?o->script:NULL, o->slen, &tag);
      int unspendable = (o->slen==1 && o->script[0]==0x6a) || (o->slen>=1 && o->script[0]==0x6a);
      if(!crypto) ret=0;
      else if(min_value==0 && !unspendable) ret=0;
      else {
        ret=1;
        if(store) cache_add(cache, o->key);
      }
    }
    printf("  [%s] miss crypto=%d ret=%d cache_n=%d store=%d\n", label, crypto, ret, cache->n, store);
  }
  return ret;
}

static void free_out(out_t *o){ free(o->proof); free(o->script); }

static void scenario(const char *title, secp256k1_context *ctx,
  out_t **seq, int nseq, out_t *f24a)
{
  cache_t cache; memset(&cache,0,sizeof cache);
  printf("\n======== %s ========\n", title);
  printf("cache empty n=0\n");
  for(int i=0;i<nseq;i++){
    int r=verify_one(ctx,&cache,seq[i],1,seq[i]->name);
    printf("  => %s %s\n", seq[i]->name, r?"ACCEPT/STORE":"REJECT");
  }
  int r=verify_one(ctx,&cache,f24a,1,"f24a:1");
  printf("RESULT f24a:1 %s | cache entries=%d\n", r?"ACCEPT":"REJECT", cache.n);
  for(int i=0;i<cache.n;i++){ printf("  cached[%d]=", i); hex32(cache.keys[i]); printf("\n"); }
}

int main(void){
  secp256k1_context *ctx=secp256k1_context_create(SECP256K1_CONTEXT_VERIFY|SECP256K1_CONTEXT_SIGN);
  out_t o71_0,o71_1,o27_0,o27_1,f24;
  if(!load_out(ctx,&o71_0,"71c9:0","final/hex/71c93d43_out0_rangeproof.hex","final/hex/71c93d43_out0_commitment.hex","final/hex/71c93d43_out0_asset.hex","final/hex/71c93d43_out0_script.hex")) return 1;
  if(!load_out(ctx,&o71_1,"71c9:1","final/hex/71c93d43_out1_rangeproof.hex","final/hex/71c93d43_out1_commitment.hex","final/hex/71c93d43_out1_asset.hex","final/hex/71c93d43_out1_script.hex")) return 1;
  if(!load_out(ctx,&o27_0,"2711:0","final/hex/27114710_out0_rangeproof.hex","final/hex/27114710_out0_commitment.hex","final/hex/27114710_out0_asset.hex","final/hex/27114710_out0_script.hex")) return 1;
  if(!load_out(ctx,&o27_1,"2711:1","final/hex/27114710_out1_rangeproof.hex","final/hex/27114710_out1_commitment.hex","final/hex/27114710_out1_asset.hex","final/hex/27114710_out1_script.hex")) return 1;
  if(!load_out(ctx,&f24,"f24a:1","final/hex/out1_rangeproof.hex","final/hex/out1_commitment.hex","final/hex/out1_asset.hex","final/hex/out1_script.hex")) return 1;

  printf("KEY COMPARISON (OLD = SHA256(proof||commit), salt omitted)\n");
  printf("f24a:1 "); hex32(f24.key); printf("\n");
  printf("71c9:0 "); hex32(o71_0.key); printf(" equal_f24a=%d\n", !memcmp(o71_0.key,f24.key,32));
  printf("71c9:1 "); hex32(o71_1.key); printf(" equal_f24a=%d\n", !memcmp(o71_1.key,f24.key,32));
  printf("2711:0 "); hex32(o27_0.key); printf(" equal_f24a=%d equal_71c9_0=%d\n", !memcmp(o27_0.key,f24.key,32), !memcmp(o27_0.key,o71_0.key,32));
  printf("2711:1 "); hex32(o27_1.key); printf(" equal_f24a=%d\n", !memcmp(o27_1.key,f24.key,32));

  out_t *A[]={&o71_0,&o71_1,&o27_0,&o27_1};
  out_t *C[]={&o71_0,&o71_1};
  out_t *D[]={&o27_0,&o27_1};
  out_t *E[]={&o71_0,&o71_1,&o27_0,&o27_1};
  out_t *F[]={&o27_0,&o27_1,&o71_0,&o71_1};

  scenario("A: 71c9 then 2711 then f24a", ctx, A, 4, &f24);
  scenario("B: empty then f24a", ctx, NULL, 0, &f24);
  scenario("C: 71c9 only then f24a", ctx, C, 2, &f24);
  scenario("D: 2711 only then f24a", ctx, D, 2, &f24);
  scenario("E: 71c9+2711 then f24a (same as A)", ctx, E, 4, &f24);
  scenario("F: reverse 2711 then 71c9 then f24a", ctx, F, 4, &f24);

  free_out(&o71_0); free_out(&o71_1); free_out(&o27_0); free_out(&o27_1); free_out(&f24);
  secp256k1_context_destroy(ctx);
  return 0;
}
