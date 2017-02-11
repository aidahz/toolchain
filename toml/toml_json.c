#ifdef NDEBUG
#under NDEBUG
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <assert.h>
#include "toml.h"


static void print_array(toml_array_t* arr);
static void print_table(toml_table_t* curtab)
{
    int i;
    const char* key;
    const char* raw;
    toml_array_t* arr;
    toml_table_t* tab;


    printf("{ ");
    for (i = 0; 0 != (key = toml_key_in(curtab, i)); i++) {

	printf("%s\"%s\" = ", i > 0 ? "," : "", key);
	
	if (0 != (raw = toml_raw_in(curtab, key))) {
	    printf("\"%s\"\n", raw);
	} else if (0 != (arr = toml_array_in(curtab, key))) {
	    print_array(arr);
	} else if (0 != (tab = toml_table_in(curtab, key))) {
	    print_table(tab);
	} else {
	    abort();
	}
    }
    printf(" } ");
}

static void print_array(toml_array_t* curarr)
{
    toml_array_t* arr;
    const char* raw;
    toml_table_t* tab;
    int i;

    printf("[ ");
    switch (toml_array_typ(curarr)) {

    case 'v': 
	for (i = 0; 0 != (raw = toml_raw_at(curarr, i)); i++) {
	    printf("%s\"%s\"", i > 0 ? "," : "", raw);
	}
	break;

    case 'a': 
	for (i = 0; 0 != (arr = toml_array_at(curarr, i)); i++) {
	    printf("%s", i > 0 ? "," : "");
	    print_array(arr);
	}
	break;
	    
    case 't': 
	for (i = 0; 0 != (tab = toml_table_at(curarr, i)); i++) {
	    printf("%s", i > 0 ? "," : "");
	    print_table(tab);
	}
	break;
	
    default:
	abort();
    }
    printf("] ");
}



static void cat(FILE* fp)
{
    char  errbuf[200];
    
    toml_table_t* tab = toml_parse_file(fp, errbuf, sizeof(errbuf));
    if (!tab) {
	fprintf(stderr, "ERROR: %s\n", errbuf);
	return;
    }

    print_table(tab);

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
