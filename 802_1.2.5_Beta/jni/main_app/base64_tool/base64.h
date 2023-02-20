
#ifndef base64_h
#define base64_h

#include <stdio.h>

#if __cplusplus
extern "C"{
#endif

    int base64_encode(unsigned char *indata, int inlen, char *outdata, int *outlen);
    int base64_decode(char *indata, int inlen, unsigned char *outdata, int *outlen);

#if __cplusplus
}
#endif

#endif /* base64_h */