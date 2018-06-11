#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "geostar.h"

int main(){
    FILE *file_p;
    file_p = fopen("../testdata/20180606.LOG","rb");
    if (file_p == NULL){
        printf("error opening file");
        return 127;
    }

    struct tm* time_var;

    gs_0x20 ds0x20;
    gs_0x21 ds0x21;
    gs_0x22 ds0x22;

    // initialize ringbuffer
    ringbuffer_t *rngb;
    rngb = gsRngbInit();

    printf("start: pos %d, write %d,read %d \n",
            rngb->dsPos[rngb->dsNmbHead],rngb->writeIndex,rngb->readIndex);
        printf("foo");
    // append data from file to ringbuffer
    int pos;
    while(pos != EOF){
        gsRngbAppend(rngb, pos = fgetc(file_p));
        // if we have a complete dataset, gsRngbDataSetEnd returns 1
        if(gsRngbDataSetEnd(rngb) > 0){
            printf("type: %2x, pos: %3d, write: %4d,read: %4d, count %i \n", 
                   rngb->dsType[rngb->dsNmbHead - 1],(int32_t)rngb->dsPos[rngb->dsNmbHead - 1],
                   rngb->writeIndex,rngb->readIndex,(int)rngb->dsNmbHead -1);
            // get the type of the last data type and chose parser
            switch(rngb->dsType[rngb->dsNmbHead - 1]){
                case 0x20:
                    printf("eval ds 0x20 \n");
                    gsParse0x20(rngb, &ds0x20, -1);
                    printf("ds 0x20: length %i, type %x\n", ds0x20.length, ds0x20.msg_type);
                    printf("ds 0x20: lat %.8f, long %.8f, heigth %.2f\n", 
                            ds0x20.latitude, ds0x20.longitude);
                    printf("ds 0x20: fix %i \n", (int)ds0x20.position_fix);
                    break;
                case 0x22:
                    printf("eval ds 0x22 \n");
                    gsParse0x22(rngb, &ds0x22, -1);
                    printf("ds 0x22 length %i, type %x\n", ds0x22.length, ds0x22.msg_type);
                    printf("ds 0x22 number sats %i\n", ds0x22.nsat);
                    for(int i = 0; i < ds0x22.nsat; i++){
                        printf("ds 0x22 sat %i: %.3e; %.3e; %.3e\n", i, ds0x22.sat[i].snr, 
                               ds0x22.sat[i].elevation, ds0x22.sat[i].azimuth);
                    }
                    break;
                case 0x21:
                    printf("eval ds 0x21 \n");
                    gsParse0x21(rngb, &ds0x21,  -1);
                    printf("ds 0x21: time %i, uptime %i\n", 
                           (int)ds0x21.time, (int)ds0x21.uptime);
                    time_var = gmtime((time_t*)&ds0x21.time);
                    char buffer[26];
                    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", time_var);
                    puts(buffer);
                    break;
            }
        }
    }

    printf("end pos: %d, write: %d,read: %d \n", 
            rngb->dsPos[0],rngb->writeIndex,rngb->readIndex);
    fclose(file_p);
}
