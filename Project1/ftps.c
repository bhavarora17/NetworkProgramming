/*
   FILE: ftps Program (Server Application)
*/

#include <netdb.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "packet.h"
#define MAX_MES_LEN 1024
#define FTPS_M1_PORT 2009

int SEND(int sockfd, const void *msg, int len, unsigned int flags, const struct sockaddr *to, int tolen) {

  if(sendto(sockfd, msg, len, flags, to, tolen) <0) {
    perror("sending datagram message");
    exit(4);
  } 
  return 1;
}

int RECV(int sockfd, void *buf, int len, unsigned int flags, struct sockaddr *from, int *fromlen) {

  int bytes_received = recvfrom(sockfd, buf, len, flags, from, fromlen);
  if (bytes_received < 0) {
    perror("Sending datagram message");
    exit(4);
  }
  return bytes_received;
}


/* client program called with host name and port number of server */
main(int argc, char *argv[])
{
  int sock, buflen, namelen;
  char buf[MAX_MES_LEN];
  struct sockaddr_in name;
  struct hostent *hp, *gethostbyname();
    
  /***** FILE DISCRIPTORS *****/
  int total_bytes_to_read;
  char* fileAddr = "/home/4/mohanr/file/";
  char* fileName;
  FILE* fp;

  /***** DATA PACKET INITIALIZATION *******/
  data_packet *packet;
  packet = calloc(1,sizeof(data_packet));

  if(argc < 1) {
    printf("usage: ftps Listen_port_number\n");
    exit(1);
  }

  /* create socket for connecting to server */
  sock = socket(AF_INET, SOCK_DGRAM,0);
  if(sock < 0) {
    perror("opening datagram socket");
    exit(2);
  }

  /* construct name for connecting to server */
  name.sin_family = AF_INET;
  name.sin_port = htons(FTPS_M1_PORT);

  /* convert hostname to IP address and enter into name */
  hp = gethostbyname("localhost");
  if(hp == 0) {
    fprintf(stderr, "%s:unknown host\n", argv[1]);
    exit(3);
  }
  bcopy((char *)hp->h_addr, (char *)&name.sin_addr, hp->h_length);

  /* send remote port number to TCPD_M1 */

  memcpy(packet->payload,argv[1],strlen(argv[1]));

  printf("FTPS sends ListenPORT:\%s\n",packet->payload);
  if(SEND(sock, packet, sizeof(data_packet), 0, (struct sockaddr *)&name, sizeof(name)) <0) {
    perror("sending datagram message");
    exit(4);
  }
    
  /******** INITIALIZTION *********/
  namelen=sizeof(struct sockaddr_in);

  /********* FILE SIZE RECIEVE **********/

  while(1){

    if(RECV(sock, packet, sizeof(data_packet),0,(struct sockaddr *)&name, &namelen) < 0) {
      perror("Receiving Error:");
      exit(4);
    }

    printf("Packet Sequence Number:\t%d\n",packet->sequence_number);
		
    if(packet->FYN == 1){
      printf("Terminating Message Recieved...Closing Connection...\n");
      break;
    }

    if(packet->sequence_number == 1){

      /********* FILE NAME RECIEVE **********/
      fprintf(stdout,"Incoming File Name:\t%s\n",packet->payload);

      /*********** FILE CREATION AND MANIPULATION **********/

      fileName  = malloc(strlen(fileAddr)+strlen(packet->payload)+1);
      strcpy(fileName,fileAddr);
      strcat(fileName,packet->payload);
		
      printf("FILE NAME WITH PATH:\t%s\n",fileName);
		   
      /* Creating File */
      if((fp = fopen(fileName,"a+")) < 0){
	fprintf(stderr,"File Not Found.\n");
	exit(5);
      } 
     // printf("FILE HANDLER: %d\n",fp);
    }

    /********* FILE WRITE *********/
    if(packet->sequence_number > 1) {
      if(strlen(packet->payload) < 1001){
	fwrite(packet->payload,sizeof(char),strlen(packet->payload) ,fp);
      }
      else{
	fwrite(packet->payload,sizeof(char),sizeof(packet->payload) ,fp);
      }
    }

    if(strlen(packet->payload) < 1001 && packet->sequence_number > 3){
      fclose(fp);
      printf("*************************************SHUTDOWN ****************************\n");
      exit(0);
    }
  }
	
  /* close connection */
  close(sock);
  exit(0);
}
