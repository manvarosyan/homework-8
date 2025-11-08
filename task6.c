#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF 4096

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

static ssize_t read_all(int fd, void *buf, size_t n) {
    char *p = (char *)buf;
    size_t got = 0;
    while (got < n) {
        ssize_t r = read(fd, p + got, n - got);
        if (r < 0) { 
		if (errno == EINTR) continue; 
		return -1; 
	}
        if (r == 0) break;
        got += (size_t)r;
    }
    return (ssize_t)got;
}

int main(void) {
    const char *fname = "numbers.txt";

    
    {
        int fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
		die("open create");
	}

        char line[32];
        for (int i = 1; i <= 10; i++) {
            int len = snprintf(line, sizeof(line), "%d\n", i);
            if (len < 0 || (size_t)len >= sizeof(line)) {
                fprintf(stderr, "snprintf failed\n");
                close(fd);
                return 1;
            }
            if (write_all(fd, line, (size_t)len) < 0) {
		    die("write lines");
	    }
        }
        if (close(fd) < 0) {
		die("close after create");
	}
    }

    int fd = open(fname, O_RDWR);
    if (fd < 0) {
	    die("open RDWR");
    }

    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == (off_t)-1) {
	    die("lseek SEEK_END (size)");
    }

    
    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
	    die("lseek SEEK_SET");
    }

    char buf[BUF];
    off_t start4 = -1;
    off_t after4 = -1;

    off_t cur_offset = 0;        
    off_t start_of_line = 0;    
    int line_no = 1;

    for (;;) {
        ssize_t r = read(fd, buf, sizeof(buf));
        if (r < 0) { 
		if (errno == EINTR) continue; 
		die("read scan"); 
	}
        if (r == 0) break;

        for (ssize_t i = 0; i < r; i++, cur_offset++) {
            if (line_no == 4 && start4 < 0) {
		    start4 = start_of_line;
	    }

            if (buf[i] == '\n') {
                line_no++;
                start_of_line = cur_offset + 1;
                if (line_no == 5 && after4 < 0) {
		       	after4 = start_of_line;
		}
            }
        }
    }

    if (start4 >= 0 && after4 < 0) {
	    after4 = file_size;
    }

    if (start4 < 0) {
        fprintf(stderr, "Could not find start of line 4 (file too short?)\n");
        close(fd);
        return 1;
    }
    if (after4 < 0) {
        fprintf(stderr, "Internal error: after4 not determined\n");
        close(fd);
        return 1;
    }

    off_t remainder_len = file_size - after4;
    char *remainder = NULL;
    if (remainder_len > 0) {
        remainder = (char *)malloc((size_t)remainder_len);
        if (!remainder) { 
		fprintf(stderr, "malloc failed\n"); 
		close(fd); 
		return 1; 
	}

        if (lseek(fd, after4, SEEK_SET) == (off_t)-1) {
		die("lseek to after4");
	}
        if (read_all(fd, remainder, (size_t)remainder_len) != remainder_len) {
            fprintf(stderr, "read remainder failed/short\n");
            free(remainder);
            close(fd);
            return 1;
        }
    }

    const char *replacement = "100\n";
    const size_t repl_len = 4;

    if (lseek(fd, start4, SEEK_SET) == (off_t)-1) {
	    die("lseek to start4");
    }
    if (write_all(fd, replacement, repl_len) < 0) {
	    die("write replacement");
    }
    if (remainder_len > 0 && write_all(fd, remainder, (size_t)remainder_len) < 0) {
	    die("write remainder");
    }


    off_t new_size = start4 + (off_t)repl_len + remainder_len;
    if (ftruncate(fd, new_size) < 0) {
	    die("ftruncate new_size");
    }

    free(remainder);

 
    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
	    die("lseek to start for print");
    }
    for (;;) {
        ssize_t r = read(fd, buf, sizeof(buf));
        if (r < 0) { 
		if (errno == EINTR) continue; 
		die("read print"); 
	}
        if (r == 0) break;
        if (write_all(STDOUT_FILENO, buf, (size_t)r) < 0) {
		die("write stdout");
	}
    }

    if (close(fd) < 0) {
	    die("close");
    }
    return 0;
}
