#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <secp256k1.h>
#include <secp256k1_generator.h>
#include <secp256k1_rangeproof.h>

static int hn(char c){ if(c>='0'&&c<='9')return c-'0'; if(c>='a'&&c<='f')return c-'a'+10; if(c>='A'&&c<='F')return c-'A'+10; return -1; }
static unsigned char *hx(const char *hex, size_t *n){ size_t L=strlen(hex); if(L%2)return NULL; *n=L/2; unsigned char *b=malloc(*n); for(size_t i=0;i<*n;i++){int a=hn(hex[2*i]),c=hn(hex[2*i+1]); if(a<0||c<0){free(b);return NULL;} b[i]=(a<<4)|c;} return b; }
static char *rf(const char *p){ FILE *f=fopen(p,"rb"); if(!f)return NULL; fseek(f,0,SEEK_END); long n=ftell(f); fseek(f,0,SEEK_SET); char *b=malloc(n+1); if(fread(b,1,n,f)!=(size_t)n){free(b);fclose(f);return NULL;} fclose(f); while(n>0&&(b[n-1]=='\n'||b[n-1]=='\r')) b[--n]=0; b[n]=0; return b; }

static int try_verify(secp256k1_context *ctx, const unsigned char *proof, size_t plen,
                      const unsigned char *commit, const unsigned char *agen, size_t alen,
                      const unsigned char *extra, size_t elen, const char *label) {
  secp256k1_pedersen_commitment c; secp256k1_generator g; unsigned char gser[33];
  uint64_t minv=0,maxv=0;
  if(!secp256k1_pedersen_commitment_parse(ctx,&c,commit)) { printf("%s: commit parse fail\n", label); return 0; }
  if(alen==33 && agen[0]==0x01) {
    if(!secp256k1_generator_generate(ctx,&g,agen+1)) { printf("%s: gen fail\n", label); return 0; }
    secp256k1_generator_serialize(ctx,gser,&g);
  } else if(alen==33) {
    if(!secp256k1_generator_parse(ctx,&g,agen)) { printf("%s: gen parse fail\n", label); return 0; }
    memcpy(gser,agen,33);
  } else if(alen==32) {
    if(!secp256k1_generator_generate(ctx,&g,agen)) { printf("%s: gen32 fail\n", label); return 0; }
    secp256k1_generator_serialize(ctx,gser,&g);
  } else { printf("%s: bad asset len %zu\n", label, alen); return 0; }
  int ok = secp256k1_rangeproof_verify(ctx,&minv,&maxv,&c,proof,plen,elen?extra:NULL,elen,&g);
  printf("%s => %d min=%llu max=%llu gen=", label, ok, (unsigned long long)minv, (unsigned long long)maxv);
  for(int i=0;i<33;i++) printf("%02x", gser[i]);
  printf("\n");
  return ok;
}

int main(void){
  char *ph=rf("data/out1_rangeproof.hex"); char *ch=rf("data/out1_commitment.hex");
  size_t pl,cl; unsigned char *proof=hx(ph,&pl); unsigned char *commit=hx(ch,&cl);
  secp256k1_context *ctx=secp256k1_context_create(SECP256K1_CONTEXT_VERIFY|SECP256K1_CONTEXT_SIGN);

  const char *assets[] = {
    "016d521c38ec1ea15734ae22b7c46064412829c0d0579f0a713d1c04ede979026f", /* L-BTC explicit */
    "0a0a488de4899d0ae757f6cf8368663184d164106111ed9eaecf510e35282ddc6d", /* L-BTC gen ser */
    "0a6ec073eb6ebf4986c0d8a93cc6d4b206a43ffd53feb1d2eaa0b44dec207de64f", /* 71c93 out1 */
    "0bd8fb44e44f7a6c2e88bd89671bb3f9b3086e8fa1b691bac5198ddfae759cfa77", /* 271147 out1 */
    "0b0957be4cbd0d1cc30d2742988f84f931f4f47d4f8b81e40dc1db42cabf4aae7b", /* f24a out2 */
    "0abc38a76bc56788e46f7c911f7863aa4926e2718fd3823166fe5f7bd66f1278df",
    NULL
  };
  const char *scripts[] = {
    "6a",
    "",
    "0014f5906a8572cf826a9d737ce1bc6a19a066868630",
    "6a43086f5d67160fc4b477954fb09ef321e5b589d7a07740a1a6df494ed2335b1d01d80a0a488de4899d0ae757f6cf8368663184d164106111ed9eaecf510e35282ddc6d6a",
    /* script without last 6a */
    "6a43086f5d67160fc4b477954fb09ef321e5b589d7a07740a1a6df494ed2335b1d01d80a0a488de4899d0ae757f6cf8368663184d164106111ed9eaecf510e35282ddc6d",
    /* just push data without OP_RETURN */
    "43086f5d67160fc4b477954fb09ef321e5b589d7a07740a1a6df494ed2335b1d01d80a0a488de4899d0ae757f6cf8368663184d164106111ed9eaecf510e35282ddc6d6a",
    NULL
  };

  int hits=0;
  for(int ai=0; assets[ai]; ai++){
    size_t al; unsigned char *a=hx(assets[ai],&al);
    for(int si=0; scripts[si]; si++){
      size_t sl=0; unsigned char *s=NULL;
      if(scripts[si][0]) s=hx(scripts[si],&sl); else { s=NULL; sl=0; }
      char label[64]; snprintf(label,sizeof(label),"A%d/S%d", ai, si);
      hits += try_verify(ctx, proof, pl, commit, a, al, s, sl, label);
      free(s);
    }
    free(a);
  }
  printf("TOTAL_HITS=%d\n", hits);

  /* Also try verifying priming proofs against attack commitment (wrong) */
  printf("\n--- cross: priming proof with attack commitment ---\n");
  char *pph=rf("data/71c93d43_out0_rangeproof.hex"); size_t ppl; unsigned char *pp=hx(pph,&ppl);
  size_t al; unsigned char *a=hx(assets[0],&al);
  size_t sl; unsigned char *s=hx(scripts[3],&sl);
  try_verify(ctx, pp, ppl, commit, a, al, s, sl, "priming_proof+attack_C+priming_script");
  try_verify(ctx, pp, ppl, commit, a, al, (unsigned char*)"\x6a", 1, "priming_proof+attack_C+6a");

  /* priming commitment with attack proof */
  char *pch=rf("data/71c93d43_out0_commitment.hex"); size_t pcl; unsigned char *pc=hx(pch,&pcl);
  try_verify(ctx, proof, pl, pc, a, al, s, sl, "attack_proof+priming_C+priming_script");

  return hits?0:1;
}
