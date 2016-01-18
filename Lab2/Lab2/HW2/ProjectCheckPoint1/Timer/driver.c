#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>


// Here we have the Struct that communicates with the timer
typedef struct {
	int time;
	short port;
	long seq_num;
	int flag;
	
}TimerMsg;



//Function to Start the timer
void starttimer(int driverSockfd, struct sockaddr_in out_timer_addr, int time, short port, long seq_num)
{
	TimerMsg * t_msg = malloc(sizeof(TimerMsg));
	t_msg->time = time;
	t_msg->port = port;
	t_msg->seq_num = seq_num;
	t_msg->flag = 1;
	
	
	
	char *a = "CharString";
	
	//int chk1 = sendto(driverSockfd,(char *)a,sizeof(a),0,(struct sockaddr*)&out_timer_addr,sizeof(out_timer_addr));
	int chk1 = sendto(driverSockfd,(char *)t_msg,sizeof(TimerMsg),0,(struct sockaddr*)&out_timer_addr,sizeof(out_timer_addr));
	if(chk1 == -1)
	{
		printf("%s \n",strerror(errno));
	}
	else
	{
		printf("Timer Begin message sent for the sequence number: %lu .. %s\n",t_msg->seq_num,a);
	}
}

void canceltimer(int driverSockfd, struct sockaddr_in out_timer_addr, long seq_num)
{
	TimerMsg * t_msg = malloc(sizeof(TimerMsg));
	t_msg->seq_num = seq_num;
	t_msg->flag = 0;
	
	int chk1 = sendto(driverSockfd,(char*)t_msg,sizeof(TimerMsg),0,(struct sockaddr*)&out_timer_addr,sizeof(out_timer_addr));
	if(chk1 == -1)
	{
		printf("%s \n",strerror(errno));
	}
	else
	{
		printf("Timer End message sent for the sequence number: %lu \n",seq_num);
	}
}



void main(int argc, char* argv[])
{
	printf("Driver Started\n");
	
	int time;
	short port;
	long seq_num;
	
	int driverSockfd;
	struct sockaddr_in driver_addr;
	int driver_addrlen;
	
	struct sockaddr_in timer_addr;
	int timer_addrlen;
	
	struct sockaddr_in out_timer_addr;
	
	TimerMsg *t_msg = malloc(sizeof(TimerMsg)); 
	
	int d_addr = atoi(argv[1]);
	int d_port = atoi(argv[2]);
	int t_port = atoi(argv[3]);
	
	/************************************************************************************************
	 * Creating socket for the driver
	 ***********************************************************************************************/
	if((driverSockfd = socket(AF_INET,SOCK_DGRAM,0)) != 1)
	{
		printf("Created Driver Socket descriptor as %d\n" ,driverSockfd); 
	}
	else
	{
		printf("Creating socket for driver error!\n");
		exit(EXIT_FAILURE);
	}
	
	/**************************************************************************************************
	*Populating struct sockaddr_in driver_addr & driver_addrlen and binding the address to the socket
	* ************************************************************************************************/
	driver_addrlen = sizeof(driver_addr);
	driver_addr.sin_family = AF_INET;
	driver_addr.sin_port = htons(d_port);					// 0 value   - system will chose a random port
	driver_addr.sin_addr.s_addr = htonl(INADDR_ANY);  		// INADDR_ANY - server's IP address will be assigned automatically
	memset(&(driver_addr.sin_zero),0,sizeof(driver_addr.sin_zero));
	int b = bind(driverSockfd,(struct sockaddr*)&driver_addr,driver_addrlen);
	if(b==0)
	{
		printf("Socket of Driver has been binded \n");
	}
	else
	{
		printf("Failed binding the socket of driver\n");
		close(driverSockfd);
		exit(EXIT_FAILURE);
	}
	
	/**********************************************************************
	 * Setting up out_timer_addr struct for sending msg to timer
	 * *******************************************************************/
	memset((char*)&out_timer_addr, 0, sizeof(out_timer_addr));
	out_timer_addr.sin_family = AF_INET;
	out_timer_addr.sin_addr.s_addr = htonl(INADDR_ANY);  		// INADDR_ANY - server's IP address will be assigned automatically
	memset(&(out_timer_addr.sin_zero),0,sizeof(out_timer_addr.sin_zero));
	out_timer_addr.sin_port = htons(t_port);	
	
	
	sleep(2);
	starttimer(driverSockfd, out_timer_addr, 20, d_port,1);
	starttimer(driverSockfd,out_timer_addr,1, d_port,2);
	starttimer(driverSockfd,out_timer_addr,30, d_port,3);
	sleep(5);
	canceltimer(driverSockfd,out_timer_addr,2);
	starttimer(driverSockfd,out_timer_addr, 20, d_port, 4);
	sleep(driverSockfd,out_timer_addr,5);
	starttimer(driverSockfd,out_timer_addr,18, d_port ,5);
	canceltimer(driverSockfd,out_timer_addr,4);
	canceltimer(driverSockfd,out_timer_addr,8); 
	
	
	/**********************************************************************************************************
	 *  Receiving the timer message from timer
	 **********************************************************************************************************/
	/*
	int chk = recvfrom(driverSockfd,t_msg,sizeof(t_msg),0,(struct sockaddr*)&timer_addr,&timer_addrlen);
	if(chk == -1)
	{
		printf("%s \n",strerror(errno));
	} */
	
	
	close(driverSockfd);
}
