/* Streaming SHA-256, API-identical to Bitcoin Core's CSHA256 for the operations
   SignatureCache uses: copy-construct a salted state, Write(...) chained, Finalize(). */
#ifndef SHIM_SHA256_H
#define SHIM_SHA256_H
#include <cstdint>
#include <cstring>
#include <cstddef>
class CSHA256 {
    uint32_t s[8]; unsigned char buf[64]; uint64_t bytes;
    static uint32_t ror(uint32_t x,int n){return (x>>n)|(x<<(32-n));}
    void Transform(const unsigned char* c){
        static const uint32_t K[64]={
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
        uint32_t w[64];
        for(int i=0;i<16;i++) w[i]=(c[4*i]<<24)|(c[4*i+1]<<16)|(c[4*i+2]<<8)|c[4*i+3];
        for(int i=16;i<64;i++){uint32_t a=ror(w[i-15],7)^ror(w[i-15],18)^(w[i-15]>>3),
                                       b=ror(w[i-2],17)^ror(w[i-2],19)^(w[i-2]>>10);
                               w[i]=w[i-16]+a+w[i-7]+b;}
        uint32_t a=s[0],b=s[1],cc=s[2],d=s[3],e=s[4],f=s[5],g=s[6],h=s[7];
        for(int i=0;i<64;i++){
            uint32_t S1=ror(e,6)^ror(e,11)^ror(e,25), ch=(e&f)^((~e)&g), t1=h+S1+ch+K[i]+w[i];
            uint32_t S0=ror(a,2)^ror(a,13)^ror(a,22), mj=(a&b)^(a&cc)^(b&cc), t2=S0+mj;
            h=g;g=f;f=e;e=d+t1;d=cc;cc=b;b=a;a=t1+t2;}
        s[0]+=a;s[1]+=b;s[2]+=cc;s[3]+=d;s[4]+=e;s[5]+=f;s[6]+=g;s[7]+=h;
    }
public:
    CSHA256(){Reset();}
    CSHA256& Reset(){s[0]=0x6a09e667;s[1]=0xbb67ae85;s[2]=0x3c6ef372;s[3]=0xa54ff53a;
        s[4]=0x510e527f;s[5]=0x9b05688c;s[6]=0x1f83d9ab;s[7]=0x5be0cd19;bytes=0;return *this;}
    CSHA256& Write(const unsigned char* d,size_t len){
        size_t bufsize=bytes%64;
        if(bufsize && bufsize+len>=64){
            memcpy(buf+bufsize,d,64-bufsize); bytes+=64-bufsize; d+=64-bufsize; len-=64-bufsize;
            Transform(buf); bufsize=0;}
        while(len>=64){Transform(d);bytes+=64;d+=64;len-=64;}
        if(len>0){memcpy(buf+bytes%64,d,len);bytes+=len;}
        return *this;}
    void Finalize(unsigned char* out){
        static const unsigned char pad[64]={0x80};
        unsigned char sizedesc[8]; uint64_t bits=bytes<<3;
        for(int i=0;i<8;i++) sizedesc[i]=(unsigned char)(bits>>(56-8*i));
        Write(pad,1+((119-(bytes%64))%64)); Write(sizedesc,8);
        for(int i=0;i<8;i++){out[4*i]=s[i]>>24;out[4*i+1]=s[i]>>16;out[4*i+2]=s[i]>>8;out[4*i+3]=s[i];}}
};
#endif
