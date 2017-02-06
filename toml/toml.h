#ifndef TOML_H
#define TOML_H

typedef struct toml_table_t toml_table_t;
typedef struct toml_array_t toml_array_t;
typedef struct toml_keyval_t toml_keyval_t;

toml_table_t* toml_parse(char* conf,
			 char* errbuf,
			 int errbufsz);
			 
void toml_free(toml_table_t* tab);

struct toml_keyval_t {
    const char* key;
    const char* val;
};


struct toml_array_t {
    const char* key;
    int typ; /* 'v'alue, 'a'rray, or 't'able */
    
    int nelem;
    char** val;
    toml_array_t** arr;
    toml_table_t** tab;
};
    

struct toml_table_t {
    const char* key;

    int             nkval;
    toml_keyval_t** kval;

    int            narr;
    toml_array_t** arr;

    int            ntab;
    toml_table_t** tab;
};

#endif /* TOML_H */
