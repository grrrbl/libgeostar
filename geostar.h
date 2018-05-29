#ifndef __GEOSTAR_LIB__
#define __GEOSTAR_LIB__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "geostar_defs.h"

// return codes for functions
enum return_code
  {
    ERR_RNGB_EMPTY = -4,
    ERR_DS_SIZE = -3,
    ERR_NO_START_POINT = -2,
    DS_CHKSM_MISMATCH = -1,
    DS_COMPLETE = 1,
  };
	
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
ringbuffer_t *gsRngbInit();
int16_t gsRngbAppend(ringbuffer_t *buffer, char byte);
int16_t gsRngbRead(ringbuffer_t *buffer, char *dataset);
int16_t gsRngbReadChar(ringbuffer_t *buffer, char *data);
int16_t gsRngbReadWord(ringbuffer_t *buffer, uint32_t *word);
int16_t gsRngbReadDouble(ringbuffer_t *rngb, uint64_t *data);
int16_t gsRngbMoveRead(ringbuffer_t *buffer, uint8_t steps);
int16_t gsRngbSearch(ringbuffer_t *rngb);
int16_t gsRngbDataSetEnd(ringbuffer_t *rngb);
int16_t gsRngbChecksum(ringbuffer_t *rngb);
uint8_t gsRngbCurhead(ringbuffer_t *rngb);

// util function
uint32_t gsRngbGenChecksum(char *msg, int32_t lenghth);
uint32_t gsConvertDouble(void *in, void *out);

// all functions concering operations with data sets in the ring buffer are labeled gsParse...
int gsParse0x10(ringbuffer_t *rnbg, gs_0x10 *ds, uint8_t nmbr);
int gsParse0x20(ringbuffer_t *rnbg, gs_0x20 *ds, uint8_t nmbr);
int gsParse0x21(ringbuffer_t *rnbg, gs_0x21 *ds, uint8_t nmbr);
int gsParse0x22(ringbuffer_t *rnbg, gs_0x22 *ds, uint8_t nmbr);

#endif
