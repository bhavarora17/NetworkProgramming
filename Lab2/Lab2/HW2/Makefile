all: server server1 client client1

server: Server/server.c
	gcc -o Server/server Server/server.c
server1: Server/tcpd_Server/tcpds.c
	gcc -o Server/tcpd_Server/tcpds Server/tcpd_Server/tcpds.c

client: Client/client.c
	gcc -o Client/client Client/client.c
client1: Client/tcpd_client/tcpdc.c
	gcc -o Client/tcpd_client/tcpdc Client/tcpd_client/tcpdc.c
