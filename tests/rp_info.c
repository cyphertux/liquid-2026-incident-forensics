#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <secp256k1.h>
#include <secp256k1_rangeproof.h>
static int hn(char c){ if(c>='0'&&c<='9')return c-'0'; if(c>='a'&&c<='f')return c-'a'+10; if(c>='A'&&c<='F')return c-'A'+10; return -1; }
static unsigned char *hx(const char *hex, size_t *n){ size_t L=strlen(hex); *n=L/2; unsigned char *b=malloc(*n); for(size_t i=0;i<*n;i++) b[i]=(hn(hex[2*i])<<4)|hn(hex[2*i+1]); return b; }
static char *rf(const char *p){ FILE *f=fopen(p,"rb"); fseek(f,0,SEEK_END); long n=ftell(f); fseek(f,0,SEEK_SET); char *b=malloc(n+1); fread(b,1,n,f); fclose(f); while(n>0&&(b[n-1]=='\n'||b[n-1]=='\r'))b[--n]=0; b[n]=0; return b; }
int main(){
  char *h=rf("data/out1_rangeproof.hex"); size_t n; unsigned char *p=hx(h,&n);
  secp256k1_context *ctx=secp256k1_context_create(SECP256K1_CONTEXT_NONE);
  int exp=0, mantissa=0; uint64_t minv=0,maxv=0;
  int ok=secp256k1_rangeproof_info(ctx,&exp,&mantissa,&minv,&maxv,p,n);
  printf("info_ok=%d exp=%d mantissa=%d min=%llu max=%llu plen=%zu\n", ok,exp,mantissa,(unsigned long long)minv,(unsigned long long)maxv,n);
  return 0;
}
