#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<time.h>

#define DNS_PORT 53
#define DNS_SERVER "8.8.8.8"

typedef struct {
    uint16_t id;
    uint16_t flags;
    uint16_t qd_count;
    uint16_t an_count;
    uint16_t ns_count;
    uint16_t ar_count;
} DNS_header;

typedef struct {
    uint16_t q_type;
    uint16_t q_class;
} DNS_question;


void handle_rerror(uint8_t code){
    char *err;
    switch(code){
        case 1:
            err = "Format Error";
            break;
        case 2:
            err = "Server Failure";
            break;
        case 3:
            err = "The domain name queried doesn't exist";
            break;
        case 4:
            err = "The server doesn't support the requested query type";
            break;
        case 5:
            err = "Server refused to perfor query";
            break;
        case 9:
            err = "The server is not authorized to respond to query";
            break;
        default:
            err = "An error occured";
    }

    printf("\n%s...\n", err);
    exit(code);
}

int craft_dns_packet(uint8_t *buffer) {
    DNS_header header;
    memset(&header, 0, sizeof(DNS_header));
    srand(time(NULL));
    header.id = htons(rand() % 65536);
    header.flags = htons(0x0100);
    header.qd_count = htons(1);

    memcpy(buffer, &header, sizeof(DNS_header));
    int offset = sizeof(DNS_header);

    uint8_t domain_label[] = { 0x03, 'w', 'w', 'w', 0x07, 'e', 'x', 'a', 'm', 'p', 'l', 'e', 0x03, 'c', 'o', 'm', 0x00 };
    memcpy(buffer + offset, domain_label, sizeof(domain_label));
    offset += sizeof(domain_label);

    DNS_question question;
    memset(&question, 0, sizeof(DNS_question));
    question.q_type = htons(1);
    question.q_class = htons(1);

    memcpy(buffer + offset, &question, sizeof(DNS_question));
    offset += sizeof(DNS_question);

    return offset;
}


void parse_dns_response(uint8_t *response, int size, int ans_offset){
    unsigned short int offset = 0;
    DNS_header *header = (DNS_header *)(response + offset);
    header->id = ntohs(header->id);
    header->flags = ntohs(header->flags);

    int rcode = 0b1111 & header->flags;
    if (rcode != 0)
        handle_rerror(rcode);
    
    header->qd_count = ntohs(header->qd_count);
    header->an_count = ntohs(header->an_count);
    header->ns_count = ntohs(header->ns_count);
    header->ar_count = ntohs(header->ar_count);

    offset += sizeof(DNS_header);

    int domain_len = 1;
    uint8_t *domain_ptr = response + offset;
    while (*domain_ptr != 0x00) {
        ++domain_ptr;
        ++domain_len;
    }
    offset = domain_ptr - response + 1;

    uint16_t *q_type = (uint16_t*)(response + offset);
    offset += sizeof(*q_type);

    uint16_t *q_class = (uint16_t*)(response + offset);
    offset += sizeof(*q_class);


    // Answer
    if ((ntohs(*(response + offset)) & (0b11 << 14)) == (0b11 << 14)) offset += 2;

    uint16_t *type = (uint16_t*)(response + offset);
    offset += sizeof(*type);

    uint16_t *class = (uint16_t*)(response + offset);
    offset += sizeof(*class);

    uint32_t *ttl = (uint32_t*)(response + offset);
    offset += sizeof(*ttl);

    uint16_t *rdlen = (uint16_t*)(response + offset);
    offset += sizeof(*rdlen);

    uint8_t *host_addr_ptr = (uint8_t*)(response + offset);

    for (int i = 0; i < ntohs(*rdlen); i++){
        printf("%d", *host_addr_ptr);
        if (i != 3) printf(".");
        host_addr_ptr++;
    }
    printf("\n");
}


int main(int argc, char *argv[]){

    uint8_t request[512];
    memset(request, 0, 512);
    int req_len = craft_dns_packet(request);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0){
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    server_address.sin_addr.s_addr = inet_addr(DNS_SERVER);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(53);
    socklen_t addr_len = sizeof(server_address);

    int result = sendto(sock, request, req_len, 0, (struct sockaddr*)&server_address, addr_len);
    if (result < 0){
        perror("Sending Failed");
        exit(EXIT_FAILURE);
    }

    uint8_t response[512];
    int bytes_recvd = recvfrom(sock, response, sizeof(response), 0, (struct sockaddr*)&server_address, &addr_len);
    if (bytes_recvd < 0){
        perror("Receving error");
        exit(EXIT_FAILURE);
    }

    parse_dns_response(response, bytes_recvd, req_len);

    return 0;
}