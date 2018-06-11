#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "geostar.h"
#define MY_LENGTH 1610612736

int main(){
    double in = -180.1232241412314;
    float out =  1;
    
    gsConvertDouble(&in,&out);
    
    printf("double in:  %.10f\n float out: %.10f\n", in, out);
}
