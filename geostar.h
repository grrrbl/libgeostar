#ifndef __GEOSTAR_LIB__
#define __GEOSTAR_LIB__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "geostar_defs.h"
	
// definitions for ringbuffer
typedef struct 
{
    int32_t readIndex;
    int32_t writeIndex;
    int32_t dsLenghth[5];
    int32_t dsType[5];
    int32_t dsPos[5];
    uint8_t dsNmbRead, dsNmbHead;
    char fifo[FIFO_SIZE];
} ringbuffer_t;

/* all functions concering operations with the ringbuffer are labeled gsRngb... *
 * ringbuffer_t *gsRngbInitialize();                                            */
ringbuffer_t *gsRngbInit(void);
void gsRngbInitialize(ringbuffer_t *ringbuffer);
int8_t gsRngbAppend(ringbuffer_t *buffer, char byte);
int8_t gsRngbRead(ringbuffer_t *buffer, char *dataset);
int8_t gsRngbReadChar(ringbuffer_t *buffer, char *data);
int16_t gsRngbSearch(ringbuffer_t *rngb);
int16_t gsRngbDataSetEnd(ringbuffer_t *rngb);
int16_t gsRngbChecksum(ringbuffer_t *rngb);
uint32_t gsRngbGenChecksum(char *msg, int32_t lenghth);
int16_t gsRngbReadDataSet();

// all functions concering operations with data sets in the ring buffer are labeled gsParse...
char gaParseReturnType(char *dataset);
uint32_t gsGenChecksum(char *dataset,int16_t lenghth);
int8_t gsParse0x10(char *dataset, gsDataSet_0x10 *ds0x10);
int8_t gsParse0x20(char *dataset, gsDataSet_0x20 *ds0x20);
int8_t gsParse0x21(char *dataset, gsDataSet_0x21 *ds0x21);
int8_t gsParse0x22(char *dataset, gsDataSet_0x22 *ds0x22);

#endif
