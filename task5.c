#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MiB (1024*1024)

static void die(const char *msg) { 
	perror(msg); 
	exit(1); 
}

static ssize_t write_all(int fd, const void *buf, size_t n) {
    const char *p = (const char *)buf;
    size_t done = 0;
    while (done < n) {
        ssize_t w = write(fd, p + done, n - done);
        if (w < 0) { 
		if (errno == EINTR) continue; 
		return -1; 
	}
        done += (size_t)w;
    }
    return (ssize_t)done;
}

int main(void){
	const char *fname = "sparse.bin";

	int fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
	       	die("open for write");
	}

	if (write_all(fd, "START", 5) < 0) {
		die("write START");
	}

	off_t off = lseek(fd, MiB, SEEK_CUR);
        if (off == (off_t)-1) {
		die("lseek forward 1 MiB");
	}

	if (write_all(fd, "END", 3) < 0) {
		die("write END");
	}

        if (close(fd) < 0) {
		die("close after write");
	}

	fd = open(fname, O_RDONLY);
        if (fd < 0) {
		die("open for read");
	}
        off_t size = lseek(fd, 0, SEEK_END);
        if (size == (off_t)-1) {
		die("lseek SEEK_END");
	}
        if (close(fd) < 0) {
		die("close after read");
	}

	printf("Apparent file size: %lld bytes\n", (long long)size);

	return 0;
}

/*
Why is disk usage(for example: `du`) much smaller than the apparent size?

Sparse files let the filesystem represent large regions of zeros as "holes"
without allocating physical blocks. Using lseek() beyond EOF creates such a hole.
The *apparent size* (from lseek/SEEK_END or `stat().st_size`) reflects the
logical highest written position+1 (here ~1 MiB), but only the actually written
data ("START" and "END") consumes disk blocks. 
Therefore `du` (which reports allocated blocks) is much smaller than the apparent size.
*/




