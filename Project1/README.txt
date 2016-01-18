********************* README FILE ***************************

****************** FILE INFORMATION ***********************
This file contains detailed instructions for compiling, executing and
evaluating Midterm Demo.

The directory should contain following files
1) ftps.c 
2) ftpc.c
3) TCPD_M1.c
4) TCPD_M2.c
5) crc.h
6) crc.c
7) packet.h
8) timerprocess.c
9) Jacobson.c
10) circularBuffer.c
11) troll
12) Makefile
13) README.txt

************* INDIVIDUAL FILE DISCRIPTIONS *****************

1) ftps.c

This file contains server hosting code. It works with one command
line argument which is used to specify port number on which you are listening from TROLL.

Execution Command:
ftps 8009

***NOTE: ftps program stores incoming file in /home/4/mohanr/file/
	 This can be modified by changing variable called "fileAddr"

2) ftpc.c

This file contains client hosting code. It works with single command
line arguments viz. filename of file to be sent over the network.

Execution Command:
ftpc 1.jpg

3) TCPD_M1.c

This file creates deamon process to receive packets from troll. It does not take any command 
line arguments.

Execution Command:
TCPD_M1

4) TCPD_M2.c

This file creates deamon process to send packets to troll. It does not take any command 
line arguments.

Execution Command:
TCPD_M2

5) crc.h

This is a header file containg definitions for crc computation.

6) crc.c

This file has function implementations for crc computation.

*** NOTE:  * Copyright (c) 2000 by Michael Barr.  This software is placed into the public domain and may be used for any purpose.  
    	   * However, this notice must not be changed or removed and no warranty is either expressed or implied by its publication 
	   * or distribution.

7) packet.h

Header files for packet definitions.

8) timerprocess.c

This file contains Delta_Timer code. It does not take any command line argument.
It is implemented using doubly-linked-list (Template) and added insertion and deletion functionalities.
It communicates with TCPD_M2 to receive and send notifications regarding new timer and timer expiry.

Execution Command:
tp

9) Jacobson.c

This file contains all library functions to compute SRTT, RTTVAR and RTO required by 
TCPD_M2 for preserving reliable communication.

10) circularBuffer.c

This file represents the necessary function for initialization and manipulation of circular
buffer at client's TCP Demon (TCPD_M2). The file supports addition, removal and read functionalities from
circular buffer.

11) troll
This is provided binary to generate network simulations with drop, garble and delays. It works
with seven command line arguments viz. 1) Packet Drop Percentage (Specified with -x flag), 

- Packet Gabriel Percentage (Specified with -g flag), 3) TCPD_M1 Host Name (Specified with -C flag), 

- TCPD_M2 Host Name (Specified with -S flag), 4) TCPD_M1 Listen PORT (Specified with -a flag),

- TCPD_M2_Send PORT (Specified with -b flag) and 6)TROLL Port 

Execution Command:
(For Communication Between TCPD_M2 to TCPD_M1) ***On beta.cse.ohio-state.edu
troll -x 10 -g 10 -C gamma -S beta -a 8009 -b 4009 4680

(For Communication Between TCPD_M1 to TCPD_M2) ***On gamma.cse.ohio-state.edu
troll -x 0 -g 0 -se 1000 -C beta -S gamma -a 7009 -b 5009 6009

***NOTE: After starting troll use command trace to view packet status.
*** PLEASE NOTE THAT TCPD_M1_Send PORT IS SET TO 4009.


12) Makefile

This file is used for compiling all necessary c files. It also has functionality
to remove executables.

Execution Command:
make

For removing compiled files and unncessary clutter.

Execution Command:
make clean

*************************** EXECUTION SEQUENCE CHECKSUM*************************
1) Start troll on beta.cse.ohio-state.edu as follows 
% troll -x 10 -g 10 -C gamma -S beta -a 8009 -b 4009 4680

2) Start troll on gamma.cse.ohio-state.edu as follows 
% troll -x 0 -g 0 -se 1000 -C beta -S gamma -a 7009 -b 5009 6009

3) Start timerprocess process on beta.cse.ohio-state.edu as follows 
% tp

4) Start TCPD_M2 on beta.cse.ohio-state.edu as follows
% TCPD_M2

5) Start TCPD_M1 on gamma.cse.ohio-state.edu as follows
% TCPD_M1

6) Start ftps on gamma.cse.ohio-state.edu as follows
% ftps 8009

7) Start ftpc on beta.cse.ohio-state.edu as follows
% ftpc 10kb.txt
*************************************************** END OF FILE ***********************





