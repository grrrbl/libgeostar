#include "geostar.h"
#include <time.h>
#define DEBUG
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
        rngb->dsNmbRead  = 0;
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
	if(buffer)
	{
	buffer->fifo[buffer->writeIndex] = word;
	//check for begin of dataset
	if ( word == 'P')
        {
        int count = 0;
        for(int i = 0; i<7; i++)
            {
            if(buffer->writeIndex - i >= 0 &&
               buffer->fifo[buffer->writeIndex - i] == MSG_BEGIN[6-i])
                count++;
            }
		if (count > 5 && buffer->writeIndex - 6 >= 0)
            {
            buffer->dsPos[buffer->dsNmbHead] = buffer->writeIndex - 6;
            }
        }
    
    buffer->writeIndex = (buffer->writeIndex + 1) % (FIFO_SIZE + 1);
	if(buffer->readIndex == buffer->writeIndex)
		buffer->readIndex = (buffer->readIndex + 1) % (FIFO_SIZE + 1);
    return 1;
	}
    else{
        return -1;
    }
}

int16_t gsRngbRead(ringbuffer_t *buffer, char *dataset)
{   
    if(!buffer)
        return -2;

    uint8_t count = 0;
    if (buffer->readIndex != buffer->writeIndex){   
        while(buffer->readIndex != buffer->writeIndex || count < sizeof(dataset))
		{
            dataset[count] = buffer->fifo[buffer->readIndex];
            buffer->readIndex = (buffer->readIndex + 1) % (FIFO_SIZE + 1);
            count++;
		}
        return count;
    } else
        return -1;
}

int16_t gsRngbReadChar(ringbuffer_t *rngb, char *data)
{
    if(!rngb)
        return -2;
    if(rngb->readIndex != rngb->writeIndex)
    {
        *data = rngb->fifo[rngb->readIndex];
        rngb->readIndex = (rngb->readIndex + 1) % (FIFO_SIZE + 1);
        return 1;
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


/* start searching for a dataset from the current writeIndex    *
 * return statu:                                                */
int16_t gsRngbSearch(ringbuffer_t *rngb)
{
    uint16_t pos = rngb->writeIndex;
    if(!(pos > 0))
        return -1; 
    for(int i=0;i<32;i++)
    {
    if(rngb->fifo[pos] == 'G')
        if(rngb->fifo[pos + 1] == 'E')
            return pos;
    pos = pos++ % (FIFO_SIZE + 1);
    return pos;
	}
    return -1;
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
    char data[18];
    int32_t save_readIndex = rngb->readIndex;
    rngb->readIndex = rngb->dsPos[rngb->dsNmbHead];
    for (int i = 0; i < sizeof(data); i++)
        gsRngbReadChar(rngb, &data[i]);
    rngb->readIndex = save_readIndex;    

    /* change type of data to type of var lenghth (16 bit), skip first  *
     * bytes with the header titel and set lenghth                      */
    uint16_t lenghth, msg_type;
    lenghth = ((uint16_t *)data)[5];
    msg_type = ((uint16_t *)data)[4];
    uint16_t sum_lenghth = (rngb->writeIndex - rngb->dsPos[rngb->dsNmbHead] - MSG_HEADER - MSG_CHECKSUM) / WORD;
    uint16_t sum_lenghth_mod = (rngb->writeIndex - rngb->dsPos[rngb->dsNmbHead] - MSG_HEADER - MSG_CHECKSUM) % WORD;
    if (lenghth > 0 && sum_lenghth < lenghth)
      {
        rngb->dsLenghth[rngb->dsNmbHead] = (int32_t)lenghth;
        rngb->dsType[rngb->dsNmbHead] = (int32_t)msg_type;
        return 0;
      }
    else if(lenghth == sum_lenghth && sum_lenghth_mod == 0 )
      {
        rngb->dsNmbHead = (rngb->dsNmbHead + 1) % 5; 
        return 1;
      }
    else
        return -1;
}

uint8_t gsRngbCurhead(ringbuffer_t *rngb)
  {
    if(rngb->dsNmbHead < 1)
        return 4;
    else   
      {
        uint8_t ret = rngb->dsNmbHead - 1;
        return ret;
      }
  }

/* this function is splitted from search end because more data operations are needed    *
 * return -3: missing pointer to ringbuffer;                                            *
 * return -2: found no starting point for data evaluation;                              *
 * return -1: data set exeeded size                                                     *
 * return  0: checksum mismatch                                                         *
 * return  1: checksum ok, dataset complete                                             */

/* util for gsRngbChecksum */
uint32_t
gsRngbGenChecksum(char *msg, int32_t lenghth)
{
    if (lenghth < 2){
        return ((((uint32_t *)msg)[1]) ^ (((uint32_t *)msg)[0]));
    }
    return ((((uint32_t *)msg)[lenghth]) ^ (gsRngbGenChecksum(msg, lenghth - 1))); 
}


int16_t
gsRngbChecksum(ringbuffer_t *rngb)
{
    if(!rngb)
        return -3;

    /* load message into local buffer. msg[] has the lenghth of the message *
     * plus Header plus one Word to save the transmitted checksum           */
    int32_t save_readIndex = rngb->readIndex;
    if(rngb->dsPos[0] < 0)
        return -2;
    rngb->readIndex = rngb->dsPos[0];
    if(!(rngb->dsLenghth[0] > 0))
        return -2;
    int32_t msg_lenghth = (rngb->dsLenghth[0] + 5) * WORD;
    char msg[msg_lenghth];
#ifdef DEBUG 
    printf("size %i \n", sizeof(msg));
    printf("lenghth %i \n", rngb->dsLenghth[0]);
#endif
    for(int i = 0; i < rngb->writeIndex; i++)
        gsRngbReadChar(rngb, &msg[i]);
    rngb->readIndex = save_readIndex;

    /* compute and compare checksum */
    if(rngb->dsLenghth[0] > MAX_MESSAGE_LENGHTH)
        return -1;
    uint32_t checksum = gsRngbGenChecksum(msg,rngb->dsLenghth[0] + 2);
    if(checksum == ((uint32_t *)msg)[rngb->dsLenghth[0] + 3] )
        {
#ifdef DEBUG
        printf("checksum: %x\n",checksum);
#endif
        return 1;
        }
    else
        {
#ifdef DEBUG
        printf("checksum: %x\n",checksum);
#endif
        return 0;
        }
}

uint32_t
gsGenChecksum(char *dataset, int16_t lenghth)
{
    if (lenghth > MAX_MESSAGE_LENGHTH || (sizeof(dataset) / 4) < lenghth)
        return 0;
    if (lenghth < 2){
        return (((uint32_t *)dataset)[1] ^ ((uint32_t *)dataset)[0]);
    }
    else {
        return ((uint32_t *)dataset)[lenghth] ^ gsGenChecksum(dataset, lenghth - 1); 
    }
}

uint32_t gsConvertDouble(void *in, void *out)
  {
    uint32_t *fp = (uint32_t *)out;
    uint64_t *dp = (uint64_t *)in;
    uint32_t fs, fm, fe;
    uint64_t d, ds, dm, de;
    d = *dp;
    int exp;

    ds = d & 0x8000000000000000u;
    de = d & 0x7FF0000000000000u;
    dm = d & 0x000FFFFFFFFFFFFFu;
    
    fs = (uint16_t)(ds >> 32);
    exp = (int)(de >> 52) - 1023 + 127;
    
    fe = (uint32_t)(exp << 23 );
    fm = (uint32_t)(dm >> 29);
    
    //rounding, funkti9niert nicht
//    if( dm & 0x0000000010000000u)
//        *fp = (fs | fe | fm) + (uint32_t) 1u;
//    else 
        *fp = (fs | fe | fm);
        
  }

int gsParse0x20(ringbuffer_t *rngb, gs_0x20 *ds, uint8_t nmbr)
{
	if(ds == NULL)
		return -2;
    if(rngb->writeIndex == rngb->dsPos[nmbr])
        return -1;
    
    /* save current read index and set start position of the dataset    *
       in the ringbuffer                                                */
    int32_t save_read_index = rngb->readIndex;
    rngb->readIndex = rngb->dsPos[nmbr];

    uint32_t cache;
    gsRngbMoveRead(rngb, 2*WORD);
    gsRngbReadWord(rngb, &cache);

    /* change type of data to type of var lenghth (16 bit), skip first  *
     * bytes with the header titel and set lenghth                      */
    ds->length = ((uint16_t *)&cache)[1];
    ds->msg_type = ((uint16_t *)&cache)[0];

    if(rngb->dsLenghth[nmbr] != ds->length)
        return -2;

    gsRngbReadDouble(rngb, (uint64_t*)&(ds->position));
    gsRngbReadDouble(rngb, (uint64_t*)&(ds->latitude));
    gsRngbReadDouble(rngb, (uint64_t*)&(ds->longitude));
    gsRngbReadDouble(rngb, (uint64_t*)&(ds->heigth));
    //gsRngbReadDouble(rngb, &(ds->geoidal_seperation));
    gsRngbMoveRead(rngb, 2*WORD);
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
    //gsRngbReadDouble(rngb, &(ds->speed));
    //gsRngbReadDouble(rngb, &(ds->course));
    rngb->readIndex = save_read_index;  
    return 0;
}

int gsParse0x21(ringbuffer_t *rngb, gs_0x21 *ds, uint8_t nmbr)
{
	if(ds == NULL || nmbr < 0 || nmbr > 5)
		return -1;
    if(rngb->writeIndex == rngb->dsPos[nmbr])
        return 0;
    int32_t save_read_index = rngb->readIndex;
    rngb->readIndex = rngb->dsPos[nmbr];    

    uint32_t cache;
    gsRngbMoveRead(rngb, 2*WORD);
    gsRngbReadWord(rngb, &cache);

    /* change type of data to type of var lenghth (16 bit), skip first  *
     * bytes with the header titel and set lenghth                      */
    ds->length = ((uint16_t *)&cache)[1];
    ds->msg_type = ((uint16_t *)&cache)[0];

    if(rngb->dsLenghth[nmbr] != ds->length)
        return -2;
    gsRngbMoveRead(rngb, 3*WORD);
    gsRngbReadWord(rngb, &cache);
    ds->uptime = cache;
    gsRngbReadWord(rngb, &cache);
    ds->time = cache + TIME_DIFF;
    gsRngbMoveRead(rngb, 3*WORD);
    rngb->readIndex = save_read_index;  

    return 0;
}

int gsParse0x22(ringbuffer_t *rngb, gs_0x22 *ds, uint8_t nmbr)
{
	if(ds == NULL || nmbr < 0 || nmbr > 5)
		return -1;
    if(rngb->writeIndex == rngb->dsPos[nmbr])
        return 0;
    int32_t save_read_index = rngb->readIndex;
    rngb->readIndex = rngb->dsPos[nmbr];    

    uint32_t cache;
    gsRngbMoveRead(rngb, 2*WORD);
    gsRngbReadWord(rngb, &cache);

    ds->length = ((uint16_t *)&cache)[1];
    ds->msg_type = ((uint16_t *)&cache)[0];

    if(rngb->dsLenghth[nmbr] != ds->length)
        return -2;

    gsRngbReadWord(rngb, &cache);
    ds->nsat = cache;
    if(!(ds->nsat > 0))
        return -1;
    
    // allocate memory for the sats file, to keep the code a little dynamic
    //gs_0x22_sat *sats = malloc(ds->nsat * sizeof(*sats));
    gs_0x22_sat *sats = calloc(ds->nsat, sizeof(*sats));
    if(sats == NULL)
        return -3;
    ds->sat = sats;
    
    for(int i = 0; i < ds->nsat; i++)
      {
        gsRngbReadWord(rngb, &cache);
        ds->sat[i].word1 = cache;
        gsRngbReadWord(rngb, &cache);
        ds->sat[i].word2 = cache;
        gsRngbReadWord(rngb, &cache);
        ds->sat[i].snr = (float)cache;
        gsRngbReadWord(rngb, &cache);
        ds->sat[i].elevation = (float)cache;
        gsRngbReadWord(rngb, &cache);
        ds->sat[i].azimuth = (float)cache;
      }
    rngb->readIndex = save_read_index;  
    return 0;
}

