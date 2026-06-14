#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static void log_line(const char *tag, const char *msg)
{
	time_t now = time(NULL);
	struct tm *tm = localtime(&now);
	printf("[%02d:%02d:%02d][%s] %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, tag, msg);
}

static void *reader(void *arg)
{
	(void)arg;
	int fd = open("/dev/mychardev0", O_RDWR);
	if (fd < 0) { perror("reader open"); return NULL; }
	log_line("READER", "read() will block until data_ready");
	char buf[256] = {0};
	ssize_t n = read(fd, buf, sizeof(buf) - 1);
	if (n >= 0) {
		char msg[320];
		snprintf(msg, sizeof(msg), "woke up and read: %s", buf);
		log_line("READER", msg);
	} else {
		perror("reader read");
	}
	close(fd);
	return NULL;
}

static void *writer(void *arg)
{
	(void)arg;
	sleep(2);
	int fd = open("/dev/mychardev1", O_WRONLY);
	if (fd < 0) { perror("writer open"); return NULL; }
	log_line("WRITER", "writing data");
	write(fd, "wake up from writer thread", 26);
	close(fd);
	return NULL;
}

int main(void)
{
	pthread_t r, w;
	pthread_create(&r, NULL, reader, NULL);
	pthread_create(&w, NULL, writer, NULL);
	pthread_join(r, NULL);
	pthread_join(w, NULL);
	return 0;
}
