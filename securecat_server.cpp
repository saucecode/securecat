#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include "dyad.h"
#include "rc4.h"

struct Connection;
typedef struct Connection Connection;

void onAccept(dyad_Event* e);
void onListen(dyad_Event* e);
void onError(dyad_Event* e);
void onData(dyad_Event* e);
void onDisconnect(dyad_Event* e);
int getFreeSlot();
int getConnection(dyad_Stream* stream);

const char NETWORK_NAME[] = "bexnet";
// ------

struct Connection {
	char* ip;
	char* username;
	int port;
	dyad_Stream* stream;
};

Connection* connections[32] = {NULL};

void onAccept(dyad_Event* e){
	printf("Received connection from %s\n", dyad_getAddress(e->remote));
	int slot = getFreeSlot();
	if(slot == -1){
		printf("Dropping connection - server is full.\n");
	}else{
		connections[slot] = (Connection*) calloc(1, sizeof(Connection));
		const char* addr = dyad_getAddress(e->remote);
		connections[slot]->ip = (char*) malloc(strlen(addr)+1);
		connections[slot]->port = dyad_getPort(e->remote);
		connections[slot]->stream = e->remote;
		memcpy(connections[slot]->ip, addr, strlen(addr));
		dyad_addListener(e->remote, DYAD_EVENT_DATA, onData, NULL);
		dyad_addListener(e->remote, DYAD_EVENT_CLOSE, onDisconnect, NULL);
	}
}

void onListen(dyad_Event* e){
	printf("Server listening.\n");
}

void onError(dyad_Event* e){
	printf("** SERVER ERROR: %s\n", e->msg);
}

void onData(dyad_Event* e){
	printf("Received data (%i) from connection %i.\n", e->size, getConnection(e->stream));
	for(int i=0; i<32; i++){
		if(connections[i] == NULL) continue;
		dyad_write(connections[i]->stream, e->data, e->size);
	}
}

void onDisconnect(dyad_Event* e){
	int id = getConnection(e->stream);
	printf("Disconnect from Id %i %s:%i\n", id, dyad_getAddress(e->stream), dyad_getPort(e->stream));
	free(connections[id]->ip);
	free(connections[id]);
	connections[id] = NULL;
}

int getFreeSlot(){
	for(int i=0; i<32; i++){
		if(connections[i] == NULL) return i;
	}
	return -1;
}

int getConnection(dyad_Stream* stream){
	const char* ip = dyad_getAddress(stream);
	int port = dyad_getPort(stream);
	for(int i=0; i<32; i++){
		if(connections[i] == NULL) continue;
		if(!strcmp(connections[i]->ip, ip) && connections[i]->port == port) return i;
	}
	return -1;
}

int main(int argc, char** argv){
	dyad_Stream *s;
	dyad_init();
	s = dyad_newStream();
	
	dyad_addListener(s, DYAD_EVENT_ERROR, onError, NULL);
	dyad_addListener(s, DYAD_EVENT_ACCEPT, onAccept, NULL);
	dyad_addListener(s, DYAD_EVENT_LISTEN, onListen, NULL);
	dyad_listen(s, 9998);
	
	while (dyad_getStreamCount() > 0) {
		dyad_update();
	}
	
	return 0;
}
