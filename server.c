// server.c - Simple multi-client chat server using TCP and select()
// Build: make
// Run: ./server 8888
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

#define BACKLOG 16
#define BUF_SIZE 1024

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);

    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) die("socket");

    int yes = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0)
        die("setsockopt");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(listener, (struct sockaddr*)&addr, sizeof(addr)) < 0) die("bind");
    if (listen(listener, BACKLOG) < 0) die("listen");

    fd_set master, readfds;
    FD_ZERO(&master);
    FD_ZERO(&readfds);
    FD_SET(listener, &master);
    int fdmax = listener;

    printf("Chat server listening on port %d ...\n", port);

    while (1) {
        readfds = master;
        if (select(fdmax + 1, &readfds, NULL, NULL, NULL) == -1) die("select");

        for (int i = 0; i <= fdmax; i++) {
            if (!FD_ISSET(i, &readfds)) continue;

            if (i == listener) {
                // handle new connection
                struct sockaddr_in cliaddr;
                socklen_t clilen = sizeof(cliaddr);
                int newfd = accept(listener, (struct sockaddr*)&cliaddr, &clilen);
                if (newfd == -1) {
                    perror("accept");
                    continue;
                }
                FD_SET(newfd, &master);
                if (newfd > fdmax) fdmax = newfd;
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(cliaddr.sin_addr), ip, sizeof ip);
                printf("New connection from %s on socket %d\n", ip, newfd);

                const char *welcome = "Welcome to C-Chat! Type and press Enter to send. /quit to exit.\n";
                if (send(newfd, welcome, strlen(welcome), 0) == -1) perror("send");
            } else {
                // handle data from a client
                char buf[BUF_SIZE];
                int nbytes = recv(i, buf, sizeof(buf)-1, 0);
                if (nbytes <= 0) {
                    if (nbytes == 0) {
                        printf("Socket %d hung up\n", i);
                    } else {
                        perror("recv");
                    }
                    close(i);
                    FD_CLR(i, &master);
                } else {
                    buf[nbytes] = '\0';
                    // If client sends /quit, close the connection
                    if (strncmp(buf, "/quit", 5) == 0) {
                        printf("Client on socket %d requested quit\n", i);
                        close(i);
                        FD_CLR(i, &master);
                        continue;
                    }
                    // Broadcast to all other clients
                    for (int j = 0; j <= fdmax; j++) {
                        if (FD_ISSET(j, &master)) {
                            if (j != listener && j != i) {
                                if (send(j, buf, nbytes, 0) == -1) {
                                    perror("send");
                                }
                            }
                        }
                    }
                    // Echo back to sender as well (optional)
                    // send(i, buf, nbytes, 0);
                }
            }
        }
    }
    return 0;
}