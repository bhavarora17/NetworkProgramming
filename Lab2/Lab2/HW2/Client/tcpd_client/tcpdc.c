//File for tcpdc.c written by Jagannath Narasimhan as part of Homework II - Network Programming
//This file accepts packets from the Client and transfers the same to Troll. 
//Accepting the packets is done at port number that the client is sending at. This is hard-coded to 2015 at the client end
//So it has to be listening to port number 2015 for the packets to be successfully sent to the Troll.
//Similarly the packets are being sent to Troll on port number 2500. Hence the Troll has to be listening on port 2500

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) 			//This is the error function. When an error is generated, this function will be called
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno, troll_port_no = 2500;  //Variable decarations for sock file descriptor, new sock FD and port number.
														  //Notice that the send port is hard-coded. Which means that the troll MUST listen to post 2500
	
	struct hostent *troll_ip;


	struct sockaddr_in serv_addr, troll_addr;
	int n;
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);	//Create a new SockFD and connect to it to receive packets from the Client.
	if (sockfd < 0) 
		error("ERROR opening socket");
	
	newsockfd = socket(AF_INET, SOCK_DGRAM, 0);	//Create a new SockFD and connect to it to send packets to Troll.
	if (newsockfd < 0) 
		error("ERROR opening socket");


	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]); 			// To convert string of argument #1 to an integer value
	printf("TCPD Client is Listening on port %d\n", portno);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno); //Convert the port number to network byte order from host byte order
	if (bind(sockfd, (struct sockaddr *) &serv_addr, //Bind to the socket that was earlier created
			sizeof(serv_addr)) < 0) 
		error("ERROR on binding");
		

	bzero((char *) &troll_addr, sizeof(troll_addr)); //Initializing buffer to zero
	
	troll_addr.sin_family = AF_INET;
	troll_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	troll_addr.sin_port = htons(troll_port_no);//Convert the port number to network byte order from host byte order

	int size_of_serv_addr = sizeof(serv_addr);

	int f_bytes, i;


	
	struct {
	struct sockaddr_in header;
	char body[1000];
	} message;

	
	printf("Getting Ready to Read\n");
	while(1)
	{	if(recvfrom(sockfd, (char *)&message, sizeof message, 0, (struct sockaddr *)&serv_addr, &size_of_serv_addr)<0)				//Read the contents being sent by Client
		{
			error("ERROR reading packets from Client/n");
			exit(1);
		}
		printf("Received and sending to Troll \n");
		
		if(sendto(newsockfd,(char *)&message, sizeof message, 0, (struct sockaddr *)&troll_addr, sizeof(troll_addr))<0) //Send the read contents to Troll
		{
			error("Unable to send the read packets to Troll: expecting Troll to listen at Port 2500 \n");
			exit(1);
		}

	}	



	return 0; 
}
