#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg) //This is the error function. When an error is generated, this function will be called
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, portno, n, i;		//Variable decarations for sock file descriptor, port number and loop variable
	struct sockaddr_in serv_addr;	//Struct to hold the sock address
	struct hostent *server;

	char buffer[1000]; 				//Buffer of 1000 bytes
	if (argc < 4) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}
	portno = atoi(argv[2]); // To convert string of argument #2 to an integer value
	sockfd = socket(AF_INET, SOCK_STREAM, 0); //Creating a new sock file descriptor
	if (sockfd < 0) 
		error("ERROR opening socket");
	server = gethostbyname(argv[1]); //Getting the IP Of the server from the argument list
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr)); //Initializing buffer to zero
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
			(char *)&serv_addr.sin_addr.s_addr,
			server->h_length);
	serv_addr.sin_port = htons(portno); // To convert from host byte order to network byte order
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR connecting" );

	FILE* fp = fopen(argv[3],"rb"); //Open the file with file name being passed in arg list in read binary mode
	if(fp == NULL)
	{
		error("File given is empty");
		exit(1);
	}

	bzero(buffer,1000); //Initialize first 1000 bytes of buffer to zero

	fseek(fp, 0L, SEEK_END);
	int f_bytes = ftell(fp); //Find the size of file in bytes and store in f_bytes
	fseek(fp, 0L, SEEK_SET);
	printf("Size of the file to be transferred is: %d\n",f_bytes);
	memcpy(buffer, &f_bytes, 4);

	if(write(sockfd,buffer,4)<0) //Write size of the file in the first 4 bytes of buffer
	{
		error("Unable to write to socket file size");
		exit(1);
	}

	bzero(buffer,4); //Zero the buffers first 4 bytes
	if(write(sockfd,argv[3],20)<0) //write the name of the file to be transferred onto the SockFD. 
	{
		error("Unable to write to socket file name");
		exit(1);
	}

	bzero(buffer,20); //Zero the buffers first 20 bytes after transfer of file name


	for( i=1000; i<f_bytes; i=i+1000){ //Start for loop which will transfer 1000 bytes at every iteration
									   //For loop runs till bytes remaining to be transferred is less that 1000 bytes
		fread(buffer, 1000, 1, fp);    //Read 1000 bytes from the file and place onto buffer

		

		if(write(sockfd,buffer,1000)<0) //Write the read 1000 bytes to the SockFD
			printf("Unable to write to socket file content");
		else
			printf("Bytes Sent: %d \n", i);

		bzero(buffer,1000);				//Zero the 1000 bytes of the buffer
	}
	i=i-1000;							//Decrement i by 1000, so that the remaining bytes can be transferred

	fread(buffer, f_bytes-i, 1, fp);    //Read the remaining bytes of the file (<1000)



	if(write(sockfd,buffer,f_bytes-i)<0) //Transfer the remaining bytes to SockFD
		error("Unable to write to socket out of loop");
	else
		printf("Bytes Sent: %d \n", f_bytes);

	close(fp);							//Close FP
	close(sockfd);						//Close SockFD

	return 0;
}
