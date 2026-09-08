/* Mission 37 — instrumented path from VerifyAmounts/CRangeCheck locus
 * up to ConnectBlock cacheStore semantics (not full elementsd).
 *
 * Models:
 *   mempool:  store=true  (AcceptToMemoryPool / CheckInputScripts cacheSigStore=true)
 *   connect:  store=false (ConnectBlock fJustCheck=false → fCacheResults=false;
 *                          still CONSULTS cache via Get(entry, erase=true))
 *
 * Lab only. Uses Mission 28 fixtures.
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
  FILE *f=fopen(path,"r"); if(!f){perror(path); return NULL;}
  fseek(f,0,SEEK_END); long n=ftell(f); fseek(f,0,SEEK_SET);
  char *t=malloc(n+1); fread(t,1,n,f); t[n]=0; fclose(f);
  while(n>0&&(t[n-1]==10||t[n-1]==13||t[n-1]==' ')) t[--n]=0;
  *L=n/2; unsigned char *b=malloc(*L);
  for(size_t i=0;i<*L;i++){unsigned v; sscanf(t+2*i,"%2x",&v); b[i]=v;}
  free(t); return b;
}

typedef struct { unsigned char key[32]; int present; int reclaimable; } slot_t;
typedef struct { slot_t slots[16]; int n; } cache_t;

static void key_unframed(unsigned char out[32], const unsigned char *salt, size_t sl,
  const unsigned char *P,size_t Pl, const unsigned char *C, const unsigned char *A,
  const unsigned char *S, size_t Sl){
  SHA256_CTX c; sha256_init(&c);
  if(sl) sha256_update(&c,salt,sl);
  sha256_update(&c,P,Pl); sha256_update(&c,C,33); sha256_update(&c,A,33);
  if(Sl) sha256_update(&c,S,Sl);
  sha256_final(&c,out);
}
static void key_framed(unsigned char out[32], const unsigned char *salt, size_t sl,
  const unsigned char *P,size_t Pl, const unsigned char *C, const unsigned char *A,
  const unsigned char *S, size_t Sl){
  SHA256_CTX c; sha256_init(&c);
  if(sl) sha256_update(&c,salt,sl);
  unsigned char L[8];
  #define WLEN(X,XL) do{ for(int i=0;i<8;i++) L[i]=(unsigned char)(((uint64_t)(XL)>>(8*i))&0xff); sha256_update(&c,L,8); sha256_update(&c,X,XL);}while(0)
  WLEN(P,Pl); WLEN(C,33); WLEN(A,33); WLEN(S,Sl);
  #undef WLEN
  sha256_final(&c,out);
}

static int cache_get(cache_t *c, const unsigned char k[32], int erase){
  for(int i=0;i<c->n;i++) if(c->slots[i].present && !memcmp(c->slots[i].key,k,32)){
    if(erase) c->slots[i].reclaimable=1;
    return 1;
  }
  return 0;
}
static void cache_set(cache_t *c, const unsigned char k[32]){
  for(int i=0;i<c->n;i++) if(!memcmp(c->slots[i].key,k,32)){ c->slots[i].present=1; c->slots[i].reclaimable=0; return; }
  memcpy(c->slots[c->n].key,k,32); c->slots[c->n].present=1; c->slots[c->n].reclaimable=0; c->n++;
}

static int crypto_verify(secp256k1_context *ctx, const unsigned char *P,size_t Pl,
  const unsigned char *C, const unsigned char *A, const unsigned char *S, size_t Sl){
  secp256k1_pedersen_commitment commit; secp256k1_generator tag; uint64_t mn=0,mx=0;
  if(!secp256k1_pedersen_commitment_parse(ctx,&commit,C)) return 0;
  if(!secp256k1_generator_parse(ctx,&tag,A)) return 0;
  if(!secp256k1_rangeproof_verify(ctx,&mn,&mx,&commit,P,Pl, Sl?S:NULL, Sl, &tag)) return 0;
  int unspendable = (Sl>0 && S[0]==0x6a) || Sl==0;
  if(mn==0 && !unspendable) return 0;
  return 1;
}

/* Mirrors CachingRangeProofChecker::VerifyRangeProof + CRangeCheck */
typedef enum { KEY_UNFRAMED=0, KEY_FRAMED=1 } keymode_t;
typedef struct {
  int hit; int crypto; int ret; int set_done;
} step_t;

static step_t CRangeCheck_sim(secp256k1_context *ctx, cache_t *cache, int store, keymode_t km,
  const unsigned char *salt, size_t sl,
  const unsigned char *P,size_t Pl, const unsigned char *C, const unsigned char *A,
  const unsigned char *S, size_t Sl, const char *label, const char *caller)
{
  step_t st={0,0,0,0};
  unsigned char entry[32];
  if(km==KEY_FRAMED) key_framed(entry,salt,sl,P,Pl,C,A,S,Sl);
  else key_unframed(entry,salt,sl,P,Pl,C,A,S,Sl);

  printf("TRACE %s\n", caller);
  printf("  → CRangeCheck(%s) store=%d\n", label, store);
  printf("  → CachingRangeProofChecker::VerifyRangeProof\n");

  if(cache_get(cache, entry, !store)){
    st.hit=1; st.crypto=-1; st.ret=1;
    printf("     → Cache HIT (native verification SKIPPED)\n");
    printf("     → ACCEPT\n");
    return st;
  }
  printf("     → Cache MISS\n");
  st.hit=0;
  if(Pl==0){ st.crypto=0; st.ret=0; printf("     → REJECT empty proof\n"); return st; }
  int ok=crypto_verify(ctx,P,Pl,C,A,S,Sl);
  st.crypto=ok;
  printf("     → native secp256k1_rangeproof_verify = %s\n", ok?"TRUE":"FALSE");
  if(!ok){ st.ret=0; printf("     → REJECT (crypto)\n"); return st; }
  if(store){ cache_set(cache, entry); st.set_done=1; printf("     → Cache SET\n"); }
  else printf("     → Cache NOT SET (store=false / ConnectBlock semantics)\n");
  st.ret=1; printf("     → ACCEPT\n");
  return st;
}

/* Simulates CheckTxInputs → VerifyAmounts → queued/sync CRangeCheck for ONE output RP */
static step_t CheckTxInputs_amount_locus(secp256k1_context *ctx, cache_t *cache, int cacheStore,
  keymode_t km, const unsigned char *salt, size_t sl,
  const unsigned char *P,size_t Pl, const unsigned char *C, const unsigned char *A,
  const unsigned char *S, size_t Sl, const char *label, const char *phase)
{
  char caller[256];
  snprintf(caller,sizeof(caller),
    "%s\n  → Consensus::CheckTxInputs(cacheStore=%d)\n  → VerifyAmounts(..., store_result=%d)",
    phase, cacheStore, cacheStore);
  return CRangeCheck_sim(ctx,cache,cacheStore,km,salt,sl,P,Pl,C,A,S,Sl,label,caller);
}

static void reset(cache_t *c){ memset(c,0,sizeof(*c)); }

int main(void){
  size_t L;
  unsigned char *P0=read_hex("final/hex/71c93d43_out0_rangeproof.hex",&L); size_t P0L=L;
  unsigned char *C0=read_hex("final/hex/71c93d43_out0_commitment.hex",&L);
  unsigned char *S0=read_hex("final/hex/71c93d43_out0_script.hex",&L); size_t S0L=L;
  unsigned char *P1=read_hex("final/hex/out1_rangeproof.hex",&L); size_t P1L=L;
  unsigned char *C1=read_hex("final/hex/out1_commitment.hex",&L);
  unsigned char *S1=read_hex("final/hex/out1_script.hex",&L); size_t S1L=L;
  unsigned char *Awire=read_hex("final/hex/out1_asset.hex",&L);
  if(!P0||!C0||!S0||!P1||!C1||!S1||!Awire){ fprintf(stderr,"fixture load fail\n"); return 1; }

  secp256k1_context *ctx=secp256k1_context_create(SECP256K1_CONTEXT_VERIFY|SECP256K1_CONTEXT_SIGN);
  unsigned char G[33]; secp256k1_generator gen;
  secp256k1_generator_generate(ctx,&gen,Awire+1);
  secp256k1_generator_serialize(ctx,G,&gen);

  unsigned char salt[32]; memset(salt,0x5A,32);
  cache_t cache;
  step_t st;

  printf("========== MISSION 37 VALIDATION-PATH HARNESS ==========\n");
  printf("Fixtures: primer=71c9:0  alias=f24a:1  G=ser(explicit L-BTC)\n\n");

  /* A: primer alone mempool */
  printf("########## SCENARIO A: primer mempool (store=true) ##########\n");
  reset(&cache);
  st=CheckTxInputs_amount_locus(ctx,&cache,1,KEY_UNFRAMED,salt,32,P0,P0L,C0,G,S0,S0L,"primer","AcceptToMemoryPool / mempool validation");
  printf("RESULT A: ret=%d hit=%d crypto=%d set=%d\n\n", st.ret,st.hit,st.crypto,st.set_done);

  /* B: alias cold */
  printf("########## SCENARIO B: alias alone cold ConnectBlock store=false ##########\n");
  reset(&cache);
  st=CheckTxInputs_amount_locus(ctx,&cache,0,KEY_UNFRAMED,salt,32,P1,P1L,C1,G,S1,S1L,"alias","ConnectBlock(fJustCheck=false) cold");
  printf("RESULT B: ret=%d hit=%d crypto=%d => %s\n\n", st.ret,st.hit,st.crypto, st.ret?"ACCEPT":"REJECT");

  /* C: primer mempool then alias connect */
  printf("########## SCENARIO C: primer mempool → alias ConnectBlock ##########\n");
  reset(&cache);
  st=CheckTxInputs_amount_locus(ctx,&cache,1,KEY_UNFRAMED,salt,32,P0,P0L,C0,G,S0,S0L,"primer","AcceptToMemoryPool");
  printf("--- then same process ConnectBlock alias ---\n");
  st=CheckTxInputs_amount_locus(ctx,&cache,0,KEY_UNFRAMED,salt,32,P1,P1L,C1,G,S1,S1L,"alias","ConnectBlock(fJustCheck=false) primed");
  printf("RESULT C: ret=%d hit=%d crypto=%d => %s (bypassed crypto=%s)\n\n",
    st.ret,st.hit,st.crypto, st.ret?"ACCEPT":"REJECT", st.crypto==-1?"YES":"NO");

  /* D: same as C labeled as transaction validation locus */
  printf("########## SCENARIO D: same locus via CheckTxInputs (transaction validation) ##########\n");
  reset(&cache);
  CheckTxInputs_amount_locus(ctx,&cache,1,KEY_UNFRAMED,salt,32,P0,P0L,C0,G,S0,S0L,"primer","CheckTxInputs mempool");
  st=CheckTxInputs_amount_locus(ctx,&cache,1,KEY_UNFRAMED,salt,32,P1,P1L,C1,G,S1,S1L,"alias","CheckTxInputs mempool warm");
  printf("RESULT D (alias in mempool after primer): ret=%d hit=%d crypto=%d\n\n", st.ret,st.hit,st.crypto);

  /* Orderings */
  printf("########## ORDERING: alias → primer (mempool) ##########\n");
  reset(&cache);
  st=CheckTxInputs_amount_locus(ctx,&cache,1,KEY_UNFRAMED,salt,32,P1,P1L,C1,G,S1,S1L,"alias","mempool first");
  printf("alias first RESULT: ret=%d crypto=%d\n", st.ret,st.crypto);
  st=CheckTxInputs_amount_locus(ctx,&cache,1,KEY_UNFRAMED,salt,32,P0,P0L,C0,G,S0,S0L,"primer","mempool second");
  printf("primer second RESULT: ret=%d hit=%d crypto=%d\n\n", st.ret,st.hit,st.crypto);

  printf("########## ORDERING: same-tx both checks sequential (primer then alias) ##########\n");
  reset(&cache);
  CheckTxInputs_amount_locus(ctx,&cache,1,KEY_UNFRAMED,salt,32,P0,P0L,C0,G,S0,S0L,"primer-out","same tx VerifyAmounts");
  st=CheckTxInputs_amount_locus(ctx,&cache,1,KEY_UNFRAMED,salt,32,P1,P1L,C1,G,S1,S1L,"alias-out","same tx VerifyAmounts");
  printf("same-tx alias RESULT: ret=%d hit=%d crypto=%d\n\n", st.ret,st.hit,st.crypto);

  /* Two nodes */
  printf("########## TWO NODES: same alias ConnectBlock ##########\n");
  cache_t nodeA, nodeB; reset(&nodeA); reset(&nodeB);
  printf("--- Node A primes primer in mempool ---\n");
  CheckTxInputs_amount_locus(ctx,&nodeA,1,KEY_UNFRAMED,salt,32,P0,P0L,C0,G,S0,S0L,"primer","NodeA mempool");
  printf("--- Node A ConnectBlock alias ---\n");
  step_t a=CheckTxInputs_amount_locus(ctx,&nodeA,0,KEY_UNFRAMED,salt,32,P1,P1L,C1,G,S1,S1L,"alias","NodeA ConnectBlock");
  printf("--- Node B cold ConnectBlock alias ---\n");
  step_t b=CheckTxInputs_amount_locus(ctx,&nodeB,0,KEY_UNFRAMED,salt,32,P1,P1L,C1,G,S1,S1L,"alias","NodeB ConnectBlock");
  printf("NODE_A_CONNECT_ALIAS: %s (hit=%d crypto=%d)\n", a.ret?"ACCEPT":"REJECT", a.hit, a.crypto);
  printf("NODE_B_CONNECT_ALIAS: %s (hit=%d crypto=%d)\n", b.ret?"ACCEPT":"REJECT", b.hit, b.crypto);
  printf("DIVERGENCE_ON_RANGEPROOF_LOCUS: %s\n\n", (a.ret!=b.ret)?"YES":"NO");

  /* Framed control */
  printf("########## FRAMED CONTROL: primer then alias ##########\n");
  reset(&cache);
  CheckTxInputs_amount_locus(ctx,&cache,1,KEY_FRAMED,salt,32,P0,P0L,C0,G,S0,S0L,"primer","mempool framed");
  st=CheckTxInputs_amount_locus(ctx,&cache,0,KEY_FRAMED,salt,32,P1,P1L,C1,G,S1,S1L,"alias","ConnectBlock framed");
  printf("FRAMED RESULT: ret=%d hit=%d crypto=%d => %s\n\n", st.ret,st.hit,st.crypto, st.ret?"ACCEPT":"REJECT");

  printf("========== END HARNESS ==========\n");
  printf("NOTE: This simulates CheckTxInputs→VerifyAmounts→CRangeCheck→CachingRangeProofChecker.\n");
  printf("It does NOT run full elementsd ConnectBlock/ActivateBestChain (UTXO, scripts, surjection, etc.).\n");

  secp256k1_context_destroy(ctx);
  return 0;
}
