/*
*   FILE          : chat-server.c
*   PROJECT       : Can We Talk System - A04
*   PROGRAMMER    : Ahmed, Valentyn, Juan Jose, Warren
*   FIRST VERSION : 03/23/2025
*   DESCRIPTION   :
*      This is the main server-side implementation for the "Can We Talk" system.
*      The chat server uses TCP/IP sockets and pthreads to handle up to 10 clients.
*      Each client is handled in a dedicated thread, and messages are broadcast
*      to all connected users. Messages are split into chunks and include
*      IP, username, and timestamp for proper formatting and display.
*/

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

  int attempts = 0;

  /* Enter main listening loop; i.e. accept users' messages */ //NOTE try killing a client while running
  while (true)
  {
    // Accept a connection
    if ((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength)) >= 0)
    {
      // Spawn a thread to handle it
      spawnClientThread(clientSocket);
      attempts++;

    } // Handle fatal errors
    else if (clientSocket < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
    {
      close(serverSocket);
      displayFatalError("accept() FAILED");
    }

    /* Check if all clients have disconnected */
    if (activeClients.numberOfClients == 2232)
    {
      printf("Server shutting");
      break;
    }

    // Simulate some non-blocking work (e.g., checking timers, other tasks)
    sleep(1); // Sleep briefly to avoid busy-waiting
  }

  close(serverSocket);
  return 0;
}

/*
 *  Function  : setUpConnection()
 *  Summary   : This function sets up the server-side socket, binds to the port, and starts listening.
 *  Params    : void
 *  Return    : int
 */
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

/*
 *  Function  : spawnClientThread()
 *  Summary   : This function creates a new thread to handle an incoming client connection.
 *  Params    : int clientSocket
 *  Return    : void
 */
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

/*
 *  Function  : handleRequest()
 *  Summary   : This function is executed by each client-handling thread. Receives and parses messages.
 *  Params    : void* clientSocket
 *  Return    : void
 */
void handleRequest(void* clientSocket)
{
  int clientSocketInt = ((int)clientSocket);
  while (true)
  {
    /* Read & parse client's message */
    char buffer[kMaxMsgLength] = {};
    char* messageParts[3] = {};
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
      break;
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
}

/*
 *  Function  : displayFatalError()
 *  Summary   : This function displays the error message specified and terminates the program.
 *  Params    : char* errorMessage
 *  Return    : void
 */
void displayFatalError(char* errorMessage)
{
  perror(errorMessage);
  exit(EXIT_FAILURE);
}

/*
 *  Function  : parseMessage()
 *  Summary   : This function takes the message and divides it into parts on pipe (|) delimiter
 *  Params    : char* message
 *              char* messageParts[]
 *  Return    : void
 */
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

/*
 *  Function  : addClient()
 *  Summary   : This function adds a client to the global client list.
 *  Params    : int clientSocket
 *              char* messageParts[]
 *  Return    : void
 */
void addClient(int clientSocket, char* messageParts[])
{
  pthread_mutex_lock(&clients_mutex);
  for (int i = 0; i < kMaxClients; i++)
  {
    if (strcmp(activeClients.clients[i].userName, "") == 0)
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

/*
 *  Function  : removeClient()
 *  Summary   : This function removes a client from the global list and shifts remaining clients.
 *  Params    : int clientSocket
 *  Return    : void
 */
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
      close(clientSocket);
      break;
    }
  }
  pthread_mutex_unlock(&clients_mutex);
}

/*
 *  Function  : broadcastMessage()
 *  Summary   : This function splits a message into 40-character chunks and sends to all clients.
 *  Params    : char* message
 *              int clientSocket
 *  Return    : void
 */

void broadcastMessage(char* message, int clientSocket)
{
  /* Parcel & format the message */
  pthread_mutex_lock(&clients_mutex);
  char messageChunks[2][kMaxMsgLength] = {""};
  strncpy(messageChunks[0], message, kChunkSize - 1);
  formatMessage(clientSocket, messageChunks[0]);
  if (strlen(message) > kChunkSize)
  {
    strncpy(messageChunks[1], message + 40, kChunkSize - 1);
    //strcpy(messageChunks[1], formatMessage(clientSocket, messageChunks[1]));
  }

  /* Broadcast the message to all clients */
  for (int i = 0; i < activeClients.numberOfClients; i++)
  {
    if (write(activeClients.clients[i].clientSocket, messageChunks[0], strlen(messageChunks[0])) < 0)
    {
      perror("Write error");
    }
    if (strlen(messageChunks[1]) > 0)
    {
      if (write(activeClients.clients[i].clientSocket, messageChunks[1], strlen(messageChunks[1])) < 0)
      {
        perror("Write error");
      }
    }
  }
  pthread_mutex_unlock(&clients_mutex);
}

/*
 *  Function  : formatMessage()
 *  Summary   : This function formats a message with IP, username, and content. Updates the passed-in message.
 *  Params    : int clientSocket
 *              char* message
 *  Return    : void
 */
void formatMessage(int clientSocket, char* message)
{
  char formattedMessage[kMaxMsgLength] = "";
  for (int i = 0; i < activeClients.numberOfClients; i++)
  {
    if (activeClients.clients[i].clientSocket == clientSocket)
    {
      sprintf(formattedMessage, "%s [%.5s] << %.40s", activeClients.clients[i].ipAddress, activeClients.clients[i].userName,
              message);
      break;
    }
  }
  message = formattedMessage;
}