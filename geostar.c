#include "geostar.h"
const char *MSG_BEGIN = "GEOSr2PS";

void gsRngbInitialize(ringbuffer_t *ringbuffer)
{
	ringbuffer->readIndex = 0;
	ringbuffer->writeIndex = 0;
	ringbuffer->dsPos = -1;
}

int8_t gsRngbAppend(ringbuffer_t *buffer, char word)
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
            if(buffer->fifo[buffer->writeIndex - i] == MSG_BEGIN[6-i])
                count++;
            }
		if (count > 5)
            buffer->dsPos = buffer->writeIndex - 6;
        }
    
    buffer->writeIndex = (buffer->writeIndex + 1) % (FIFO_SIZE + 1);
	if(buffer->readIndex == buffer->writeIndex)
		buffer->readIndex = (buffer->readIndex + 1) % (FIFO_SIZE + 1);
	}
    else{
        return -1;
    }
}

int8_t gsRngbRead(ringbuffer_t *buffer, char *dataset)
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

int8_t gsRngbReadChar(ringbuffer_t *rngb, char *data)
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


/* start searching for a dataset from the current writeIndex    *
 * return statu:                                                */
int16_t gsRngbSearch(ringbuffer_t *rngb)
{
    uint16_t pos = rngb->writeIndex;
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

int16_t gsRngbDataSetComplete(ringbuffer_t *rngb)
{
    /* chekc if it's save to run function                               */
    if (!rngb)
        return -2;

    if (rngb->writeIndex <= rngb->dsPos + 16)
        return -1;

    /* write data to local cache, so pointer arithemitc doesn't break   *
     * at the end of the ringbuffer (writeIndex - 12 > FIFO_SIZE)       *
     * Save ringbuffer position and reset after reading, so we don't    *
     * change teh ringbuffer read position                              */
    char data[18];
    int32_t save_readIndex = rngb->readIndex;
    rngb->readIndex = rngb->dsPos;
    for (int i = 0; i < sizeof(data); i++)
        gsRngbReadChar(rngb, &data[i]);
    rngb->readIndex = save_readIndex;    

    /* change type of data to type of var lenghth (16 bit), skip first  *
     * bytes with the header titel and set lenghth                      */
    uint16_t lenghth, msg_type;
    lenghth = ((uint16_t *)data)[5];
    msg_type = ((uint16_t *)data)[4];
    if (lenghth > 0){
        printf("%x \n", msg_type);
        rngb->dsLenghth = (int32_t)lenghth;
        return lenghth;
    }
    else
        return -2;
}

int checkDSComplete(ringbuffer_t *buffer)
{
	uint16_t lenghth = buffer->dsPos + 6;
	if (buffer->dsPos + lenghth < buffer->writeIndex)
//		gsCheckSum() gs checsum has to be rewroitten for the use of buffer.
//		add abstaction layer file io/data
		return 1;
}

uint32_t gsGenChecksum(char *dataset, int16_t lenghth){
    if (lenghth > MAX_MESSAGE_LENGHTH || (sizeof(dataset) / 4) < lenghth)
        return 0;
    if (lenghth < 2){
        //return (((uint32_t *)dataset)[1] ^ dataset_typecast[0]);
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
int8_t gsParse0x20(char *dataset, gsDataSet_0x20 *ds0x22)
{
	if (dataset == NULL)
		return -1;

//	fseek(file_p, p->data_position+3*WORD,SEEK_SET); 
	/*
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
	}

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
		fseek(file_p,lenght*WORD+CHECKSUM,SEEK_CUR);
		ds_count++;
		printf("Nummer code %x \n",number);
	}while(ds_read > 0);

	return ds_count;
}
