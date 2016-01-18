/*
   FILE: circularbuffer Program (Circular Buffer Library)
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
 
typedef struct {
	int size; 
	int start;
	int end;  
	data_packet *packets[21]; 
} CircularBuffer;


void cbInit(CircularBuffer *cb){
	int i;
	cb->size = 21;
	cb->start = 0;
	cb->end = 0;
	for(i = 0 ; i < 21 ; i++){
		cb->packets[i]= (data_packet *)calloc(1, sizeof(data_packet));
		cb->packets[i]->sequence_number = -1 ;
	}
}

void cbFree(CircularBuffer *cb, int indx) {
     if(cb->packets[indx] != NULL){
		cb->packets[indx] = NULL; 
	}
	else{
		return;
	}
}
	
int cbIsFull(CircularBuffer *cb) {
	return (cb->end + 1) % cb->size == cb->start; 
}
 
int cbIsEmpty(CircularBuffer *cb) {
    return cb->end == cb->start; 
}
 
 
 
void cbWrite(CircularBuffer *cb, data_packet *packet) {
	if(cb->packets[(cb->end%20)] == NULL){
		cb->packets[(cb->end%20)] = (data_packet *)calloc(1, sizeof(data_packet));
	}
	
	strcpy(cb->packets[(cb->end%20)]->payload,packet->payload);
	cb->packets[(cb->end%20)]->sequence_number = packet->sequence_number;
	cb->packets[(cb->end%20)]->FYN = packet->FYN;
	cb->packets[(cb->end%20)]->checksum = packet->checksum;
	
	cb->end = ((cb->end%20) + 1); //% (cb->size - 1)
	printf("CIRCULAR BUFFER WRITE: [Start: %d, End: %d]\n", cb->start, cb->end);	
} 

data_packet * cbRead(CircularBuffer *cb, int index) {
	if(cbIsEmpty(cb)) return;
	if(cb->packets[index %(cb->size-1)] == NULL) {
		printf("CBREAD : NULL found ! No Packets at the location : %d \n",index);
		return;
	}
	return cb->packets[(index %(cb->size-1))];
}

void cbAdjustStart(CircularBuffer *cb){
	while(cb->packets[cb->start] == NULL){
		cb->start = (cb->start+1) % (cb->size-1);
	}
}

void cbRemove(CircularBuffer *cb, int index){
	int loc;
	if(cbIsEmpty(cb))
		return;
	loc = (index) % (cb->size-1);
	if(cb->packets[loc] != NULL){
		cbFree(cb, loc);
		cbAdjustStart(cb);
		printf("CIRCULAR BUFFER REMOVE: [Start: %d, End: %d]\n", cb->start, cb->end);
		fflush(stdout);
	}
	else{
		return;
	}
}
