#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdbool.h>

// The simple server recieve messages from clients and echoes back the same message
void main(int argc, char *argv[]){

    if (argc != 3){
        printf("Use: %s <server ip> <server port>\n", argv[0]);
        exit(1);
    }

    char *s_ip = argv[1];
    int s_port = atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0){
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(s_port);
    server_addr.sin_addr.s_addr = inet_addr(s_ip);

    char buffer[1024];

    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("Bind failed!");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", s_port);

    int bytes_recvd;
    
    while (true){
        bytes_recvd = recvfrom(sock, buffer, 1024, 0, (struct sockaddr*)&client_addr, &addr_len);
        if (bytes_recvd < 0){
            perror("Recieve failed");
            close(sock);
            exit(EXIT_FAILURE);
        }

        buffer[bytes_recvd] = '\0';

        printf("Recieved message: %s\n", buffer);
        printf("From IP: %s, Port: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        if (sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, addr_len) < 0){
            perror("Send Failed");
            close(sock);
            exit(EXIT_FAILURE);
        }

        printf("Sent message: %s\n", buffer);
    }
    
    close(sock);
    return;
}