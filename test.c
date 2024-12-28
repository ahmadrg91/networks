#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>

int main(){
    uint32_t host = 0x12345678;
    unsigned int network = htonl(0x12345678);

    uint8_t *p_host = (uint8_t *)&host;
    uint8_t *p_network = (uint8_t *)&network;

    printf("Host Order:\n");
    for (int i = 0; i < 4; i++){
        printf("%02X ", *(p_host + i));
    }

    printf("\nNetwork Order:\n");
    for (int i = 0; i < 4; i++){
        printf("%02X ", *(p_network + i));
    }

    printf("\n");
}