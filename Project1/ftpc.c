/*
   FILE: FTPC Program (Client Application)
*/

#include <netdb.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "packet.h"

/*********** COMMUNICATION PORTS **********/
#define ftpc_M2_PORT 2468
#define M2_ftpc_PORT 8642

main(int argc, char *argv[])
{
  int ftpc_M2_sock, M2_ftpc_sock;
  char buf[1000] ;
  char buff[20];
  struct sockaddr_in fptc_M2_name;		 				// ftpc to M2 Communication 
  struct sockaddr_in M2_ftpc_name; 						// M2 to ftpc Communication 
  struct hostent *hp, *gethostbyname();

  /********** FLAGS AND NECESSARY VARIABLES *************/
  int flag = 0;
  int flags;
  int zero_byte_flag = 0;
  socklen_t namelen;
    
  /********* FILE DISCRIPTORS AND VARIABLES **********/
  int filesz;           // File Size Variable 
  FILE *fp;		// File Name

  /********** DATA AND ACKNOWLEDGEME PACKET INITAILIZATION ***********/
  struct data_packet *packet;		// DATA PACKET
  struct data_packet *ack_packet;		
	
  packet = calloc(1,sizeof(struct data_packet));	
  ack_packet = calloc(1,sizeof(struct data_packet)); 	
	
  /*********** CRC COMPUTATION INITIALIZATOIN ***************/
  crcInit();				// CRC initialization
  namelen = sizeof(struct sockaddr_in);	// namelen initialization


	
  /******* MAIN PROGRAM *********/
	
  if(argc < 2) {
    printf("Usage: ftpc filename\n");
    exit(1);
  }
	
  /********* CREATE SOCK TO SEND TO TCPD_M2 **********/
  ftpc_M2_sock = socket(AF_INET, SOCK_DGRAM,0);
  if(ftpc_M2_sock < 0) {
    perror("opening datagram socket ftpc_M2_sock");
    exit(2);
  }
  fptc_M2_name.sin_family = AF_INET;
  fptc_M2_name.sin_port = htons(ftpc_M2_PORT); 
  hp = gethostbyname("localhost");
  if(hp == 0) {
    fprintf(stderr, "%s:unknown host\n", argv[1]);
    exit(3);
  }
  bcopy((char *)hp->h_addr, (char *)&fptc_M2_name.sin_addr, hp->h_length);

  /****** CREATE SOCK TO RECEIVE FROM TCPD_M2 **********/
  M2_ftpc_sock = socket(AF_INET, SOCK_DGRAM,0);
  if(M2_ftpc_sock < 0) {
    perror("opening datagram socket M2_ftpc_sock");
    exit(2);
  }
  flags = fcntl(M2_ftpc_sock, F_GETFL);
  flags |= O_NONBLOCK;
  fcntl(M2_ftpc_sock, F_SETFL, flags);

  M2_ftpc_name.sin_family = AF_INET;
  M2_ftpc_name.sin_port = htons(M2_ftpc_PORT);
  hp = gethostbyname("localhost");
  if(hp == 0) {
    fprintf(stderr, "unknown host\n");
    exit(3);
  }
  bcopy((char *)hp->h_addr, (char *)&M2_ftpc_name.sin_addr, hp->h_length);
  if(bind(M2_ftpc_sock, (struct sockaddr *)&M2_ftpc_name, sizeof(M2_ftpc_name)) < 0) { 		// binding listning socket to listing name
    perror("getting socket name");
    exit(2);
  }
	
  /********** FILE MANIPULATION ***********/
	
  if(strlen(argv[1]) > 20) {
    fprintf(stderr, "Please Specify File Name less than 20 Characters...\n");
  }
  if((fp = fopen(argv[1],"rb")) < 0){ 
    fprintf(stderr,"File not found \n");
    return 0;
  }
	
  /******* SENDING FILE SIZE TO M2 ***************/

  fseek(fp,0,SEEK_END);
  filesz = (int) ftell(fp);
  rewind(fp);
	
  bzero(packet->payload, 1000);
  memcpy(packet->payload,&filesz,sizeof(int));
  packet->sequence_number = 0;
  bzero(buf,1000);
  memcpy(buf,packet,1000);
  packet->checksum = crcFast(buf,1000);
	
  fprintf(stdout,"Packet Sequence_Number:%d\tChecksum:%hu\n",packet->sequence_number,packet->checksum);
  if(sendto(ftpc_M2_sock, packet,sizeof(struct data_packet), 0, (struct sockaddr *)&fptc_M2_name, sizeof(fptc_M2_name)) <0) {
    perror("SENDING FILE SIZE DATAGRAM TO M2");
    exit(1);
  }


  /******* SENDING FILE NAME TO M2 ***************/

  bzero(packet->payload,1000);
  memcpy(packet->payload,argv[1],strlen(argv[1]));
  packet->sequence_number = 1;
  bzero(buf,1000);
  memcpy(buf,packet->payload,1000);
  packet->checksum = crcFast(buf,1000);
	
  fprintf(stdout,"Packet data:%s\tSeq_Number:%d\tChecksum:%hu\n",packet->payload,packet->sequence_number,packet->checksum);
  if(sendto(ftpc_M2_sock, packet,sizeof(struct data_packet), 0, (struct sockaddr *)&fptc_M2_name, sizeof(fptc_M2_name)) <0) {
    perror("SENDING FILE SIZE DATAGRAM TO M2");
    exit(1);
  }
	
  /******* SENDING FILE DATA TO M2 ***************/

  while(1){
    int bytes_sent = 0;
    int bytes_read;
		
    sleep(1);
    if(flag == 0){
      bzero(packet->payload,1000);
      bytes_read = fread(packet->payload, sizeof(char),sizeof(packet->payload),fp);

      if(bytes_read > 0){
	packet->sequence_number = packet->sequence_number+1;
	bzero(buf,1000);
	memcpy(buf,packet->payload,1000);
	packet->checksum = crcFast(buf,1000);

	printf("\nClient sends:- Packet Sequence_Number: %d\tChecksum : %hu\n",packet->sequence_number,packet->checksum);
	bytes_sent = sendto(ftpc_M2_sock, packet,sizeof(struct data_packet), 0, (struct sockaddr *)&fptc_M2_name, sizeof(fptc_M2_name)) ;
	if (bytes_sent < 0) {
	  perror("ERROR IN SENDING FILE DATAGRAM TO M2");
	  exit(1);
	}
      }
      else if(bytes_read < 0 ){
	fprintf(stderr,"\n ERROR IN READING DATA FROM THE FILE \n");
      }
      else if(bytes_read == 0){
	sleep(1);
	flag =1;
	if(zero_byte_flag == 0){
	  zero_byte_flag = 1;
	  fprintf(stdout, "\nFILE READ COMPLETE...\n");
	  packet->FYN = '1';
	  packet->sequence_number = packet->sequence_number+1;
	  bzero(buf,1000);
	  memcpy(buf,packet->payload,1000);	
	  packet->checksum = crcFast(buf,1000);

	  printf("\nClient sends:- FYN PACKET WITH SEQ_NO:\t%d",packet->sequence_number);
	  if(sendto(ftpc_M2_sock, packet,sizeof(struct data_packet), 0, (struct sockaddr *)&fptc_M2_name, sizeof(fptc_M2_name)) <0) {
	    perror("ERROR IN SENDING FYN PACKET TO M2");
	    exit(1);
	  }
	}
      }
    }
		
    if(recvfrom(M2_ftpc_sock, buff, sizeof(buff), 0, (struct sockaddr *)&M2_ftpc_name, &namelen) > 0){
      if(strcmp(buff, "acked") == 0){
	printf("\n*****FYN PACKED IS ACKED FROM SEVER WAIT FOR 2 MSL AND SHUTDOWN****\n");
	sleep(2);
	fprintf(stdout, "\n\n....CONNECTION SHUTDOWN...\n");
	break;
      }
      else if(strcmp(buff, "full") == 0){
	printf("\n*****BUFFER FULL CONTROL MESSAGE FROM TCPD_M2****\n");
	flag = 1;
      }
      else{
	flag = 0;
      }
    }
  }	
}
