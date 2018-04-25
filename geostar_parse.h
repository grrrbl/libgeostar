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

struct _data_set
typedef struct _data_set {
	char head[16];
	uint16_t lenght, number;
	uint32_t checksum;
	uint64_t position;
	} data_set;

// reads a compleet data set file
int read_header(data_set * p);
int read_raw_meas(data_set * p);

#endif
