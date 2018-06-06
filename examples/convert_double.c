#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "geostar.h"
#include "geostar_defs.h"
#define MY_LENGTH 1610612736

uint64_t byteswap64(uint64_t input) 
{
    uint64_t output = (uint64_t) input;
    output = (output & 0x00000000FFFFFFFF) << 32 | (output & 0xFFFFFFFF00000000) >> 32;
    output = (output & 0x0000FFFF0000FFFF) << 16 | (output & 0xFFFF0000FFFF0000) >> 16;
    output = (output & 0x00FF00FF00FF00FF) << 8  | (output & 0xFF00FF00FF00FF00) >> 8;
    return output;
}

int main(){
    double in = -180.1232241412314;
    float out =  1;
    
    gsConvertDouble(&in,&out);
    
    printf("double in:  %.10f\n float out: %.10f\n", in, out);
}
