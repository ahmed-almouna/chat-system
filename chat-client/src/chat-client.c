/*
*   FILE          : chat-client.c
*   PROJECT       : Can We Talk System - A04
*   PROGRAMMER    : Ahmed, Valentyn, Juan Jose, Warren
*   FIRST VERSION : 03/23/2025
*   DESCRIPTION   :
*      This is the main client file for the "Can We Talk" system.
*      It connects to the chat-server via TCP/IP, registers the user with their
*      username and IP address, and provides a terminal-based UI using ncurses.
*      The client sends and receives chat messages, formats and parses them, and
*      handles special commands like >>bye<< and >>history<<.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ncurses.h>
#include <netinet/in.h>

#define PORT 13000
#define BUFFER_SIZE 1024
#define MAX_MESSAGE_LENGTH 80
#define MAX_USERNAME_LENGTH 5
#define MAX_HISTORY 50

int sockfd;                          // Socket file descriptor
char username[MAX_USERNAME_LENGTH + 1];  // Username with null terminator
char client_ip[INET_ADDRSTRLEN];         // To store client's IP address

// Store message history
char message_history[MAX_HISTORY][BUFFER_SIZE];   // Array to store history messages
int message_count = 0;                            // Track the number of saved messages

/*
 *  Function  : receive_messages()
 *  Summary   : Runs in a separate thread to receive messages from the server continuously.
 *              Saves them in history and prints them to the screen.
 *  Params    : void* arg
 *  Return    : void*
 */
void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);   // Clear buffer for new data
        ssize_t bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);

        if (bytes_received <= 0) {        // Check if server closed the connection
            printw("\nDisconnected from server.\n");
            refresh();
            break;
        }

        // Add received message to history
        if (message_count < MAX_HISTORY) {
            strncpy(message_history[message_count], buffer, BUFFER_SIZE);
            message_count++;
        } else {
            // If history is full, shift older messages to make room for new ones
            for (int i = 1; i < MAX_HISTORY; i++) {
                strncpy(message_history[i - 1], message_history[i], BUFFER_SIZE);
            }
            strncpy(message_history[MAX_HISTORY - 1], buffer, BUFFER_SIZE);
        }

        printw("%s\n", buffer);   // Display the message in the chat
        refresh();
    }
    return NULL;
}

/*
 *  Function  : show_message_history()
 *  Summary   : Displays the last 50 received messages stored in message_history.
 *  Params    : void
 *  Return    : void
 */
void show_message_history() {
    for (int i = 0; i < message_count; i++) {
        printw("%s\n", message_history[i]);
    }
    refresh();
}

/*
 *  Function  : init_ncurses()
 *  Summary   : Initializes the ncurses UI for text-based chat display.
 *              It enables line buffering, disables echoing, and supports scrolling.
 *  Params    : void
 *  Return    : void
 */
void init_ncurses() {
    initscr();               // Start ncurses mode
    cbreak();                 // Disable line buffering
    noecho();                 // Don't display typed characters
    scrollok(stdscr, TRUE);   // Enable scrolling
    keypad(stdscr, TRUE);     // Enable keypad input
}

/*
 *  Function  : cleanup()
 *  Summary   : Cleans up UI and closes socket before exiting.
 *              It ensures that the terminal UI resets when the program exits.
 *  Params    : void
 *  Return    : void
 */
void cleanup() {
    endwin();
    close(sockfd);
}

int main(int argc, char *argv[]) {
    // Validate command-line arguments
    // The program expects exactly 4 arguments:
    // - `-user` followed by the username
    // - `-server` followed by the server IP
    // If the arguments are incorrect, it prints usage instructions and exits.
    if (argc != 5 || strcmp(argv[1], "-user") != 0 || strcmp(argv[3], "-server") != 0) {
        fprintf(stderr, "Usage: %s -user <username> -server <server_ip>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Copy the username into the global variable
    strncpy(username, argv[2], MAX_USERNAME_LENGTH);
    username[MAX_USERNAME_LENGTH] = '\0';   // Ensure null termination

    char *server_ip = argv[4];    // Store the server IP address

    // Create the client socket
    // This socket allows the client to establish a connection with the server.
    // If the socket creation fails, the program exits with an error.
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Define server and client socket addresses
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Configure the server address structure
    server_addr.sin_family = AF_INET;             // IPv4 protocol
    server_addr.sin_port = htons(PORT);           // Use defined port (8080)
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);  // Convert IP string to binary

    // Attempt to connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Retrieve the local IP address
    // After connecting, the program retrieves and displays the client's IP address.
    // This is included in the initial "Hello" message sent to the server.
    getsockname(sockfd, (struct sockaddr *)&client_addr, &addr_len);
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

    // Send the initial "Hello" message to the server
    // This message uses the format: "Hello|<username>|<client_ip>"
    // It lets the server identify the client and store the IP address.
    char hello_msg[BUFFER_SIZE];
    snprintf(hello_msg, sizeof(hello_msg), "Hello|%s|%s", username, client_ip);
    send(sockfd, hello_msg, strlen(hello_msg), 0);

    // Initialize ncurses UI
    init_ncurses();

    // Create the receiving thread
    // This thread continuously listens for incoming messages
    // from the server and displays them in real-time.
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_messages, NULL);

    char message[MAX_MESSAGE_LENGTH + 1];

    // Main message loop
    // This loop lets the user type and send messages continuously.
    // It handles special commands:
    // - `>>bye<<`: Disconnects from the server
    // - `>>history<<`: Shows message history
    while (1) {
        memset(message, 0, sizeof(message));
        printw("[%s]: ", username);
        getnstr(message, MAX_MESSAGE_LENGTH);

        // Handle disconnection command
        if (strcmp(message, ">>bye<<") == 0) {
            send(sockfd, message, strlen(message), 0);
            break;
        }

        // Handle message history command
        if (strcmp(message, ">>history<<") == 0) {
            show_message_history();
            continue;
        }

        // Format and send the message in the required protocol format
        char formatted_msg[BUFFER_SIZE];
        snprintf(formatted_msg, sizeof(formatted_msg), "Message|%s", message);
        send(sockfd, formatted_msg, strlen(formatted_msg), 0);
    }

    // Clean up and close the program
    // This terminates the receiving thread and closes the socket properly.
    cleanup();
    pthread_cancel(recv_thread);
    pthread_join(recv_thread, NULL);

    return 0;
}
