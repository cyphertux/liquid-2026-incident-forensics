
#include <stdio.h>
#include <string.h>
#include <secp256k1.h>
#include <secp256k1_generator.h>
static int readhex(const char*p,unsigned char*o,size_t e){FILE*f=fopen(p,"r");if(!f)return 0;char t[128];fgets(t,sizeof t,f);fclose(f);size_t n=strlen(t);while(n&&(t[n-1]==10||t[n-1]==13))n--;if(n!=e*2)return 0;for(size_t i=0;i<e;i++){unsigned v;sscanf(t+2*i,"%2x",&v);o[i]=v;}return 1;}
int main(){
  unsigned char A[33],Gclaim[33],Gser[33];
  readhex("final/hex/out1_asset.hex",A,33);
  /* G from script */
  FILE*f=fopen("final/hex/71c93d43_out0_script.hex","r"); char t[512]; fgets(t,sizeof t,f); fclose(f);
  /* script 6a43 + C1(33) + G(33) + 6a ; G starts at byte offset 2+33=35 */
  size_t n=strlen(t); while(n&&(t[n-1]==10||t[n-1]==13))n--;
  for(int i=0;i<33;i++){unsigned v;sscanf(t+2*(35+i),"%2x",&v);Gclaim[i]=v;}
  secp256k1_context*ctx=secp256k1_context_create(SECP256K1_CONTEXT_NONE);
  secp256k1_generator gen;
  if(A[0]!=0x01){printf("asset not explicit\n");return 1;}
  if(!secp256k1_generator_generate(ctx,&gen,A+1)){printf("gen fail\n");return 1;}
  if(!secp256k1_generator_serialize(ctx,Gser,&gen)){printf("ser fail\n");return 1;}
  printf("A %02x...\n",A[0]);
  printf("G_from_script "); for(int i=0;i<33;i++)printf("%02x",Gclaim[i]); printf("\n");
  printf("G_from_asset  "); for(int i=0;i<33;i++)printf("%02x",Gser[i]); printf("\n");
  printf("equal %d\n", memcmp(Gclaim,Gser,33)==0);
  return 0;
}
