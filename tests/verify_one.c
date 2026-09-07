#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <secp256k1.h>
#include <secp256k1_generator.h>
#include <secp256k1_rangeproof.h>

static int hex_nibble(char c){ if(c>='0'&&c<='9')return c-'0'; if(c>='a'&&c<='f')return c-'a'+10; if(c>='A'&&c<='F')return c-'A'+10; return -1;}
static unsigned char *hex_decode(const char *hex, size_t *out_len){ size_t n=strlen(hex); if(n%2)return NULL; *out_len=n/2; unsigned char *b=malloc(*out_len); for(size_t i=0;i<*out_len;i++){int a=hex_nibble(hex[2*i]),c=hex_nibble(hex[2*i+1]); if(a<0||c<0){free(b);return NULL;} b[i]=(a<<4)|c;} return b;}
static char *read_file(const char *path){ FILE *f=fopen(path,"rb"); if(!f)return NULL; fseek(f,0,SEEK_END); long n=ftell(f); fseek(f,0,SEEK_SET); char *b=malloc(n+1); fread(b,1,n,f); fclose(f); while(n>0&&(b[n-1]=='\n'||b[n-1]=='\r')) b[--n]=0; b[n]=0; return b;}
int main(int argc,char**argv){
  if(argc<5){fprintf(stderr,"usage: proof commit asset script\n");return 2;}
  char *ph=read_file(argv[1]), *ch=read_file(argv[2]), *ah=read_file(argv[3]), *sh=read_file(argv[4]);
  size_t pl,cl,al,sl; unsigned char *p=hex_decode(ph,&pl),*c=hex_decode(ch,&cl),*a=hex_decode(ah,&al),*s=hex_decode(sh,&sl);
  secp256k1_context *ctx=secp256k1_context_create(SECP256K1_CONTEXT_VERIFY|SECP256K1_CONTEXT_SIGN);
  secp256k1_pedersen_commitment commit; secp256k1_generator gen; unsigned char gser[33];
  uint64_t minv=0,maxv=0;
  if(!secp256k1_pedersen_commitment_parse(ctx,&commit,c)){puts("commit parse fail");return 1;}
  if(al==33 && a[0]==0x01){ if(!secp256k1_generator_generate(ctx,&gen,a+1)){puts("gen fail");return 1;} secp256k1_generator_serialize(ctx,gser,&gen);}
  else { if(!secp256k1_generator_parse(ctx,&gen,a)){puts("gen parse fail");return 1;} memcpy(gser,a,33);}
  int ok=secp256k1_rangeproof_verify(ctx,&minv,&maxv,&commit,p,pl,sl?s:NULL,sl,&gen);
  printf("verify=%d min=%llu max=%llu gen=", ok,(unsigned long long)minv,(unsigned long long)maxv);
  for(int i=0;i<33;i++) printf("%02x", gser[i]); printf("\n");
  return ok?0:1;
}
