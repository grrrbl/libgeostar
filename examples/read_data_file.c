#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "geostar.h"
#define MY_LENGTH 1610612736

uint64_t byteswap64(uint64_t input) 
{
    uint64_t output = (uint64_t) input;
    output = (output & 0x00000000FFFFFFFF) << 32 | (output & 0xFFFFFFFF00000000) >> 32;
    output = (output & 0x0000FFFF0000FFFF) << 16 | (output & 0xFFFF0000FFFF0000) >> 16;
    output = (output & 0x00FF00FF00FF00FF) << 8  | (output & 0xFF00FF00FF00FF00) >> 8;
    return output;
}

int main(int argc, char *argv[]){
    FILE *file_p;
  //  printf("enter filename:\n");
    
  //  char filename[20];
  //  scanf("%s", filename);
//    file_p = fopen("20180418-135518.pr0","rb");
    file_p = fopen(argv[1],"rb");
    if (file_p == NULL)
      {
        printf("error opening file");
        return 127;
      }

    int16_t error = 0;

    struct tm* time_var;
    gs_0x20 ds0x20;
    gs_0x21 ds0x21;
    gs_0x22 ds0x22;
    time_t foo;
    uint32_t uptime;
    ringbuffer_t *rngb;
    rngb = gsRngbInit();
    printf("start: pos %d, write %d,read %d \n",
            rngb->dsPos[rngb->dsNmbHead],rngb->writeIndex,rngb->readIndex);
        printf("foo");
    int count, my_eof;
    for(int i = 0; i<MY_LENGTH; i++)
      {
        gsRngbAppend(rngb, my_eof = fgetc(file_p));
        if(my_eof == EOF){
            printf("end of dataset\n");
            break;
          }
        if(gsRngbDataSetEnd(rngb) > 0)
        {
        printf("type: %2x, pos: %3d, write: %4d,read: %4d, count %i \n", 
               rngb->dsType[rngb->dsNmbHead - 1],(int32_t)rngb->dsPos[rngb->dsNmbHead - 1],
               rngb->writeIndex,rngb->readIndex,(int)rngb->dsNmbHead -1);
        count++;
        if(rngb->dsType[rngb->dsNmbHead - 1] == 0x20)
           {
            printf("eval ds 0x20 \n");
            float latitude, longitude;
            gsConvertDouble((void *) &ds0x20.longitude, (void *)&longitude);
            gsConvertDouble((void *) &ds0x20.latitude, (void *)&latitude);
            error = gsParse0x20(rngb, &ds0x20, -1);
            printf("ds 0x20: length %i, type %x, return %i\n", ds0x20.length, ds0x20.msg_type, error);
            printf("ds 0x20: lat %.8f, long %.8f, heigth %.2f\n", latitude, longitude, ds0x20.position_fix);
            printf("ds 0x20: fix %i \n", (int)ds0x20.position_fix);
            }

        if(rngb->dsType[rngb->dsNmbHead - 1] == 0x22)
            {
            printf("eval ds 0x22 \n");
            gsParse0x22(rngb, &ds0x22, -1);
            printf("ds 0x22 length %i, type %x\n", ds0x22.length, ds0x22.msg_type);
            printf("ds 0x22 number sats %i\n", ds0x22.nsat);
            for(int i = 0; i < ds0x22.nsat; i++)
              {
                printf("ds 0x22 sat %i: %.3e; %.3e; %.3e\n", i, ds0x22.sat[i].snr, 
                       ds0x22.sat[i].elevation, ds0x22.sat[i].azimuth);
              }
  //          if(ds0x22.sat != NULL)
//                free(ds0x22.sat);
            }

        if(rngb->dsType[rngb->dsNmbHead - 1] == 0x21)
            {
            //  printf("eval ds 0x21 \n");
            gsParse0x21(rngb, &ds0x21,  -1);
            printf("ds 0x21: time %i, uptime %i\n", 
                    (int)ds0x21.time, (int)ds0x21.uptime);
            gsParseGetTime(rngb, &foo, &uptime);
            printf("%i, %i \n",(int)foo, (int)uptime);
            time_var = gmtime((time_t*)&ds0x21.time);
            char buffer[26];
            strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", time_var);
            puts(buffer);
            time_var = gmtime(&foo);
            strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", time_var);
            puts(buffer);
            }

        }

      }
    
    printf("end pos: %d, write: %d,read: %d \n", 
            rngb->dsPos[0],rngb->writeIndex,rngb->readIndex);
    fclose(file_p);
}
