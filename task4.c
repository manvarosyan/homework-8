#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INBUF_MAX 4096

static ssize_t write_all(int fd, const void *buf, size_t n) {
    const char *p = (const char *)buf;
    size_t sent = 0;
    while (sent < n) {
        ssize_t w = write(fd, p + sent, n - sent);
        if (w < 0) { 
		if (errno == EINTR) continue;
	       	return -1; 
	}
        sent += (size_t)w;
    }
    return (ssize_t)sent;
}

int main(void) {
    int fd = open("log.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) { 
	    perror("open log.txt"); 
	    return 1; 
    }

    char inbuf[INBUF_MAX];
    size_t used = 0;
    int saw_nl = 0;

    while (used < INBUF_MAX) {
        char c;
        ssize_t r = read(STDIN_FILENO, &c, 1);
        if (r < 0) { 
		if (errno == EINTR) continue; 
		perror("read stdin"); 
		close(fd); 
		return 1; 
	}
        if (r == 0) break;        
        inbuf[used++] = c;
        if (c == '\n') { 
		saw_nl = 1; 
		break; 
	} 
    }

    if (used > 0 && inbuf[used-1] == '\n') { 
	    used--; 
	    saw_nl = 1; 
    }

    char prefix[64];
    pid_t pid = getpid();
    int plen = snprintf(prefix, sizeof(prefix), "PID=%ld: ", (long)pid);
    if (plen < 0 || (size_t)plen >= sizeof(prefix)) {
        fprintf(stderr, "prefix snprintf failed\n");
        close(fd);
        return 1;
    }

   
    if (write_all(fd, prefix, (size_t)plen) < 0) { 
	    perror("write prefix"); 
	    close(fd); 
	    return 1; 
    }
    
    if (used > 0 && write_all(fd, inbuf, used) < 0)  { 
	    perror("write line"); 
	    close(fd); 
	    return 1; 
    }
    
    if (write_all(fd, "\n", 1) < 0) { 
	    perror("write newline"); 
	    close(fd); 
	    return 1; 
    }

   
    off_t off = lseek(fd, 0, SEEK_CUR);
    if (off == (off_t)-1) { 
	    perror("lseek SEEK_CUR"); 
	    close(fd); 
	    return 1; 
    }

  
    printf("Final offset after append: %lld\n", (long long)off);

    if (close(fd) < 0) { 
	    perror("close"); 
	    return 1; 
    }
    return 0;
}

/*
Why does SEEK_CUR still grow with O_APPEND?

O_APPEND makes each write be positioned at the file's current end (the kernel
atomically sets the write position to EOF and then writes). Although the *write
position* for the operation ignores the file descriptor's current offset,
the kernel still updates the descriptor's file offset to the position after
the data that was appended. 

Therefore, lseek(fd, 0, SEEK_CUR) returns a growing offset that reflects the file size after each append.
*/
