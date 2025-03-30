#include "../inc/chat-server.h"

int main(void)
{
  // Set up tcp connection
  int serverSocket = setUpConnection();

  // Initialize variables to store clients' details
  int clientSocket;
  struct sockaddr_in clientAddress;
  socklen_t clientAddressLength = sizeof(clientAddress);
  activeClients.numberOfClients = 0;

  sleep(1); //Change to 10

  /* Enter main listening loop; i.e. accept users' messages */ //NOTE try killing a client while running
  while (true)
  {
    // Accept a connection
    if ((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength)) >= 0)
    {
      // Spawn a thread to handle it
      spawnClientThread(clientSocket);

    } // Handle fatal errors
    else if (clientSocket < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
    {
      close(serverSocket);
      close(clientSocket);
      displayFatalError("accept() FAILED");
    }

    /* Check if all clients have disconnected */
    if (activeClients.numberOfClients == 11)
    {
      break;
    }

    // Simulate some non-blocking work (e.g., checking timers, other tasks)
    sleep(1); // Sleep briefly to avoid busy-waiting
  }

  close(serverSocket);
  return 0;
}

int setUpConnection(void)
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

  /* Bind to the socket */
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

  /* Start listening to the socket */
  if (listen (serverSocket, 5) < 0) // 5 is the max number of pending connections (UNIX uses 5 by default)
  {
    close(serverSocket);
    displayFatalError("listen() FAILED");
  }

  return serverSocket;
}

void spawnClientThread(int clientSocket)
{
  // Fire a thread to handle client request
  pthread_t clientThread;
  if (pthread_create(&clientThread, nullptr, handleRequest, (void*)clientSocket) != 0)
  {
    displayFatalError("pthread_create() FAILED");
  }
  pthread_detach(clientThread);
}

// This functions handles a request being received
void handleRequest(void* clientSocket)
{
  /* Read & parse client's message */
  char buffer[kMaxMsgLength] = {};
  char* messageParts[3];
  int clientSocketInt = ((int)clientSocket);
  read(clientSocketInt, buffer, sizeof(buffer));
  parseMessage(buffer, messageParts);

  /* Perform appropriate operation based on message */
  if (strcmp(messageParts[0], "Hello") == 0)
  {
    addClient(clientSocketInt, messageParts);
  }
  else if (strcmp(messageParts[0], ">>bye<<") == 0)
  {
    removeClient(clientSocketInt);
  }
  else if (strcmp(messageParts[0], "Message") == 0)
  {
    broadcastMessage(messageParts[1], clientSocketInt);
  }
  else
  {
    //message is invalid
  }
}

// This function displays the error message specified and terminates the program.
void displayFatalError(char* errorMessage)
{
  perror(errorMessage);
  exit(EXIT_FAILURE);
}

// This function takes the message and divides it into parts on pipe (|) delimiter
void parseMessage(char* message, char* messageParts[])
{
  /* Divide string into parts */
  char* token = strtok(message, "|");
  int i = 0;
  while (token != NULL )
  {
    messageParts[i] = token;
    token = strtok(nullptr, "|");
    i++;
  }
}

void addClient(int clientSocket, char* messageParts[])
{
  pthread_mutex_lock(&clients_mutex);
  for (int i = 0; i < kMaxClients; i++)
  {
    if (strcpy(activeClients.clients[i].userName, ""))
    {
      activeClients.clients[i].clientSocket = clientSocket;
      strcpy(activeClients.clients[i].userName, messageParts[1]);
      strcpy(activeClients.clients[i].ipAddress, messageParts[2]);
      activeClients.numberOfClients++;
      break;
    }
  }
  pthread_mutex_unlock(&clients_mutex);
}

void removeClient(int clientSocket)
{
  pthread_mutex_lock(&clients_mutex);
  for (int i = 0; i < activeClients.numberOfClients; i++)
  {
    if (activeClients.clients[i].clientSocket == clientSocket)
    {
      /* Update clients list */
      for(int j = i; j < activeClients.numberOfClients - 1; j++)
      {
        activeClients.clients[j] = activeClients.clients[j + 1];
      }

      /* Handle special case (client is last in array) */
      if (i == activeClients.numberOfClients - 1)
      {
        ClientInfo emptyClient = {};
        activeClients.clients[i] = emptyClient;
      }

      activeClients.numberOfClients--;
      break;
    }
  }
  pthread_mutex_unlock(&clients_mutex);
}

void broadcastMessage(char* message, int clientSocket)
{
  /* Parcel & format the message */
  pthread_mutex_lock(&clients_mutex);
  char messageChunks[2][kMaxMsgLength] = {""};
  strncpy(messageChunks[0], message, kChunkSize - 1);
  strcpy(messageChunks[0], formatMessage(clientSocket, messageChunks[0]));
  if (strlen(message) > kChunkSize)
  {
    strncpy(messageChunks[1], message + 40, kChunkSize - 1);
    strcpy(messageChunks[1], formatMessage(clientSocket, messageChunks[1]));
  }

  /* Broadcast the message to all clients */
  for (int i = 0; i < activeClients.numberOfClients; i++)
  {
    if (write(activeClients.clients[i].clientSocket, messageChunks[0], strlen(messageChunks[0])) < 0)
    {
      perror("Write error");
    }
    if (messageChunks[1] != NULL)
    {
      if (write(activeClients.clients[i].clientSocket, messageChunks[1], strlen(messageChunks[1])) < 0)
      {
        perror("Write error");
      }
    }
  }
  pthread_mutex_unlock(&clients_mutex);
}

char* formatMessage(int clientSocket, char* message)
{
  char* formattedMessage = nullptr;
  for (int i = 0; i < activeClients.numberOfClients; i++)
  {
    if (activeClients.clients[i].clientSocket == clientSocket)
    {
      sprintf(formattedMessage, "%s [%.5s] << %.40s ", activeClients.clients[i].ipAddress, activeClients.clients[i].userName,
              message);
    }
  }
  return formattedMessage;
}

// void splitSendMessage(char *message, clientT *sender) {
//   int len = strlen(message);
//   int numChunks = len / kChunkSize + (len % kChunkSize ? 1 : 0);
//   char chunk[kChunkSize];
//   time_t now;
//   struct tm *timeinfo;
//
//   for (int i = 0; i < numChunks; i++) {
//     strncpy(chunk, message + (i * kChunkSize), kChunkSize);
//     chunk[kChunkSize - 1] = '\0';
//
//     time(&now);
//     timeinfo = localtime(&now);
//
//     char formattedMsg[kBufferSize];
//
//   }
// }