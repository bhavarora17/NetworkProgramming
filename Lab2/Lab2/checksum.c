
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

long checksumcreate(unsigned short *addr, unsigned int count)
{
	    /* Compute Internet Checksum for "count" bytes
            *         beginning at location "addr".
            */
       register long sum = 0;


        while( count > 1 )  {
           /*  This is the inner loop */
               sum += * addr++;
               count -= 2;
       }
           /*  Add left-over byte, if any */
       if( count > 0 )
               sum += * (unsigned char *) addr;

           /*  Fold 32-bit sum to 16 bits */
       while (sum>>16)
           sum = (sum & 0xffff) + (sum >> 16);

       return ~sum;
 
	
}

void main(){
char a[] = "4500003h";
long sum = checksumcreate(a,8);
printf("The Checksum is %l", sum);

}
