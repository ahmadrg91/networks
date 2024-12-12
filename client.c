#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// The simple client sends a message to server, receive one message and prints it out
int main(int argc, char *argv[]){
    
    if (argc != 4){
        printf("Use: %s <server ip> <port number> <message>\n", argv[0]);
        exit(1);
    }

    char *s_ip = argv[1];
    int s_port = atoi(argv[2]);
    const char *message = argv[3];

    int sock;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0){
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(s_ip);
    server_addr.sin_port = htons(s_port);

    int result = sendto(sock, message, strlen(message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

    if (result < 0){
        perror("Send Failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Message sent to server\n");
    printf("Waiting for a response from the server...\n");

    char buffer[1024];
    int bytes_recvd = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&server_addr, (socklen_t *)sizeof(server_addr));
    buffer[bytes_recvd] = '\0';

    printf("Message Received: %s\n", buffer);

    close(sock);
    return 0;
}