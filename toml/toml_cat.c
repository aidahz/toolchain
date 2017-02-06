#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "toml.h"


static char* readfile(FILE* fp)
{
    int bufsz = 100;
    char* buf = malloc(bufsz);
    int off = 0;

    for (;;) {
	int n = fread(buf + off, 1, bufsz - off, fp);
	if (n < bufsz - off)
	    break;
	char* x = realloc(buf, bufsz + 100);
	if (!x) {
	    fprintf(stderr, "ERROR: out of memory\n");
	    free(buf);
	    return 0;
	}
	buf = x;
	bufsz = bufsz + 100;
	off += n;
    }
    return buf;
}

static void cat(FILE* fp)
{
    char* conf = readfile(fp);
    char  errbuf[200];
    
    toml_table_t* tab = toml_parse(conf, errbuf, sizeof(errbuf));
    free(conf);
    
    if (!tab) {
	fprintf(stderr, "ERROR: %s\n", errbuf);
	return;
    }

    toml_free(tab);
}


int main(int argc, const char* argv[])
{
    int i;
    if (argc == 1) {
	cat(stdin);
    } else {
	for (i = 1; i < argc; i++) {
	    FILE* fp = fopen(argv[i], "r");
	    if (!fp) {
		fprintf(stderr, "ERROR: cannot open %s: %s\n",
			argv[i], strerror(errno));
		exit(1);
	    }
	    cat(fp);
	    fclose(fp);
	}
    }
    return 0;
}
