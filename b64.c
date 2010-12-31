/* b64 - base64 encoder and decoder, reads from files and writes to stdout 
 * author: Eugene Ma (edma2) */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define BUFSIZE_RAW            48
#define BUFSIZE_ENCODED        64

int encode(uint8_t *buf_raw, uint8_t *buf_encoded, int len);
int decode(uint8_t *buf_encoded, uint8_t *buf_raw, int len);

/* base64 to ascii */
const uint8_t map[64] = { 'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',
                'J',  'K',  'L',  'M',  'N',  'O',  'P',  'Q',  'R',  'S',  'T',
                'U',  'V',  'W',  'X',  'Y',  'Z', 'a',  'b',  'c',  'd',
                'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',  'n',
                'o',  'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z',
                '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9', '+', '/' };
/* ascii to base64 */
const uint8_t unmap[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 0, 0, 0, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 
        5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
        25, 0, 0, 0, 0, 0, 0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
        39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 0, 0, 0, 0, 0};

int main(int argc, char *argv[]) {
        FILE *f;
	int len;
        int raw_total = 0;
        int encoded_total = 0;
	uint8_t buf_raw[BUFSIZE_RAW];
        uint8_t buf_encoded[BUFSIZE_ENCODED];

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
                /* read up to BUFSIZE_RAW bytes */
                while ((len = fread(buf_raw, sizeof(uint8_t), BUFSIZE_RAW, f)) > 0) {
                        raw_total += len;
                        /* return number of bytes encoded */
                        len = encode(buf_raw, buf_encoded, len);
                        encoded_total += len;
                        if (fwrite(buf_encoded, sizeof(uint8_t), len, stdout) != len) {
                                fprintf(stderr, "b64: write error\n");
                                break;
                        }
                }
                fprintf(stderr, "%d raw bytes\n", raw_total);
                fprintf(stderr, "%d encoded bytes\n", encoded_total);
                if (len < 0)
                        fprintf(stderr, "b64: read error\n");
        /* decode */
        } else if (strcmp(argv[1], "-d") == 0) {
                /* read up to BUFSIZE_ENCODED bytes */
                while ((len = fread(buf_encoded, sizeof(uint8_t), BUFSIZE_ENCODED, f)) > 0) {
                        /* return number of bytes decoded */
                        len = decode(buf_encoded, buf_raw, len);
                        if (fwrite(buf_raw, sizeof(uint8_t), len, stdout) != len) {
                                fprintf(stderr, "b64: write error\n");
                                break;
                        }
                }
                if (len < 0)
                        fprintf(stderr, "b64: read error\n");
        } else {
                fprintf(stderr, "b64: missing flag (-e for encode, -d for decode)\n");
        }
        fclose(f);

	return 0;
}

int decode(uint8_t *buf_encoded, uint8_t *buf_raw, int len) {
        uint8_t c;
        int i, j;

        /* each iteration reads four bytes at a time until we've read up to len bytes */
        for (i = 0, j = 0; i < len; i += 4) {
                /* first byte */
                c = ((unmap[buf_encoded[i]] << 2) | ((unmap[buf_encoded[i+1]] & 0x30) >> 4));
                buf_raw[j++] = c;
                /* second byte */
                c = (((unmap[buf_encoded[i+1]] & 0xf) << 4) | ((unmap[buf_encoded[i+2]] & 0x3c) >> 2));
                buf_raw[j++] = c;
                /* last byte */
                c = (((unmap[buf_encoded[i+2]] & 0x3) << 6) | unmap[buf_encoded[i+3]]);
                buf_raw[j++] = c;
        }
        /* check if we've reached the end of the file */
        i -= 4;
        /* write one less byte for each padding */
        if (buf_encoded[i+2] == '=')
                j--;
        if (buf_encoded[i+3] == '=')
                j--;

        return j;
}

int encode(uint8_t *buf_raw, uint8_t *buf_encoded, int len) {
        uint8_t c;
        int i, j;

        /* read up to len bytes */
        for (i = 0, j = 0; i < len; i += 3) {
                /* extract first 6 bits */
                c = ((buf_raw[i] & 0xfc) >> 2); 
                buf_encoded[j++] = map[c];
                /* extract second set */
                c = ((buf_raw[i] & 0x3) << 4) | ((buf_raw[i+1] & 0xf0) >> 4); 
                buf_encoded[j++] = map[c];
                /* extract third set */
                if (len - i > 1) {
                        c = ((buf_raw[i+1] & 0xf) << 2) | ((buf_raw[i+2] & 0xc0) >> 6); 
                        buf_encoded[j++] = map[c];
                } else {
                        buf_encoded[j++] = '=';
                }
                /* extract fourth set */
                if (len - i > 2) {
                        c = (buf_raw[i+2] & 0x3f);
                        buf_encoded[j++] = map[c];
                } else {
                        buf_encoded[j++] = '=';
                }
        }

        return j;
}
