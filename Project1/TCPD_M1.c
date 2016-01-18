/*
   FILE: TCPD_M1 Program (Server Application)
*/

#include <netdb.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "packet.h"
#define MAX_MES_LEN 1024

/******** FTPS TO TCPD_M1 COMMUNICATION PORT ******/
#define FTPS_M1_PORT 2009

/******* TCPD_M1 TO TROLLS COMMUNICATION ********/
#define M1_TROLL_PORT 5009
#define M1_TROLL_PORT_BUF "6009"
#define TROLL_ADDR "gamma"

recv_bufferInit(data_packet recv_buffer[], int size){
  int i;
  for(i = 0; i < size; i++){
    recv_buffer[i].sequence_number = -3;
  }
}

int Isrecv_bufferEmpty(data_packet recv_buffer[] , int size){
  int i, flag;
  for(i = 0 ; i < size; i++){
    if(recv_buffer[i].sequence_number != -3){
      flag = 0;
      break;
    }
  }
  return flag;
}

/* TCPD_M1 Program called with No Arguments */
main()
{
  int ftps_M1_sock, troll_M1_sock, M1_troll_sock;
  int namelen, buflen;
  char TCPD_M1_buf[MAX_MES_LEN];
  struct sockaddr_in ftps_M1_name;
  struct sockaddr_in troll_M1_name;
  struct sockaddr_in M1_listen_name;
  struct sockaddr_in M1_comm_name;
  struct sockaddr_in M1_troll_name;
  struct hostent *hp, *gethostbyname();
  
  /******** DATA AND ACKNOWLEDGEMENT PACKET ******/
  data_packet *packet, *ack_packet, *FYN_packet;
  packet = calloc(1,sizeof(data_packet));
  ack_packet = calloc(1,sizeof(data_packet));
  FYN_packet = calloc(1,sizeof(data_packet));

  /********** RECEIVER BUFFER *********/
  data_packet recv_buffer[1000] ; 
  int recv_buffer_head = 0;

  int loopj=0;
  /*
    int loopi = 0 ; 

  for (loopi = 0 ; loopi < 1000; loopi++){
    recv_buffer->sequence_number = -3;
  }
  */

  /********** create socket for connecting to FTPS ************/
  ftps_M1_sock = socket(AF_INET, SOCK_DGRAM,0);
  if(ftps_M1_sock < 0) {
    perror("opening datagram socket");
    exit(2);
  }

  /********* construct name for connecting to FTPS **********/
  ftps_M1_name.sin_family = AF_INET;
  ftps_M1_name.sin_port = htons(FTPS_M1_PORT);
  ftps_M1_name.sin_addr.s_addr = INADDR_ANY;

  if(bind(ftps_M1_sock, (struct sockaddr *)&ftps_M1_name, sizeof(ftps_M1_name)) < 0) {
    perror("getting socket name");
    exit(2);
  }
	
  /*create a sock for M1 to Troll */
  M1_troll_sock = socket(AF_INET, SOCK_DGRAM,0);
  if(M1_troll_sock < 0) {
    perror("opening datagram socket: for reverse communication from M2 to Troll");
    exit(2);
  }
    
  /* construct name for connecting M1 to TROLL */
  bzero((char*)&M1_comm_name, sizeof M1_comm_name);
  M1_comm_name.sin_family = AF_INET;
  M1_comm_name.sin_port = htons(M1_TROLL_PORT);
  M1_comm_name.sin_addr.s_addr = INADDR_ANY;
  
  /* Copy Source Address and Source Port Number into Socket(M1_troll_sock) */
  if(bind(M1_troll_sock, (struct sockaddr *)&M1_comm_name, sizeof(M1_comm_name)) < 0) {
    perror("getting socket name");
    exit(2);
  }
  /* construct name for connecting to TROLL */
  bzero ((char*)&M1_troll_name, sizeof M1_troll_name);
  M1_troll_name.sin_family = AF_INET;
  M1_troll_name.sin_port = htons(atoi(M1_TROLL_PORT_BUF));
  
  /* convert hostname to IP address and enter into name */
  hp = gethostbyname(TROLL_ADDR);
  if(hp == 0) {
    fprintf(stderr, "%s:unknown host\n",TROLL_ADDR);
    exit(3);
  }
  bcopy((char *)hp->h_addr, (char *)&M1_troll_name.sin_addr, hp->h_length);
    
  /* Variable Size Initialization */
  namelen=sizeof(struct sockaddr_in);
  buflen = MAX_MES_LEN;
  crcInit();

  /* waiting for connection from client on name and print what client sends */

  if(recvfrom(ftps_M1_sock, packet, sizeof(data_packet), 0, (struct sockaddr *)&ftps_M1_name, &namelen) < 0) {
    perror("error receiving"); 
    exit(4);
  }
  printf("TCPD_M1 receives Packet from FTPS (ListenPort):\t%s\n", packet->payload);

  /************* REMOTE COMMUNICATION INITIALIZATION **********/
  /* create socket for connecting to troll */
  troll_M1_sock = socket(AF_INET, SOCK_DGRAM,0);
  if(troll_M1_sock < 0) {
    perror("opening datagram socket");
    exit(2);
  }

  /* construct name for local port and address */
  M1_listen_name.sin_family = AF_INET;
  M1_listen_name.sin_port = htons(atoi(packet->payload));
  M1_listen_name.sin_addr.s_addr = INADDR_ANY;

  if(bind(troll_M1_sock, (struct sockaddr *)&M1_listen_name, sizeof(M1_listen_name)) < 0) {
    perror("Unable to bind (TROLL_to_M1 Sock)");
    exit(2);
  }

  fprintf(stdout,"Waiting for connection from client via troll at %d ...\n",ntohs(M1_listen_name.sin_port));
    
  /************** INFORMATION RECIEPT ************/
 
  /********* FILE SIZE **************/
  recv_bufferInit(recv_buffer, sizeof(recv_buffer)/sizeof(data_packet));
    
  while(1){
		
    if(recvfrom(troll_M1_sock, packet, sizeof(data_packet), 0, (struct sockaddr *)&troll_M1_name, &namelen) < 0) {
      perror("error receiving"); 
      exit(4);
    }
    printf("TCPD_M2 --> TCPD_M1 :: RECVD PACKET-> Sequence Number: %d\t", packet->sequence_number);

    /************** DATA PACKET MANIPULATION ************/

    if(packet->sequence_number <= 2000 && packet->FYN != '1'){
    /******* CRC COMPUTATION **********/
      if(crcFast(packet->payload,1000) == packet->checksum) {

	printf("Packet Health: GOOD\n");

	/***** ACK COMMUNICATION *********/
	ack_packet->sequence_number = packet->sequence_number ;

	/******** ADDING INTO RECEIVER BUFFER ********/
	if(recv_buffer[packet->sequence_number].sequence_number == -3){
	  recv_buffer[packet->sequence_number] = *packet ; 
	}

	/************ SENDING ACK FOR VALID DATA PACKET***********/
	  printf("TCPD_M1 --> TCPD_M2 :: SEND ACK_PACKET-> sequence_number: %d \n",ack_packet->sequence_number);
	  if(sendto(M1_troll_sock, ack_packet, sizeof(data_packet), 0, (struct sockaddr *)&M1_troll_name, sizeof(M1_troll_name)) < 0) {
	    perror("error Sending ack");
	    exit(4);
	  }

	/*********** ORDERED BYTES TRANSFER TO FTPC FROM TCPD_M1 ***********/
	  if(recv_buffer[recv_buffer_head].sequence_number != -3){
	  /*************** INITIATING COMMUNICATION BETWEEN TCPD_M1 AND FTPS **********/
	  printf("TCPD_M1 --> FTPS :: SEND DATA_PACKET-> sequence number: %d\n\n", recv_buffer[recv_buffer_head].sequence_number); 
	  if(sendto(ftps_M1_sock, &recv_buffer[recv_buffer_head], sizeof(data_packet), 0, (struct sockaddr *)&ftps_M1_name, sizeof(ftps_M1_name)) < 0) {
	    perror("error Sending");
	    exit(4);
	  }
	  recv_buffer[recv_buffer_head].sequence_number = -3;
	  recv_buffer_head++; // MOVING HEAD TO NEXT AVAILABLE PACKET IN RECEIVER BUFFER
	}
      }
      else {
	printf("Packet Health: CORRUPTED\n");
	printf("TCPD_M2 --> TCPD_M1 :: DROP DATA_PACKET-> sequence_number: %d\n",packet->sequence_number);
      }
    }

    /********** EXPLICIT CHECKS FOR FYN, FYN+ACK PACKETS *************/
    else if(packet->FYN == '1'){ 

      FYN_packet->FYN = '1';

      /******* CRC COMPUTATION **********/
      if(crcFast(packet->payload,1000) == packet->checksum){
  
	printf("Packet Health: GOOD\n");

	/***** ACK COMMUNICATION *********/
	ack_packet->sequence_number = packet->sequence_number ;

	/*****CONNECTION SHUTDOWN CHECK AND CORRESPONDING ACKS***/
	if(packet->FYN == '1' && ack_packet->sequence_number == 1){
	  printf("TCPD_M1 --> TCPD_M2 :: RECVD ACK_PACKET-> sequence_number: %d\n",ack_packet->sequence_number);
	  if(Isrecv_bufferEmpty(recv_buffer,sizeof(recv_buffer)/sizeof(data_packet))){
	    if(sendto(ftps_M1_sock, FYN_packet, sizeof(data_packet), 0, (struct sockaddr *)&ftps_M1_name, sizeof(ftps_M1_name)) < 0) {
	      perror("error Sending");
	      exit(4);
	    }
	  }  
	  break;
	}
	else if(packet->FYN == '1'){
	  ack_packet->FYN = '1';
	  printf("TCPD_M1 --> TCPD_M2 :: SEND FYN+ACK PACKET-> sequence_number: %d \n",ack_packet->sequence_number);
	  if(sendto(M1_troll_sock, ack_packet, sizeof(data_packet), 0, (struct sockaddr *)&M1_troll_name, sizeof(M1_troll_name)) < 0) {
	    perror("error Sending ack");
	    exit(4);
	  }
	}
      }
      else {
	printf("Packet Health: CORRUPTED\n");
	printf("TCPD_M2 --> TCPD_M1 :: DROP FYN||FYN+ACK PACKET-> sequence_number: %d\n",ack_packet->sequence_number);
      }
    }
    else{
      printf("Packet Health: CORRUPTED\n");
      printf("TCPD_M2 --> TCPD_M1 :: DROP PACKET SEQUENCT NUMBER CORRUPUTED.\n");
    }
  }
  
    for(loopj=recv_buffer_head;loopj<1000;loopj++){
		if(recv_buffer[loopj].sequence_number != -3){
			if(sendto(ftps_M1_sock, &recv_buffer[loopj], sizeof(data_packet), 0, (struct sockaddr *)&ftps_M1_name, sizeof(ftps_M1_name)) < 0) {
				perror("error Sending");
				exit(4);
			}
		}		
	}
	fprintf(stdout, "\n\n....CONNECTION SHUTDOWN...\n");
	
  /* close connection */
  close(troll_M1_sock);
  close(ftps_M1_sock);
  exit(0);
}
