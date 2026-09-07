#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <secp256k1.h>
#include <secp256k1_generator.h>
#include <secp256k1_rangeproof.h>

static int hn(char c){ if(c>='0'&&c<='9')return c-'0'; if(c>='a'&&c<='f')return c-'a'+10; if(c>='A'&&c<='F')return c-'A'+10; return -1; }
static unsigned char *hx(const char *hex, size_t *n){
  size_t L=strlen(hex); if(L%2) return NULL; *n=L/2; unsigned char *b=malloc(*n? *n:1);
  for(size_t i=0;i<*n;i++){ int a=hn(hex[2*i]), c=hn(hex[2*i+1]); if(a<0||c<0){free(b);return NULL;} b[i]=(a<<4)|c; }
  return b;
}
static char *rf(const char *p){ FILE *f=fopen(p,"rb"); if(!f)return NULL; fseek(f,0,SEEK_END); long n=ftell(f); fseek(f,0,SEEK_SET); char *b=malloc(n+1); if(fread(b,1,n,f)!=(size_t)n){free(b);fclose(f);return NULL;} fclose(f); while(n>0&&(b[n-1]=='\n'||b[n-1]=='\r'||b[n-1]==' ')) b[--n]=0; b[n]=0; return b; }

int main(int argc, char **argv){
  const char *proof_path = argc>1?argv[1]:"data/out1_rangeproof.hex";
  const char *commit_path = argc>2?argv[2]:"data/out1_commitment.hex";
  const char *list_path = argc>3?argv[3]:"final/data/candidate_contexts.json";
  char *ph=rf(proof_path), *ch=rf(commit_path);
  size_t pl,cl; unsigned char *proof=hx(ph,&pl), *commit=hx(ch,&cl);
  if(!proof||!commit||cl!=33){ fprintf(stderr,"bad inputs\n"); return 2; }
  secp256k1_context *ctx=secp256k1_context_create(SECP256K1_CONTEXT_VERIFY|SECP256K1_CONTEXT_SIGN);
  secp256k1_pedersen_commitment ped;
  if(!secp256k1_pedersen_commitment_parse(ctx,&ped,commit)){ fprintf(stderr,"commit parse fail\n"); return 2; }

  /* Minimal JSON walker: find "asset":"..." and "script":"..." pairs in order */
  char *js=rf(list_path); if(!js){fprintf(stderr,"no list\n"); return 2;}
  int hits=0, tested=0;
  char *p=js;
  while((p=strstr(p, "\"asset\""))){
    p=strchr(p, ':'); if(!p) break; p++; while(*p==' '||*p=='"') { if(*p=='"'){p++; break;} p++; }
    char asset[256]={0}; int ai=0; while(*p && *p!='"' && ai<255) asset[ai++]=*p++;
    char *s=strstr(p, "\"script\""); if(!s) break;
    s=strchr(s, ':'); if(!s) break; s++; while(*s==' '||*s=='"'){ if(*s=='"'){s++; break;} s++; }
    char script[4096]={0}; int si=0; while(*s && *s!='"' && si<4095) script[si++]=*s++;
    p=s;

    size_t al,sl=0; unsigned char *a=hx(asset,&al); unsigned char *sc=NULL;
    if(script[0]) sc=hx(script,&sl); else { sc=NULL; sl=0; }
    if(!a || al!=33){ free(a); free(sc); continue; }

    secp256k1_generator gen; unsigned char gser[33];
    int gok=0;
    if(a[0]==0x01){ gok=secp256k1_generator_generate(ctx,&gen,a+1); if(gok) secp256k1_generator_serialize(ctx,gser,&gen); }
    else { gok=secp256k1_generator_parse(ctx,&gen,a); if(gok) memcpy(gser,a,33); }
    uint64_t minv=0,maxv=0; int ok=0;
    if(gok) ok=secp256k1_rangeproof_verify(ctx,&minv,&maxv,&ped,proof,pl,sl?sc:NULL,sl,&gen);
    tested++;
    printf("TEST %d ok=%d asset=%s script_len=%zu script=%s min=%llu max=%llu\n",
           tested, ok, asset, sl, script[0]?script:"(empty)", (unsigned long long)minv,(unsigned long long)maxv);
    if(ok){ hits++; printf("HIT generator="); for(int i=0;i<33;i++) printf("%02x", gser[i]); printf("\n"); }
    free(a); free(sc);
  }
  printf("SUMMARY tested=%d hits=%d\n", tested, hits);
  return hits?0:1;
}
