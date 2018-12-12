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
#include "server.h"

// Global variables
int server_sockfd = 0, client_sockfd = 0;
ClientList *root, *now;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int open = 0;

void catch_ctrl_c_and_exit(int sig) {
	ClientList *tmp;
	while (root != NULL) {
		printf("\nClose socketfd: %d\n", root->data);
		close(root->data); // close all socket include server_sockfd
		tmp = root;
		root = root->link;
		free(tmp);
	}
	printf("Bye\n");
	exit(EXIT_SUCCESS);
}

void send_to_all_clients(ClientList *np, char tmp_buffer[]) {
	ClientList *tmp = root->link;
	while (tmp != NULL) { //all client until null
		if (np->data != tmp->data) { // all clients except itself.
			printf("Send to sockfd %d: \"%s\" \n", tmp->data, tmp_buffer);
			send(tmp->data, tmp_buffer, LENGTH_SEND, 0);
		}
		tmp = tmp->link;
	}
}

void client_handler(void *p_client) {
	int leave_flag = 0;
	int sbsockfd = 0,sbflag = 0;
	char nickname[LENGTH_NAME] = {};
	char recv_buffer[LENGTH_MSG] = {};
	char send_buffer[LENGTH_SEND] = {};
	char option_buffer[LENGTH_NAME] = {};
	ClientList *np = (ClientList *)p_client;
	ClientList *tmp = root;

	// Naming(client nickname)
	if (recv(np->data, nickname, LENGTH_NAME, 0) <= 0 || strlen(nickname) < 2 || strlen(nickname) >= LENGTH_NAME-1) { //error nickname
		printf("%s didn't input name.\n", np->ip);
		leave_flag = 1;
	} 
	else {
		strcpy(send_buffer, "send_sen");
		send_to_all_clients(np, send_buffer);
		strncpy(np->name, nickname, LENGTH_NAME);
		printf("%s(%s)(%d) join the chatroom.\n", np->name, np->ip, np->data);
		sprintf(send_buffer, "%s(%s) join the chatroom.", np->name, np->ip);
		send_to_all_clients(np, send_buffer);
		memset(send_buffer, 0, sizeof(send_buffer));
	}

	// Conversation
	while (1)	
	{
		tmp = root;
		if (leave_flag) {
			break;
		}
		memset(recv_buffer, 0, sizeof(recv_buffer));
		int receive = recv(np->data, recv_buffer, LENGTH_MSG, 0); //read_condition
		if (receive > 0) //read
		{ 
			if (strlen(recv_buffer) == 0) {
				continue;
			}
			else if(strcmp(recv_buffer, "all_sentence") == 0) 
			{
				strcpy(send_buffer, "send_sen"); //send condition to clients
				send_to_all_clients(np, send_buffer);
				memset(recv_buffer, 0, sizeof(recv_buffer));
				receive = recv(np->data, recv_buffer, LENGTH_MSG, 0); //read_message(sentence)
				if (receive > 0) { 
					if (strlen(recv_buffer) == 0) {
						continue;
					}
					memset(send_buffer, 0, sizeof(send_buffer));
					sprintf(send_buffer, "[PUBLIC] %s：%s ", np->name, recv_buffer);
				} 
				else if (receive == 0 || strcmp(recv_buffer, "exit") == 0) {
					strcpy(send_buffer, "send_sen");
					send_to_all_clients(np, send_buffer);
					printf("%s(%s)(%d) leave the chatroom.\n", np->name, np->ip, np->data);
					sprintf(send_buffer, "%s(%s) leave the chatroom.", np->name, np->ip);
					leave_flag = 1;
				} 
				else {
					printf("A:Fatal Error: -1\n");
					leave_flag = 1;
				}
				send_to_all_clients(np, send_buffer);
			}

			else if(strcmp(recv_buffer, "all_file") == 0)
			{
				printf("Still working on it !\n");
			/*
			    strcpy(send_buffer, "send_file"); //send condition to clients
			    send_to_all_clients(np, send_buffer);

			    memset(recv_buffer, 0, sizeof(recv_buffer));
			    receive = recv(np->data, recv_buffer, LENGTH_MSG, 0); //yes or no
			    send_to_all_clients(np, send_buffer);

			    memset(recv_buffer, 0, sizeof(recv_buffer));
			    receive = recv(np->data, recv_buffer, LENGTH_MSG, 0); //filename

			    memset(send_buffer, 0, sizeof(send_buffer));
			    sprintf(send_buffer, "%s", recv_buffer);
			    send_to_all_clients(np, send_buffer); //send filename to clients

			    memset(send_buffer, 0, sizeof(send_buffer));
			    memset(recv_buffer, 0, sizeof(recv_buffer));

			    receive = recv(np->data, recv_buffer, LENGTH_MSG, 0);
			    while(strcmp(recv_buffer, "EOF") != 0)
			    {
			    if (receive == 0 || strcmp(recv_buffer, "exit") == 0) {
			    strcpy(send_buffer, "send_sen");
			    send_to_all_clients(np, send_buffer);
			    printf("%s(%s)(%d) leave the chatroom.\n", np->name, np->ip, np->data);
			    sprintf(send_buffer, "%s(%s) leave the chatroom.", np->name, np->ip);
			    leave_flag = 1;
			    break;
			    }
			    memset(send_buffer, 0, sizeof(send_buffer));
			    sprintf(send_buffer, "%s", recv_buffer);
			    send_to_all_clients(np, send_buffer);
			    memset(recv_buffer, 0, sizeof(recv_buffer));
			    receive = recv(np->data, recv_buffer, LENGTH_MSG, 0);
			    }
			    sprintf(send_buffer, "%s", recv_buffer);
			    send_to_all_clients(np, send_buffer);*/
			}
			else if(strcmp(recv_buffer, "sbname") == 0)
			{
				sbflag = 0;
				memset(recv_buffer, 0, sizeof(recv_buffer));
				receive = recv(np->data, recv_buffer, LENGTH_MSG, 0); //read_somebodyname
				while (tmp != NULL) {
					if(strcmp(tmp->name, recv_buffer) == 0) {
						sbsockfd = tmp->data;
						sbflag = 1;
						break;
					}
					tmp = tmp->link;
				}
				if(sbflag == 0) {
					printf("This name is not exist\n");
					send(np->data, "error", LENGTH_SEND, 0);
					continue;
				}
				memset(recv_buffer, 0, sizeof(recv_buffer));
				receive = recv(np->data, recv_buffer, LENGTH_MSG, 0); //read_sb_condition
				if(strcmp(recv_buffer, "sb_sentence") == 0)
				{
					strcpy(send_buffer, "send_sen"); //send condition to a client(sb)
					printf("Send to sockfd %d: \"%s\" \n", sbsockfd, send_buffer);
					send(sbsockfd, send_buffer, LENGTH_SEND, 0);
					memset(send_buffer, 0, sizeof(send_buffer));
					memset(recv_buffer, 0, sizeof(recv_buffer));
					receive = recv(np->data, recv_buffer, LENGTH_MSG, 0); //read_message(sentence)
					if (receive > 0) {
						if (strlen(recv_buffer) == 0) {
							continue;
						}
						sprintf(send_buffer, "[PRIVATE] %s：%s ", np->name, recv_buffer);
					}
					else if (receive == 0 || strcmp(recv_buffer, "exit") == 0) {
						strcpy(send_buffer, "send_sen");
						send_to_all_clients(np, send_buffer);
						printf("%s(%s)(%d) leave the chatroom.\n", np->name, np->ip, np->data);
						sprintf(send_buffer, "%s(%s) leave the chatroom.", np->name, np->ip);
						leave_flag = 1;
					}
					else {
						printf("B:Fatal Error: -1\n");
						leave_flag = 1;
					}
					printf("Send to sockfd %d: \"%s\" \n", sbsockfd, send_buffer);
					send(sbsockfd, send_buffer, LENGTH_SEND, 0);
				}
				else if(strcmp(recv_buffer, "sb_file") == 0)
				{
					strcpy(send_buffer, "send_file"); //send condition to clients
					printf("Send to sockfd %d: \"%s\" \n", sbsockfd, send_buffer);
					send(sbsockfd, send_buffer, LENGTH_SEND, 0);
					
					pthread_cond_wait(&cond, &mutex);					   
					if(open == 1)
					{
						strcpy(send_buffer, "yes");
						printf("Send to sockfd %d: \"%s\" \n", np->data, send_buffer);
                                		send(np->data, send_buffer, LENGTH_SEND, 0);

						send(sbsockfd, "accept", LENGTH_SEND, 0);
						memset(recv_buffer, 0, sizeof(recv_buffer));
						receive = recv(np->data, recv_buffer, LENGTH_MSG, 0); //filename

						memset(send_buffer, 0, sizeof(send_buffer));
						sprintf(send_buffer, "%s", recv_buffer);
						printf("Send to sockfd %d: \"%s\" \n", sbsockfd, send_buffer);
						send(sbsockfd, send_buffer, LENGTH_SEND, 0); //send filename to clients

						memset(send_buffer, 0, sizeof(send_buffer));
						memset(recv_buffer, 0, sizeof(recv_buffer));
						sleep(2);
						receive = recv(np->data, recv_buffer, LENGTH_MSG, 0);
						while(strcmp(recv_buffer, "EOF") != 0)
						{
							if (receive == 0 || strcmp(recv_buffer, "exit") == 0) {
								strcpy(send_buffer, "send_sen");
								send_to_all_clients(np, send_buffer);
								printf("%s(%s)(%d) leave the chatroom.\n", np->name, np->ip, np->data);
								sprintf(send_buffer, "%s(%s) leave the chatroom.", np->name, np->ip);
								leave_flag = 1;
								break;
							}
							memset(send_buffer, 0, sizeof(send_buffer));
							sprintf(send_buffer, "%s", recv_buffer);
							printf("Send to sockfd %d: \"%s\" \n", sbsockfd, send_buffer);
							send(sbsockfd, send_buffer, strlen(send_buffer), 0);
							memset(recv_buffer, 0, sizeof(recv_buffer));
							receive = recv(np->data, recv_buffer, LENGTH_MSG, 0);
						}
						sprintf(send_buffer, "%s", recv_buffer);
						printf("Send to sockfd %d: \"%s\" \n", sbsockfd, send_buffer);
						send(sbsockfd, send_buffer, LENGTH_SEND, 0);
						open = 0;
					}
					else if(open  == 0)
					{
						strcpy(send_buffer, "no");
						printf("Send to sockfd %d: \"%s\" \n", np->data, send_buffer);
                                		send(np->data, send_buffer, LENGTH_SEND, 0);
						send(sbsockfd, "refuse", LENGTH_MSG, 0);
						printf("sockfd(%d) refuse the file\n", sbsockfd);
						continue;
					}
					else break;
				}
			}
			else if(strcmp(recv_buffer, "yes") == 0)
			{
				printf("P%s\n",recv_buffer);
				sprintf(send_buffer,"%s",recv_buffer);
				//printf("Send to sockfd %d: \"%s\" \n", np->data, send_buffer);
				//send(np->data, send_buffer, LENGTH_SEND, 0); //send yes
				printf("Success\n");

				open = 1;
				pthread_cond_signal(&cond);
				//printf("signal\n");
				//printf("%d\n",open);
			}
			else if(strcmp(recv_buffer, "no") == 0)
			{
				printf("P%s\n",recv_buffer);
                                sprintf(send_buffer,"%s",recv_buffer);
                                //printf("Send to sockfd %d: \"%s\" \n", np->data, send_buffer);
                                //send(np->data, send_buffer, LENGTH_SEND, 0);
				open = 0;
				pthread_cond_signal(&cond);
			}

			else if(strcmp(recv_buffer, "check") == 0)
			{
				tmp = root->link;
				while (tmp != NULL) {
					strcpy(send_buffer, "send_check");
					send(np->data, send_buffer, LENGTH_SEND, 0);
					strcpy(send_buffer, tmp->name);
					send(np->data, send_buffer, LENGTH_SEND, 0);
					tmp = tmp->link;
				}

			}

		} 
		else if (receive == 0 || strcmp(recv_buffer, "exit") == 0) {
			strcpy(send_buffer, "send_sen");
			send_to_all_clients(np, send_buffer);
			printf("%s(%s)(%d) leave the chatroom.\n", np->name, np->ip, np->data);
			sprintf(send_buffer, "%s(%s) leave the chatroom.", np->name, np->ip);
			leave_flag = 1;
			send_to_all_clients(np, send_buffer);
		} 
		else {
			printf("D:Fatal Error: -1\n");
			leave_flag = 1;
		}
	}

	// Remove Node
	close(np->data);
	if (np == now) { // remove an edge node
		now = np->prev;
		now->link = NULL;
	} 
	else { // remove a middle node
		np->prev->link = np->link;
		np->link->prev = np->prev;
	}
	free(np);
}

int main()
{
	signal(SIGINT, catch_ctrl_c_and_exit);

	// Create socket
	server_sockfd = socket(AF_INET , SOCK_STREAM , 0);
	if (server_sockfd == -1) {
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
	server_info.sin_addr.s_addr = INADDR_ANY;
	server_info.sin_port = htons(8888);

	// Bind and Listen
	bind(server_sockfd, (struct sockaddr *)&server_info, s_addrlen);
	listen(server_sockfd, 5);

	// Print Server IP
	getsockname(server_sockfd, (struct sockaddr*) &server_info, (socklen_t*) &s_addrlen);
	printf("Start Server on: %s:%d\n", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));

	// Initial linked list for clients
	root = newNode(server_sockfd, inet_ntoa(server_info.sin_addr));
	now = root;

	while (1) {
		client_sockfd = accept(server_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);

		// Print Client IP
		getpeername(client_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
		printf("Client %s:%d come in.\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

		// Append linked list for clients
		ClientList *c = newNode(client_sockfd, inet_ntoa(client_info.sin_addr));
		c->prev = now;
		now->link = c;
		now = c;

		pthread_t id;
		if (pthread_create(&id, NULL, (void *)client_handler, (void *)c) != 0) {
			perror("Create pthread error!\n");
			exit(EXIT_FAILURE);
		}
	}
	close(server_sockfd);
	return 0;
}
