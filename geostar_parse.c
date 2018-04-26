#include "geostar_parse.h"

size_t gsParseRawData(gsDataSet * p, FILE *file_p ){
	size_t pos;
	fread(p->header,1,8,file_p);
	fread(&(p->number),1,2,file_p);
	pos = fread(&(p->lenght),1,2,file_p);
	return pos;
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
	}while(ds_read > 0);

	return ds_count;
}
