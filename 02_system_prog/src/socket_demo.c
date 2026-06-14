#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define NC "\033[0m"
#define GREEN "\033[32m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define SERVER(fmt, ...) printf(YELLOW "[SOCK][SERVER] " fmt NC "\n", ##__VA_ARGS__)
#define CLIENT(fmt, ...) printf(GREEN "[SOCK][CLIENT] " fmt NC "\n", ##__VA_ARGS__)
#define ERR(fmt, ...) fprintf(stderr, RED "[SOCK][ERR] " fmt NC "\n", ##__VA_ARGS__)

static void handle_client(int cfd) {
    char buf[256];
    ssize_t n = recv(cfd, buf, sizeof(buf) - 1, 0);
    if (n > 0) {
        buf[n] = '\0';
        SERVER("received: %s", buf);
        send(cfd, buf, (size_t)n, 0);
    }
    close(cfd);
}

static void *server_thread(void *arg) {
    (void)arg;
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    struct sockaddr_in addr = {.sin_family = AF_INET, .sin_addr.s_addr = htonl(INADDR_LOOPBACK), .sin_port = htons(9090)};
    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) { ERR("bind: %s", strerror(errno)); return NULL; }
    listen(sfd, 8);
    SERVER("listening on 127.0.0.1:9090");

    fd_set rfds;
    struct timeval tv = {.tv_sec = 5, .tv_usec = 0};
    FD_ZERO(&rfds);
    FD_SET(sfd, &rfds);
    if (select(sfd + 1, &rfds, NULL, NULL, &tv) > 0) {
        int cfd = accept(sfd, NULL, NULL);
        if (cfd >= 0) {
            pid_t pid = fork();
            if (pid == 0) { handle_client(cfd); _exit(0); }
            close(cfd);
            waitpid(pid, NULL, 0);
        }
    }
    close(sfd);
    return NULL;
}

int main(void) {
    pthread_t tid;
    pthread_create(&tid, NULL, server_thread, NULL);
    sleep(1);

    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {.sin_family = AF_INET, .sin_port = htons(9090)};
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (connect(cfd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
        const char *msg = "hello echo server";
        char buf[256] = {0};
        send(cfd, msg, strlen(msg), 0);
        recv(cfd, buf, sizeof(buf) - 1, 0);
        CLIENT("echo response: %s", buf);
    } else {
        ERR("connect: %s", strerror(errno));
    }
    close(cfd);
    pthread_join(tid, NULL);
    return 0;
}
