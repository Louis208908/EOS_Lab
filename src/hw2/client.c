#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#define MAX 80
#define PORT 8080
#define SA struct sockaddr

char recv_buff[MAX];
char send_buff[MAX];
int sockfd, connfd;
volatile int stop;

void func(int sockfd)
{
	int n;
	for (;;) {
		bzero(recv_buff, sizeof(recv_buff));
		printf("To server : ");
		n = 0;
		while ((recv_buff[n++] = getchar()) != '\n');
		write(sockfd, recv_buff, sizeof(recv_buff) - 1);
		bzero(recv_buff, sizeof(recv_buff));
		read(sockfd, recv_buff, sizeof(recv_buff));
		printf("From Server :%s\n", recv_buff);
		if(strstr(recv_buff, "Please wait a few seconds...\n") != NULL){
			bzero(recv_buff, sizeof(recv_buff));
			read(sockfd, recv_buff, sizeof(recv_buff));
			printf("From Server :%s\n", recv_buff);

		}
		if ((strncmp(recv_buff, "Exit", 4)) == 0) {
			printf("Client Exit...\n");
			break;
		}
	}
}


void intHandler(int signum)
{
    stop = 1;
    close(connfd);
    exit(-1);
}


int main(int argc, char **argv)
{

	signal(SIGINT, intHandler);
	
	struct sockaddr_in servaddr, cli;

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons((unsigned short)atoi(argv[1]));

	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))!= 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("connected to the server..\n");

	// function for chat
	func(sockfd);
    // while(1){
	// 	memset(recv_buff,'\0',sizeof(recv_buff));
	// 	memset(send_buff,'\0',sizeof(send_buff));
    //     read(sockfd, recv_buff, sizeof(recv_buff));
	// 	scanf("%s", send_buff);
    //     write(sockfd, send_buff, sizeof(send_buff));
    //     printf("%s\n", recv_buff);
        
    // }

	// close the socket
	close(sockfd);
}
