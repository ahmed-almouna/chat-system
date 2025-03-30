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
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>

// Constants
#define kServerPort 13000
#define kMaxClients 10
#define kMaxMsgLength 90
#define kChunkSize 41
#define kUserNameLength 6
#define kGenericStringLength 100

// Data structures
typedef struct ClientInfo
{
    int clientSocket;
    char ipAddress[INET_ADDRSTRLEN];
    char userName[kGenericStringLength];
} ClientInfo;

typedef struct ClientsList
{
   int numberOfClients;
   ClientInfo clients[kMaxClients];
} ClientsList;

ClientsList activeClients;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;


//Function prototypes
int setUpConnection(void);
void spawnClientThread(int clientSocket);
void handleRequest(void* arg);
void parseMessage(char* message, char* messageParts[]);
void addClient(int clientSocket, char* messageParts[]);
void removeClient(int userId);
void broadcastMessage(char* message, int senderUserId);
void formatMessage(int clientSocket, char* message);
void displayFatalError(char* errorMessage);

#endif //CHAT_SERVER_H