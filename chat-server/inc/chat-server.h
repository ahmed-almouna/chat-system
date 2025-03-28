//
// Created on 27/03/25.
//

#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

// Include statements
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <threads.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

// Constants
#define kServerPort 13000
#define kMaxClients 10
#define kBufferSize 1024
#define kMaxMsgLength 80
#define kChunkSize 41
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
void displayFatalError(char* errorMessage);
void addClient(clientT *client);
void removeClient(int userId);
void sendMessage(char *message, int senderUserId);
void splitSendMessage(char *message, clientT *sender);
void *handleClient(void *arg);
void handleRequest(void);

#endif //CHAT_SERVER_H