

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
	int sockfd, newsockfd, portno;  //Variable decarations for sock file descriptor, new sock FD and port number.
	socklen_t clilen;
	char buffer[1000];				//Buffer of 1000 bytes

	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);	//Create a new SockFD and connect to it.
	if (sockfd < 0) 
		error("ERROR opening socket");


	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]); 			// To convert string of argument #1 to an integer value
	printf("Server is Listening on port %d\n", portno);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno); //Convert the port number to network byte order from host byte order
	if (bind(sockfd, (struct sockaddr *) &serv_addr, //Bind to the socket that was earlier created
			sizeof(serv_addr)) < 0) 
		error("ERROR on binding");
	listen(sockfd,5);							//Listen on the socket
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, 					//Accept the incoming connection and create a new Sock FD
			(struct sockaddr *) &cli_addr, 
			&clilen);
	if (newsockfd < 0) 
		error("ERROR on accept");
	bzero(buffer,1000);							//Bzero the buffer (initialize to zero)

	n = read(newsockfd,buffer,4);				//Read the first 4 bytes on the buffer

	int f_bytes, i;

	memcpy(&f_bytes, buffer, 4);				//Read the size of the file being transferred

	if (n < 0) error("ERROR reading from socket");

	printf("Size of File to be received is: %d \n",f_bytes);

	bzero(buffer,4);							//Bzero the first four bytes of the buffer

	if(read(newsockfd,buffer,20)<0)				//Read the file name (20 bytes of buffer)
	{
		error("ERROR reading for socket for file name");
		exit(1);
	}

	printf("Name of the file to be received is %s\n", buffer);




	FILE* fp = fopen(buffer,"wb");			//Open a file with the same file name as that that was transferred
											// in binary mode

	bzero(buffer,20);						//Bzero the 20 bytes of the buffer

	for(i=1000; i<f_bytes; i=i+1000){		//Begin the loop if the size of the file is greater than 1000 bytes

		if(read(newsockfd,buffer,1000)<0)	//Read 1000 bytes from the buffer till the loop condition holds true
			error("ERROR reading for socket for file content in loop");

	

		if (fp){							//Write the 1000 bytes from the buffer to the created file
			fwrite(buffer, 1000, 1, fp);
			printf("Number of Bytes written to file: %d\n",i);
			//fp=fp+10;
		}
		bzero(buffer,1000);					//Bzero the 1000 bytes in the buffer
	}
	i=i-1000;								//decrement i by 1000 so that the remaining bytes can be read

	if(read(newsockfd,buffer,f_bytes-i)<0)	//Read the remaining bytes of the file (after loop condition fails)
		error("ERROR reading for socket for file content out of loop");

	
	if (fp){
		fwrite(buffer,f_bytes-i, 1, fp);	//Write the remaining bytes from the buffer to the file
		printf("Number of Bytes written to file: %d\n", f_bytes);

	}

	
	close(fp);								//Close file


	close(newsockfd);						//Close the newsock FD
	close(sockfd);							//Close SockFD
	return 0; 
}
