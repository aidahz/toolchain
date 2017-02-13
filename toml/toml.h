#ifndef TOML_H
#define TOML_H

#ifdef __cplusplus
#define TOML_EXTERN extern "C"
#else
#define TOML_EXTERN extern
#endif

typedef struct toml_table_t toml_table_t;
typedef struct toml_array_t toml_array_t;

TOML_EXTERN toml_table_t* toml_parse_file(FILE* fp, 
					  char* errbuf,
					  int errbufsz);
TOML_EXTERN toml_table_t* toml_parse(char* conf, /* NUL terminated pls. */
				     char* errbuf,
				     int errbufsz);

/* free the table returned by toml_parse() or toml_parse_file(). */
TOML_EXTERN void toml_free(toml_table_t* tab);

/* keys in table */
TOML_EXTERN const char* toml_key_in(toml_table_t* tab, int keyidx);

/* table lookup */
TOML_EXTERN const char* toml_raw_in(toml_table_t* tab, const char* key);
TOML_EXTERN toml_array_t* toml_array_in(toml_table_t* tab, const char* key);
TOML_EXTERN toml_table_t* toml_table_in(toml_table_t* tab, const char* key);

/* array type: 't'able, 'a'rray, 'v'alue */
TOML_EXTERN char toml_array_typ(toml_array_t* arr);

/* array index */
TOML_EXTERN const char* toml_raw_at(toml_array_t* arr, int idx);
TOML_EXTERN toml_array_t* toml_array_at(toml_array_t* arr, int idx);
TOML_EXTERN toml_table_t* toml_table_at(toml_array_t* arr, int idx);


/* These functions return 0 on success, -1 on failure. */
/* for toml_raw2string, caller must call free() on the return value */
TOML_EXTERN int toml_raw2string(const char* s, char** ret);
TOML_EXTERN int toml_raw2bool(const char* s, int* ret);
TOML_EXTERN int toml_raw2int(const char* s, int64_t* ret);
TOML_EXTERN int toml_raw2double(const char* s, double* ret);


typedef struct toml_timestamp_t toml_timestamp_t;
struct toml_timestamp_t {
    struct {
	int year, month, day;
	int hour, minute, second;
	char z[10];
    } __buffer;
    int *year, *month, *day;
    int *hour, *minute, *second;
    char* z;
};
TOML_EXTERN int toml_raw2timestamp(const char* s, toml_timestamp_t* ret);




#endif /* TOML_H */
