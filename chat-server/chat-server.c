#include "chat-server.h"

int main(int argc, char *argv[]) {

  return 0;
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