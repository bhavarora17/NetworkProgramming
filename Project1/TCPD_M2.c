/*
   FILE: TCPD_M2 Program (TCP Domain Application on Client Side)
*/

#include <netdb.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include "packet.h"
#include "Jacobson.c"
#include "circularBuffer.c"

/*****FOR FTPC TO M2 COMMUNICATION PORTS***/
#define FTPC_M2_PORT "2468"				// M2 Listening Port
#define M2_FTPC_PORT "8642"				// FTPC Listening Port

/***********FOR TROLL ON CLINET SIDE COMMUNICATIONS ******************/
#define TROLLC_M2_SEND_PORT 4009                       // M2 Sends to Trollc via SEND_PORT
#define TROLLC_ADDR "164.107.112.68"                   // TROLL Address
#define TROLLC_PORT_BUF 4680                           // TROLL Listening Port

/***********FROM TROLL ON SERVER SIDE COMMUNICATION PORTS ******************/
#define TROLLS_M2_LISTEN_PORT 7009                    // M2 Listening from TROLLS on LISTEN_PORT

/***********FOR TIMER COMMUNICATION PORTS******************/
#define M2_TIMER_PORT 4689                            // TIMER Listening Port
#define TIMER_M2_PORT 9864                            // M2 Listening from Timer

main()
{
  int ftpc_M2_sock, M2_ftpc_sock, M2_trollc_sock, trolls_M2_sock, M2_timer_sock, timer_M2_sock;	
  struct sockaddr_in ftpc_M2_name;		 // Receiving Name (ftpc->TCPD_M2)  
  struct sockaddr_in M2_ftpc_name; 		 // Sending Name (TCPD_M2->ftpc) 
  struct sockaddr_in M2_trollc_name;		 // Sending Name (TCPD_M2->trollc)  
  struct sockaddr_in M2_send_name;		 // Binding port for outgoing packet from TCPD_M2
  struct sockaddr_in trolls_M2_name;	 // Receiving Name (trolls->TCPD_M2)
  struct sockaddr_in M2_timer_name;	 // Sending Name (TCPD_M2->timer)
  struct sockaddr_in timer_M2_name;	 // Receiving Name (timer->TCPD_M2)
  struct hostent *hp, *gethostbyname();

  char FileName[20];

  /******** DATA, ACKNOWLEDGEMENT AND TIMER PACKETS ***********/
  struct data_packet *packet;		// Data packet 
  struct data_packet *ack_packet;
	
  packet = calloc(1,sizeof(struct data_packet));	
  ack_packet = calloc(1,sizeof(struct data_packet));
	
  struct timer_packet *new_timer;
  struct timer_packet *cancel_timer;

  new_timer = calloc(1, sizeof(struct timer_packet));
  cancel_timer = calloc(1, sizeof(struct timer_packet));
	
  /******** NECESSARY VARIABLE DECLARATIONS *********/
  CircularBuffer cB;										//Circular Buffer Definition
  cbInit(&cB);											//Circular Buffer Initialization
	
  int namelen;
  namelen = sizeof(struct sockaddr_in);
	
  fd_set readset;
  struct timeval now ; 	

  /**** INITIALIZE SRTT AND RTTVAR****/	
  initialize(25);					
	
  /********** MAIN PROGRAM **********/
  /*********** FTPC to TCPD_M2 COMMUNICATION INITAILIZATION ****************/
  ftpc_M2_sock = socket(AF_INET, SOCK_DGRAM,0);
  if(ftpc_M2_sock < 0) {
    perror("opening datagram socket");
    exit(2);
  }
  ftpc_M2_name.sin_family = AF_INET;
  ftpc_M2_name.sin_port = htons(atoi(FTPC_M2_PORT));
  ftpc_M2_name.sin_addr.s_addr = INADDR_ANY;
  if(bind(ftpc_M2_sock, (struct sockaddr *)&ftpc_M2_name, sizeof(ftpc_M2_name)) < 0) {
    perror("ERROR IN BINDING ftpc_M2_sock to ftpc_M2_name");
    exit(2);
  }

  /*********** TCPD_M2 to FTPC COMMUNICATION **********/
  M2_ftpc_sock = socket(AF_INET, SOCK_DGRAM,0);
  if(M2_ftpc_sock < 0) {
    perror("opening datagram socket");
    exit(2);
  }
  M2_ftpc_name.sin_family = AF_INET;
  M2_ftpc_name.sin_port = htons(atoi(M2_FTPC_PORT));  
  hp = gethostbyname("localhost");
  if(hp == 0) {
    fprintf(stderr, "%s:unknown host\n");
    exit(3);
  }
  bcopy((char *)hp->h_addr, (char *)&M2_ftpc_name.sin_addr, hp->h_length);
	
  /************ TCPD_M2 to TROLLC COMMUNICATION INITIALIZATION ****************/  
  M2_trollc_sock = socket(AF_INET, SOCK_DGRAM,0);
  if(M2_trollc_sock < 0) {
    perror("opening datagram socket M2_trollc_sock");
    exit(2);
  }	

  /*binding the TROLLC_M2_SEND_PORT to the M2_send_name so that it can be used by troll in -a arg */
  M2_send_name.sin_family = AF_INET;
  M2_send_name.sin_port = htons(TROLLC_M2_SEND_PORT);
  M2_send_name.sin_addr.s_addr = INADDR_ANY;
  if(bind(M2_trollc_sock, (struct sockaddr *)&M2_send_name, sizeof(M2_send_name)) < 0) {
    perror("getting socket name");
    exit(2);
  }
	
  /********** TCPD_M2 to TROLLC COMMUNICATION ************/
  M2_trollc_name.sin_family = AF_INET;
  M2_trollc_name.sin_port = htons(TROLLC_PORT_BUF); 
  M2_trollc_name.sin_addr.s_addr = inet_addr(TROLLC_ADDR);
	
  /************* TROLLS to TCPD_M2 COMMUNICATION INITIALIZATION **********/
  trolls_M2_sock = socket(AF_INET, SOCK_DGRAM,0);
  if(trolls_M2_sock < 0) {
    perror("opening datagram socket trolls_to_m2");
    exit(2);
  }
  trolls_M2_name.sin_family = AF_INET;
  trolls_M2_name.sin_port = htons(TROLLS_M2_LISTEN_PORT);
  trolls_M2_name.sin_addr.s_addr = INADDR_ANY;
  if(bind(trolls_M2_sock, (struct sockaddr *)&trolls_M2_name, sizeof(trolls_M2_name)) < 0) {
    perror("Unable to bind TROLLS_to_M2 Sock to trolls_M2_name");
    exit(2);
  }
	
  /************ Timmer to M2 COMMUNICATION INITIALIZATION ***************/
  timer_M2_sock = socket(AF_INET, SOCK_DGRAM,0);
  if(timer_M2_sock < 0) {
    perror("opening datagram socket m2_to_timer");
    exit(2);
  }
  timer_M2_name.sin_family = AF_INET;
  timer_M2_name.sin_port = htons(TIMER_M2_PORT);
  timer_M2_name.sin_addr.s_addr = INADDR_ANY;
  if(bind(timer_M2_sock, (struct sockaddr *)&timer_M2_name, sizeof(timer_M2_name)) < 0) {
    perror("Unable to bind (TROLLS_to_M2 Sock)");
    exit(2);
  }
	
  /*********** M2 to TIMER COMMUNICATION INITALIZATION ***********/
  M2_timer_sock = socket(AF_INET, SOCK_DGRAM,0);
  if(M2_timer_sock < 0) {
    perror("opening datagram socket");
    exit(2);
  }
  M2_timer_name.sin_family = AF_INET;
  M2_timer_name.sin_port = htons(M2_TIMER_PORT);  
  hp = gethostbyname("localhost");
  if(hp == 0) {
    fprintf(stderr, "%s:unknown host\n");
    exit(3);
  }
  bcopy((char *)hp->h_addr, (char *)&M2_timer_name.sin_addr, hp->h_length);
	
  /******** CRC INITIALIZATION *********/
  crcInit();
		
  while(1){
    float rto = 0;

    FD_ZERO(&readset);  
		
    FD_SET(ftpc_M2_sock, &readset);
    FD_SET(trolls_M2_sock, &readset);
    FD_SET(timer_M2_sock, &readset);
		
    if(select(FD_SETSIZE, &readset, NULL, NULL, NULL) < 0 ) {
      perror("ERROR ON SELECT WAITING");
      exit (1);
    }	
    
    /************* DATA PACKET FROM FTPC TO TCPD_M2 ************/
    if(FD_ISSET(ftpc_M2_sock, &readset)){
      bzero(packet,sizeof(packet));
      if(recvfrom(ftpc_M2_sock, packet, sizeof(struct data_packet), 0, (struct sockaddr *)&ftpc_M2_name, &namelen) < 0) {
	perror("ERROR IN RECIEVING PACKET FROM FTPC"); 
	exit(4);
      }
      printf("FTPC --> M2 :: RECVD DATA_PACKET-> sequence_number :  %d \n", packet->sequence_number);

      if(!cbIsFull(&cB)){
	cbWrite(&cB, packet);

	/***** CALCULATING RTO USING JACOBSON *****/
	set_time(packet->sequence_number); 
	if(packet->sequence_number == 1){
	  rto = 250;
	}
	else{
	  rto = computeRTO();
	}
				
	/***** CREATING AND SENDING TIMER PACKET TO TIMER PROCESS *****/

	new_timer->time = (rto > 1000 ? ((int)rto/1000)+3 : (int)3);
	new_timer->sequence_number = packet->sequence_number ;
	new_timer->type = 0 ;

	if (! new_timer) {
	  fprintf (stderr, "Calloc failed for creating new timer in TCPD_M2 \n");
	  exit (EXIT_FAILURE);
	}
	if(sendto(M2_timer_sock, new_timer, sizeof(struct timer_packet), 0, (struct sockaddr *)&M2_timer_name, sizeof(M2_timer_name)) < 0) {
	  perror("ERROR SENDING PACKET TO TIMER");
	  exit(4);
	}

	/***** SENDING PACKET TO TROLLC *****/
	if(packet->FYN == '1' ){
	  if(cB.end - cB.start == 1){
	    if(sendto(M2_trollc_sock, packet, sizeof(struct data_packet), 0, (struct sockaddr *)&M2_trollc_name, sizeof(M2_trollc_name)) < 0) {
	      perror("ERROR SENDING PACKET TO TROLLC");
	      exit(4);
	    }
	  }
	}
	else{
	  if(sendto(M2_trollc_sock, packet, sizeof(struct data_packet), 0, (struct sockaddr *)&M2_trollc_name, sizeof(M2_trollc_name)) < 0) {
	    perror("ERROR SENDING PACKET TO TROLLC");
	    exit(4);
	  }
	}
      }	
    }

    /********* ACK_PACKET FROM TROLLS TO TCPD_M2 *********/
    if(FD_ISSET(trolls_M2_sock, &readset)) {
      if(recvfrom(trolls_M2_sock, ack_packet, sizeof(struct data_packet), 0, (struct sockaddr *)&trolls_M2_name, &namelen) < 0) {
	perror("ERROR RECIEVING ACK FROM TROLLS"); 
	exit(4);
      }
      printf("TCPD_M1 --> TCPD_M2 :: RECVD ACK_PACKET-> sequence_number : %d \n",ack_packet->sequence_number);
      
      gettimeofday(&now,NULL);
      calc_rtt(now,timehash[ack_packet->sequence_number]);
      
      printf("***UPDATE:- rtt ack_packet-> sequence_number : %d2\n",ack_packet->sequence_number);
      fflush(stdout);

      cbRemove(&cB, ack_packet->sequence_number);
      if(!cbIsFull(&cB)){
	char buff[20];
	bzero(buff, 20);
	memcpy(buff, "empty", 20);
	if(sendto(M2_ftpc_sock, buff, sizeof(buff), 0, (struct sockaddr *)&M2_ftpc_name, sizeof(M2_ftpc_name)) < 0) {
	  perror("ERROR SENDING BUFFER FULL MSG TO FTPC");
	  exit(4);
	}			
      }

      /***** TIMER CANCELATION REQUEST FOR ACK_PACKET FROM TCPD_M2 TO TIMER ********/
      printf("M2 --> TIMER :: SEND CANCEL TIMER_PACKET-> sequence_number: %d\n", ack_packet->sequence_number);
      cancel_timer->sequence_number = ack_packet->sequence_number;
      cancel_timer->type = 1;
      if(sendto(M2_timer_sock, cancel_timer, sizeof(struct timer_packet), 0, (struct sockaddr *)&M2_timer_name, sizeof(M2_timer_name)) < 0) {
	perror("ERROR SENDING PACKET TO TIMER");
	exit(4);
      }

      /*****CONNECTION SHUTDOWN PROCEDURE***/
      if(ack_packet->FYN == '1'){
	char buff[20];
	bzero(buff, 20);
	memcpy(buff, "acked", 20);
				
	printf("\nFile Transfer Complete...\n");
	ack_packet->sequence_number = 1;
	bzero(ack_packet, 1000);
	ack_packet->checksum = crcFast(ack_packet->payload,1000);
				
	/****SENDING ACKED MSG TO FTPC*****/
	if(sendto(M2_ftpc_sock, buff, sizeof(buff), 0, (struct sockaddr *)&M2_ftpc_name, sizeof(M2_ftpc_name)) < 0) {
	  perror("ERROR SENDING BUFFER FULL MSG TO FTPC");
	  exit(4);
	}	
				
	/*****SENDING FYN ACK TO M1*****/
	if(sendto(M2_trollc_sock, ack_packet, sizeof(struct data_packet), 0, (struct sockaddr *)&M2_trollc_name, sizeof(M2_trollc_name)) < 0) {
	  perror("ERROR SENDING PACKET TO TROLLC");
	  exit(4);
	}
	//sleep(1);
	break;
      }
    }

    /************** TIMER PACKET (RTO EXPIRATIONS) FROM TIMER TO TCPD_M2 *****************/
    if(FD_ISSET(timer_M2_sock, &readset)) {
      int exp_seq_num ; 
      if(recvfrom(timer_M2_sock, &exp_seq_num, sizeof(int), 0, (struct sockaddr *)&timer_M2_name, &namelen) < 0) {
	perror("ERROR RECIEVING TIMER EXPIRE MESSAGE FROM TIMERPROCESS"); 
	exit(4);
      }
      printf("TIMER --> M2 :: EXPIRED PACKET-> sequence_number: %d\n", exp_seq_num);
      fflush(stdout);

      /**** RETRANSMISSION OF PACKET *****/
      packet = cbRead(&cB, exp_seq_num);
      printf("TIMER --> M2 :: SEND RETRANSMIT PACKET-> sequence_number: %d \n",packet->sequence_number);	
      fflush(stdout);

      /***** CALCULATING RTO USING JACOBSON *****/
      set_time(packet->sequence_number); 
      rto = computeRTO();
			
      /***** CREATING AND SENDING TIMER PACKET TO TIMER PROCESS *****/
      new_timer->time = (rto > 1000 ? ((int)rto/1000)+3 : (int)3);
      new_timer->sequence_number = packet->sequence_number ;
      new_timer->type = 0 ;
      if (! new_timer) {
	fprintf (stderr, "Calloc failed for creating new timer in TCPD_M2 \n");
	exit (EXIT_FAILURE);
      }
      if(sendto(M2_timer_sock, new_timer, sizeof(struct timer_packet), 0, (struct sockaddr *)&M2_timer_name, sizeof(M2_timer_name)) < 0) {
	perror("ERROR SENDING PACKET TO TIMER");
	exit(4);
      }
			
      /*****SENDING PACKET TO TROLL*****/
      if(sendto(M2_trollc_sock, packet, sizeof(struct data_packet), 0, (struct sockaddr *)&M2_trollc_name, sizeof(M2_trollc_name)) < 0) {
	perror("ERROR SENDING PACKET TO TROLLC");
	exit(4);
      }
    }
    if(cbIsFull(&cB)){
      char buff[20];
      bzero(buff, 20);
      memcpy(buff, "full", 20);
      if(sendto(M2_ftpc_sock, buff, sizeof(buff), 0, (struct sockaddr *)&M2_ftpc_name, sizeof(M2_ftpc_name)) < 0) {
	perror("ERROR SENDING BUFFER FULL MSG TO FTPC");
	exit(4);
      }
    }
  }
	
  if(cbIsEmpty(&cB)){
    fprintf(stdout, "\n\n....CONNECTION SHUTDOWN....\n");
  }
	
  close(ftpc_M2_sock);
  close(M2_ftpc_sock);
  close(M2_trollc_sock);	
  close(trolls_M2_sock);
  close(M2_timer_sock);
  close(timer_M2_sock);

  exit(0);	
}

