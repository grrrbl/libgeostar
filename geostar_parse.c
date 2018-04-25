#include "geostar_parse.h"

size_t gsParseRawData(gsDataSet * p, FILE *file_p ){
	size_t pos;
	fread(p->header,1,8,file_p);
	fread(&(p->number),1,2,file_p);
	pos = fread(&(p->lenght),1,2,file_p);
	return pos;
	}

