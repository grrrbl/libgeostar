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

ringbuffer_t *gsRngbInitialize();
int gsRngbAappend(char *data, ringbuffer_t *buffer);
int gsRngbRead(char *data, ringbuffer_t *buffer);
int gsRngbSearch(ringbuffer_t *rngb);

uint32_t gsGenerateChecksum(uint32_t data_field[], uint16_t lenght);
long int gsParseRawData(gsDataSet * p, FILE *file_ptr, long int offset);
int32_t  gsGetNumberDataSet(FILE *file_p);
uint32_t gsParseGCBDS(gsDataSet * p, gsDataSet_0x20 *p0, FILE *file_p );
//uint32_t gsCheckChecksum(char *dataSet[],uint16_t  lenghth);

#endif
