#define _GNU_SOURCE
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/if.h>
#include <netpacket/packet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#define CYAN "\033[36m"
#define NC "\033[0m"

struct stat_line { char name[IFNAMSIZ]; unsigned long long rx, tx; };

static int read_stats(struct stat_line *stats, int max) {
    FILE *fp = fopen("/proc/net/dev", "r");
    if (!fp) return 0;
    char line[512];
    int n = 0;
    fgets(line, sizeof(line), fp); fgets(line, sizeof(line), fp);
    while (n < max && fgets(line, sizeof(line), fp)) {
        char *colon = strchr(line, ':');
        if (!colon) continue;
        *colon = '\0';
        sscanf(line, " %15s", stats[n].name);
        sscanf(colon + 1, " %llu %*u %*u %*u %*u %*u %*u %*u %llu", &stats[n].rx, &stats[n].tx);
        n++;
    }
    fclose(fp);
    return n;
}

static void get_mac(const char *ifname, char *out, size_t outsz) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", ifname);
    if (fd >= 0 && ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
        unsigned char *m = (unsigned char *)ifr.ifr_hwaddr.sa_data;
        snprintf(out, outsz, "%02x:%02x:%02x:%02x:%02x:%02x", m[0], m[1], m[2], m[3], m[4], m[5]);
    } else {
        snprintf(out, outsz, "-");
    }
    if (fd >= 0) close(fd);
}

static void stats_for(const char *name, struct stat_line *stats, int n, unsigned long long *rx, unsigned long long *tx) {
    *rx = *tx = 0;
    for (int i = 0; i < n; ++i) if (!strcmp(name, stats[i].name)) { *rx = stats[i].rx; *tx = stats[i].tx; return; }
}

int main(void) {
    struct ifaddrs *ifaddr;
    struct stat_line stats[64];
    int ns = read_stats(stats, 64);
    if (getifaddrs(&ifaddr) == -1) { perror("getifaddrs"); return 1; }
    printf(CYAN "%-12s %-40s %-18s %12s %12s\n" NC, "IFACE", "ADDRESS", "MAC", "RX", "TX");
    for (struct ifaddrs *ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;
        int fam = ifa->ifa_addr->sa_family;
        char addr[INET6_ADDRSTRLEN] = "-";
        if (fam == AF_INET) inet_ntop(fam, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, addr, sizeof(addr));
        else if (fam == AF_INET6) inet_ntop(fam, &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr, addr, sizeof(addr));
        else continue;
        char mac[32];
        unsigned long long rx, tx;
        get_mac(ifa->ifa_name, mac, sizeof(mac));
        stats_for(ifa->ifa_name, stats, ns, &rx, &tx);
        printf("%-12s %-40s %-18s %12llu %12llu\n", ifa->ifa_name, addr, mac, rx, tx);
    }
    freeifaddrs(ifaddr);
    return 0;
}
