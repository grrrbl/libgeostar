#ifndef __PARSE_GEOSTAR_BIN__
#define __PARSE_GEOSTAR_BIN__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define WORD 4
#define CHECKSUM 4
#define RAW_MEASUREMENT 0x10
#define SVS_IN_VIEW 0x22
#define	BASIC_DATA_SET 0x20

typedef struct _data_set {
	char header[16];
	uint16_t lenght, number;
	uint32_t checksum;
	long int data_position;
	} gsDataSet;

typedef struct _data_set_0x20 {
	double val_double[12];
	uint32_t val_uint[4];
	} gsDataSet_0x20;

// reads a compleet data set file
uint32_t gsGenerateChecksum(uint32_t data_field[], uint16_t lenght);
long int gsParseRawData(gsDataSet * p, FILE *file_ptr, long int offset);
int32_t gsGetNumberDataSet(FILE *file_p);
uint32_t gsParseGCBDS(gsDataSet * p, gsDataSet_0x20 *p0, FILE *file_p );

#endif
