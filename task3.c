#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void chomp(char *s) {
    if (!s) return;
    size_t n = strcspn(s, "\n");
    s[n] = '\0';
}

static ssize_t read_retry(int fd, void *buf, size_t n) {
    for (;;) {
        ssize_t r = read(fd, buf, n);
        if (r < 0 && errno == EINTR) continue;
        return r;
    }
}

static ssize_t write_all(int fd, const void *buf, size_t n) {
    const char *p = buf;
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
	char path[4096];

	printf("Enter file path: ");
        if (!fgets(path, sizeof(path), stdin)) {
             fprintf(stderr, "Error: failed to read path from stdin\n");
             return 1;
        }
        chomp(path);

	 int fd = open(path, O_RDONLY);
         if (fd < 0) { 
		 perror("open"); 
		 return 1; 
	 }

	 off_t size = lseek(fd, 0, SEEK_END);
         if (size == (off_t)-1) { 
		 perror("lseek SEEK_END"); 
		 close(fd); 
		 return 1; 
	 }

	 if (size == 0) {
                 if (write_all(STDOUT_FILENO, "\n", 1) < 0) { 
			 perror("write newline"); 
		 }
                 close(fd);
                 return 0;
          }


	  unsigned char byte;
	  for (off_t pos = size; pos > 0; ){
		  pos--;
		  if(lseek(fd, pos, SEEK_SET) == (off_t)-1){
			  perror("lseek SEEK_SET");
			  close(fd);
			  return 1;
		  }

		  ssize_t r = read_retry(fd, &byte, 1);
		  if (r < 0){
			  perror("read");
			  close(fd);
			  return 1;
		  }

		  if (r == 0){
			  fprintf(stderr, "Error");
			  close(fd);
			  return 1;
		  }

		  if (write_all(STDOUT_FILENO, &byte, 1) < 0){
			  perror("write stdout");
			  close(fd);
			  return 1;
		  }

		  if (write_all(STDOUT_FILENO, "\n", 1) < 0){
			  perror("write newline");
		  }
	  }

		  if (close(fd) < 0) {
			  perror("close");
			  return 1;
		  }

		  return 0;
}

