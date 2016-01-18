//File for tcpds.c written by Jagannath Narasimhan as part of Homework II - Network Programming
//TCP-deamon has to listen to port no 6000 as the troll on the sending end at a remote location is forwarding packets at this port number.

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
	int sockfd, newsockfd, recvportno, sendportno;
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);	//Create a new SockFD and connect to it. To receive packets from Troll
	if (sockfd < 0) 
		error("ERROR opening socket");
	
	newsockfd = socket(AF_INET, SOCK_DGRAM, 0);	//Create a new SockFD and connect to it. To send packets to Server
	if (newsockfd < 0) 
		error("ERROR opening socket");
	
	struct sockaddr_in serv_addr, my_addr;
	
	struct {
	struct sockaddr_in header;
	char body[1000];
	} message;
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(4000); //Address of sending port
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = INADDR_ANY;
	my_addr.sin_port = htons(6000);	//Address of receiving port
	
	if (bind(sockfd, (struct sockaddr *) &my_addr, //Bind to the socket that was earlier created for the recceiving port
			sizeof(my_addr)) < 0) 
		error("ERROR on binding");
	
	int size_of_my_addr = sizeof(my_addr);
		
	while(1){
		if(recvfrom(sockfd, (char *)&message, sizeof message, 0, (struct sockaddr *)&my_addr, &size_of_my_addr)<0)				//Read the contents from Troll
		{
			error("ERROR reading packet from Troll");
			exit(1);
		}
		printf("Received at TCP-deamon Server end and sending to server \n");
		
		if(sendto(newsockfd,(char *)message.body, sizeof(message.body), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0) //Send contents received from troll on port 6000 to Server port 4000
		{
			error("Unable to send received packet from Troll to Server");
			exit(1);
		}
	}
	
	close(sockfd);
	close(newsockfd);
}