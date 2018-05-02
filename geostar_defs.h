#ifndef __GEOSTAR_DEFS__
#define __GEOSTAR_DEFS__

#define WORD 4
#define CHECKSUM 4
#define RAW_MEASUREMENT 0x10
#define SVS_IN_VIEW 0x22
#define BASIC_DATA_SET 0x20


typedef struct _data_set_0x10 {
	double val_double[12];
	uint32_t val_uint[4];
	} gsDataSet_0x10;

typedef struct _data_set_0x20 {
	double val_double[12];
	uint32_t val_uint[4];
	} gsDataSet_0x20;

typedef struct _data_set_0x21 {
	double val_double[12];
	uint32_t val_uint[4];
	} gsDataSet_0x21;

typedef struct _data_set_0x22 {
	double val_double[12];
	uint32_t val_uint[4];
	} gsDataSet_0x22;

#endif

