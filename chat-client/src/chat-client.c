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
#include <time.h>   // For timestamps

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
int header_displayed = 0;                         // Ensure message header is shown only once

/*
 *  Function  : get_timestamp()
 *  Summary   : Generates a timestamp in (HH:MM:SS) format.
 *  Params    : char* buffer - The buffer to store the timestamp.
 *              size_t size  - The size of the buffer.
 *  Return    : void
 */
void get_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    strftime(buffer, size, "(%H:%M:%S)", local);
}

/*
 *  Function  : display_header()
 *  Summary   : Displays the message header "----- Messages -----" only once.
 *  Params    : void
 *  Return    : void
 */
void display_header() {
    if (!header_displayed) {
        printw("----- Messages -----\n");
        header_displayed = 1;
        refresh();
    }
}

/*
 *  Function  : receive_messages()
 *  Summary   : Runs in a separate thread to receive messages from the server continuously.
 *              Adds timestamps, stores them in history, and prints them to the screen.
 *  Params    : void* arg
 *  Return    : void*
 */
void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);   // Clear buffer for new data
        ssize_t bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);

        if (bytes_received <= 0) {        // Check if server closed the connection
            printw("\n[Disconnected from server.]\n");
            refresh();
            break;
        }

        // Add timestamp
        char timestamp[10];
        get_timestamp(timestamp, sizeof(timestamp));

        // Add spaces after username
        char sender[BUFFER_SIZE], message[BUFFER_SIZE];
        sscanf(buffer, "%s %[^\n]", sender, message);

        char padded_sender[MAX_USERNAME_LENGTH + 3];  // Add spacing for alignment
        snprintf(padded_sender, sizeof(padded_sender), "%-6s", sender);  // Right-padded username

        // Format message with timestamp and receiving arrow (>>)
        char formatted_msg[BUFFER_SIZE];
        snprintf(formatted_msg, sizeof(formatted_msg), "%s >> %s %*s", padded_sender, message, 15, timestamp);

        // Store received message in history
        if (message_count < MAX_HISTORY) {
            strncpy(message_history[message_count], formatted_msg, BUFFER_SIZE);
            message_count++;
        } else {
            for (int i = 1; i < MAX_HISTORY; i++) {
                strncpy(message_history[i - 1], message_history[i], BUFFER_SIZE);
            }
            strncpy(message_history[MAX_HISTORY - 1], formatted_msg, BUFFER_SIZE);
        }

        // Display header once
        display_header();

        // Print message with timestamp
        printw("\n%s\n", formatted_msg);
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
    display_header();
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
    if (argc != 5 || strcmp(argv[1], "-user") != 0 || strcmp(argv[3], "-server") != 0) {
        fprintf(stderr, "Usage: %s -user <username> -server <server_ip>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Store username and server IP
    strncpy(username, argv[2], MAX_USERNAME_LENGTH);
    username[MAX_USERNAME_LENGTH] = '\0';

    char *server_ip = argv[4];

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Get client IP
    getsockname(sockfd, (struct sockaddr *)&client_addr, &addr_len);
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

    // Send "Hello" message
    char hello_msg[BUFFER_SIZE];
    snprintf(hello_msg, sizeof(hello_msg), "Hello|%s|%s", username, client_ip);
    send(sockfd, hello_msg, strlen(hello_msg), 0);

    init_ncurses();

    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_messages, NULL);

    char message[MAX_MESSAGE_LENGTH + 1];

    while (1) {
        memset(message, 0, sizeof(message));
        printw("> [%s]: ", username);
        getnstr(message, MAX_MESSAGE_LENGTH);

        // Handle commands
        if (strcmp(message, ">>bye<<") == 0) {
            send(sockfd, message, strlen(message), 0);
            break;
        }
        if (strcmp(message, ">>history<<") == 0) {
            show_message_history();
            continue;
        }

        // Send message with arrow and timestamp
        char timestamp[10];
        get_timestamp(timestamp, sizeof(timestamp));

        char formatted_msg[BUFFER_SIZE];
        snprintf(formatted_msg, sizeof(formatted_msg), "%s << %s %*s", username, message, 15, timestamp);
        send(sockfd, formatted_msg, strlen(formatted_msg), 0);
    }

    cleanup();
    pthread_cancel(recv_thread);
    pthread_join(recv_thread, NULL);

    return 0;
}
