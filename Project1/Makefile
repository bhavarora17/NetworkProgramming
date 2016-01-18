# Makefile for client and server

CC = gcc
OBJCLI = ftpc.c crc.c
OBJSRV = ftps.c
OBJTCPD_M2 = TCPD_M2.c crc.c
OBJTCPD_M1 = TCPD_M1.c crc.c
OBJTP = timerprocess.c
CFLAGS = 
# setup for system
LIBS =

all: ftpc ftps TCPD_M2 TCPD_M1 tp

ftpc:	$(OBJCLI)
	$(CC) $(CFLAGS) -o $@ $(OBJCLI) $(LIBS)

ftps:	$(OBJSRV)
	$(CC) $(CFLAGS) -o $@ $(OBJSRV) $(LIBS)

TCPD_M2:	$(OBJTCPD_M2)
	$(CC) $(CFLAGS) -o $@ $(OBJTCPD_M2) $(LIBS)

TCPD_M1:	$(OBJTCPD_M1)
	$(CC) $(CFLAGS) -o $@ $(OBJTCPD_M1) $(LIBS)

tp:	$(OBJTP)
	$(CC) $(CFLAGS) -o $@ $(OBJTP) $(LIBS)

clean:
	rm ftpc ftps TCPD_M2 TCPD_M1 tp
	
	
