#include "../inc/chat-server.h"

#include <threads.h>

int main(void)
{
  int serverSocket;

  /* Create a socket */
  if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    displayFatalError("socket() FAILED");
  }

  /* Set server's address & port */
  struct sockaddr_in serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddress.sin_port = htons(kServerPort);

  /* Bind socket */
  if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
  {
    close(serverSocket);
    displayFatalError("bind() FAILED");
  }

  /* Set the socket to non-blocking mode */
  if (fcntl(serverSocket, F_SETFL, O_NONBLOCK) < 0)
  {
    close(serverSocket);
    displayFatalError("fcntl() FAILED");
  }

  /* Start listening for connections */
  if (listen (serverSocket, 5) < 0) // 5 is the max number of pending connections (UNIX uses 5 by default)
  {
    close(serverSocket);
    displayFatalError("listen() FAILED");
  }

  int clientSocket;
  struct sockaddr_in clientAddress; // Holds client's details
  socklen_t clientAddressLength = sizeof(clientAddress);
  sleep(1);

  /* Enter listening loop; accept users' messages */ //NOTE try killing a client while running
  while (true)
  {
    /* Accept a connection */
    if ((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength)) >= 0)
    {
      //Fire thread to handle
      // thrd_t clientThread;
      // thrd_create(clientThread, )



    } /* Handle fatal accept errors */
    else if (clientSocket < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
    {
      // might need to clean up
      displayFatalError("accept() FAILED");
    }

    // Simulate some non-blocking work (e.g., checking timers, other tasks)
    sleep(1); // Sleep briefly to avoid busy-waiting
  }



  return 0;
}

void handleRequest(void)
{

}
// This function displays the error message specified and terminates the program.
void displayFatalError(char* errorMessage) //might need to go in common
{
  perror(errorMessage);
  exit(EXIT_FAILURE);
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