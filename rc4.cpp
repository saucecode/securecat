#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include "rc4.h"

/*
	void* rc4(void* data, unsigned int data_len, void* _key, unsigned int key_len);
	Implementation of the arc-4 stream cipher for raw byte data.
	Returns a pointer on the heap of the same size data_len - don't forget to free it!
*/
void* rc4(void* _data, unsigned int data_len, void* _key, unsigned int key_len){
	unsigned char* data = (unsigned char*) _data;
	unsigned char* key = (unsigned char*) _key;
	int x = 0;
	unsigned char* box = (unsigned char*) calloc(256, 1);
	for(int i=0; i<256; i++) box[i] = i;
	for(int i=0; i<256; i++){
		x = (x + box[i] + (int)( key[i % key_len] )) % 256;
		//x = (x + box[i] + (int)(key[i % key_length]) ) % 256;
		box[i] ^= box[x];
		box[x] ^= box[i];
		box[i] ^= box[x];
	}
	x = 0;
	int y = 0;
	unsigned char* output = (unsigned char*) calloc(data_len, 1);
	for(int i=0; i<data_len; i++){
		unsigned char ch = data[i];
		x = (x + 1) % 256;
		y = (y + box[x]) % 256;
		
		box[x] ^= box[y];
		box[y] ^= box[x];
		box[x] ^= box[y];
		
		output[i] = (unsigned char) ((int)(ch) ^ box[(box[x] + box[y]) % 256]);
	}
	free(box);
	return (void*) output;
}


/*int main(){
	char* message = "Attack At Dawn";
	char* key = "Secret";
	
	char* data = (char*) rc4(message, strlen(message), key, strlen(key));
	//printf("%s", (char*) message);
	for(int i=0; i<strlen(message); i++){
		printf("%hhx", (int)data[i]);
	}
	printf("\n");
	//printf("%s", (char*) rc4(data, strlen(message), key, strlen(key)));
	
	return 0;
}
*/
