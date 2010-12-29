/* base 64 encoder-decoder 
 * author: Eugene Ma (edma2) */

#include <stdio.h>
#include <string.h>

#define BUFSIZE_RAW            48
#define BUFSIZE_ENCODED        64

int encode(char *raw, unsigned char *buf, int size);
int decode(unsigned char *encode, char *buf, int count);

const char map[64] = { 'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',
                'J',  'K',  'L',  'M',  'N',  'O',  'P',  'Q',  'R',  'S',  'T',
                'U',  'V',  'W',  'X',  'Y',  'Z', 'a',  'b',  'c',  'd',
                'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',  'n',
                'o',  'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z',
                '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9', '+', '/' };
const char unmap[128] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 0, 0, 0, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 
        5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
        25, 0, 0, 0, 0, 0, 0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
        39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 0, 0, 0, 0, 0};

int main(int argc, char *argv[]) {
        FILE *f;
	int count;
	char buf_raw[BUFSIZE_RAW];
        unsigned char buf_encoded[BUFSIZE_ENCODED];

	if (argc != 3) {
		fprintf(stderr, "usage: b64 <mode> <input file>\n");
		return -1;
	}

        /* open source file */
        f = fopen(argv[2], "r");
        if (f == NULL) {
                fprintf(stderr, "b64: cannot open file\n");
                return -1;
        }

        /* encode */
        if (strcmp(argv[1], "-e") == 0) {
                /* returns the number of bytes read */
                while ((count = fread(buf_raw, sizeof(char), BUFSIZE_RAW, f)) > 0) {
                        /* return number of bytes encoded */
                        count = encode(buf_raw, buf_encoded, count);
                        if (fwrite(buf_encoded, sizeof(char), count, stdout) != count) {
                                fprintf(stderr, "b64: write error\n");
                                break;
                        }
                }
                if (count < 0)
                        fprintf(stderr, "b64: read error\n");
        /* decode */
        } else if (strcmp(argv[1], "-d") == 0) {
                /* return the number of bytes read */
                while ((count = fread(buf_encoded, sizeof(char), BUFSIZE_ENCODED, f)) > 0) {
                        /* return number of bytes decoded */
                        count = decode(buf_encoded, buf_raw, count);
                        if (count < 0) {
                                fprintf(stderr, "b64: not a base64 file\n");
                                fclose(f);
                                return -1;
                        }
                        if (fwrite(buf_raw, sizeof(char), count, stdout) != count) {
                                fprintf(stderr, "b64: write error\n");
                                break;
                        }
                }
                if (count < 0)
                        fprintf(stderr, "b64: read error\n");
        } else {
                fprintf(stderr, "b64: missing switch flag (-e for encode, -d for decode)\n");
        }
        fclose(f);

	return 0;
}

int decode(unsigned char *encode, char *buf, int count) {
        char c;
        int i = 0, j = 0;

        for (; count > 0; count -= 4) {
                if (encode[i] > sizeof(unmap) || encode[i+1] > sizeof(unmap) \
                                || encode[i+2] > sizeof(unmap) || encode[i+3] > sizeof(unmap))
                        return -1;
                /* get current byte and first two bits of next byte */
                c = ((unmap[encode[i]] << 2) | ((unmap[encode[i+1]] & 0x30) >> 4));
                buf[j++] = c;
                /* get second byte */
                c = (((unmap[encode[i+1]] & 0xf) << 4) | ((unmap[encode[i+2]] & 0x3c) >> 2));
                buf[j++] = c;
                /* last byte */
                c = (((unmap[encode[i+2]] & 0x3) << 6) | unmap[encode[i+3]]);
                buf[j++] = c;
                i += 4;
        }

        return j;
}

/* count is the number of bytes to encode */
int encode(char *raw, unsigned char *buf, int count) {
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
