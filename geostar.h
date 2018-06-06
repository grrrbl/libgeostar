#ifndef __GEOSTAR_LIB__
#define __GEOSTAR_LIB__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// size of ringbuffer in bytes
#define FIFO_SIZE 1024

// human readable length descriptions
#define WORD 4
#define MSG_CHECKSUM 4
#define MSG_HEADER 12

// limits
#define MAX_MESSAGE_LENGHTH 340
#define MAX_NUMBER_SATS 40

// human readable names for dataset types
#define RAW_MEASUREMENT 0x10
#define SVS_IN_VIEW 0x22
#define BASIC_DATA_SET 0x20

//difference between unix time to time of geostar
#define TIME_DIFF 1199145600

// return codes for functions
enum return_code
  {
    ERR_NULL_POINTER = -10,
    ERR_DS_SIZE = -3,
    ERR_NO_START_POINT = -2,
    DS_CHKSM_MISMATCH = -1,
    OK = 0,
    DS_COMPLETE = 1,
  };
	
typedef struct gs_0x10_sat {
    uint32_t word1;
    float snr;
    double pseudorange, pseudorange_rate, adr, carrier_phase;
    float pseudorange_error_estimate, pseudorange_rate_error_estimate;
    float pseudorange_residuals, pseudorange_rate_residuals;
} gs_0x10_sat; 

typedef struct gs_0x10 {
    uint16_t length, msg_type;
    double time;
    int32_t number_cycles, number_svs;
    gs_0x10_sat *sat; 
} gs_0x10;

typedef struct gs_0x20 {
    uint16_t length, msg_type;
    uint32_t msg_length;
	double position, latitude, longitude, heigth, geoidal_seperation;
	uint32_t numbers_sv, receiver_status;
    uint64_t gdop, pdop, tdop, hdop, vdop;
    uint32_t position_fix, continuous_fixes;
    uint64_t speed, course;
    uint32_t checksum;
} gs_0x20;

typedef struct gs_0x21 {
    uint16_t length, msg_type;
    uint32_t checksum;
    uint32_t receiver_status_word, receiver_configuration, 
             uptime, time, word5, word6;
} gs_0x21;

/* consist of two parts, nested */
typedef struct gs_0x22_sat{
    char sv_number, channel_nr;
    uint16_t carrier_frequency;
    uint32_t word1, word2;
    float    snr,elevation, azimuth;
} gs_0x22_sat;

typedef struct gs_0x22 {
    uint16_t length, msg_type;
    uint32_t checksum;
	uint32_t nsat;
    gs_0x22_sat *sat; 
} gs_0x22;
// definitions for ringbuffer
typedef struct 
{
    int32_t readIndex;
    int32_t writeIndex;
    int32_t dsLenghth[5];
    int32_t dsType[5];
    int32_t dsPos[5];
    uint8_t dsNmbHead;
    char fifo[FIFO_SIZE];
} ringbuffer_t;

/* all functions concering operations with the ringbuffer are labeled gsRngb... *
 * ringbuffer_t *gsRngbInitialize();                                            */
ringbuffer_t *gsRngbInit();
int16_t gsRngbAppend(ringbuffer_t *buffer, char byte);
int16_t gsRngbRead(ringbuffer_t *buffer, char *dataset);
int16_t gsRngbReadChar(ringbuffer_t *buffer, char *data);
int16_t gsRngbReadWord(ringbuffer_t *buffer, uint32_t *word);
int16_t gsRngbReadDouble(ringbuffer_t *rngb, uint64_t *data);
int16_t gsRngbMoveRead(ringbuffer_t *buffer, uint8_t steps);
int16_t gsRngbDataSetEnd(ringbuffer_t *rngb);
int16_t gsRngbChecksum(ringbuffer_t *rngb);

// util function
uint32_t gsRngbGenChecksum(char *msg, int32_t lenghth);
uint32_t gsConvertDouble(void *in, void *out);
uint32_t gsConvertRad(float *rad, float *deg);

// all functions concering operations with data sets in the ring buffer are labeled gsParse...
int gsParse0x10(ringbuffer_t *rnbg, gs_0x10 *ds, int8_t nmbr);
int gsParse0x20(ringbuffer_t *rnbg, gs_0x20 *ds, int8_t nmbr);
int gsParse0x21(ringbuffer_t *rnbg, gs_0x21 *ds, int8_t nmbr);
int gsParse0x22(ringbuffer_t *rnbg, gs_0x22 *ds, int8_t nmbr);

int gsParseGetTime(ringbuffer_t *rnbg, time_t *time_var, uint32_t *uptime);

#endif
