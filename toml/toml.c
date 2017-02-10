#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include "toml.h"

typedef struct toml_keyval_t toml_keyval_t;
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


static void xfree(const void* x) { if (x) free((void*)x); }


enum tokentype_t {
    INVALID,
    DOT,
    COMMA,
    EQUAL,
    LBRACE,
    RBRACE,
    NEWLINE,
    LBRACKET,
    RBRACKET,
    LBRACKET2,
    RBRACKET2,
    STRING,
    MLSTRING,
};
typedef enum tokentype_t tokentype_t;

typedef struct token_t token_t;
struct token_t {
    tokentype_t tok;
    int         lineno;
    char*       ptr;
    int         len;
    int         eof;
};


typedef struct context_t context_t;
struct context_t {
    char* start;
    char* stop;
    char* errbuf;
    int   errbufsz;
    jmp_buf jmp;

    token_t tok;
    toml_table_t* root;
    toml_table_t* curtab;
};

#define STRINGIFY(x) #x
#define TOSTRING(x)  STRINGIFY(x)
#define FLINE __FILE__ ":" TOSTRING(__LINE__)
static int outofmemory(context_t* ctx, const char* fline);
static tokentype_t next_token(context_t* ctx, int dotisspecial);

static void internal_error(context_t* ctx, const char* fline)
{
    snprintf(ctx->errbuf, ctx->errbufsz, "internal error (%s)", fline);
    longjmp(ctx->jmp, 1);
}

static void syntax_error(context_t* ctx, int lineno, const char* msg)
{
    snprintf(ctx->errbuf, ctx->errbufsz, "line %d: %s", lineno, msg);
    longjmp(ctx->jmp, 1);
}

static void bad_key(context_t* ctx, int lineno)
{
    snprintf(ctx->errbuf, ctx->errbufsz, "line %d: bad key", lineno);
    longjmp(ctx->jmp, 1);
}

static void noimpl(context_t* ctx, const char* feature)
{
    snprintf(ctx->errbuf, ctx->errbufsz, "not implemented: %s", feature);
    longjmp(ctx->jmp, 1);
}

typedef struct normkey_t normkey_t;
struct normkey_t {
    char str[201];
};

static void check_key(context_t* ctx, int lineno, normkey_t* norm)
{
    const char* p = norm->str;
    const char* q = p + strlen(p);
    if (*p != '\'') bad_key(ctx, lineno);
    p++;

    if (! (p < q && q[-1] == '\'')) bad_key(ctx, lineno);
    q--;

    for ( ; p < q; p++) {
	if (*p == '\'' || *p == '\n') bad_key(ctx, lineno);
    }
}

static void normalize_key(context_t* ctx, token_t key, normkey_t* norm)
{
    char* p = norm->str;
    char* q = norm->str + sizeof(norm->str);
    char* s = key.ptr;
    char* t = key.ptr + key.len;
    int lineno = key.lineno;
    int escape = 0;

    *p++ = '\'';
    if (0 == strncmp(s, "'''", 3)) {
	s += 3;
	const char* end = strstr(s, "'''");
	if (end + 3 >= t) bad_key(ctx, lineno);
	while (p < q && s < end) *p++ = *s++;

    } else if (s[0] == '\'') {
	s++;
	const char* end = strchr(s, '\'');
	if (end + 1 >= t) bad_key(ctx, lineno);
	while (p < q && s < end) *p++ = *s++;

    } else if (0 == strncmp(s, "\"\"\"", 3)) {
	s += 3;
	for ( ;p < q && s < t; *p++ = *s++) {
	    if (escape) { escape = 0; continue; }
	    if (*p == '\\') { escape = 1; continue; }
	    if (0 == strncmp(s, "\"\"\"", 3)) {
		if (s + 3 >= t) bad_key(ctx, lineno);
		break;
	    }
	}

    } else if (s[0] == '"') {
	s++;
	int escape = 0;
	for ( ;p < q && s < t; *p++ = *s++) {
	    if (escape) { escape = 0; continue; }
	    if (*p == '\\') { escape = 1; continue; }
	    if (*s == '"' && s + 1 >= t) bad_key(ctx, lineno);
	}
	if (escape) bad_key(ctx, lineno);

    } else {
	while (p < q && s < t) *p++ = *s++;
    }

    if (escape)  bad_key(ctx, lineno);
    if (p + 1 >= q)  bad_key(ctx, lineno);
    
    *p++ = '\'';
    *p++ = 0;
    check_key(ctx, key.lineno, norm);
}

static toml_keyval_t* create_keyval_in_table(toml_table_t* tab, normkey_t* key)
{
    /* find key in tab */
    int i;
    toml_keyval_t* dest = 0;
    for (i = 0; i < tab->nkval && !dest; i++) {
	toml_keyval_t* xxx = tab->kval[i];
	if (0 == strcmp(xxx->key, key->str))
	    dest = xxx;
    }
    if (!dest) {
	/* not found, make a new entry */
	int n = tab->nkval;
	toml_keyval_t** base;
	if (0 == (base = realloc(tab->kval, (n+1) * sizeof(*base))))
	    return 0;
	tab->kval = base;
	
	if (0 == (base[n] = calloc(1, sizeof(*base[n]))))
	    return 0;

	dest = base[n];
	tab->nkval++;
    }

    /* set the key */
    if (dest->key == 0) {
	if (0 == (dest->key = strdup(key->str)))
	    return 0;
    }

    return dest;
}


static toml_table_t* create_keytable_in_table(toml_table_t* tab, normkey_t* key)
{
    /* find key in tab */
    int i;
    toml_table_t* dest = 0;
    for (i = 0; i < tab->ntab && !dest; i++) {
	toml_table_t* xxx = tab->tab[i];
	if (0 == strcmp(xxx->key, key->str)) 
	    dest = xxx;
    }
    if (!dest) {
	/* not found, make a new entry */
	int n = tab->ntab;
	toml_table_t** base;
	if (0 == (base = realloc(tab->tab, (n+1) * sizeof(*base))))
	    return 0;
	tab->tab = base;
	
	if (0 == (base[n] = calloc(1, sizeof(*base[n]))))
	    return 0;

	dest = base[n];
	tab->ntab++;
    }

    /* set the key */
    if (dest->key == 0) {
	if (0 == (dest->key = strdup(key->str)))
	    return 0;
    }

    return dest;
}

static void parse_keyval(context_t* ctx, toml_table_t* tab);

static void parse_table(context_t* ctx, toml_table_t* tab)
{
    if (ctx->tok.tok != LBRACE) internal_error(ctx, FLINE);

    /* eat the { */
    next_token(ctx, 1);

    /* skip new lines */
    while (ctx->tok.tok == NEWLINE) next_token(ctx, 1);

    while (ctx->tok.tok != RBRACE) {
	switch (ctx->tok.tok) {
	case STRING:
	case MLSTRING:
	    parse_keyval(ctx, tab);
	    break;

	default:
	    syntax_error(ctx, ctx->tok.lineno, "syntax error");
	}

	/* skip new lines */
	while (ctx->tok.tok == NEWLINE) next_token(ctx, 1);

	/* There can only be three valid cases here:
	 * 1. COMMA RBRACE
	 * 2. COMMA non-RBRACE
	 * 3. RBRACE
	 */
	if (ctx->tok.tok == COMMA) {
	    /* case 1 and case 2 */
	    next_token(ctx, 1);
	    while (ctx->tok.tok == NEWLINE) next_token(ctx, 1);
	}
	else if (ctx->tok.tok == RBRACE) {
	    /* case 3 */
	    ;
	} else {
	    syntax_error(ctx, ctx->tok.lineno, "syntax error");
	}
    }

    /* eat the } */
    next_token(ctx, 1);
}



static toml_array_t* create_keyarray_in_table(toml_table_t* tab, normkey_t* key)
{
    /* find key in tab */
    int i;
    toml_array_t* dest = 0;
    for (i = 0; i < tab->narr && !dest; i++) {
	toml_array_t* xxx = tab->arr[i];
	if (0 == strcmp(xxx->key, key->str))
	    dest = xxx;
    }
    if (!dest) {
	/* not found, make a new entry */
	int n = tab->narr;
	toml_array_t** base;
	if (0 == (base = realloc(tab->arr, (n+1) * sizeof(*base))))
	    return 0;
	tab->arr = base;
	
	if (0 == (base[n] = calloc(1, sizeof(*base[n]))))
	    return 0;

	dest = base[n];
	tab->narr++;
    }

    /* set the key */
    if (dest->key == 0) {
	if (0 == (dest->key = strdup(key->str)))
	    return 0;
    }

    return dest;
}

static void parse_array(context_t* ctx, toml_array_t* arr)
{
    if (ctx->tok.tok != LBRACKET) internal_error(ctx, FLINE);

    /* eat the [ */
    next_token(ctx, 1);

    /* skip new lines */
    while (ctx->tok.tok == NEWLINE) next_token(ctx, 1);

    while (ctx->tok.tok != RBRACKET) {
	switch (ctx->tok.tok) {
	case STRING:
	case MLSTRING:
	    {
		char* val = ctx->tok.ptr;
		int   vlen = ctx->tok.len;
		
		if (arr->typ == 0) arr->typ = 'v';
		if (arr->typ != 'v') {
		    syntax_error(ctx, ctx->tok.lineno,
				 "a string array can only contain strings");
		}
		
		char** tmp = realloc(arr->val, (arr->nelem+1) * sizeof(*tmp));
		if (!tmp) outofmemory(ctx, FLINE);
		arr->val = tmp;
		
		if (0 == (arr->val[arr->nelem] = strndup(val, vlen)))
		    outofmemory(ctx, FLINE);
		arr->nelem++;

		next_token(ctx, 1);
		break;
	    }

	case LBRACKET:
	    { /* [ [array], [array] ... ] */
		noimpl(ctx, "array in array");
	    }

	case LBRACE:
	    { /* [ {table}, {table} ... ] */
		noimpl(ctx, "table in array");
	    }
	    
	default:
	    syntax_error(ctx, ctx->tok.lineno, "syntax error");
	}

	/* skip new lines */
	while (ctx->tok.tok == NEWLINE) next_token(ctx, 1);

	/* There can only be three valid cases here:
	 * 1. COMMA RBRACKET
	 * 2. COMMA non-RBRACKET
	 * 3. RBRACKET
	 */
	if (ctx->tok.tok == COMMA) {
	    /* case 1 and case 2 */
	    next_token(ctx, 1);
	    while (ctx->tok.tok == NEWLINE) next_token(ctx, 1);
	}
	else if (ctx->tok.tok == RBRACKET) {
	    /* case 3 */
	    ;
	} else {
	    syntax_error(ctx, ctx->tok.lineno, "syntax error");
	}
    }

    /* eat the ] */
    next_token(ctx, 1);
}

    

/* handle lines like these:
    key = "value"
    key = [ array ]
    key = { table }
*/
static void parse_keyval(context_t* ctx, toml_table_t* tab)
{
    if (ctx->tok.tok != STRING)
	internal_error(ctx, FLINE);

    normkey_t key;
    normalize_key(ctx, ctx->tok, &key);

    if (next_token(ctx, 1) != EQUAL) 
	syntax_error(ctx, ctx->tok.lineno, "missing =");

    next_token(ctx, 0 /* dot is not special */ );

    switch (ctx->tok.tok) {
    case STRING:
    case MLSTRING:
	{ /* key = "value" */
	    toml_keyval_t* keyval = create_keyval_in_table(tab, &key);
	    if (!keyval) outofmemory(ctx, FLINE);

	    token_t val = ctx->tok;
	    xfree(keyval->val);
	    keyval->val = strndup(val.ptr, val.len);
	    if (! keyval->val) outofmemory(ctx, FLINE);

	    next_token(ctx, 1);	/* eat the STRING */
	    
	    if (ctx->tok.tok != NEWLINE) 
		syntax_error(ctx, ctx->tok.lineno, "extra chars after value");
	    next_token(ctx, 1);	/* eat the NEWLINE */
	    
	    return;
	}

    case LBRACKET:
	{ /* key = [ array ] */
	    toml_array_t* arr = create_keyarray_in_table(tab, &key);
	    if (!arr) outofmemory(ctx, FLINE);

	    parse_array(ctx, arr);

	    if (ctx->tok.tok != NEWLINE)
		syntax_error(ctx, ctx->tok.lineno, "extra chars after array");
	    next_token(ctx, 1);	/* eat the NEWLINE */
	    return;
	}

    case LBRACE:
	{ /* key = { table } */
	    toml_table_t* nxttab = create_keytable_in_table(tab, &key);
	    if (!nxttab) outofmemory(ctx, FLINE);

	    parse_table(ctx, nxttab);
	    
	    if (ctx->tok.tok != NEWLINE)
		syntax_error(ctx, ctx->tok.lineno, "extra chars after table");
	    next_token(ctx, 1);	/* eat the NEWLINE */
	    return;
	}

    default:
	syntax_error(ctx, ctx->tok.lineno, "syntax error");
    }
}

/* handle lines like these:
   [ x.y.z ]
*/

typedef struct tabpath_t tabpath_t;
struct tabpath_t {
    int     cnt;
    token_t key[10];
};

/* at [x.y.z] or [[x.y.z]]
 * Scan forward and fill tabpath until it enters ] or ]]
 */
static void fill_tabpath(context_t* ctx, tabpath_t* tabpath)
{
    int leftside = ctx->tok.tok;
    int rightside = 0;
    int lineno = ctx->tok.lineno;
    
    if (! (leftside == LBRACKET || leftside == LBRACKET2))
	internal_error(ctx, FLINE);

    next_token(ctx, 1);
    tabpath->cnt = 0;
    for (;;) {
	if (tabpath->cnt >= 10)
	    syntax_error(ctx, lineno, "table path is too deep; max allowed is 10.");

	token_t key = ctx->tok;
	if (key.tok != STRING)
	    syntax_error(ctx, lineno, "invalid or missing key");

	tabpath->key[tabpath->cnt++] = key;

	next_token(ctx, 1);

	if (ctx->tok.tok == RBRACKET || ctx->tok.tok == RBRACKET2) {
	    rightside = ctx->tok.tok;
	    break;
	}

	if (ctx->tok.tok != DOT)
	    syntax_error(ctx, lineno, "invalid key");

	next_token(ctx, 1);
    }

    /* make sure they matched */
    if (! ((leftside == LBRACKET && rightside == RBRACKET)
	   || (leftside == LBRACKET2 && rightside == RBRACKET2))) {
	syntax_error(ctx, lineno, "error near ]");
    }
	
    next_token(ctx, 1);
    if (ctx->tok.tok != NEWLINE)
	syntax_error(ctx, lineno, "extra chars after ]");
}
    
/* Walk tabpath from the root, and create new tables on the way.
 * Sets ctx->curtab to the final table.
 */
static void walk_tabpath(context_t* ctx, tabpath_t* tabpath)
{
    /* start from root */
    toml_table_t* curtab = ctx->root;
    int i;

    for (i = 0; i < tabpath->cnt; i++) {
	const char* key = tabpath->key[i].ptr;
	int klen = tabpath->key[i].len;

	/* lookup key in curtab */
	toml_table_t* nexttab = 0;
	int t;
	for (t = 0; t < curtab->ntab && !nexttab; t++) {
	    if (0 == strncmp(curtab->tab[t]->key, key, klen) &&
		0 == curtab->tab[t]->key[klen]) 
		nexttab = curtab->tab[t];
	}

	/* if not found, make a new tab */
	if (! nexttab) {
	    int n = curtab->ntab;
	    toml_table_t** base = realloc(curtab->tab, (n+1) * sizeof(*base));
	    if (0 == base) outofmemory(ctx, FLINE);
	    curtab->tab = base;

	    if (0 == (base[n] = calloc(1, sizeof(*base[n]))))
		outofmemory(ctx, FLINE);

	    if (0 == (base[n]->key = strndup(key, klen)))
		outofmemory(ctx, FLINE);

	    nexttab = curtab->tab[curtab->ntab++];
	}

	/* switch to next tab */
	curtab = nexttab;
    }

    /* save it */
    ctx->curtab = curtab;
}

    
/* handle [x.y.z] */
static void parse_select_table(context_t* ctx)
{
    if (ctx->tok.tok != LBRACKET)
	internal_error(ctx, FLINE);

    tabpath_t tpath;
    fill_tabpath(ctx, &tpath);
    walk_tabpath(ctx, &tpath);
}



/* handle [[x.y.z]] */
static void parse_array_of_tables(context_t* ctx)
{
    if (ctx->tok.tok != LBRACKET2)
	internal_error(ctx, FLINE);

    tabpath_t tpath;
    fill_tabpath(ctx, &tpath);

    /* For [[x.y.z]], the table is [x.y], and z is 
     * is an array of tables in x.y
     * Remove z from tpath. 
     */
    token_t z = tpath.key[tpath.cnt-1];
    tpath.cnt--;
    
    /* Walk down [x.y] */
    walk_tabpath(ctx, &tpath);

    normkey_t key;
    normalize_key(ctx, z, &key);

    toml_array_t* arr = create_keyarray_in_table(ctx->curtab, &key);
    if (arr->typ == 0) arr->typ = 't';
    if (arr->typ != 't') syntax_error(ctx, z.lineno, "array mismatch");

    toml_table_t* dest = 0;
    {
	int n = arr->nelem;
	toml_table_t** base = realloc(arr->tab, (n+1) * sizeof(*base));
	if (0 == base) outofmemory(ctx, FLINE);
	arr->tab = base;

	if (0 == (base[n] = calloc(1, sizeof(*base[n]))))
	    outofmemory(ctx, FLINE);

	if (0 == (base[n]->key = strdup("__anon__")))
	    outofmemory(ctx, FLINE);

	dest = arr->tab[arr->nelem++];
    }

    ctx->curtab = dest;
}


toml_table_t* toml_parse(char* conf,
			 char* errbuf,
			 int errbufsz)
{
    context_t ctx_;
    context_t* ctx = &ctx_;

    if (errbufsz <= 0) errbufsz = 0;
    if (errbufsz > 0)  errbuf[0] = 0;

    memset(ctx, 0, sizeof(*ctx));
    ctx->start = conf;
    ctx->stop = ctx->start + strlen(conf);
    ctx->errbuf = errbuf;
    ctx->errbufsz = errbufsz;

    // start with an artificial newline of length 0
    ctx->tok.tok = NEWLINE; 
    ctx->tok.lineno = 1;
    ctx->tok.ptr = conf;
    ctx->tok.len = 0;

    // make a root table
    if (0 == (ctx->root = calloc(1, sizeof(*ctx->root)))) {
	outofmemory(ctx, FLINE);
	return 0;
    }
    
    ctx->curtab = ctx->root;

    if (0 == setjmp(ctx->jmp)) {
	token_t tok;

	for (tok = ctx->tok; ! tok.eof ; tok = ctx->tok) {
	    switch (tok.tok) {

	    case NEWLINE:
		next_token(ctx, 1);
		break;

	    case STRING:
		parse_keyval(ctx, ctx->curtab);
		break;

	    case LBRACKET:  /* [ x.y.z ] */
		parse_select_table(ctx);
		break;

	    case LBRACKET2: /* [[ x.y.z ]] */
		parse_array_of_tables(ctx);
		break;
		
	    default:
		snprintf(ctx->errbuf, ctx->errbufsz, "line %d: syntax error", tok.lineno);
		longjmp(ctx->jmp, 1);
	    }
	}

	/* success */
	return ctx->root;
    }

    /* bailed from a long_jmp */
    toml_free(ctx->root);
    return 0;
}


toml_table_t* toml_parse_file(FILE* fp,
			      char* errbuf,
			      int errbufsz)
{
    int bufsz = 0;
    char* buf = 0;
    int off = 0;

    /* read from fp into buf */
    while (! feof(fp)) {
	bufsz += 100;
	
	/* Allocate 1 extra byte because we will tag on a NUL */
	char* x = realloc(buf, bufsz + 1);
	if (!x) {
	    snprintf(errbuf, errbufsz, "out of memory");
	    return 0;
	}
	buf = x;

	errno = 0;
	int n = fread(buf + off, 1, bufsz - off, fp);
	if (ferror(fp)) {
	    snprintf(errbuf, errbufsz, "%s",
		     errno ? strerror(errno) : "Error reading file");
	    free(buf);
	    return 0;
	}
	off += n;
    }

    /* tag on a NUL to cap the string */
    buf[off] = 0; /* we accounted for this byte in the realloc() above. */

    /* parse it, cleanup and finish */
    toml_table_t* ret = toml_parse(buf, errbuf, errbufsz);
    free(buf);
    return ret;
}


static void xfree_kval(toml_keyval_t* p)
{
    xfree(p->key);
    xfree(p->val);
    xfree(p);
}

static void xfree_tab(toml_table_t* p);

static void xfree_arr(toml_array_t* p)
{
    if (!p) return;

    xfree(p->key);

    int i;
    for (i = 0; i < p->nelem; i++) {
	switch (p->typ) {
	case 'v': xfree(p->val[i]); break;
	case 'a': xfree_arr(p->arr[i]); break;
	case 't': xfree_tab(p->tab[i]); break;
	}
    }
    switch (p->typ) {
    case 'v': xfree(p->val); break;
    case 'a': xfree(p->arr); break;
    case 't': xfree(p->tab); break;
    }
    
    xfree(p);
}


static void xfree_tab(toml_table_t* p)
{
    int i;
    
    if (!p) return;
    
    xfree(p->key);
    
    for (i = 0; i < p->nkval; i++) xfree_kval(p->kval[i]);
    xfree(p->kval);

    for (i = 0; i < p->narr; i++) xfree_arr(p->arr[i]);
    xfree(p->arr);

    for (i = 0; i < p->ntab; i++) xfree_tab(p->tab[i]);
    xfree(p->tab);

    xfree(p);
}


void toml_free(toml_table_t* tab)
{
    xfree_tab(tab);
}


static tokentype_t ret_token(context_t* ctx, tokentype_t tok, int lineno, char* ptr, int len)
{
    token_t t;
    t.tok = tok;
    t.lineno = lineno;
    t.ptr = ptr;
    t.len = len;
    t.eof = 0;
    ctx->tok = t;
    return tok;
}

static tokentype_t ret_eof(context_t* ctx, int lineno)
{
    ret_token(ctx, NEWLINE, lineno, ctx->stop, 0);
    ctx->tok.eof = 1;
    return ctx->tok.tok;
}
    

static tokentype_t scan_string(context_t* ctx, char* p, int lineno, int dotisspecial)
{
    char* orig = p;
    if (0 == strncmp(p, "'''", 3)) {
	p = strstr(p + 3, "'''");
	if (0 == p)
	    syntax_error(ctx, lineno, "unterminated triple-s-quote");

	return ret_token(ctx, MLSTRING, lineno, orig, p + 3 - orig);
    }

    if (0 == strncmp(p, "\"\"\"", 3)) {
	int escape = 0;
	int count = 0;
	for (p += 3; *p && count < 3; p++) {
	    if (escape)     { escape = 0; continue; }
	    if (*p == '\\') { escape = 1; continue; }
	    if (*p == '"')  { count++; continue; }
	}
	if (count != 3)
	    syntax_error(ctx, lineno, "unterminated triple-quote");

	return ret_token(ctx, MLSTRING, lineno, orig, p - orig);
    }

    if ('\'' == *p) {
	for (p++; *p && *p != '\n' && *p != '\''; p++);
	if (*p != '\'')
	    syntax_error(ctx, lineno, "unterminated s-quote");

	return ret_token(ctx, STRING, lineno, orig, p + 1 - orig);
    }

    if ('\"' == *p) {
	int escape = 0;
	for (p++; *p && *p; p++) {
	    if (escape)     { escape = 0; continue; }
	    if (*p == '\\') { escape = 1; continue; }
	    if (*p == '"')  { break; }
	    if (*p == '\n') { break; }
	}
	if (*p != '"') 
	    syntax_error(ctx, lineno, "unterminated quote");

	return ret_token(ctx, STRING, lineno, orig, p + 1 - orig);
    }

    for ( ; *p && *p != '\n'; p++) {
	int ch = *p;
	if (ch == '.' && dotisspecial) break;
	if ('A' <= ch && ch <= 'Z') continue;
	if ('a' <= ch && ch <= 'z') continue;
	if ('0' <= ch && ch <= '9') continue;
	if (strchr("+-_.", ch)) continue;
	break;
    }

    return ret_token(ctx, STRING, lineno, orig, p - orig);
}


static tokentype_t next_token(context_t* ctx, int dotisspecial)
{
    int   lineno = ctx->tok.lineno;
    char* p = ctx->tok.ptr;
    int i;

    /* eat this tok */
    for (i = 0; i < ctx->tok.len; i++) {
	if (*p++ == '\n')
	    lineno++;
    }

    /* make next tok */
    while (p < ctx->stop) {
	/* skip comment. stop just before the \n. */
	if (*p == '#') {
	    for (p++; p < ctx->stop && *p != '\n'; p++);
	    continue;
	}

	if (dotisspecial && *p == '.')
	    return ret_token(ctx, DOT, lineno, p, 1);
	
	switch (*p) {
	case ',': return ret_token(ctx, COMMA, lineno, p, 1);
	case '=': return ret_token(ctx, EQUAL, lineno, p, 1);
	case '{': return ret_token(ctx, LBRACE, lineno, p, 1);
	case '}': return ret_token(ctx, RBRACE, lineno, p, 1);
	case '[': return ((p+1 < ctx->stop && p[1] == '[') 
			  ? ret_token(ctx, LBRACKET2, lineno, p, 2)
			  : ret_token(ctx, LBRACKET, lineno, p, 1));
	case ']': return ((p+1 < ctx->stop && p[1] == ']') 
			  ? ret_token(ctx, RBRACKET2, lineno, p, 2)
			  : ret_token(ctx, RBRACKET, lineno, p, 1));
	case '\n': return ret_token(ctx, NEWLINE, lineno, p, 1);
	case '\r': case ' ': case '\t':
	    /* ignore white spaces */
	    p++;
	    continue;
	}

	return scan_string(ctx, p, lineno, dotisspecial);
    }

    return ret_eof(ctx, lineno);
}

static int outofmemory(context_t* ctx, const char* fline)
{
    snprintf(ctx->errbuf, ctx->errbufsz, "ERROR: out of memory (%s)", fline);
    return -1;
}

const char* toml_key_in(toml_table_t* tab, int keyidx)
{
    if (keyidx < tab->nkval) return tab->kval[keyidx]->key;
    
    keyidx -= tab->nkval;
    if (keyidx < tab->narr)  return tab->arr[keyidx]->key;
    
    keyidx -= tab->narr;
    if (keyidx < tab->ntab)  return tab->tab[keyidx]->key;

    return 0;
}


const char* toml_raw_in(toml_table_t* tab, const char* key)
{
    int i;
    for (i = 0; i < tab->nkval; i++) {
	if (0 == strcmp(key, tab->kval[i]->key))
	    return tab->kval[i]->val;
    }
    return 0;
}

toml_array_t* toml_array_in(toml_table_t* tab, const char* key)
{
    int i;
    for (i = 0; i < tab->narr; i++) {
	if (0 == strcmp(key, tab->arr[i]->key))
	    return tab->arr[i];
    }
    return 0;
}


toml_table_t* toml_table_in(toml_table_t* tab, const char* key)
{
    int i;
    for (i = 0; i < tab->ntab; i++) {
	if (0 == strcmp(key, tab->tab[i]->key))
	    return tab->tab[i];
    }
    return 0;
}

const char* toml_raw_at(toml_array_t* arr, int idx)
{
    if (arr->typ != 'v')
	return 0;
    if (! (0 <= idx && idx < arr->nelem))
	return 0;
    return arr->val[idx];
}

char toml_array_typ(toml_array_t* arr)
{
    return arr->typ;
}



toml_array_t* toml_array_at(toml_array_t* arr, int idx)
{
    if (arr->typ != 'a')
	return 0;
    if (! (0 <= idx && idx < arr->nelem))
	return 0;
    return arr->arr[idx];
}

toml_table_t* toml_table_at(toml_array_t* arr, int idx)
{
    if (arr->typ != 't')
	return 0;
    if (! (0 <= idx && idx < arr->nelem))
	return 0;
    return arr->tab[idx];
}


int toml_raw2bool(const char* src, int* ret)
{
    if (!src) return -1;
    
    if (0 == strcmp(src, "true")) {
	*ret = 1;
	return 0;
    }
    if (0 == strcmp(src, "false")) {
	*ret = 0;
	return 0;
    }
    return -1;
}


int toml_raw2int(const char* src, int64_t* ret)
{
    if (!src) return -1;
    
    char buf[100];
    char* p = buf;
    const char* s = src;
    
    if (*s == '+')
	*p++ = *s++;
    else if (*s == '-')
	*p++ = *s++;

    /* if 0, no other digits after it.*/
    if (s[0] == '0' && s[1]) return -1;

    while (p - buf < 99) {
	int ch = *s++;
	if ('0' <= ch && ch <= '9')
	    *p++ = ch;
	else if (ch == '_')
	    ;
	else
	    return -1;
    }
    *p++ = 0;
    
    if (p - buf >= 100)
	return -1;

    errno = 0;
    *ret = strtoll(buf, 0, 0);
    return errno ? -1 : 0;
}


int toml_raw2string(const char* src, char** ret)
{
    if (!src) return -1;
    
    char* buf = strdup(src);
    if (!buf) return -1;

    char* p = buf;
    if (0 == strncmp(src, "'''", 3)) {
	const char* s = src + 3;

	/* skip first new line right after ''' */
	if (*s == '\n')
	    s++;
	else if (s[0] == '\r' && s[1] == '\n')
	    s += 2;

	for (;;) {
	    if (s[0] == '\\') {
		if (s[1] == '\n' || (s[1] == '\r' && s[2] == '\n')) {
		    /* line ending backslash */
		    s++;		/* skip this backslash */
		    s += strspn(s, " \t\r\n"); /* skip till next non-whitespace */
		    continue;
		}
	    }
	    if (0 == strncmp(s, "'''", 3)) break;
	    *p++ = *s++;
	}
	
	*p++ = 0;
	*ret = buf;
	return 0;
    }

    if (*src == '\'') {
	const char* s = src + 1;
	while (*s != '\'') { *p++ = *s++; }
	*p++ = 0;
	*ret = buf;
	return 0;
    }

    if (0 == strncmp(src, "\"\"\"", 3)) {
	const char* s = src + 3;

	/* skip first new line right after """ */
	if (*s == '\n')
	    s++;
	else if (s[0] == '\r' && s[1] == '\n')
	    s++, s++;

	for (;;) {
	    if (s[0] == '\\') {
		if (s[1] == '\n' || (s[1] == '\r' && s[2] == '\n')) {
		    /* line ending backslash */
		    s++;		/* skip this backslash */
		    s += strspn(s, " \t\r\n"); /* skip till next non-whitespace */
		    continue;
		}
		switch (s[1]) {
		case 'b': *p++ = '\b'; s += 2; break;
		case 't': *p++ = '\t'; s += 2; break;
		case 'n': *p++ = '\n'; s += 2; break;
		case 'f': *p++ = '\f'; s += 2; break;
		case 'r': *p++ = '\r'; s += 2; break;
		case '"': *p++ = '"'; s += 2; break;
		case '\'': *p++ = '\''; s += 2; break;
		case '\\': *p++ = '\\'; s += 2; break;
		default:
		    *p++ = *s++; break;
		}		    
		continue;
	    }
	    if (0 == strncmp(s, "\"\"\"", 3)) break;
	    *p++ = *s++;
	}
	*p++ = 0;
	*ret = buf;
	return 0;
    }
	
    if (*src == '"') {
	const char* s = src + 1;
	for (;;) {
	    if (*s == '\\') {
		switch (s[1]) {
		case 'b': *p++ = '\b'; s += 2; break;
		case 't': *p++ = '\t'; s += 2; break;
		case 'n': *p++ = '\n'; s += 2; break;
		case 'f': *p++ = '\f'; s += 2; break;
		case 'r': *p++ = '\r'; s += 2; break;
		case '"': *p++ = '"'; s += 2; break;
		case '\'': *p++ = '\''; s += 2; break;
		case '\\': *p++ = '\\'; s += 2; break;
		default:
		    *p++ = *s++; break;
		}		    
		continue;
	    }
	    if (*s == '"') break;
	    *p++ = *s++;
	}
	*p++ = 0;
	*ret = buf;
	return 0;
    }

    *ret = buf;
    return 0;
}
