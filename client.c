// client.c - Simple chat client using TCP and select()
// Build: make
// Run: ./client 127.0.0.1 8888
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUF_SIZE 1024

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }
    const char *server_ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) die("socket");

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &servaddr.sin_addr) <= 0) die("inet_pton");

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) die("connect");
    printf("Connected to %s:%d\n", server_ip, port);
    printf("Type messages and press Enter. Use /quit to exit.\n");

    fd_set readfds;
    char buf[BUF_SIZE];
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);       // stdin
        FD_SET(sockfd, &readfds);  // socket
        int fdmax = sockfd;

        if (select(fdmax + 1, &readfds, NULL, NULL, NULL) == -1) die("select");

        if (FD_ISSET(0, &readfds)) {
            // read from stdin
            if (!fgets(buf, sizeof(buf), stdin)) break;
            if (send(sockfd, buf, strlen(buf), 0) == -1) die("send");
            if (strncmp(buf, "/quit", 5) == 0) break;
        }
        if (FD_ISSET(sockfd, &readfds)) {
            int nbytes = recv(sockfd, buf, sizeof(buf)-1, 0);
            if (nbytes <= 0) {
                if (nbytes == 0) fprintf(stderr, "Server closed connection\n");
                else perror("recv");
                break;
            }
            buf[nbytes] = '\0';
            printf("%s", buf);
            fflush(stdout);
        }
    }
    close(sockfd);
    return 0;
}