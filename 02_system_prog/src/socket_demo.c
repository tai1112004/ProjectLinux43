#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define HOST "127.0.0.1"
#define PORT 9090
#define NC "\033[0m"
#define GREEN "\033[32m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define SERVER(fmt, ...) printf(YELLOW "[HTTP][SERVER] " fmt NC "\n", ##__VA_ARGS__)
#define OK(fmt, ...) printf(GREEN "[HTTP][OK] " fmt NC "\n", ##__VA_ARGS__)
#define ERR(fmt, ...) fprintf(stderr, RED "[HTTP][ERR] " fmt NC "\n", ##__VA_ARGS__)

static volatile sig_atomic_t running = 1;

static void stop_server(int sig)
{
    (void)sig;
    running = 0;
}

static int send_all(int fd, const char *data, size_t length)
{
    size_t sent = 0;

    while (sent < length) {
        ssize_t n = send(fd, data + sent, length - sent, 0);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        sent += (size_t)n;
    }
    return 0;
}

static void handle_client(int client_fd, const struct sockaddr_in *client_addr)
{
    char request[4096] = {0};
    char response[8192];
    const char *body =
        "<!doctype html>"
        "<html lang=\"vi\">"
        "<head>"
        "<meta charset=\"utf-8\">"
        "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
        "<title>Linux Socket Demo</title>"
        "<style>"
        "body{margin:0;min-height:100vh;display:grid;place-items:center;"
        "background:#f4f7fb;color:#172033;font:16px/1.6 system-ui,sans-serif}"
        "main{width:min(680px,calc(100% - 40px));border:1px solid #d9e1ec;"
        "background:white;padding:32px;border-radius:8px;box-shadow:0 16px 40px #17203318}"
        "h1{margin:0 0 12px;font-size:30px}code{background:#edf2f7;padding:3px 6px;"
        "border-radius:4px}.ok{color:#087a4b;font-weight:700}"
        "</style>"
        "</head>"
        "<body><main>"
        "<p class=\"ok\">HTTP request received successfully</p>"
        "<h1>Linux System Programming Socket Demo</h1>"
        "<p>Trang nay duoc tra ve boi chuong trinh C <code>socket_demo</code>.</p>"
        "<p>Server dang lang nghe tai <code>127.0.0.1:9090</code> va dung cac ham "
        "<code>socket</code>, <code>bind</code>, <code>listen</code>, "
        "<code>accept</code>, <code>recv</code> va <code>send</code>.</p>"
        "</main></body></html>";
    ssize_t received = recv(client_fd, request, sizeof(request) - 1, 0);

    if (received <= 0)
        return;

    request[received] = '\0';
    char *line_end = strstr(request, "\r\n");
    if (line_end)
        *line_end = '\0';

    SERVER("request from %s:%u: %s",
           inet_ntoa(client_addr->sin_addr),
           ntohs(client_addr->sin_port),
           request);
    fflush(stdout);

    int response_length = snprintf(
        response,
        sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "Cache-Control: no-store\r\n"
        "\r\n"
        "%s",
        strlen(body),
        body);

    if (response_length > 0 && (size_t)response_length < sizeof(response))
        send_all(client_fd, response, (size_t)response_length);
}

int main(void)
{
    int server_fd;
    int reuse = 1;
    struct sockaddr_in address = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_LOOPBACK),
        .sin_port = htons(PORT),
    };
    struct sigaction stop_action = {0};
    struct sigaction child_action = {0};

    stop_action.sa_handler = stop_server;
    sigemptyset(&stop_action.sa_mask);
    sigaction(SIGINT, &stop_action, NULL);
    sigaction(SIGTERM, &stop_action, NULL);

    child_action.sa_handler = SIG_IGN;
    child_action.sa_flags = SA_NOCLDWAIT;
    sigemptyset(&child_action.sa_mask);
    sigaction(SIGCHLD, &child_action, NULL);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        ERR("socket: %s", strerror(errno));
        return 1;
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        ERR("bind %s:%d: %s", HOST, PORT, strerror(errno));
        close(server_fd);
        return 1;
    }
    if (listen(server_fd, 16) < 0) {
        ERR("listen: %s", strerror(errno));
        close(server_fd);
        return 1;
    }

    SERVER("listening on http://%s:%d/", HOST, PORT);
    OK("open the URL in a browser; press Ctrl+C to stop");
    fflush(stdout);

    while (running) {
        struct sockaddr_in client_address;
        socklen_t client_length = sizeof(client_address);
        int client_fd = accept(
            server_fd,
            (struct sockaddr *)&client_address,
            &client_length);

        if (client_fd < 0) {
            if (errno == EINTR)
                continue;
            ERR("accept: %s", strerror(errno));
            break;
        }

        pid_t child = fork();
        if (child == 0) {
            close(server_fd);
            handle_client(client_fd, &client_address);
            close(client_fd);
            _exit(0);
        }
        if (child < 0)
            ERR("fork: %s", strerror(errno));
        close(client_fd);
    }

    close(server_fd);
    SERVER("stopped");
    return 0;
}
