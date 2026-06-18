#include <stdio.h>
#include <stdlib.h>

#define CYAN "\033[36m"
#define NC "\033[0m"

int main(void) {
    for (;;) {
        printf(CYAN "\n┌────────────────────────────┐\n│ SYSTEM PROGRAMMING DEMOS   │\n└────────────────────────────┘\n" NC);
        printf("1) process_mgmt\n2) file_ops\n3) HTTP socket server\n4) network_info\n0) Exit\nChoose: ");
        int c = 0;
        if (scanf("%d", &c) != 1) return 0;
        switch (c) {
            case 1: system("./bin/process_mgmt"); break;
            case 2: system("./bin/file_ops"); break;
            case 3:
                puts("Open http://127.0.0.1:9090/ in a browser.");
                puts("Press Ctrl+C here to stop the HTTP server.");
                system("./bin/socket_demo");
                break;
            case 4: system("./bin/network_info"); break;
            case 0: return 0;
            default: puts("Invalid choice");
        }
    }
}
