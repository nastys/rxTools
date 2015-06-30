#include <stdio.h>
#include <stdlib.h>
#ifdef __APPLE__
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#include <string.h>

typedef struct{
	unsigned int off;
	unsigned int size;
	unsigned int hash;
	unsigned int edited;
	char name[16];
}PackEntry;

unsigned int HashGen(unsigned char* file, unsigned int size){ //BSD checksum
	unsigned int HASH = 0;
	unsigned int i;
	for(i = 0; i < size; HASH = (((HASH >> 1) | (HASH << 15)) + file[i++]) & 0xffff);
	return HASH;
}

char* getFileName(char* fn){
	char* ret = fn;
	while(*fn != 0){
		fn++;
		if(*fn == '\\' || *fn == '/') ret = fn + 1;
	}
	return ret;
}

int main(int argc, char** argv){
	unsigned int i;
	if(argc < 3) return -1;
	FILE* out = fopen(argv[argc-1], "wb");
	if(!out){
		printf("Cannot create %s!\n", argv[argc-1]);
		return -1;
	}
	unsigned int nfiles = argc - 2;
	if(nfiles == 0){
		printf("No files specified to pack!");
	}
	fwrite(&nfiles, 1, 4, out); 
	for(i = 0; i < 12; i++) 
		fputc(0x00, out);
	for(i = 0; i < nfiles*sizeof(PackEntry); i++) fputc(0x00, out);
	for(i = 0; i < nfiles; i++){
		FILE* file = fopen(argv[1 + i], "rb");
		if(!file){
			printf("Cannot open %s!\n", argv[1 + i]);
			return -1;
		}
		unsigned int size;
		fseek(file, 0, 2); size = ftell(file); rewind(file);
		unsigned char* buf = (unsigned char*) malloc (size);
		fread(buf, 1, size, file);
		fclose(file);
		fseek(out, 0, 2);
		unsigned int off = ftell(out);
		fwrite(buf, 1, size, out);
		PackEntry Entry;
		
		Entry.size = size; Entry.off = off;
		Entry.hash = HashGen(buf, size); Entry.edited = 0;
		memset(Entry.name, 0, 16); strncpy(Entry.name, getFileName(argv[1 + i]), 16);
		printf("%s	\n", getFileName(argv[1 + i]));
		fseek(out, i*sizeof(PackEntry) + 0x10, 0); fwrite(&Entry, 1, sizeof(PackEntry), out);
		free(buf);
	}
	fclose(out);
	return 0;
}
