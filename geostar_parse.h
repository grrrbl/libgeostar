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
	uint64_t position;
	} gsDataSet;

// reads a compleet data set file
size_t gsParseRawData(gsDataSet * p, FILE *file_ptr);

#endif
