//
// Created on 27/03/25.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#define kMaxClients 10
#define kBufferSize 1024
#define kMaxMsgLength 80
#define kChunchSize 40
#define kUserNameLength 6

typedef struct {
  struct sockaddr_in address;
  int sockfd;
  int userId;
  char userName[kUserNameLength];
  char ip[INET_ADDRSTRLEN];
} clientT;

clientT *clients[kMaxClients];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
static int userId = 0;

//Function prototypes
void addClient(clientT *client);
void removeClient(int userId);
void sendMessage(char *message, int senderUserId);
void splitSendMessage(char *message, clientT *sender);
void *handleClient(void *arg);


#endif //CHAT_SERVER_H
