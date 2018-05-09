#include "geostar.h"
#define DEBUG
const char *MSG_BEGIN = "GEOSr3PS";

ringbuffer_t* 
gsRngbInit(void)
{
    ringbuffer_t *rngb = malloc(sizeof(*rngb)); 
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
	//check for DS begin
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
        char cache[4];
        for(int i = 0; i<4; i++)
        {
          cache[i] = rngb->fifo[rngb->readIndex];
          rngb->readIndex = (rngb->readIndex + 1) % (FIFO_SIZE + 1);
        }
        data = (uint32_t)cache;  
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

/*
long int gsParseRawData(gsDataSet * p, FILE *file_p, long int offset){
	fread(p->header,1,8,file_p);
	fread(&(p->number),1,2,file_p);
	fread(&(p->lenght),1,2,file_p);
	fseek(file_p,p->lenght*WORD,SEEK_CUR);
	fread(&(p->checksum),1,WORD,file_p);
	p->data_position = ftell(file_p);
	return ftell(file_p);
	}
*/

/*uint32_t gsCheckChecksum(gsDataSet * p, FILE *file_p ){
	uint32_t data_field[p->lenght];
	for(int i; i <= p->lenght;i++){
		fread(&(data_field[i]),1,WORD,file_p);
		}
	
	}
*/

int gsParse0x20(ringbuffer_t *rngb, gs_0x20 *ds, char nmbr)
{
	if(ds == NULL)
		return -1;
    uint32_t *cache;
    gsRngbMoveRead(rngb, 2*WORD);
    gsRngbReadWord(rngb, cache);
    if(cache == NULL)
        return -2;
    ds->length = ((uint16_t*)cache)[0];
    ds->msg_type = ((uint16_t*)cache)[1];
    gsRngbReadWord(rngb, cache);
    if(rngb->dsLenghth[nmbr] != *cache)
        return -2;
}


	/* old parse snippet
	for(int i = 0; i<5; i++){
		fread(&(ds0x22->val_double[i]),1,2*WORD,file_p);
		}
	for(int i = 0; i<2; i++){
		fread(&(p0->val_uint[i]),1,WORD,file_p);
		}
	for(int i = 7; i<12; i++){
		fread(&(p0->val_double[i]),1,2*WORD,file_p);
		}
	for(int i = 12; i<14; i++){
		fread(&(p0->val_uint[i]),1,WORD,file_p);
		}
	for(int i = 14; i<16; i++){
		fread(&(p0->val_double[i]),1,2*WORD,file_p);
		}*/

int32_t gsGetNumberDataSet(FILE *file_p){
	if(file_p == NULL)
		return -1;

	int32_t ds_count = 0;
	uint32_t lenght = 0;
	uint32_t number = 0;
	size_t ds_read;

	do{
		fseek(file_p,2*WORD,SEEK_CUR);
		ds_read = fread(&number,1,2,file_p);
		fread(&lenght,1,2,file_p);
		fseek(file_p,lenght*WORD+MSG_CHECKSUM,SEEK_CUR);
		ds_count++;
		printf("Nummer code %x \n",number);
	}while(ds_read > 0);

	return ds_count;
}
