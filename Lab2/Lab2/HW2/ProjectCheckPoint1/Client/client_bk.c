//File for client.c written by Jagannath Narasimhan as part of Homework II - Network Programming
//This code allows for a file to be read that is passed as an argument to it.
//The information that is read is then encapsulated into a message structure that can be understood by troll.
//This message structure is then passed as packets to TCP-Deamon on the client side which is waiting on the port 2015
//Port 2015 is hard-coded as the listening port of the TCP-Deamon Client side
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

unsigned int crc32(char *message) {
 int i, j;
 unsigned int byte, crc, mask;
 static unsigned int table[256];
 /* Set up the table, if necessary. */
 if (table[1] == 0) {
 for (byte = 0; byte <= 255; byte++) {
 crc = byte;
 for (j = 7; j >= 0; j--) { // Do eight times.
 mask = -(crc & 1);
 crc = (crc >> 1) ^ (0xEDB88320 & mask);
 }
 table[byte] = crc;
 }
 }
 /* Through with table setup, now calculate the CRC. */
 i = 0;
 crc = 0xFFFFFFFF;
 while ((byte = message[i]) != 0) {
 crc = (crc >> 8) ^ table[(crc ^ byte) & 0xFF];
 i = i + 1;
 }
 return ~crc;
}


void error(const char *msg) //This is the error function. When an error is generated, this function will be called
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, portno, n, i;		//Variable decarations for sock file descriptor, port number and loop variable
	struct sockaddr_in serv_addr, tcpd_addr;	//Struct to hold the sock address
	struct hostent *server;

	
	if (argc < 4) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}
	portno = atoi(argv[2]); // To convert string of argument #2 to an integer value
	sockfd = socket(AF_INET, SOCK_DGRAM, 0); //Creating a new sock file descriptor
	
	
	
	if (sockfd < 0) 
		error("ERROR opening socket");

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = htons(AF_INET);
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(portno); // To convert from host byte order to network byte order
	
	


	FILE* fp = fopen(argv[3],"rb"); //Open the file with file name being passed in arg list in read binary mode
	if(fp == NULL)
	{
		error("File given is empty");
		exit(1);
	}

	

	fseek(fp, 0L, SEEK_END);
	int f_bytes = ftell(fp); //Find the size of file in bytes and store in f_bytes
	fseek(fp, 0L, SEEK_SET);
	printf("Size of the file to be transferred is: %d\n",f_bytes);
	

	struct {
	struct sockaddr_in header;
	char body[1000];
	} message;
	message.header = serv_addr;
	bzero(message.body,1000);
	
	tcpd_addr.sin_family = AF_INET;
	tcpd_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	tcpd_addr.sin_port = htons(2015); //TCP-Deamon Client Side address (listening on port 2015)
	
	memcpy(message.body, &f_bytes, 4);
	
	unsigned int checksum;

	checksum = crc32(message.body);
	
	printf("Checksum is %u",checksum);
	
	memcpy(message.body+4, &checksum, 4);
	
	
	
	if((sendto(sockfd, (char *)&message, sizeof message, 0, (struct sockaddr *)&tcpd_addr, sizeof(tcpd_addr)))<1)
	{
		error("Unable to write to socket file size");
		exit(1);
	} 
	
	exit(1);
	
	bzero(message.body,4); //Zero the message.body first 4 bytes 
	
	


	
	/* fill message.body with data */
	
	memcpy(message.body,argv[3],20);
	
	if((sendto(sockfd, (char *)&message, sizeof message, 0, (struct sockaddr *)&tcpd_addr, sizeof(tcpd_addr)))<1)
	{
		error("Unable to write to socket file name");
		exit(1);
	} 
	

	bzero(message.body,20); //Zero the message.body first 20 bytes after transfer of file name


	for( i=1000; i<f_bytes; i=i+1000){ //Start for loop which will transfer 1000 bytes at every iteration
									   //For loop runs till bytes remaining to be transferred is less that 1000 bytes
		fread(message.body, 1000, 1, fp);    //Read 1000 bytes from the file and place onto message.body

		

		if((sendto(sockfd, (char *)&message, sizeof message, 0, (struct sockaddr *)&tcpd_addr, sizeof(tcpd_addr)))<0) //Write the read 1000 bytes to the SockFD
			printf("Unable to write to socket file content");
		else
			printf("Bytes Sent: %d \n", i);

		bzero(message.body,1000);				//Zero the 1000 bytes of the message.body
	}
	i=i-1000;							//Decrement i by 1000, so that the remaining bytes can be transferred

	fread(message.body, f_bytes-i, 1, fp);    //Read the remaining bytes of the file (<1000)



	if((sendto(sockfd, (char *)&message, sizeof message, 0, (struct sockaddr *)&tcpd_addr, sizeof(tcpd_addr)))<0) //Transfer the remaining bytes to SockFD
		error("Unable to write to socket out of loop");
	else
		printf("Bytes Sent: %d \n", f_bytes);
 
	fclose(fp);							//Close FP
						//Close SockFD
 
	return 0;
}
