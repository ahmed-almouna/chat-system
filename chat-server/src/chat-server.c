#include "../inc/chat-server.h"

int main(void)
{
  int serverSocket;

  /* Create socket */
  if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    displayFatalError("socket() FAILED");
  }


  return 0;
}


// This function displays the error message specified and terminates the program.
void displayFatalError(char* errorMessage) //might need to go in common
{
  printf("Error: %s\n", errorMessage);
  exit(1);
}

void addClient(clientT *client) {
  pthread_mutex_lock(&clients_mutex);
  for (int i = 0; i < kMaxClients; i++) {
    if (clients[i] == NULL) {
      clients[i] = client;
      break;
    }
  }
  pthread_mutex_unlock(&clients_mutex);
}

void removeClient(int userId) {
  pthread_mutex_lock(&clients_mutex);
  for (int i = 0; i < kMaxClients; i++) {
    if (clients[i] != NULL && clients[i]->userId == userId) {
      clients[i] = NULL;
      break;
    }
  }
  pthread_mutex_unlock(&clients_mutex);
}

void sendMessage(char *message, int senderUserId) {
  pthread_mutex_lock(&clients_mutex);
  for (int i = 0; i < kMaxClients; i++) {
    if (clients[i] != NULL && clients[i]->userId == senderUserId) {
      if(write(clients[i]->sockfd, message, strlen(message)) < 0) {
        perror("Write error");
        break;
      }
    }
  }
  pthread_mutex_unlock(&clients_mutex);
}

void splitSendMessage(char *message, clientT *sender) {
  int len = strlen(message);
  int numChunks = len / kChunkSize + (len % kChunkSize ? 1 : 0);
  char chunk[kChunkSize];
  time_t now;
  struct tm *timeinfo;

  for (int i = 0; i < numChunks; i++) {
    strncpy(chunk, message + (i * kChunkSize), kChunkSize);
    chunk[kChunkSize - 1] = '\0';

    time(&now);
    timeinfo = localtime(&now);

    char formattedMsg[kBufferSize];

  }
}