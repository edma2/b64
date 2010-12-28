/* base 64 encoder-decoder 
 * author: Eugene Ma (edma2) */

#include <stdio.h>
#include <string.h>

#define PERMS                  0666
#define RAW_BUFSIZE            48
#define ENCODED_BUFSIZE        64

int encode(char *raw, char *buf, int size);
int decode(char *encode, char *buf, int count);
void init_unmap_table(void);

char map[64] = { 'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',
                'J',  'K',  'L',  'M',  'N',  'O',  'P',  'Q',  'R',  'S',  'T',
                'U',  'V',  'W',  'X',  'Y',  'Z', 'a',  'b',  'c',  'd',
                'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',  'n',
                'o',  'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z',
                '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9', '+', '/' };
char unmap[128];

void init_unmap_table(void) {
        int i;

        for (i = 'A'; i <= 'Z'; i++)
                unmap[i] = i - 'A';
        for (i = 'a'; i <= 'z'; i++)
                unmap[i] = i - 'a' + 26;
        for (i = '0'; i <= '9'; i++) 
                unmap[i] = i - '0' + 52;
        unmap['+'] = 62;
        unmap['/'] = 63;
        unmap['='] = 0;
}

int main(int argc, char *argv[]) {
        FILE *f;
	int count;
	char buf_raw[RAW_BUFSIZE];
        char buf_encoded[ENCODED_BUFSIZE];

	if (argc != 3) {
		fprintf(stderr, "usage: b64 <mode> <file>\n");
		return -1;
	}

        /* encode - read from file, write to stdout */
        if (!strcmp(argv[1], "-e")) {
                /* open source file */
                f = fopen(argv[2], "r");
                if (f == NULL) {
                        fprintf(stderr, "error: cannot open file\n");
                        return -1;
                }

                /* returns the number of bytes read */
                while ((count = fread(buf_raw, sizeof(char), RAW_BUFSIZE, f)) > 0) {
                        /* return number of bytes encoded */
                        count = encode(buf_raw, buf_encoded, count);
                        if (fwrite(buf_encoded, sizeof(char), count, stdout) != count) {
                                fprintf(stderr, "error: write error\n");
                                break;
                        }
                }

                if (count < 0)
                        fprintf(stderr, "error: read error\n");
        /* decode - read from stdin, write to file */ 
        } else if (!strcmp(argv[1], "-d")) {
                f = fopen(argv[2], "w+");
                if (f == NULL) {
                        fprintf(stderr, "error: cannot create file\n");
                        return -1;
                }

                init_unmap_table();
                /* return the number of bytes read */
                while ((count = fread(buf_encoded, sizeof(char), ENCODED_BUFSIZE, stdin)) > 0) {
                        /* return number of bytes decoded */
                        count = decode(buf_encoded, buf_raw, count);
                        if (fwrite(buf_raw, sizeof(char), count, f) != count) {
                                fprintf(stderr, "error: write error\n");
                                break;
                        }
                }

                if (count < 0)
                        fprintf(stderr, "error: read error\n");
        } else {
                fprintf(stderr, "error: missing switch flag (-e for encode, -d for decode)\n");
        }
        fclose(f);

	return 0;
}

int decode(char *encode, char *buf, int count) {
        char c;
        int i = 0, j = 0;

        for (; count > 0; count -= 4) {
                /* get current byte and first two bits of next byte */
                c = ((unmap[(int)encode[i]] << 2) | ((unmap[(int)encode[i+1]] & 0x30) >> 4));
                buf[j++] = c;
                /* get second byte */
                c = (((unmap[(int)encode[i+1]] & 0xf) << 4) | ((unmap[(int)encode[i+2]] & 0x3c) >> 2));
                buf[j++] = c;
                /* last byte */
                c = (((unmap[(int)encode[i+2]] & 0x3) << 6) | unmap[(int)encode[i+3]]);
                buf[j++] = c;
                i += 4;
        }

        return j;
}

/* count is the number of bytes to encode */
int encode(char *raw, char *buf, int count) {
        int c;
        int i = 0, j = 0;

        for (; count > 0; count -= 3) {
                /* extract first 6 bits */
                c = ((raw[i] & 0xfc) >> 2); 
                buf[j++] = map[c];

                /* extract second set */
                c = ((raw[i] & 0x3) << 4) | ((raw[i+1] & 0xf0) >> 4); 
                buf[j++] = map[c];

                /* extract third set */
                if (count > 1) {
                        c = ((raw[i+1] & 0xf) << 2) | ((raw[i+2] & 0xc0) >> 6); 
                        buf[j++] = map[c];
                }

                /* extract fourth set */
                if (count > 2) {
                        c = (raw[i+2] & 0x3f);
                        buf[j++] = map[c];
                }
                i += 3;
        }
        /* add padding */
        if (count < 0) 
                buf[j++] = '=';
        if (count < -1) 
                buf[j++] = '=';

        return j;
}
