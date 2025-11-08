#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF 8192

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

int main(void){
	char p1[4096], p2[4096];

	printf("Enter first file path: ");
	if (!fgets(p1, sizeof(p1), stdin)) {
                  fprintf(stderr, "Error: failed to read first path\n");
                  return 2;
        }
        chomp(p1);


	printf("Enter second file path: ");
        if (!fgets(p2, sizeof(p2), stdin)) {
                  fprintf(stderr, "Error: failed to read second path\n");
                  return 2;
        }
        chomp(p2);


	int fd1 = open(p1, O_RDONLY);
        if (fd1 < 0) { 
		perror("open first"); 
		return 2; 
	}
        int fd2 = open(p2, O_RDONLY);
        if (fd2 < 0) { 
		perror("open second"); 
		close(fd1); 
		return 2; 
	}

	unsigned char b1[BUF], b2[BUF];
        unsigned long long offset = 0ULL;


	for (;;) {
                  ssize_t r1 = read_retry(fd1, b1, sizeof(b1));
                  if (r1 < 0) { 
			  perror("read first"); 
			  close(fd1); 
			  close(fd2); 
			  return 2; 
		  }

                  ssize_t r2 = read_retry(fd2, b2, sizeof(b2));
                  if (r2 < 0) { 
			  perror("read second"); 
			  close(fd1); 
			  close(fd2); 
			  return 2; 
		  }

      
                  ssize_t m = (r1 < r2) ? r1 : r2;
                  for (ssize_t i = 0; i < m; i++) {
                          if (b1[i] != b2[i]) {
                                printf("Files differ at byte %llu\n", offset + (unsigned long long)i);
                                close(fd1); 
			        close(fd2);
                                return 1;
                          }
                   }
	  
		   if (r1 == 0 && r2 == 0){
			   printf("Files are identical\n");
			   close(fd1);
			   close(fd2);
			   return 0;
		   }

		   if (r1 != r2){
			   unsigned long long first_diff = offset + (unsigned long long)m;
			   printf("Files differ at byte %llu\n", first_diff);
			   close(fd1);
			   close(fd2);
			   return 1;
		   }

		   offset += (unsigned long long)m;
	}
}
