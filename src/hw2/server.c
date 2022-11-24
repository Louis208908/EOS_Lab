#pragma optimize( "", off) 

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()
#include <signal.h>
#include "stdbool.h"



#define min(X,Y) ((X) < (Y) ? (X) : (Y))
#define max(X,Y) ((X) > (Y) ? (X) : (Y))

#define MAX 80
#define PORT 8080
#define SA struct sockaddr
char send_buff[MAX];
char recv_buff[MAX];
int sockfd, connfd, len;
volatile int stop = 0;


void intHandler(int signum){
    stop = 1;
    close(connfd);
    exit(-1);
}

void *process(void *);

// Function designed for chat between client and server.
void func(int connfd)
{
	int n;
	// infinite loop for chat
	for (;;) {
		bzero(recv_buff, MAX);

		// read the message from client and copy it in buffer
		read(connfd, recv_buff, sizeof(recv_buff));
		// print buffer which contains the client contents
		printf("From client: %s\nTo client : ", recv_buff);
		bzero(recv_buff, MAX);
		n = 0;
		// copy server message in the buffer
		while ((recv_buff[n++] = getchar()) != '\n');

		// and send that buffer to client
		write(connfd, recv_buff, sizeof(recv_buff));

		// if msg contains "Exit" then server exit and chat ended.
		if (strncmp("exit", recv_buff, 4) == 0) {
			printf("Server Exit...\n");
			break;
		}
	}
}



typedef struct Case{
    int total_case;
    int mild_case[9];
    int severe_case[9];

    // add function pointer to set case here
    void (*set_case)(struct Case *self, int area, int case_condition, int case_amount);
    // add function pointer returning both cases by reference here
    void (*get_case)(struct Case *self, int area, int *mild_case, int *severe_case);
    // add function pointer to check if an area has cases
    bool (*has_case)(struct Case *self, int area);
    // add function pointer to get total cases by reference
    void (*get_total_case)(struct Case *self, int *total_case);
    
    

}Case;

Case case_record;

// accept user input and set case
void set_case(Case *self, int area, int case_condition, int case_amount){
    if(case_condition == 'm'){
        self->mild_case[area] += case_amount;
    }
    else if(case_condition == 's'){
        self->severe_case[area] += case_amount;
    }
}

// actual implementation of get_case
void get_case(Case *self, int area, int *mild_case, int *severe_case){
    *mild_case = self->mild_case[area];
    *severe_case = self->severe_case[area];
}

// check if an area has cases
bool has_case(Case *self, int area){
    if(self->mild_case[area] > 0 || self->severe_case[area] > 0){
        return true;
    }
    else{
        return false;
    }
}

// get total cases
void get_total_case(Case *self, int *total_case){
    int i;
    for(i = 0; i < 9; i++){
        *total_case += self->mild_case[i];
        *total_case += self->severe_case[i];
    }
}


// Driver function
int main(int argc, char **argv){

	signal(SIGINT, intHandler);
	struct sockaddr_in servaddr, cli;
	pthread_t threadd,thr;


	// initializing function pointer
	case_record.set_case = set_case;
	case_record.get_case = get_case;
	case_record.has_case = has_case;
	case_record.get_total_case = get_total_case;




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
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons((unsigned short)atoi(argv[1]));

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		printf("socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

	// Now server is ready to listen and verification
	if ((listen(sockfd, 5)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	len = sizeof(cli);

	// Accept the data packet from client and verification
	connfd = accept(sockfd, (SA*)&cli, &len);
	if (connfd < 0) {
		printf("server accept failed...\n");
		exit(0);
	}
	else
		printf("server accept the client...\n");
	

	// Function for chatting between client and server
	// func(connfd);
    while(!stop){
        // int temp;
        // scanf("%d", &temp);
        // sprintf(send_buff,"%d\0", temp);
        // write(connfd, send_buff, sizeof(send_buff));
		int pth_c = pthread_create(&threadd, NULL, process, (void *)connfd);
		// write(connfd,send_buff,MAX);
    }

	// After chatting close the socket
	close(sockfd);
}



void *process(void *connfd){
	while(1){
		int n = 0;
		bzero(recv_buff,MAX);
		bzero(send_buff,MAX);
		// while(1){
		if(( n = read((intptr_t)connfd,recv_buff,MAX)) == -1){
			// printf("recv length = %d\n", n);
			printf("Error: read() \n");
		}
		// recv_buff[n] = '\0';
		// printf("%d\n%s", strcmp(recv_buff, "list"), recv_buff);
		if(strstr(recv_buff, "list") != 0){
			printf("Get a list cmd.\n");
			int len = sprintf(send_buff,"\n1. Confirmed case\n2. Reporting system\n3. Exit\n");
			write((intptr_t)connfd,send_buff,len);

		}
		else{
			char *order[100];
			char delim[] = "|";
			char* cmd;
			int cmd_amount;


			cmd = strtok(recv_buff,delim);
			int i = 0;
			while(cmd != NULL){
				order[i++] = cmd;
				// printf("%s\n", cmd);
				// write((intptr_t)connfd,cmd,strlen(cmd));
				cmd = strtok(NULL,delim);
			}
			cmd_amount = i;

			if(strstr(order[0], "Confirmed case") != 0){
				// Confirmed cases at all area
				if(cmd_amount == 1){
					printf("Get a confirmed case cmd.\n");
					char total_report[100];
					strcat(total_report, "\n");
					for(int i = 0; i < 9; i ++){
						// creating a string to store the area number and area total cases
						char area[10];
						int mid_case, ser_case;
						// get cases of the area and add them all
						case_record.get_case(&case_record, i, &mid_case, &ser_case);
						int total_case = mid_case + ser_case;
						// convert the area number to string
						sprintf(area, "%d : %d\n", i,total_case);
						// concat the area string to the total report string
						strcat(total_report, area);

					}
					// write the total report to the client
					write((intptr_t)connfd,total_report,strlen(total_report));
				}
				else{
					// Confirmed cases at specific area
					printf("Get a confirmed case cmd.\n");

					char total_report[100];
					char *which_area[100];
					char delim[] = " ";
					char* area;
					int area_amount;
					strcat(total_report, "\n");
					for(int i = 1; i < cmd_amount; i++){
						int j = 0;
						char area_report[100];
						area = strtok(order[i],delim);
						while(area != NULL){
							which_area[j++] = area;
							area = strtok(NULL,delim);
						}
						int area_number = atoi(which_area[1]);
						int mid_case, ser_case;
						case_record.get_case(&case_record, area_number, &mid_case, &ser_case);
						sprintf(area_report, "Area %d - Mild : %d | Severe %d\n", area_number, mid_case , ser_case);
						strcat(total_report, area_report);
					}
					write((intptr_t)connfd,total_report,strlen(total_report));
				}
				
			}

			if(strstr(order[0], "Reporting system") != 0){
				printf("Get a reporting system cmd.\n");
				char wait_report[100];
				sprintf(wait_report,"Please wait a few seconds...\n");
				write((intptr_t)connfd,wait_report,strlen(wait_report));
				char *which_area[100];
				char *case_report[100];
				// printf("%s\n", order[1]);
				// printf("%s\n", order[2]);
				char *area_indicator;
				char *cases;
                char cmd_report[100];
				int max_sleep_amount = 0;

				for(int j = 1; j < cmd_amount; j += 2){
					// printf("Now j = %d\n", j);
					int i = 0;
					area_indicator = strtok(order[j]," ");
					// area_indicator = strtok(order[1]," ");
					while(area_indicator != NULL){
						which_area[i++] = area_indicator;
						area_indicator = strtok(NULL," ");
					}

					i = 0;
					cases = strtok(order[j + 1]," ");
					// cases = strtok(order[2]," ");
					while(cases != NULL){
						case_report[i++] = cases;
						cases = strtok(NULL," ");
					}

					int target_area = atoi(which_area[1]);
					int target_case_amount = atoi(case_report[1]);
					max_sleep_amount = max(max_sleep_amount, target_case_amount);

					// sleep(target_area);

					if(strstr(case_report[0], "Mild") != 0){
						// the case is mild
						printf("At area: %d, reporting %d mild cases.\n", target_area, target_case_amount);
						case_record.set_case(&case_record, target_area, 'm' , target_case_amount);
					}
					else{
						printf("At area: %d, reporting %d severe cases.\n", target_area, target_case_amount);
						case_record.set_case(&case_record, target_area, 's' , target_case_amount);
					}
                    char area_report[100];
                    sprintf(area_report, "Area %d | %s %d\n", target_area, case_report[0], target_case_amount);
                    strcat(cmd_report, area_report);
				}
				sleep(max_sleep_amount);
                write((intptr_t)connfd,cmd_report,strlen(cmd_report));

			}

			if(strstr(order[0], "Exit") != 0){
				printf("Get an exit cmd.\n");
				char exit_report[100];
				sprintf(exit_report, "Exit\n");
				write((intptr_t)connfd,exit_report,strlen(exit_report));
				stop = 1;
				break;
			}
		}
	}

}