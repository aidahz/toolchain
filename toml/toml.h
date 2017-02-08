#ifndef TOML_H
#define TOML_H

#ifdef __cplusplus
#define TOML_EXTERN extern "C"
#else
#define TOML_EXTERN extern
#endif

typedef struct toml_table_t toml_table_t;
typedef struct toml_array_t toml_array_t;

TOML_EXTERN toml_table_t* toml_parse(char* conf,
				 char* errbuf,
				 int errbufsz);

TOML_EXTERN void toml_free(toml_table_t* tab);

TOML_EXTERN const char* toml_get_raw(toml_table_t* tab, const char* key);
TOML_EXTERN toml_array_t* toml_get_array(toml_table_t* tab, const char* key);
TOML_EXTERN toml_table_t* toml_get_table(toml_table_t* tab, const char* key);

TOML_EXTERN const char* toml_index_raw(toml_array_t* arr, int idx);
TOML_EXTERN toml_array_t* toml_index_array(toml_array_t* arr, int idx);
TOML_EXTERN toml_table_t* toml_index_table(toml_array_t* arr, int idx);


/* These functions return 0 on success, -1 on failure. */
/* for toml_raw2string, caller must call free() on the return value */
TOML_EXTERN int toml_raw2string(const char* s, char** ret);
TOML_EXTERN int toml_raw2bool(const char* s, int* ret);
TOML_EXTERN int toml_raw2int(const char* s, int64_t* ret);


#endif /* TOML_H */
