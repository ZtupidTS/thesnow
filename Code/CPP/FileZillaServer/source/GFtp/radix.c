/*
 * Radix encoding function. Could also be used standalone - aks
 */
#include <string.h>

static const char *radixN =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const char pad = '=';

radix_encode(void *in, void *out, int *len, int decode)
{
	unsigned char *inbuf = (unsigned char *)in;
	unsigned char *outbuf = (unsigned char *)out;
	int i,j,D;
	char *p;
	unsigned char c = 0;

	if (decode) {
		for (i=0,j=0; inbuf[i] && inbuf[i] != pad; i++) {
		    if ((p = strchr(radixN, inbuf[i])) == NULL) return(1);
		    D = p - radixN;
		    switch (i&3) {
			case 0:
			    outbuf[j] = D<<2;
			    break;
			case 1:
			    outbuf[j++] |= D>>4;
			    outbuf[j] = (D&15)<<4;
			    break;
			case 2:
			    outbuf[j++] |= D>>2;
			    outbuf[j] = (D&3)<<6;
			    break;
			case 3:
			    outbuf[j++] |= D;
		    }
		}
		switch (i&3) {
			case 1: return(3);
			case 2: if (D&15) return(3);
				if (strcmp((char *)&inbuf[i], "==")) return(2);
				break;
			case 3: if (D&3) return(3);
				if (strcmp((char *)&inbuf[i], "="))  return(2);
		}
		*len = j;
	} else {
		for (i=0,j=0; i < *len; i++)
		    switch (i%3) {
			case 0:
			    outbuf[j++] = radixN[inbuf[i]>>2];
			    c = (inbuf[i]&3)<<4;
			    break;
			case 1:
			    outbuf[j++] = radixN[c|inbuf[i]>>4];
			    c = (inbuf[i]&15)<<2;
			    break;
			case 2:
			    outbuf[j++] = radixN[c|inbuf[i]>>6];
			    outbuf[j++] = radixN[inbuf[i]&63];
			    c = 0;
		    }
		if (i%3) outbuf[j++] = radixN[c];
		switch (i%3) {
			case 1: outbuf[j++] = pad;
			case 2: outbuf[j++] = pad;
		}
		outbuf[*len = j] = '\0';
	}
	return(0);
}

char *radix_error(e)
{
	switch (e) {
	    case 0:  return("Success");
	    case 1:  return("Bad character in encoding");
	    case 2:  return("Encoding not properly padded");
	    case 3:  return("Decoded # of bits not a multiple of 8");
	    default: return("Unknown error");
	}
}