#ifndef __PARSE_GEOSTAR_BIN__
#define __PARSE_GEOSTAR_BIN__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "geostar_defs.h"
	
// definitions for ringbuffer
typedef struct 
{
        int32_t readIndex;
        int32_t writeIndex;
	uint32_t dsPos;
        char fifo[FIFO_SIZE];
} ringbuffer_t;

// all functions concering operations with the ringbuffer are labeled gsRngb... ringbuffer_t *gsRngbInitialize();
int8_t gsRngbAppend(ringbuffer_t *buffer, char byte);
int8_t gsRngbRead(ringbuffer_t *buffer, char *dataset);
int16_t gsRngbSearch(ringbuffer_t *rngb);
int16_t gsRngbDataSetComplete(ringbuffer_t *rngb);

// all functions concering operations with data sets in the ring buffer are labeled gsParse...
char gaParseReturnType(char *dataset);
uint32_t gsGenChecksum(char *dataset,int16_t lenghth);
int8_t gsParse0x10(char *dataset, gsDataSet_0x10 *ds0x10);
int8_t gsParse0x20(char *dataset, gsDataSet_0x20 *ds0x20);
int8_t gsParse0x21(char *dataset, gsDataSet_0x21 *ds0x21);
int8_t gsParse0x22(char *dataset, gsDataSet_0x22 *ds0x22);

#endif
