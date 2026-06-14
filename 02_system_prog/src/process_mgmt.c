#define _GNU_SOURCE
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define NC "\033[0m"
#define GREEN "\033[32m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define INFO(fmt, ...) printf(YELLOW "[PROC][INFO] " fmt NC "\n", ##__VA_ARGS__)
#define OK(fmt, ...) printf(GREEN "[PROC][OK] " fmt NC "\n", ##__VA_ARGS__)
#define ERR(fmt, ...) fprintf(stderr, RED "[PROC][ERR] " fmt NC "\n", ##__VA_ARGS__)

static void on_sigterm(int sig) { INFO("received signal %d", sig); }

static void print_proc_status(pid_t pid) {
    char path[64], line[256];
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    FILE *fp = fopen(path, "r");
    if (!fp) { ERR("open %s: %s", path, strerror(errno)); return; }
    INFO("selected /proc status lines for pid %d:", pid);
    while (fgets(line, sizeof(line), fp)) {
        if (!strncmp(line, "Name:", 5) || !strncmp(line, "State:", 6) ||
            !strncmp(line, "Pid:", 4) || !strncmp(line, "PPid:", 5)) {
            printf("  %s", line);
        }
    }
    fclose(fp);
}

int main(void) {
    struct sigaction sa = {0};
    sa.sa_handler = on_sigterm;
    sigaction(SIGTERM, &sa, NULL);

    pid_t sleeper = fork();
    if (sleeper == 0) {
        INFO("signal demo child pid=%d waiting", getpid());
        pause();
        _exit(0);
    }
    sleep(1);
    print_proc_status(sleeper);
    OK("sending SIGTERM to pid %d", sleeper);
    kill(sleeper, SIGTERM);
    waitpid(sleeper, NULL, 0);

    pid_t child = fork();
    if (child == 0) {
        INFO("exec child pid=%d running /bin/ls", getpid());
        execl("/bin/ls", "ls", "-1", ".", NULL);
        perror("execl");
        _exit(127);
    }
    int status = 0;
    waitpid(child, &status, 0);
    if (WIFEXITED(status)) OK("exec child exited with status %d", WEXITSTATUS(status));
    return 0;
}
