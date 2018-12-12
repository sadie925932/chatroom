#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "proto.h"
#include "string.h"

// Global variables
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int open = 0;
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char nickname[LENGTH_NAME] = {};

void catch_ctrl_c_and_exit(int sig) {
	flag = 1;
}

void recv_msg_handler() {
	char tmp[20] = {};
	char filename[LENGTH_NAME] = {};
	char receiveMessage[LENGTH_SEND] = {};
	FILE *fd;
	while (1) {
		int receive = recv(sockfd, receiveMessage, LENGTH_SEND, 0);
		if(strcmp(receiveMessage, "send_sen") == 0)
		{
			memset(receiveMessage, 0, sizeof(receiveMessage));
			receive = recv(sockfd, receiveMessage, LENGTH_SEND, 0);
			if (receive > 0) {
				printf("\r%s\n", receiveMessage);
				str_overwrite_stdout();
			} else if (receive == 0) {
				break;
			} else { 
				// -1 
			}
		}
		else if(strcmp(receiveMessage, "send_check") == 0)
		{
			memset(receiveMessage, 0, sizeof(receiveMessage));
			receive = recv(sockfd, receiveMessage, LENGTH_SEND, 0);
			if (receive > 0) {
				printf("Online : %s\n", receiveMessage);
				printf("-----------------------\n");
				str_overwrite_stdout();
			} else if (receive == 0) {
				break;
			} else {
				// -1
			}
		}
		else if(strcmp(receiveMessage, "send_file") == 0)
		{
			printf("Do you want to receive the file?\n");
			printf("yes or no\n");
			str_overwrite_stdout();

			receive = recv(sockfd, receiveMessage, LENGTH_SEND, 0); 
			if(strcmp(receiveMessage, "refuse") == 0) {
				printf("Refuse Success\n");
				continue;
			}
			else if(strcmp(receiveMessage, "accept") == 0)
			{
				printf("Accept success! Please Wait! \n");
				memset(receiveMessage, 0, sizeof(receiveMessage));
				receive = recv(sockfd, filename, LENGTH_NAME, 0); //filename
				printf("Filename : %s\n",filename);
				fd = fopen(filename, "w+");
				receive = recv(sockfd, receiveMessage, LENGTH_SEND, 0); //text
				if(receive > 0)
				{
					while(strcmp(receiveMessage, "EOF") != 0)
					{
						fprintf(fd,"%s",receiveMessage);
						memset(receiveMessage, 0, sizeof(receiveMessage));
						receive = recv(sockfd, receiveMessage, LENGTH_SEND, 0);
					}
					fclose(fd);
					printf("Create the file : %s\n", filename);
					//	str_overwrite_stdout();
				}
			}
			else break;
		}
		else if(strcmp(receiveMessage, "yes") == 0)
		{
			printf("get yes\n");
			open = 1;
			pthread_cond_signal(&cond);
		}
		else if(strcmp(receiveMessage, "no") == 0)
		{
			printf("get no\n");
			open = 0;
			pthread_cond_signal(&cond);
			continue;
		}
		else {
			printf("receive error\n");
			break;
		}
		memset(receiveMessage, 0, sizeof(receiveMessage));
	}
}

void send_msg_handler() {
	char recv_buffer[LENGTH_MSG] = {};
	char message[LENGTH_MSG] = {};
	char option[35] = {};
	char condition[20] = {};
	char filename[20] = {};
	char tmp[20]= {};
	FILE *fd;
	while (1) 
	{
		open = 0;
		memset(option, 0, sizeof(option));
		memset(condition, 0,sizeof(condition));
		printf("1.Send to all peple 2.Send to somebody 3.Online people\n");
		str_overwrite_stdout(); //print >
		if(fgets(option, LENGTH_NAME, stdin) != NULL) //choose people
		{
			str_trim_lf(option, LENGTH_NAME);
			if(strcmp(option, "1") == 0) //all people
			{ 
				memset(option, 0, sizeof(option));
				printf("1.Send a sentence 2.Send a file\n");
				str_overwrite_stdout(); //print >
				if(fgets(option, LENGTH_NAME, stdin) != NULL) {
					str_trim_lf(option, LENGTH_NAME);
					if(strcmp(option, "1") == 0) //send text to all people
					{
						str_overwrite_stdout(); //print >
						strcpy(condition, "all_sentence");
						send(sockfd, condition, 20, 0);
						while (fgets(message, LENGTH_MSG, stdin) != NULL) {
							str_trim_lf(message, LENGTH_MSG);
							if (strlen(message) == 0) {
								str_overwrite_stdout(); //printf >
							} 
							else 
								break;
						}	
						send(sockfd, message, LENGTH_MSG, 0);
					}
					else if(strcmp(option, "2") == 0) //send file to all people
					{
						continue;
					}
					else
					{
						continue;
					}
				}
				else break;
			}
			else if(strcmp(option, "2") == 0) // send to someone
			{
				strcpy(condition, "sbname");
				send(sockfd, condition, 20, 0);
				memset(option, 0, sizeof(option));
				printf("Please enter his/her name\n");
				str_overwrite_stdout(); //print >
				if(fgets(option, LENGTH_NAME, stdin) != NULL) { //someone name
					str_trim_lf(option, LENGTH_NAME);
					memset(condition, 0, sizeof(condition));
					send(sockfd, option, LENGTH_NAME, 0);
				}
				memset(option, 0, sizeof(option));
				printf("1.Send a sentence 2.Send a file\n");
				str_overwrite_stdout(); //print >
				if(fgets(option, LENGTH_NAME, stdin) != NULL) {
					str_trim_lf(option, LENGTH_NAME);
					if(strcmp(option, "1") == 0) {//send text to all people
						str_overwrite_stdout(); //print >
						while (fgets(message, LENGTH_MSG, stdin) != NULL) {
							str_overwrite_stdout();
							str_trim_lf(message, LENGTH_MSG);
							if (strlen(message) == 0) {
								str_overwrite_stdout(); //printf >
							}
							else
								break;
						}
						strcpy(condition, "sb_sentence");
						send(sockfd, condition, 20 , 0);
						send(sockfd, message, LENGTH_MSG, 0);
					}
					else if(strcmp(option, "2") == 0) //send file
					{
						memset(condition, 0, sizeof(condition));
						strcpy(condition, "sb_file");
						send(sockfd, condition, 20, 0);
						pthread_cond_wait(&cond,&mutex);
						if(open == 1)
						{
							printf("Please enter your file name\n");
							str_overwrite_stdout(); //print >
							while(fgets(filename, 20, stdin) !=NULL) { //filename
								str_trim_lf(filename, LENGTH_NAME);
								send(sockfd, filename, LENGTH_NAME, 0);
								fd = fopen(filename, "r");
								if(!fd) {
									printf("Fail to open the file\n");
									continue;
								}
								while(fgets(message, LENGTH_MSG, fd) !=NULL) {
							                //str_trim_file(message, LENGTH_MSG); //
									if (strlen(message) == 0) {
										str_overwrite_stdout(); //printf >
									}
									else
										send(sockfd, message, strlen(message), 0);
								}
								sleep(3);
								memset(message, 0, sizeof(message));
								strcpy(message, "EOF");
								send(sockfd, message, LENGTH_MSG, 0);
								fclose(fd);
								break;
							}
						}
						else {								
							printf("He/Her doens't receive the file\n");
							continue;
						}

					}
					else
					{
						continue;
					}
				}		
			}
			else if(strcmp(option, "3") == 0)
			{
				strcpy(condition, "check");
				send(sockfd, condition, 20, 0);

			}
			else if(strcmp(option, "yes") == 0)
			{
				memset(condition, 0, sizeof(condition));
				printf("-------PLEASE WAIT-------\n");
				printf("-------SENDINGINFO-------\n");
				strcpy(condition, "yes");
				send(sockfd, condition, 20, 0);
			}
			else if(strcmp(option, "no") == 0)
			{
				memset(condition, 0, sizeof(condition));
				sleep(1);
				printf("-------------------------\n");
				strcpy(condition, "no");
				send(sockfd, condition, 20 ,0);
			}
			else break;
		}
		if (strcmp(option, "exit") == 0) {
			break;
		}
	}
	catch_ctrl_c_and_exit(2); 
}

int main()
{
	signal(SIGINT, catch_ctrl_c_and_exit);

	// Naming
	printf("Please enter your name: ");
	if (fgets(nickname, LENGTH_NAME, stdin) != NULL) {
		str_trim_lf(nickname, LENGTH_NAME);
	}
	if (strlen(nickname) < 2 || strlen(nickname) >= LENGTH_NAME-1) {
		printf("\nName must be more than one and less than thirty characters.\n");
		exit(EXIT_FAILURE);
	}

	// Create socket
	sockfd = socket(AF_INET , SOCK_STREAM , 0);
	if (sockfd == -1) {
		printf("Fail to create a socket.");
		exit(EXIT_FAILURE);
	}

	// Socket information
	struct sockaddr_in server_info, client_info;
	int s_addrlen = sizeof(server_info);
	int c_addrlen = sizeof(client_info);
	memset(&server_info, 0, s_addrlen);
	memset(&client_info, 0, c_addrlen);
	server_info.sin_family = PF_INET;
	server_info.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_info.sin_port = htons(8888);

	// Connect to Server
	int err = connect(sockfd, (struct sockaddr *)&server_info, s_addrlen);
	if (err == -1) {
		printf("Connection to Server error!\n");
		exit(EXIT_FAILURE);
	}

	// Names
	getsockname(sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
	getpeername(sockfd, (struct sockaddr*) &server_info, (socklen_t*) &s_addrlen);
	printf("Connect to Server: %s:%d\n", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));
	printf("You are: %s:%d\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

	send(sockfd, nickname, LENGTH_NAME, 0);

	pthread_t send_msg_thread;
	if (pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0) {
		printf ("Create pthread error!\n");
		exit(EXIT_FAILURE);
	}

	pthread_t recv_msg_thread;
	if (pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0) {
		printf ("Create pthread error!\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		if(flag) {
			printf("\nBye\n");
			break;
		}
	}

	close(sockfd);
	return 0;
}
