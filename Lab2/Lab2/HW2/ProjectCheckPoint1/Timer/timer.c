#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

/*********************************************************************************
 * Structure for communicating with the TCPD client deamon
 * ******************************************************************************/
typedef struct {
	int time;
	short port;
	long seq_num;
	int flag;
	
}TimerMsg;


/*********************************************************************************
 * Structure of the Node inside the doubly linked list
 * ******************************************************************************/
typedef struct
{
	int time;
	short port;
	long seq_num;
	struct Node *prev;
	struct Node *next;

}Node;


/*********************************************************************************
 * Structure of the doubly linked list
 * ******************************************************************************/
typedef struct 
{
	Node *head;
	Node *tail;
}DLL;


/*********************************************************************************
 * Method for adding any new incoming packets .
 * ******************************************************************************/
void addNode(DLL* dll, int time, short port, long seq_num)
{
	Node *newNode = malloc(sizeof(Node));
	newNode->time = time;
	newNode->port = port;
	newNode->seq_num = seq_num;
	
	Node *dllhead = dll->head;
	Node *dlltail = dll->tail;
	
	if(dllhead == NULL)
	{
		printf("First node\n");
		dll->head = newNode;
		dll->tail = newNode;
	}
	else if(dllhead->time >= newNode->time)
	{
		printf("New node < Head Node time\n");
		
		dllhead->time = dllhead->time - newNode->time;
		
		newNode->next = (struct Node *)dllhead;
		dllhead->prev = (struct Node *)newNode;
		
		dll->head = newNode;
		
	}
	else if(dllhead->time < newNode->time)
	{
		printf("New Node > Head Node time\n");
		int sum =0;
		int flag=0;
		Node *temp = dll->head;
		sum+= temp->time;
		
		while(temp->next != NULL)
		{
			if(sum > newNode->time)
			{
				flag =1;
				sum = sum - temp->time;
				//printf("new sum : %d" ,sum);
				//printf("temp seq num : %lu" ,temp->seq_num);
				break;
			}
			temp = (Node *)temp->next;
			sum+= temp->time;
			//printf("%d \t %d \n",sum,temp->time );
		}
		
		if(sum > newNode->time)
		{
			flag =1;
			sum = sum - temp->time;
			//printf("new sum : %d" ,sum);
			//printf("temp seq num : %lu" ,temp->seq_num);
		}
		
		
		//printf("\n sum : %d \n" , sum);
		
		if(flag)
		{
			printf("Node added in middle of list\n");
			Node *temp2 = (Node *)temp->prev;
			temp2->next = (struct Node*)newNode;
			newNode->prev = temp->prev;
			newNode->next = (struct Node *)temp;
			temp->prev = (struct Node *)newNode;
			
			newNode->time = newNode->time - sum;
			temp->time =  temp->time - newNode->time;
		}
		else
		{
			printf("Node added to the end of List\n");
			temp->next = (struct Node *)newNode;
			newNode->prev = (struct Node *)temp;
			
			newNode->time = newNode->time - sum;		
		}	
	}
	else
	{
		printf("... \n");
	}
	//printf("New Node's addr : %d \n",newNode);
}


/***************************************************************************
 * Printing all the nodes values in the linked list at that given moment
 * ************************************************************************/
void printDLL(DLL *dll)
{
	Node *temp = dll->head;
		
		while(temp != NULL)
		{
			printf("Current time : %d , Port Number : %hu , Sequence Number : %lu \n" , temp->time,temp->port,temp->seq_num);
			temp = (Node *)temp->next;
		}
}


/***************************************************************************
 * Removing nodes from the linked list for which time == 0 and 
 * notifying the tcpd client daemon about it.
 * ************************************************************************/
Node* removeHead(DLL *dll, int timerSockfd )
{
	Node *temp = dll->head;
	Node *rlsNode ;
	TimerMsg *msg = malloc(sizeof(TimerMsg));
	
	struct sockaddr_in tcpd_addr;
	/**********************************************************************
	 * Setting up tcpd_adddr struct for sending data to tcpd
	 * *******************************************************************/
	memset((char*)&tcpd_addr, 0, sizeof(tcpd_addr));
	tcpd_addr.sin_family = AF_INET;
	tcpd_addr.sin_addr.s_addr = htonl(INADDR_ANY);  		// INADDR_ANY - server's IP address will be assigned automatically
	memset(&(tcpd_addr.sin_zero),0,sizeof(tcpd_addr.sin_zero));
	
	if(temp != NULL)
	{
		while(temp->time == 0)
		{
			rlsNode = temp;
			printf(" TimeOut. Removing from List %lu\n",rlsNode->seq_num);
			msg->time = rlsNode->time;
			msg->port = rlsNode->port;
			msg->seq_num = rlsNode->seq_num;
			
			tcpd_addr.sin_port = htons(rlsNode->port);	
			//code for sending info to
			/*
			int chk = sendto(timerSockfd,(char*)&msg,sizeof(msg),0,(struct sockaddr*)&tcpd_addr,sizeof(tcpd_addr));
			printf("chk %d \n",chk);
			if(chk == -1)
			{
				printf("%s \n",strerror(errno));
			}*/
			 
			temp = (Node*)temp->next;
		}
		temp->prev = NULL;
		dll->head = temp;
	}
	
}


/****************************************************************************
 * Updating time field of the head of the doubly linked list .
 * *************************************************************************/
void updateDLLHead(DLL *dll)
{
	if(dll->head != NULL)
	{
		Node *head = dll->head;
		head->time = head->time -1;
	}
}


void cancelNode(DLL *dll , long seq_num)
{
	Node *temp = dll->head;
	if(temp != NULL)
	{
		while(temp != NULL && temp->seq_num != seq_num)
		{
			temp = (Node *)temp->next;
		}
		
		if(temp != NULL)
		{
			Node *prevtemp = (Node *)temp->prev;
			Node *nexttemp = (Node *)temp->next;
			if(prevtemp == NULL)
			{
				printf("Cancelling first node\n");
				dll->head = nexttemp;
				nexttemp->prev = NULL;
				nexttemp->time = nexttemp->time + temp->time;
			}
			else if(nexttemp == NULL)
			{
				printf("Cancelling Last Node\n");
				prevtemp->next = NULL;
			}
			else
			{
				printf("Cancelling Node in Between\n");
				nexttemp->time = nexttemp->time + temp->time;
				nexttemp->prev = (struct Node *)prevtemp;
				prevtemp->next = (struct Node *)nexttemp;
			}
		}
		else
		{
			printf(" Given Sequence Number is  %lu  which is not present\n", seq_num);
		}
	}
	
}



void main(int argc, char* argv[])
{
	printf("Starting Timer\n");
	
	DLL *dll = malloc(sizeof(DLL));
	
	int time;
	short port;
	long seq_num;
	
	int timerSockfd;
	struct sockaddr_in timer_addr;
	int timer_addrlen;
	
	struct sockaddr_in tcpd_addr;
	int tcpd_addrlen;
	
	
	TimerMsg *t_msg = malloc(sizeof(TimerMsg)); 
	//TimerMsg t_msg; 
	
	char* t_addr = argv[1];
	char* t_port = argv[2];
	
	/************************************************************************************************
	 * Creating socket for the timer
	 ***********************************************************************************************/
	if((timerSockfd = socket(AF_INET,SOCK_DGRAM,0)) != 1)
	{
		printf("Created Timer. Descriptor - %d\n" ,timerSockfd); 
	}
	else
	{
		printf("Timer Socket Creation Failed\n");
		exit(EXIT_FAILURE);
	}
	
	/**************************************************************************************************
	*Populating struct sockaddr_in timer_addr & timer_addrlen and binding the address to the socket
	* ************************************************************************************************/
	timer_addrlen = sizeof(timer_addr);
	timer_addr.sin_family = AF_INET;
	timer_addr.sin_port = htons(atoi(t_port));					// 0 value   - system will chose a random port
	timer_addr.sin_addr.s_addr = htonl(INADDR_ANY);  		// INADDR_ANY - server's IP address will be assigned automatically
	memset(&(timer_addr.sin_zero),0,sizeof(timer_addr.sin_zero));
	int b = bind(timerSockfd,(struct sockaddr*)&timer_addr,timer_addrlen);
	if(b==0)
	{
		printf("Timer Socket binded\n");
	}
	else
	{
		printf("Timer Socket binding failed\n");
		close(timerSockfd);
		exit(EXIT_FAILURE);
	}
	
	int time1=0;
	
	while(1)
	{			
		
		
		struct timeval tv;
		tv.tv_sec=2;
		tv.tv_usec = 000000;
		
		
		int nflag,rv;
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(timerSockfd,&readfds);
		nflag = timerSockfd+1;
		rv = select(nflag,&readfds,NULL,NULL,&tv);
		
		
		
		if (rv == -1)
        {
                perror("select"); // error occurred in select()
        }
        else if (rv == 0)
        {
            printf("-\n");
        }
        else
        {
            if (FD_ISSET(timerSockfd, &readfds))
            {
                int chk = recvfrom(timerSockfd,(char *)t_msg,sizeof(TimerMsg),0,(struct sockaddr*)&tcpd_addr,&tcpd_addrlen);
				if(chk == -1)
				{
					printf("rcv error : %s \n",strerror(errno));
				}
				else
				{
					if(t_msg->flag == 1)
					{
						printf("add - seq : %lu , time : %d\n", t_msg->seq_num, t_msg->time);
						addNode(dll, t_msg->time, t_msg->port,t_msg->seq_num);
						printDLL(dll);
						
						printf("\n");
						memset((void*)t_msg,'\0',sizeof(TimerMsg));
					}
					else
					{
						printf("cancel - seq : %lu \n", t_msg->seq_num);
						cancelNode(dll, t_msg->seq_num);
						printDLL(dll);
						memset((void*)t_msg,'\0',sizeof(TimerMsg));
					}	
				}
                
                
            }
            
            sleep(2);
        }
        
        updateDLLHead(dll);
        removeHead(dll,timerSockfd);
        
        printf("time : %d\n",++time1);
		
	}	
	getchar();
	close(timerSockfd);
	printf("Sockets Closed \n");
	printf("Timer Ended\n");
}
