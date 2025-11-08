#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void die(const char *msg){
	perror(msg);
	exit(1);
}

static ssize_t write_all(int fd, const void *buf, size_t n){
	const char *p = buf;
	size_t done = 0;
	while (done < n){
		ssize_t w = write(fd, p + done, n - done);
		if (w < 0){
			if (errno == EINTR){
				continue;
			}
			return -1;
		}
		done += (size_t)w;
	}
	return (ssize_t)done;
}

int main(void){
	 const char *fname = "data.txt";
         const char *text  = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	 int fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	 if (fd < 0) {
		 die("open create");
	 }

	 if (write_all(fd, text, 26) < 0){
		 die("write");
	 }
	 if (close(fd) < 0){
		 die("close after write");
	 }

	 fd = open(fname, O_RDWR);
	 if (fd < 0) {
		 die("open rdwr");
	 }

	 off_t sz_before = lseek(fd, 0, SEEK_END);
         if (sz_before == (off_t)-1) {
		 die("lseek SEEK_END (before)");
	 }
         printf("Size before truncate: %lld bytes\n", (long long)sz_before);


	 if (ftruncate(fd, 10) < 0) {
		 die("ftruncate");
	 }
         off_t sz_after = lseek(fd, 0, SEEK_END);
         if (sz_after == (off_t)-1) {
		 die("lseek SEEK_END (after)");
	 }
         printf("Size after  truncate: %lld bytes\n", (long long)sz_after);


	 if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
		 die("lseek SEEK_SET");
	 }
         char buf[256];
         ssize_t r = read(fd, buf, sizeof(buf));
         if (r < 0) {
		 die("read");
	 }
         
	 printf("\n");
         printf("Remaining content (%zd bytes): ", r);
	 fflush(stdout);
         if (write_all(STDOUT_FILENO, buf, (size_t)r) < 0) {
		 die("write stdout");
	 }
         printf("\n");


	 if(close(fd) < 0){
		 die("close final");
	 }
	 return 0;
}
