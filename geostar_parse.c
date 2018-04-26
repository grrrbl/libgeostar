#include "geostar_parse.h"
uint32_t gsGenerateChecksum(uint32_t data_field[], uint16_t lenght){
	if (lenght < 2){
		return (data_field[1] ^ data_field[0]);
		}
	return data_field[lenght] ^ gsGenerateChecksum(data_field, lenght-1); 
	}

size_t gsParseRawData(gsDataSet * p, FILE *file_p ){
	size_t pos;
	fread(p->header,1,8,file_p);
	fread(&(p->number),1,2,file_p);
	pos = fread(&(p->lenght),1,2,file_p);
	return pos;
	}

uint32_t gsChecksum(gsDataSet * p, FILE *file_p ){
	fseek(file_p, p->data_position + 3*WORD, SEEK_SET);
	uint32_t data_field[p->lenght];
	for(int i; i <= p->lenght;i++){
		fread(&(data_field[i]),1,WORD,file_p);
		}
	
	}

uint32_t gsParseGCBDS(gsDataSet * p, gsDataSet_0x20 *p0, FILE *file_p ){
	if (file_p == NULL)
		return -1;

	fseek(file_p, p->data_position,SEEK_SET); 
	
	for(int i = 0; i<5; i++){
		fread(&(p0->val_double[i]),1,2*WORD,file_p);
		}
	for(int i = 5; i<7; i++){
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
