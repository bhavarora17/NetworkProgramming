/*
   FILE: Timer Program (Timer Implementation on Client Side)
*/

/****NOTE: Doubly linked list code adapted from : http://www.lemoda.net/c/doubly-linked-list/dll.c
*/
#include <netdb.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "packet.h"
#define MAX_MES_LEN 1024

#define TCPD_PORT_NUMBER 4689
#define TIMER_M2_PORT 9864


/*struct for data in timer list */
typedef struct timer_data timer_data ;

struct timer_data{
  void* info ;
  float dtime;
  int sequence_number;
};

/* The type link_t needs to be forward-declared in order that a
   self-reference can be made in "struct link" below. */

typedef struct link link_t;

/* A link_t contains one of the links of the linked list. */

struct link {
    timer_data *data;
    link_t *prev;
    link_t *next;
};

/* linked_list_t contains a linked list. */

typedef struct linked_list {
    link_t *first;
    link_t *last;
}
linked_list_t;

//function definitions
static void copy(struct timer_packet*, struct timer_data*); 
static void linked_list_init (linked_list_t* );
static void linked_list_add (linked_list_t* , timer_data*);
static void linked_list_delete (linked_list_t* , link_t*,unsigned long );
static void linked_list_traverse (linked_list_t* , void (*callback) (timer_data*));
static void linked_list_traverse_in_reverse (linked_list_t*,void (*callback) (timer_data*));
static void linked_list_del_seq_no (linked_list_t* , int, unsigned long );
static void linked_list_free (linked_list_t* );
static void print_list (timer_data*);
//end of function definitions
 

/* The following function copies fields from source structure into
   destination struncture */

static void copy(struct timer_packet* src, struct timer_data* dest) {
  dest->dtime = src->time;
  dest->sequence_number = src->sequence_number;
}


/* The following function initializes the linked list by putting zeros
   into the pointers containing the first and last links of the linked
   list. */

static void linked_list_init (linked_list_t * list){
    list->first = list->last = 0;
}

/* The following function adds a new link to the end of the linked
   list. It allocates memory for it. The contents of the link are
   copied from "data". */

static void linked_list_add (linked_list_t * list, timer_data *data){
    
  link_t * link;
  link_t *list_node;
    /* calloc sets the "next" field to zero. */
    link = calloc (1, sizeof (link_t));
    if (! link) {
        fprintf (stderr, "calloc failed.\n");
        exit (EXIT_FAILURE);
    }
    link->data = data;

    list_node = calloc (1, sizeof (link_t));
    if (! list_node) {
        fprintf (stderr, "calloc failed.\n");
        exit (EXIT_FAILURE);
    }

    //List is Empty
    if(list->first == NULL){
      list->first = link;
      list->last = link;
    }
    else{
      list_node = list->first;
    
      while(1){
	/* Only Single Element in the list */
	if(list_node->prev == NULL && list_node->next == NULL){
	  if(link->data->dtime < list_node->data->dtime){
	    link->next = list_node;
	    list_node->prev = link;
	    list_node->data->dtime = list_node->data->dtime - link->data->dtime;
	    list->first = link;
	    break;
	  }
	  else {
	    link->prev = list_node;
	    list_node->next = link;
	    link->data->dtime = link->data->dtime - list_node->data->dtime;
	    list->last = link;
	    break;
	  }
	}
	/* First Element in the List */
	else if(list_node->prev == NULL && list_node->next != NULL){
	  if(link->data->dtime < list_node->data->dtime){
	    link->next = list_node;
	    list_node->prev = link;
	    list_node->data->dtime = list_node->data->dtime - link->data->dtime;
	    list->first = link;
	    break;
	  }
	  else {
	    link->data->dtime = link->data->dtime - list_node->data->dtime;
	    list_node = list_node->next;
	  }
	}
	/* Last Element in the List */
	else if(list_node->prev != NULL && list_node->next == NULL){
	  if(link->data->dtime < list_node->data->dtime){
	    link->next = list_node;
	    link->prev = list_node->prev;
	    list_node->prev->next = link;
	    list_node->prev = link;
	    list_node->data->dtime = list_node->data->dtime - link->data->dtime;
	    break;
	  }
	  else {
	    link->prev = list_node;
	    list_node->next = link;
	    link->data->dtime = link->data->dtime - list_node->data->dtime;
	    list->last = link;
	    break;
	  }
	}
	else {
	  if(link->data->dtime < list_node->data->dtime){
	    link->next = list_node;
	    link->prev = list_node->prev;
	    list_node->prev->next = link;
	    list_node->prev = link;
	    list_node->data->dtime = list_node->data->dtime - link->data->dtime;
	    break;
	  }
	  else {
	    link->data->dtime = link->data->dtime - list_node->data->dtime;
	    list_node = list_node->next;
	  }
	}
      }
    }      
    linked_list_traverse (list,print_list);
}

/* The following function is used to remove specified node from delta_timer List while
   simultaneously accounting for passed_time */

static void linked_list_delete (linked_list_t * list, link_t * link, unsigned long passed_time){
    link_t * prev;
    link_t * next;

    prev = link->prev;
    next = link->next;
    if (prev) {
        if (next) {
            /* Both the previous and next links are valid, so just
               bypass "link" without altering "list" at all. */
            prev->next = next;
            next->prev = prev;
	    next->data->dtime = next->data->dtime + link->data->dtime ;
 	    list->first->data->dtime = list->first->data->dtime - passed_time;
        }
        else {
            /* Only the previous link is valid, so "prev" is now the
               last link in "list". */
            prev->next = 0;
            list->last = prev;
 	    list->first->data->dtime = list->first->data->dtime - passed_time;
        }
    }
    else {
        if (next) {
            /* Only the next link is valid, not the previous one, so
               "next" is now the first link in "list". */
            next->prev = 0;
            list->first = next;
 	    list->first->data->dtime = list->first->data->dtime + (link->data->dtime - passed_time);
        }
        else {
            /* Neither previous nor next links are valid, so the list
               is now empty. */
            list->first = 0;
            list->last = 0;
        }
    }
    free (link);
    linked_list_traverse (list,print_list);
}

/* The function is used to traverse the entire Delta_Timer List */

static void linked_list_traverse (linked_list_t * list, void (*callback) (timer_data*)){
    link_t * link;
    printf(" ******current delta list ****** \n");
    for (link = list->first; link; link = link->next) {
        callback ((void *) link->data);
    }
	printf("\n");
}

static void linked_list_traverse_in_reverse (linked_list_t *list,void (*callback) (timer_data*)){
    link_t * link;

    for (link = list->last; link; link = link->prev) {
        callback ((void *) link->data);
    }
}


static void linked_list_del_seq_no (linked_list_t * list, int sequence_number, unsigned long passed_time){
    link_t * link;

    for (link = list->first; link; link = link->next) {
        if (link->data->sequence_number== sequence_number) {
	  linked_list_delete (list, link, passed_time);
        }
    }
}


/* Free the list's memory. */

static void linked_list_free (linked_list_t * list) {
    link_t * link;
    link_t * next;
    for (link = list->first; link; link = next) {
        /* Store the next value so that we don't access freed
           memory. */
        next = link->next;
        free (link);
    }
}


/* Example callback function. */

static void print_list (timer_data *data){
  printf("[%.2f , %d]-->",data->dtime,data->sequence_number);	   
}

/* Make a list of words and then print them out. */

int main () {

  int sock,send_sock, namelen, timer_requestlen, msg, exp_seq_num ; 
  fd_set readset;
  struct timer_packet * timer_request = calloc(1, sizeof(struct timer_packet));  
  struct sockaddr_in tcpd_name;
  struct sockaddr_in timer_M2_name;
  struct hostent *hp, *gethostbyname();
  linked_list_t list;

  /*create socket*/
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if(sock < 0) {
    perror("opening datagram socket");
    exit(1);
  }
  send_sock = socket(AF_INET, SOCK_DGRAM, 0);
  if(send_sock < 0) {
    perror("opening datagram socket");
    exit(1);
  }
	
  /* create name with parameters and bind name to socket */
  tcpd_name.sin_family = AF_INET;
  tcpd_name.sin_port = htons(TCPD_PORT_NUMBER);
  tcpd_name.sin_addr.s_addr = INADDR_ANY;
  if(bind(sock, (struct sockaddr *)&tcpd_name, sizeof(tcpd_name)) < 0) {
    perror("getting socket name");
    exit(2);
  }
  /****create name with parameters****/
  timer_M2_name.sin_family = AF_INET;
  timer_M2_name.sin_port = htons(TIMER_M2_PORT);
  hp = gethostbyname("localhost");
  if(hp == 0) {
    fprintf(stderr, "%s:unknown host\n");
    exit(3);
  }
  bcopy((char *)hp->h_addr, (char *)&timer_M2_name.sin_addr, hp->h_length);
	
  namelen=sizeof(struct sockaddr_in);

  /* List Manipulation */
  linked_list_init(&list);
    
  while(1) {
    // CHECK FOR LIST IS EMPTY
    FD_ZERO(&readset);
    FD_SET(sock, &readset);
    printf("IN WAIT...\n");
    if(list.first == NULL) {
      printf("Delta Timer Empty...\n");
      if(select(FD_SETSIZE+1, &readset, NULL, NULL, NULL) < 0 ) {
	perror("select Line 335");
	exit (EXIT_FAILURE);
      }
      else {
	if(FD_ISSET(sock, &readset)) {
	  if(recvfrom(sock, timer_request, MAX_MES_LEN, 0, (struct sockaddr *)&tcpd_name, &namelen) < 0) {
	    perror("error receiving"); 
	    exit(4);
	  }
	  else {
	    printf("TCPD_M2 --> TIMER :: RECVD TIMER_PACKET-> sequence_number: %d\n",timer_request->sequence_number);
	    if(timer_request->type == 0){
	      struct timer_data * packet = calloc (1,sizeof(struct timer_data));
	      printf("\nEVENT: START TIMER FOR Packet-> Sequence Number: %d\t Expire Time: %.2f\n",timer_request->sequence_number,timer_request->time);
	      copy(timer_request,packet);
	      linked_list_add(&list, packet);
	    }
	    else if (timer_request->type == 1) {
	      perror("Empty List...Cannot Delete...\n");
	      exit(7);
	    }
	    else {
	      perror("Undefined Packet Type");
	      exit(6);
	    }
	  }
	}
	// SINCE LIST IS EMPTY AND TIMER EXPIRED WITHOUT FD_ISSET
	else {
	  perror("FD_ISSET");
	  exit(5);
	}
      }
    }
    else{
      struct timeval tv;
      unsigned long wait_time;
      /* Wait up to dtime in head of the list. */
      wait_time = list.first->data->dtime;
      tv.tv_sec = list.first->data->dtime;
      tv.tv_usec = 0;
			
       if(select(FD_SETSIZE+1, &readset, NULL, NULL, &tv) < 0 ) {
	perror("select Line 377");
	exit (EXIT_FAILURE);
      }
      else {
	if(FD_ISSET(sock, &readset)) {
	  if(recvfrom(sock, timer_request, MAX_MES_LEN, 0, (struct sockaddr *)&tcpd_name, &namelen) < 0) {
	    perror("error receiving"); 
	    exit(4);
	  }
	  else {
	    printf("TCPD_M2 --> TIMER :: RECVD TIMER_PACKET-> sequence_number: %d\n",timer_request->sequence_number);
	    if(timer_request->type == 0){
	      struct timer_data * packet = calloc (1,sizeof(struct timer_data));
	      printf("\nEVENT: START TIMER FOR Packet-> sequence Number: %d\t Expire Time: %.2f\n",timer_request->sequence_number,timer_request->time);
	      copy(timer_request,packet);
	      list.first->data->dtime = tv.tv_sec;
	      linked_list_add(&list, packet);
	    }
	    else if (timer_request->type == 1) {
	      printf("\nEVENT: CANCLE TIMER FOR PACKET-> sequence Number: %d\n",timer_request->sequence_number);
	      linked_list_del_seq_no(&list, timer_request->sequence_number, wait_time - tv.tv_sec);
	    }
	    else {
	      perror("Undefined Packet Type");
	      exit(6);
	    }
	  }
	}
	else {
	  printf("\nEVENT: TIMER EXPIRED Packet-> sequence Number: %d\n",list.first->data->sequence_number);
	  linked_list_traverse (&list,print_list);
	  exp_seq_num = list.first->data->sequence_number ;
	  printf("TIMER --> TCPD_M2 :: SEND EXPIRED PACKET-> sequence_number: %d\n",exp_seq_num);
	  if(sendto(send_sock, &exp_seq_num, sizeof(int), 0, (struct sockaddr *)&timer_M2_name, sizeof(timer_M2_name)) < 0) {
	    perror("ERROR SENDING PACKET TO TIMER");
	    exit(4);
	  }
	  linked_list_del_seq_no(&list,list.first->data->sequence_number, wait_time);
	}
      }
    }     
  }
  return 0;
}
