#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#define BUF_SIZE 8192

static void chomp(char *s){
	if(!s) return;
	size_t n = strcspn(s, "\n");
	s[n] = '\0';
}

static ssize_t write_all(int fd, const void *buf, size_t n){
	const char *p = (const char *)buf;
	size_t total = 0;
	while (total < n){
		ssize_t w = write(fd, p + total, n - total);
		if (w < 0){
			if(errno == EINTR) continue;
			return -1;
		}
		total += (size_t)w;
	}
	return (ssize_t)total;
}

int main(void){
	char source_path[4096];
	char destination_path[4096];

	printf("Enter source file path: ");
	if (!fgets(source_path, sizeof(source_path), stdin)){
		fprintf(stderr, "Error occured: failed to read source path from stdin\n");
		return 1;
	}
	chomp(source_path);

	printf("Enter destination file path: ");
	if (!fgets(destination_path, sizeof(destination_path), stdin)){
		fprintf(stderr, "Error occured: failed to read destination path from stdin\n");
		return 1;
	}
	chomp(destination_path);

	int source_fd = open(source_path, O_RDONLY);
	if (source_fd < 0){
		perror("open(source)");
		return 1;
	}

	int destination_fd = open(destination_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (destination_fd < 0){
		perror("open(destination)");
		close(source_fd);
		return 1;
	}

	char buf[BUF_SIZE];
	unsigned long long total_bytes = 0ULL;

	for(;;) {
		ssize_t r = read(source_fd, buf, sizeof(buf));

		if (r < 0){
			if(errno == EINTR){
				continue;
			}

			perror("read");
			close(source_fd);
			close(destination_fd);
			return 1;
		}
		if (r == 0) {
			break;
		}

		if (write_all(destination_fd, buf, (size_t)r) < 0){
			perror("write");
			close(source_fd);
			close(destination_fd);
			return 1;
		}
		total_bytes += (unsigned long long)r;
	}

	if (close(source_fd) < 0){
		perror("close(source)");
	}

	if (close(destination_fd) < 0) {
		perror("close(destination)");
		return 1;
	}

	printf("Bytes copied: %llu\n", total_bytes);
	return 0;
}
