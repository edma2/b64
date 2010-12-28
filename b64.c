/* base 64 encoder-decoder 
 * author: Eugene Ma (edma2) */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define PERMS                  0666
#define RAW_BUFSIZE            48
#define ENCODED_BUFSIZE        64

static int encode(char *raw, char *buf, int size);
static int decode(char *encode, char *buf, int count);
static char map_char(char c);
static char unmap_char(char c);

int main(int argc, char *argv[]) {
	int fd;
	int count;
	char buf_raw[RAW_BUFSIZE];
        char buf_encoded[ENCODED_BUFSIZE];

	if (argc != 3) {
		fprintf(stderr, "usage: b64 [SWITCH] ... [FILE]\n");
		return -1;
	}

        /* encode - read from file, write to stdout */
        if (!strcmp(argv[1], "-e")) {
                /* open source file */
                fd = open(argv[2], O_RDONLY, 0);
                if (fd < 0)
                        fprintf(stderr, "error: cannot open file\n");

                /* returns the number of bytes read */
                while ((count = read(fd, buf_raw, RAW_BUFSIZE)) > 0) {
                        /* return number of bytes encoded */
                        count = encode(buf_raw, buf_encoded, count);
                        if (write(STDOUT_FILENO, buf_encoded, count) != count)
                                fprintf(stderr, "error: write error\n");
                }

                /* read() returns -1 on error */
                if (count < 0)
                        fprintf(stderr, "error: read error\n");
        /* decode - read from stdin, write to file */ 
        } else if (!strcmp(argv[1], "-d")) {
                fd = creat(argv[2], PERMS);
                if (fd < 0)
                        fprintf(stderr, "error: cannot create file\n");

                while ((count = read(STDIN_FILENO, buf_encoded, ENCODED_BUFSIZE)) > 0) {
                        /* return number of bytes decoded */
                        count = decode(buf_encoded, buf_raw, count);
                        if (write(fd, buf_raw, count) != count)
                                fprintf(stderr, "error: write error\n");
                }

                if (count < 0)
                        fprintf(stderr, "error: read error\n");
        } else {
                fprintf(stderr, "error: missing switch flag (-e for encode, -d for decode)\n");
        }
        close(fd);

	return 0;
}

static int decode(char *encode, char *buf, int count) {
        char c;
        int i = 0, j = 0;

        while (count > 0) {
                /* get current byte and first two bits of next byte */
                c = ((unmap_char(encode[i]) << 2) | ((unmap_char(encode[i+1]) & 0x30) >> 4));
                buf[j++] = c;
                /* get second byte */
                c = (((unmap_char(encode[i+1]) & 0xf) << 4) | ((unmap_char(encode[i+2]) & 0x3c) >> 2));
                buf[j++] = c;
                /* last byte */
                c = (((unmap_char(encode[i+2]) & 0x3) << 6) | unmap_char(encode[i+3]));
                buf[j++] = c;
                
                i += 4;
                count -= 4;
        }

        return j;
}

/* count is the number of bytes to encode */
static int encode(char *raw, char *buf, int count) {
        char c;
        int i = 0, j = 0;

        while (count > 0) {
                /* extract first 6 bits */
                c = ((raw[i] & 0xfc) >> 2); 
                buf[j++] = map_char(c);

                /* extract second set */
                c = ((raw[i] & 0x3) << 4) | ((raw[i+1] & 0xf0) >> 4); 
                buf[j++] = map_char(c);

                /* extract third set */
                if (count > 1) {
                        c = ((raw[i+1] & 0xf) << 2) | ((raw[i+2] & 0xc0) >> 6); 
                        buf[j++] = map_char(c);
                }

                /* extract fourth set */
                if (count > 2) {
                        c = (raw[i+2] & 0x3f);
                        buf[j++] = map_char(c);
                }

                i += 3;
                count -= 3;
        }
        /* add padding */
        if (count < 0) 
                buf[j++] = '=';
        if (count < -1) 
                buf[j++] = '=';

        return j;
}

static char map_char(char c) {
        if (c < 26)
                return c + 'A';
        else if (c < 52)
                return c + 'a' - 26;
        else if (c < 62)
                return c + '0' - 52;
        else if (c == 62)
                return '+';
        else
                return '/';
}

static char unmap_char(char c) {
        if (c >= 'A' && c <= 'Z')
                return c - 'A';
        else if (c >= 'a' && c <= 'z')
                return c - 'a' + 26;
        else if (c >= '0' && c <= '9')
                return c - '0' + 52;
        else if (c == '+')
                return 62;
        else if (c == '/')
                return 63;
        /* padding */
        else
                return 0;
}
