#include "read_bin.h"

int main(){
	unsigned char buffer[512];
	uint32_t seek = 0; 

	for(int i = 0; i<sizeof(buffer);i++)
	buffer[i]='0';
	FILE *hex_file;

	hex_file = fopen("foo","rb");

	// read one word (32bit) is fread(buffer,1,4,hex_file)
	fread(buffer,1,8,hex_file);
	
	for(int i = 0; i<8; i++){
		printf("%c ", buffer[i]);
	}
	printf("\n");

	uint16_t lenght;
	uint16_t number;
	fread(&number,1,2,hex_file);
	fread(&lenght,1,2,hex_file);

/*	for(int i = 0; i<512; i++){
		printf("%c ", buffer[i]);
	}
*/
	printf("Länge Datensatz: %u \n", lenght);
	printf("Typ Datensatz: %#x \n", number);

	fseek(hex_file, WORD*lenght+CHECKSUM, SEEK_CUR);
	fread(buffer,1,8,hex_file);
	
	for(int i = 0; i<8; i++){
		printf("%c ", buffer[i]);
	}
	printf("\n");
	fread(&number,1,2,hex_file);
	fread(&lenght,1,2,hex_file);

	printf("Länge Datensatz: %u \n", lenght);
	printf("Typ Datensatz: %#x \n", number);
}
