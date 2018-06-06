/****************************************************************
 *                                                              *  
 * simple example that opens a ringbuffer, reads chars from  a  *
 * file stream and append them to the ringbuffer                *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "geostar.h"
#define READS 1610612736
#define FIFO 64

int main(){
    // initialize ringbuffer
    ringbuffer_t *rngb;
    rngb = gsRngbInit();

    // open file
    FILE *file_p;
    file_p = fopen("../testdata/20180606.LOG","rb");
    if (file_p == NULL){
        printf("error opening file");
        return 127;
    }
    
    // print our position in ringbuffer
    printf("start position: %d, write position %d,read postion %d \n",
            rngb->dsPos[rngb->dsNmbHead],rngb->writeIndex,rngb->readIndex);

    int file_postion;
    // loop through file untile either READS is exeeded or EOF is reached
    for(int i = 0; i<READS; i++){
    // append char to ringbuffer
        gsRngbAppend(rngb, file_postion = fgetc(file_p));
        if(file_postion == EOF){
            printf("end of dataset\n");
            break;
          }
        printf("start position: %d, write position %d,read postion %d \n",
               rngb->dsPos[rngb->dsNmbHead],rngb->writeIndex,rngb->readIndex);

    }
    
    printf("end position: %d, write: %d,read: %d \n", 
            rngb->dsPos[0],rngb->writeIndex,rngb->readIndex);
    fclose(file_p);
}
