#ifndef __GEOSTAR_DEFS__
#define __GEOSTAR_DEFS__

#define WORD 4
#define MSG_CHECKSUM 4
#define MSG_HEADER 12
#define RAW_MEASUREMENT 0x10
#define SVS_IN_VIEW 0x22
#define BASIC_DATA_SET 0x20
#define MAX_MESSAGE_LENGHTH 340

#define FIFO_SIZE 1023

typedef struct {
    char header[16]; //kan eigentlich raus, aber zum debuggen ganz praktisch
    uint32_t data[MAX_MESSAGE_LENGHTH];
    uint16_t lenght, number;
    uint32_t checksum;
    uint16_t dsPos; //kann raus
} _gsDataSet;

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

