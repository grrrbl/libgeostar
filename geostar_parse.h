#ifndef __PARSE_GEOSTAR_BIN__
#define __PARSE_GEOSTAR_BIN__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "geostar_defs.h"

#define FIFO_SIZE 1024
	
// definitions for ringbuffer
typedef struct
{
        int readIndex;
        int writeIndex;
        char fifo[FIFO_SIZE];
} ringbuffer_t;

void  appendFIFO(char *data, ringbuffer_t *buffer);
int   readFIFO(char *data, ringbuffer_t *buffer);
int   searchFIFO(ringbuffer_t *buffer);
int * checkBeginDataSet(char *string);

typedef struct _data_set {
	char header[16];
	uint16_t lenght, number;
	uint32_t checksum;
	long int data_position;
	} gsDataSet;

// reads a compleet data set file
uint32_t gsGenerateChecksum(uint32_t data_field[], uint16_t lenght);
long int gsParseRawData(gsDataSet * p, FILE *file_ptr, long int offset);
int32_t gsGetNumberDataSet(FILE *file_p);
uint32_t gsParseGCBDS(gsDataSet * p, gsDataSet_0x20 *p0, FILE *file_p );

#endif
