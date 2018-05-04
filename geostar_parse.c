#include "geostar_parse.h"

ringbuffer_t *gsRngbInitialize()
{
	ringbuffer_t *ringbuffer;
	ringbuffer->readIndex = -1;
	ringbuffer->writeIndex = 0;
	ringbuffer->dsPos = -1;
	return ringbuffer; 
}

int gsRngbAppend(char *data, ringbuffer_t *buffer)
{
	if(buffer)
	{
	buffer->fifo[buffer->writeIndex] = *data;
	//check for DS begin
	if (*data == 'E')
		if (buffer->fifo[buffer->writeIndex - 1] == 'G')
			buffer->dsPos = buffer->writeIndex - 1;
	buffer->writeIndex = buffer->writeIndex++ % (FIFO_SIZE + 1);
	if(buffer->readIndex == buffer->writeIndex)
		buffer->readIndex = buffer->readIndex ++ % (FIFO_SIZE + 1);
	}
    else
    return -1;
}

int gsRngbRead(char *data, ringbuffer_t *buffer)
{
	if(buffer)
	{
		if(buffer->readIndex != buffer->writeIndex)
		{
			*data = buffer->fifo[buffer->readIndex];
			buffer->readIndex = buffer->readIndex++ % (FIFO_SIZE + 1);
			return 1;
		}
		else
			return -1;
	} else
		return -1;  
}

/* start searching for a dataset from the current writeIndex    *
 * return statu:                                                */
int gsRngbSearch(ringbuffer_t *rngb)
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

int checkDSComplete(ringbuffer_t *buffer)
{
	uint16_t lenghth = buffer->dsPos + 6;
	if (buffer->dsPos + lenghth < buffer->writeIndex)
//		gsCheckSum() gs checsum has to be rewroitten for the use of buffer.
//		add abstaction layer file io/data
		return 1;
}

uint32_t gsGenerateChecksum(uint32_t data_field[], uint16_t lenght){
	if (lenght < 2){
		return (data_field[1] ^ data_field[0]);
		}
	return data_field[lenght] ^ gsGenerateChecksum(data_field, lenght-1); 
	}

long int gsParseRawData(gsDataSet * p, FILE *file_p, long int offset){
	fread(p->header,1,8,file_p);
	fread(&(p->number),1,2,file_p);
	fread(&(p->lenght),1,2,file_p);
	fseek(file_p,p->lenght*WORD,SEEK_CUR);
	fread(&(p->checksum),1,WORD,file_p);
//	p->data_position = ftell(file_p);
	return ftell(file_p);
	}

uint32_t gsCheckChecksum(gsDataSet * p, FILE *file_p ){
//	fseek(file_p, p->data_position + 3*WORD, SEEK_SET);
	uint32_t data_field[p->lenght];
	for(int i; i <= p->lenght;i++){
		fread(&(data_field[i]),1,WORD,file_p);
		}
	
	}

uint32_t gsParseGCBDS(gsDataSet * p, gsDataSet_0x20 *p0, FILE *file_p ){
	if (file_p == NULL)
		return -1;

//	fseek(file_p, p->data_position+3*WORD,SEEK_SET); 
	
	for(int i = 0; i<5; i++){
		fread(&(p0->val_double[i]),1,2*WORD,file_p);
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
		}
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
