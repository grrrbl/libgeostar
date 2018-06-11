#include "geostar.h"
#include "debug.h"
#include <time.h>
#define DEBUG_LVL 4
#define RngbNmbSave 3

const char *MSG_BEGIN = "GEOSr3PS";

ringbuffer_t* 
gsRngbInit(void)
{
    // allocate memory 
    ringbuffer_t *rngb = malloc(sizeof(*rngb)); 
    // if available initialize vars as zero
    if(rngb != NULL)
      {
        rngb->readIndex  = 0;
	    rngb->writeIndex = 0;
        rngb->dsNmbHead  = 0;
	    for(int i = 0;i < sizeof(rngb->dsPos);i++)
          rngb->dsPos[i] = -1;
	    for(int i = 0;i < sizeof(rngb->dsType);i++)
          rngb->dsType[i] = -1;
	    for(int i = 0;i < sizeof(rngb->dsLenghth);i++)
          rngb->dsLenghth[i] = -1;
      }
    return rngb; 
}

int16_t gsRngbAppend(ringbuffer_t *buffer, char word)
{
	if(buffer){
	   buffer->fifo[buffer->writeIndex] = word;
	//check for begin of dataset
	if ( word == 'P'){
        int count = 0;
        for(int i = 0; i<7; i++){
            if(buffer->writeIndex - i >= 0 &&
               buffer->fifo[buffer->writeIndex - i] == MSG_BEGIN[6-i]){
                count++;
            }
        }
		if (count > 5 && buffer->writeIndex - 6 >= 0)
            buffer->dsPos[buffer->dsNmbHead] = buffer->writeIndex - 6;
        }
    
    buffer->writeIndex = (buffer->writeIndex + 1) % (FIFO_SIZE + 1);
	if(buffer->readIndex == buffer->writeIndex)
		buffer->readIndex = (buffer->readIndex + 1) % (FIFO_SIZE + 1);
    return 1;
	} else {
        return -1;
    }
}

int16_t gsRngbReadChar(ringbuffer_t *rngb, char *data)
{
    if(!rngb)
        return -2;
    if(rngb->readIndex != rngb->writeIndex)
    {
        *data = rngb->fifo[rngb->readIndex];
        rngb->readIndex = (rngb->readIndex + 1) % (FIFO_SIZE + 1);
        return 0;
    }
    else
        return -1;
}

int16_t gsRngbReadWord(ringbuffer_t *rngb, uint32_t *data)
{
    if(!rngb)
        return -2;
    if(rngb->readIndex != rngb->writeIndex)
    {
        for(int i = 0; i<4; i++)
        {
          ((char *)data)[i] = rngb->fifo[rngb->readIndex];
          rngb->readIndex = (rngb->readIndex + 1) % (FIFO_SIZE + 1);
        }
        return 1;
    }
    else
        return 0;
}

int16_t 
gsRngbReadDouble(ringbuffer_t *rngb, uint64_t *data)
{
    if(!rngb)
        return -2;
    if(rngb->readIndex != rngb->writeIndex)
    {
        for(int i = 0; i<8; i++)
        {
          ((uint8_t*)data)[i] = rngb->fifo[rngb->readIndex];
          rngb->readIndex = (rngb->readIndex + 1) % (FIFO_SIZE + 1);
        }
        return 1;
    }
    else
        return 0;
}

int16_t gsRngbMoveRead(ringbuffer_t *rngb, uint8_t steps)
{
    if(!rngb && steps > 255)
        return -2;
    for(int i = 0; i < steps; i++)
      {
        if(rngb->readIndex != rngb->writeIndex)
          {
            rngb->readIndex = (rngb->readIndex + 1) % (FIFO_SIZE + 1);
          }
        else
            return steps;
      }
      return 0;
}

int16_t gsRngbDataSetEnd(ringbuffer_t *rngb)
{
    /* check if it's save to run function                               */
    if (!rngb)
        return -2;

    /* we want at least the header info */
    if (rngb->writeIndex <= rngb->dsPos[rngb->dsNmbHead] + 16)
        return -1;
    if(rngb->dsPos[rngb->dsNmbHead] < 0)
        return 0;

    /* write data to local cache, so pointer arithemitc doesn't break   *
     * at the end of the ringbuffer (writeIndex - 12 > FIFO_SIZE)       *
     * Save ringbuffer position and reset after reading, so we don't    *
     * change teh ringbuffer read position                              */
    // modulo about poiunter were more elegant...
    char data[18];
    int32_t save_readIndex = rngb->readIndex;
    rngb->readIndex = rngb->dsPos[rngb->dsNmbHead];
    for (int i = 0; i < sizeof(data); i++)
        gsRngbReadChar(rngb, &data[i]);
    rngb->readIndex = save_readIndex;    

    /* change type of data to type of var lenghth (16 bit), skip first  *
     * 32 bytes with the header titel and set lenghth                   */
    uint16_t length, msg_type;
    length = ((uint16_t *)data)[5];
    msg_type = ((uint16_t *)data)[4];
    uint16_t sum_length = (rngb->writeIndex - rngb->dsPos[rngb->dsNmbHead] - MSG_HEADER - MSG_CHECKSUM) / WORD;
    uint16_t sum_length_mod = (rngb->writeIndex - rngb->dsPos[rngb->dsNmbHead] - MSG_HEADER - MSG_CHECKSUM) % WORD;
    if (length > 0 && sum_length < length)
      {
        rngb->dsLenghth[rngb->dsNmbHead] = (int32_t)length;
        rngb->dsType[rngb->dsNmbHead] = (int32_t)msg_type;
        return 0;
      }
    else if(length == sum_length && sum_length_mod == 0 )
      {
        rngb->dsNmbHead = (rngb->dsNmbHead + 1) % 5; 
        return 1;
      }
    else
        return -1;
}

/* this function is splitted from search end because more data operations are needed    *
 * return -3: missing pointer to ringbuffer;                                            *
 * return -2: found no starting point for data evaluation;                              *
 * return -1: data set exeeded size                                                     *
 * return  0: checksum mismatch                                                         *
 * return  1: checksum ok, dataset complete                                             */

int16_t
gsRngbCheckChecksum(ringbuffer_t *rngb, int8_t nmbr)
{
    // check if it's save to run
    if(!rngb)
        return -3;

    if(rngb->dsPos[rngb->dsNmbHead] < 0)
        return -2;

    // save read index
    int32_t save_readIndex;
    uint8_t dsNmbr;
    save_readIndex = rngb->readIndex;
    if(nmbr > -1){  
        rngb->readIndex = rngb->dsPos[nmbr];
        dsNmbr = nmbr;
    } else {
        // have to substract 1 to read the last finished dataset
        dsNmbr = --rngb->dsNmbHead;
        rngb->readIndex = rngb->dsPos[dsNmbr];
    }
    
    /* read in first word, loop over message and do an xor              *
     * for each. Number of cyles limited by size of uint8_t             */
    uint32_t read_word, checksum_gen;
    gsRngbReadWord(rngb, &checksum_gen);
    uint8_t msg_length = rngb->dsLenghth[dsNmbr] + 2;
    while(msg_length){
        gsRngbReadWord(rngb, &read_word);
        checksum_gen ^= read_word;
        msg_length--;
    }

    /* read in checksum from message    */
    uint32_t checksum_msg;
    gsRngbReadWord(rngb, &checksum_msg);

    // compare checksum 
    if(checksum_gen == checksum_msg){
        return 1;
    } else {
        return 0;
    }
    
    // reset read index
    rngb->readIndex = save_readIndex;
}

uint32_t gsConvertRad(float *rad, float *deg)
  {
    *deg = *rad * 57.2957795130823208;
  }

uint32_t gsConvertDouble(void *in, void *out)
  {
    uint32_t *fp = (uint32_t *)out;
    uint64_t *dp = (uint64_t *)in;
    uint32_t fs, fm, fe;
    uint64_t ds, dm, de;
    int32_t exp;

    ds = *dp & 0x8000000000000000u;
    de = *dp & 0x7FF0000000000000u;
    dm = *dp & 0x000FFFFFFFFFFFFFu;
    
    fs = (uint16_t)(ds >> 32LL);
    exp = (int32_t)(de >> 52LL) - 1023 + 127;
    
    fe = (uint32_t)(exp << 23LL );
    fm = (uint32_t)(dm >> 29LL);
    
    //rounding, funkti9niert nicht
//    if( dm & 0x0000000010000000u)
//        *fp = (fs | fe | fm) + 1uLL;
//    else 
        *fp = (fs | fe | fm);
        
  }

int gsParse0x10(ringbuffer_t *rngb, gs_0x10 *ds, int8_t nmbr)
{
	if(ds == NULL)
		return -2;
    if(rngb->writeIndex == rngb->dsPos[nmbr])
        return -1;

    //save number dataset in function
    int8_t dsNmbr;
    int32_t save_read_index;

    // change read Index. Uncomment here and at the end to disable.
    if(nmbr > -1)
      {  
        save_read_index = rngb->readIndex;
        rngb->readIndex = rngb->dsPos[nmbr];
        dsNmbr = nmbr;
      }
    else
      {
      /* have to substract 1 to read the last finisched dataset,       *
         otherwise we end at the current read head                     */
      dsNmbr = --rngb->dsNmbHead;
      rngb->readIndex = rngb->dsPos[dsNmbr];
      }

    /* save current read index and set start position of the dataset    *
       in the ringbuffer                                                */
    uint32_t cache;
    gsRngbMoveRead(rngb, 2*WORD);
    gsRngbReadWord(rngb, &cache);

    /* change type of data to type of var lenghth (16 bit), skip first  *
     * bytes with the header titel and set lenghth                      */
    ds->length = ((uint16_t *)&cache)[1];
    ds->msg_type = ((uint16_t *)&cache)[0];


    if(nmbr > -1)
        rngb->readIndex = save_read_index;  

    return 0;
}

int gsParse0x20(ringbuffer_t *rngb, gs_0x20 *ds, int8_t nmbr)
{
	if(ds == NULL)
		return -2;
    if(rngb->writeIndex == rngb->dsPos[nmbr])
        return -1;

    //save number dataset in function
    int8_t dsNmbr;
    int32_t save_read_index;

    // change read Index. Uncomment here and at the end to disable.
    if(nmbr > -1)
      {  
        save_read_index = rngb->readIndex;
        rngb->readIndex = rngb->dsPos[nmbr];
        dsNmbr = nmbr;
      }
    else
      {
      /* have to substract 1 to read the last finisched dataset,       *
         otherwise we end at the current read head                     */
      dsNmbr = --rngb->dsNmbHead;
      rngb->readIndex = rngb->dsPos[dsNmbr];
      }

    /* save current read index and set start position of the dataset    *
       in the ringbuffer                                                */
    uint32_t cache;
    gsRngbMoveRead(rngb, 2*WORD);
    gsRngbReadWord(rngb, &cache);

    /* change type of data to type of var lenghth (16 bit), skip first  *
     * bytes with the header titel and set lenghth                      */
    ds->length = ((uint16_t *)&cache)[1];
    ds->msg_type = ((uint16_t *)&cache)[0];

    if(rngb->dsLenghth[dsNmbr] != ds->length)
        return -2;

    gsRngbReadDouble(rngb, (uint64_t*)&(ds->position_fix_time));
    gsRngbReadDouble(rngb, (uint64_t*)&(ds->latitude));
    gsRngbReadDouble(rngb, (uint64_t*)&(ds->longitude));
    gsRngbReadDouble(rngb, (uint64_t*)&(ds->heigth));
    gsRngbReadDouble(rngb, (uint64_t*)&(ds->geoidal_seperation));
    gsRngbReadWord(rngb, &(ds->numbers_sv));
    gsRngbReadWord(rngb, &(ds->receiver_status));
    /* skip for now */
    gsRngbMoveRead(rngb, 10*WORD);
    /*
    gsRngbReadDouble(rngb, &(ds->gdop));
    gsRngbReadDouble(rngb, &(ds->pdop));
    gsRngbReadDouble(rngb, &(ds->tdop));
    gsRngbReadDouble(rngb, &(ds->hdop));
    gsRngbReadDouble(rngb, &(ds->vdop));
    */
    gsRngbReadWord(rngb, &(ds->position_fix));
    gsRngbReadWord(rngb, &(ds->continuous_fixes));
    gsRngbReadDouble(rngb, (uint64_t*)&(ds->speed));
    gsRngbReadDouble(rngb, (uint64_t*)&(ds->course));

    if(nmbr > -1)
        rngb->readIndex = save_read_index;  

    return 0;
}

int gsParse0x21(ringbuffer_t *rngb, gs_0x21 *ds, int8_t nmbr)
{
	if(ds == NULL || nmbr > 5)
		return -1;
    if(rngb->writeIndex == rngb->dsPos[nmbr])
        return 0;

    //save number dataset in function
    int8_t dsNmbr;
    int32_t save_read_index;

    // if specific read index, read this dataset, else read the last on stack
    if(nmbr > -1)
      {  
        save_read_index = rngb->readIndex;
        rngb->readIndex = rngb->dsPos[nmbr];
        dsNmbr = nmbr;
      }
    else
      {
        //have to substract 1 to read the last finished dataset
        dsNmbr = --rngb->dsNmbHead;
        rngb->readIndex = rngb->dsPos[dsNmbr];
      }

    uint32_t cache;
    gsRngbMoveRead(rngb, 2*WORD);
    gsRngbReadWord(rngb, &cache);

    /* change type of data to type of var lenghth (16 bit), skip first  *
     * bytes with the header titel and set lenghth                      */
    ds->length = ((uint16_t *)&cache)[1];
    ds->msg_type = ((uint16_t *)&cache)[0];

    if(rngb->dsLenghth[dsNmbr] != ds->length)
        return -2;
    
    // according to manual here should be a shift of two words, but only
    // 3 works.
    gsRngbMoveRead(rngb, 3*WORD);
    gsRngbReadWord(rngb, (uint32_t*)&(ds->uptime));
    gsRngbReadWord(rngb, (uint32_t*)&(ds->time));
    /* convert time from geostar time to unix time by adding diff   *
     * (2008 - 1970) secs                                           */
    ds->time += TIME_DIFF;
    // skip next to words to exit clean
    gsRngbMoveRead(rngb, 2*WORD);

    if(nmbr > -1)
        rngb->readIndex = save_read_index;  

    return 0;
}

// dataset 0x22 has 4 + n*NSat words 
int gsParse0x22(ringbuffer_t *rngb, gs_0x22 *ds, int8_t nmbr)
{
    //save number dataset in function
    int8_t dsNmbr;
    int32_t save_read_index;

    //check if nmbr is in a reasonable range 
	if(ds == NULL || nmbr > RngbNmbSave)
		return -1;
    //check we are not at the head of the stack
    if(rngb->writeIndex == rngb->dsPos[nmbr])
        return 0;
    
    // if specigic read index, read this dataset, else read the last on stack
    if(nmbr > -1){  
        save_read_index = rngb->readIndex;
        rngb->readIndex = rngb->dsPos[nmbr];
        dsNmbr = nmbr;
    } else {
        //have to substract 1 to read the last finished dataset
        dsNmbr = rngb->dsNmbHead  - 1;
        rngb->readIndex = rngb->dsPos[dsNmbr];
    }

    uint32_t cache;
    gsRngbMoveRead(rngb, 2*WORD);
    gsRngbReadWord(rngb, &cache);

    ds->length = ((uint16_t *)&cache)[1];
    ds->msg_type = ((uint16_t *)&cache)[0];

    if(rngb->dsLenghth[dsNmbr] != ds->length)
        return -2;

    gsRngbReadWord(rngb, (uint32_t*)&(ds->nsat));

    if(!(ds->nsat > 0))
        return -1;
    
    /* allocate memory for the sats file, to keep the code a little *
     * dynamic gs_0x22_sat *sats = malloc(ds->nsat * sizeof(*sats)) *
     * abort if there are more than MAX_NUMBER_SATS to prevent      *
     * allocating kind of infinie memory and crash                  */

    if(ds->nsat > MAX_NUMBER_SATS)
        return -3;
    gs_0x22_sat *sats = calloc(ds->nsat, sizeof(*sats));
    if(sats == NULL)
        return -3;
    ds->sat = sats;
    
    for(int i = 0; i < ds->nsat; i++)
      {
        gsRngbReadWord(rngb, (uint32_t*)&(sats->word1));
        gsRngbReadWord(rngb, (uint32_t*)&(sats->word2));
        gsRngbReadWord(rngb, (uint32_t*)&(sats->snr));
        gsRngbReadWord(rngb, (uint32_t*)&(sats->elevation));
        gsRngbReadWord(rngb, (uint32_t*)&(sats->azimuth));
        sats++;
      }
    
    if(nmbr > -1)
        rngb->readIndex = save_read_index;  

    return 0;
}

int gsParseGetTime(ringbuffer_t *rngb, time_t *time, uint32_t *uptime)
{
    int nmbr = --rngb->dsNmbHead;
	if(rngb == NULL)
		return -5;
    if(rngb->writeIndex == rngb->dsPos[nmbr])
        return -1;

    // if specific read index, read this dataset, else read the last on stack
    int32_t save_read_index = rngb->readIndex;
    rngb->readIndex = rngb->dsPos[nmbr];
    rngb->readIndex = rngb->dsPos[nmbr];

    gsRngbMoveRead(rngb, 6*WORD);
    gsRngbReadWord(rngb, (uint32_t*)uptime);
    gsRngbReadWord(rngb, (uint32_t*)time);
    /* convert time from geostar time to unix time by adding diff   *
     * (2008 - 1970) secs                                           */
    *time = *time + TIME_DIFF;
    // skip next to words to exit clean
    rngb->readIndex = save_read_index;  

    return 0;
}

