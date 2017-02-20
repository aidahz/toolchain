#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include "toml.h"


int toml_utf8_to_ucs(const char* orig, int len, int64_t* ret)
{
    const unsigned char* buf = (const unsigned char*) orig;
    unsigned i = *buf++;
    int64_t v;
    
    /* 0x00000000 - 0x0000007F:
       0xxxxxxx
    */
    if (0 == (i >> 7)) {
	if (len < 1) return -1;
	v = i;
	return *ret = v, 1;
    }
    /* 0x00000080 - 0x000007FF:
       110xxxxx 10xxxxxx
    */
    if (0x6 == (i >> 5)) {
	if (len < 2) return -1;
	v = i & 0x1f;
	i = *(++buf);
	if (0x2 != (i >> 6)) return -1;
	v = (v << 6) | (i & 0x3f);
	return *ret = v, (const char*) buf - orig;
    }

    /* 0x00000800 - 0x0000FFFF:
       1110xxxx 10xxxxxx 10xxxxxx
    */
    if (0xE == (i >> 4)) {
	if (len < 3) return -1;
	v = i & 0x0F;
	for (int j = 0; j < 2; j++) {
	    i = *(++buf);
	    if (0x2 != (i >> 6)) return -1;
	    v = (v << 6) | (i & 0x3f);
	}
	return *ret = v, (const char*) buf - orig;
    }

    /* 0x00010000 - 0x001FFFFF:
       11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    */
    if (0x1E == (i >> 3)) {
	if (len < 4) return -1;
	v = i & 0x07;
	for (int j = 0; j < 3; j++) {
	    i = *(++buf);
	    if (0x2 != (i >> 6)) return -1;
	    v = (v << 6) | (i & 0x3f);
	}
	return *ret = v, (const char*) buf - orig;
    }
    
    /* 0x00200000 - 0x03FFFFFF:
       111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    */
    if (0x3E == (i >> 2)) {
	if (len < 5) return -1;
	v = i & 0x03;
	for (int j = 0; j < 4; j++) {
	    i = *(++buf);
	    if (0x2 != (i >> 6)) return -1;
	    v = (v << 6) | (i & 0x3f);
	}
	return *ret = v, (const char*) buf - orig;
    }

    /* 0x04000000 - 0x7FFFFFFF:
       1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    */
    if (0x7e == (i >> 1)) {
	if (len < 6) return -1;
	v = i & 0x01;
	for (int j = 0; j < 5; j++) {
	    i = *(++buf);
	    if (0x2 != (i >> 6)) return -1;
	    v = (v << 6) | (i & 0x3f);
	}
	return *ret = v, (const char*) buf - orig;
    }
    return -1;
}


int toml_ucs_to_utf8(int64_t code, char buf[6])
{
    /* http://stackoverflow.com/questions/6240055/manually-converting-unicode-codepoints-into-utf-8-and-utf-16 */
    /* The UCS code values 0xd800–0xdfff (UTF-16 surrogates) as well
     * as 0xfffe and 0xffff (UCS noncharacters) should not appear in
     * conforming UTF-8 streams.
     */
    if (0xd800 <= code && code <= 0xdfff) return -1;
    if (0xfffe <= code && code <= 0xffff) return -1;

    /* 0x00000000 - 0x0000007F:
       0xxxxxxx
    */
    if (code < 0) return -1;
    if (code <= 0x7F) {
	buf[0] = (unsigned char) code;
	return 1;
    }

    /* 0x00000080 - 0x000007FF:
       110xxxxx 10xxxxxx
    */
    if (code <= 0x000007FF) {
	buf[0] = 0xc0 | (code >> 6);
	buf[1] = 0x80 | (code & 0x3f);
	return 2;
    }

    /* 0x00000800 - 0x0000FFFF:
       1110xxxx 10xxxxxx 10xxxxxx
    */
    if (code <= 0x0000FFFF) {
	buf[0] = 0xe0 | (code >> 12);
	buf[1] = 0x80 | ((code >> 6) & 0x3f);
	buf[2] = 0x80 | (code & 0x3f);
	return 3;
    }

    /* 0x00010000 - 0x001FFFFF:
       11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    */
    if (code <= 0x001FFFFF) {
	buf[0] = 0xf0 | (code >> 18);
	buf[1] = 0x80 | ((code >> 12) & 0x3f);
	buf[2] = 0x80 | ((code >> 6) & 0x3f);
	buf[3] = 0x80 | (code & 0x3f);
	return 4;
    }

    /* 0x00200000 - 0x03FFFFFF:
       111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    */
    if (code <= 0x03FFFFFF) {
	buf[0] = 0xf8 | (code >> 24);
	buf[1] = 0x80 | ((code >> 18) & 0x3f);
	buf[2] = 0x80 | ((code >> 12) & 0x3f);
	buf[3] = 0x80 | ((code >> 6) & 0x3f);
	buf[4] = 0x80 | (code & 0x3f);
	return 5;
    }
    
    /* 0x04000000 - 0x7FFFFFFF:
       1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    */
    if (code <= 0x7FFFFFFF) {
	buf[0] = 0xfc | (code >> 30);
	buf[1] = 0x80 | ((code >> 24) & 0x3f);
	buf[2] = 0x80 | ((code >> 18) & 0x3f);
	buf[3] = 0x80 | ((code >> 12) & 0x3f);
	buf[4] = 0x80 | ((code >> 6) & 0x3f);
	buf[5] = 0x80 | (code & 0x3f);
	return 5;
    }

    return -1;
}


typedef struct toml_keyval_t toml_keyval_t;
struct toml_keyval_t {
    const char* key;
    const char* val;
};


struct toml_array_t {
    const char* key;
    int kind; /* 'v'alue, 'a'rray, or 't'able */
    int type; /* 'i'nt, 'd'ouble, 'b'ool, 's'tring, 't'ime, 'D'ate, 'T'imestamp */
    
    int nelem;
    char** val;
    toml_array_t** arr;
    toml_table_t** tab;
};
    

struct toml_table_t {
    const char* key;
    int implicit;
    
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

    struct {
	int     top;
	char*   key[10];
	token_t tok[10];
    } tpath;

};

#define STRINGIFY(x) #x
#define TOSTRING(x)  STRINGIFY(x)
#define FLINE __FILE__ ":" TOSTRING(__LINE__)
static int outofmemory(context_t* ctx, const char* fline);
static tokentype_t next_token(context_t* ctx, int dotisspecial);

static int internal_error(context_t* ctx, const char* fline)
{
    snprintf(ctx->errbuf, ctx->errbufsz, "internal error (%s)", fline);
    longjmp(ctx->jmp, 1);
    return -1;
}

static int syntax_error(context_t* ctx, int lineno, const char* msg)
{
    snprintf(ctx->errbuf, ctx->errbufsz, "line %d: %s", lineno, msg);
    longjmp(ctx->jmp, 1);
    return -1;
}

static int bad_key(context_t* ctx, int lineno)
{
    snprintf(ctx->errbuf, ctx->errbufsz, "line %d: bad key", lineno);
    longjmp(ctx->jmp, 1);
    return -1;
}

static int noimpl(context_t* ctx, const char* feature)
{
    snprintf(ctx->errbuf, ctx->errbufsz, "not implemented: %s", feature);
    longjmp(ctx->jmp, 1);
    return -1;
}

static int key_exists(context_t* ctx, token_t keytok)
{
    char buf[100];
    int i;
    for (i = 0; i < keytok.len && i < (int)sizeof(buf) - 1; i++) {
	buf[i] = keytok.ptr[i];
    }
    buf[i] = 0;

    snprintf(ctx->errbuf, ctx->errbufsz,
	     "line %d: key %s exists", keytok.lineno, buf);
    longjmp(ctx->jmp, 1);
    return -1;
}



/* 
 * unescape all special chars. 
 * returns the new string. 
 * if error, returns NULL, and message written to errbuf;
 */
static char* normalize_string(const char* src, int srclen,
			      int kill_line_ending_backslash,
			      char* errbuf, int errbufsz)
{
    char* dst = 0;
    int   max = 0;
    int   off = 0;
    const char* sp = src;
    const char* sq = src + srclen;
    int ch;

    for (;;) {
	if (off >=  max - 10) { /* have some slack for misc stuff */
	    char* x = realloc(dst, max += 100);
	    if (!x) {
		if (dst) free(dst);
		snprintf(errbuf, errbufsz, "out of memory");
		return 0;
	    }
	    dst = x;
	}
	
	if (sp >= sq) break;	/* finished */

	ch = *sp++;
	if (ch != '\\') {
	    dst[off++] = ch;
	    continue;
	}

	/* ch == backslash. we expect the escape char. */
	if (sp >= sq) {
	    snprintf(errbuf, errbufsz, "last backslash is invalid");
	    free(dst);
	    return 0;
	}

	if (kill_line_ending_backslash) {
	    if (*sp == '\n' || (*sp == '\r' && sp[1] == '\n')) {
		sp += strspn(sp, " \t\r\n");
		continue;
	    }
	}
	    
	ch = *sp++;
	switch (ch) {
	case 'u': case 'U':
	    {
		int64_t ucs = 0;
		for (int i = (ch == 'u' ? 4 : 8); i; i--) {
		    if (sp >= sq) {
			snprintf(errbuf, errbufsz, "\\%c expects %d hex chars", ch, (ch == 'u' ? 4 : 8));
			free(dst);
			return 0;
		    }
		    ch = *sp++;
		    int v = ('0' <= ch && ch <= '9')
			? ch - '0'
			: (('A' <= ch && ch <= 'F') ? ch - 'A' + 10 : -1);
		    if (-1 == v) {
			snprintf(errbuf, errbufsz, "invalid hex chars for \\u or \\U");
			free(dst);
			return 0;
		    }
		    ucs = ucs * 16 + v;
		}
		int n = toml_ucs_to_utf8(ucs, &dst[off]);
		if (-1 == n) {
		    snprintf(errbuf, errbufsz, "illegal ucs code in \\u or \\U");
		    free(dst);
		    return 0;
		}
		off += n;
	    }
	    continue;

	case 'b': ch = '\b'; break;
	case 't': ch = '\t'; break;
	case 'n': ch = '\n'; break;
	case 'f': ch = '\f'; break;
	case 'r': ch = '\r'; break;
	case '"':  ch = '"'; break;
	case '\\': ch = '\\'; break;
	default: 
	    snprintf(errbuf, errbufsz, "illegal escape char \\%c", ch);
	    free(dst);
	    return 0;
	}

	dst[off++] = ch;
    }

    dst[off++] = 0;
    return dst;
}


static char* normalize_key(context_t* ctx, token_t strtok)
{
    const char* sp = strtok.ptr;
    const char* sq = strtok.ptr + strtok.len;
    int lineno = strtok.lineno;
    char* ret;
    int ch = *sp;
    char ebuf[80];

    if (ch == '\'' || ch == '\"') {
	if (sp[1] == ch && sp[2] == ch) 
	    sp += 3, sq -= 3;
	else
	    sp++, sq--;

	if (ch == '\'') {
	    if (! (ret = strndup(sp, sq - sp))) outofmemory(ctx, FLINE);
	} else {
	    ret = normalize_string(sp, sq - sp, 0, ebuf, sizeof(ebuf));
	    if (!ret) {
		snprintf(ctx->errbuf, ctx->errbufsz, "line %d: %s", lineno, ebuf);
		longjmp(ctx->jmp, 1);
	    }
	}
	if (strchr(ret, '\n')) {
	    /* newlines are not allowed in keys */
	    free(ret);
	    bad_key(ctx, lineno);
	}
	return ret;
    }
	
    /* for barekey allow only this regex: [A-Za-z0-9_-]+ */
    const char* xp;
    for (xp = sp; xp != sq; xp++) {
	int ch = *xp;
	if ('A' <= ch && ch <= 'Z') continue;
	if ('a' <= ch && ch <= 'z') continue;
	if ('0' <= ch && ch <= '9') continue;
	if (ch == '_' || ch == '-') continue;
	bad_key(ctx, lineno);
    }
    if (! (ret = strndup(sp, sq - sp)))
	outofmemory(ctx, FLINE);
    
    return ret;
}


static int check_key(toml_table_t* tab, const char* key)
{
    int i;
    for (i = 0; i < tab->nkval; i++) {
	if (0 == strcmp(key, tab->kval[i]->key)) return 'v';
    }
    for (i = 0; i < tab->narr; i++) {
	if (0 == strcmp(key, tab->arr[i]->key)) return 'a';
    }
    for (i = 0; i < tab->ntab; i++) {
	if (0 == strcmp(key, tab->tab[i]->key)) return 't';
    }
    return 0;
}

static toml_keyval_t* create_keyval_in_table(context_t* ctx, toml_table_t* tab, token_t keytok)
{
    char* newkey = normalize_key(ctx, keytok);
    if (check_key(tab, newkey)) {
	free(newkey);
	key_exists(ctx, keytok);
    }
    
    int n = tab->nkval;
    toml_keyval_t** base;
    if (0 == (base = realloc(tab->kval, (n+1) * sizeof(*base)))) {
	free(newkey);
	outofmemory(ctx, FLINE);
    }
    tab->kval = base;
    
    if (0 == (base[n] = calloc(1, sizeof(*base[n])))) {
	free(newkey);
	outofmemory(ctx, FLINE);
    }
    
    toml_keyval_t* dest = tab->kval[tab->nkval++];
    dest->key = newkey;
    return dest;
}


static toml_table_t* create_keytable_in_table(context_t* ctx, toml_table_t* tab, token_t keytok)
{
    char* newkey = normalize_key(ctx, keytok);

    if (check_key(tab, newkey)) {
	/* special case: if table exists, but was created implicitly ... */
	int i;
	for (i = 0; i < tab->ntab; i++) {
	    if (0 == strcmp(newkey, tab->tab[i]->key)) break;
	}
	if (i < tab->ntab && tab->tab[i]->implicit) {
	    free(newkey);
	    tab->tab[i]->implicit = 0;
	    return tab->tab[i];
	}
	free(newkey);
	key_exists(ctx, keytok);
    }
    
    int n = tab->ntab;
    toml_table_t** base;
    if (0 == (base = realloc(tab->tab, (n+1) * sizeof(*base)))) {
	free(newkey);
	outofmemory(ctx, FLINE);
    }
    tab->tab = base;
	
    if (0 == (base[n] = calloc(1, sizeof(*base[n])))) {
	free(newkey);
	outofmemory(ctx, FLINE);
    }

    toml_table_t* dest = tab->tab[tab->ntab++];
    dest->key = newkey;
    return dest;
}


static toml_array_t* create_keyarray_in_table(context_t* ctx,
					      toml_table_t* tab,
					      token_t keytok,
					      int skip_if_exist)
{
    char* newkey = normalize_key(ctx, keytok);
    if (check_key(tab, newkey)) {
	if (skip_if_exist) {
	    int i;
	    for (i = 0; i < tab->narr; i++) {
		if (0 == strcmp(newkey, tab->arr[i]->key)) {
		    free(newkey);
		    return tab->arr[i];
		}
	    }
	}
	free(newkey);
	key_exists(ctx, keytok);
    }
    
    int n = tab->narr;
    toml_array_t** base;
    if (0 == (base = realloc(tab->arr, (n+1) * sizeof(*base)))) {
	free(newkey);
	outofmemory(ctx, FLINE);
    }
    tab->arr = base;
	
    if (0 == (base[n] = calloc(1, sizeof(*base[n])))) {
	free(newkey);
	outofmemory(ctx, FLINE);
    }

    toml_array_t* dest = tab->arr[tab->narr++];
    dest->key = newkey;
    return dest;
}

static toml_array_t* create_array_in_array(context_t* ctx,
					   toml_array_t* parent)
{
    int n = parent->nelem;
    toml_array_t** base;
    if (0 == (base = realloc(parent->arr, (n+1) * sizeof(*base)))) {
	outofmemory(ctx, FLINE);
    }
    parent->arr = base;
    
    if (0 == (base[n] = calloc(1, sizeof(*base[n])))) {
	outofmemory(ctx, FLINE);
    }

    return parent->arr[parent->nelem++];
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

static int valtype(const char* val)
{
    toml_timestamp_t ts;
    if (*val == '\'' || *val == '"') return 's';
    if (0 == toml_rtob(val, 0)) return 'b';
    if (0 == toml_rtoi(val, 0)) return 'i';
    if (0 == toml_rtod(val, 0)) return 'd';
    if (0 == toml_rtots(val, &ts)) {
	if (ts.year && ts.hour) return 'T'; /* timestamp */
	if (ts.year) return 'D'; /* date */
	return 't'; /* time */
    }
    return 'u'; /* unknown */
}


static void parse_array(context_t* ctx, toml_array_t* arr)
{
    if (ctx->tok.tok != LBRACKET) internal_error(ctx, FLINE);

    /* eat the [ */
    next_token(ctx, 0);

    /* skip new lines */
    while (ctx->tok.tok == NEWLINE) next_token(ctx, 0);

    while (ctx->tok.tok != RBRACKET) {
	switch (ctx->tok.tok) {
	case STRING:
	case MLSTRING:
	    {
		char* val = ctx->tok.ptr;
		int   vlen = ctx->tok.len;
		
		if (arr->kind == 0) arr->kind = 'v';
		if (arr->kind != 'v') {
		    syntax_error(ctx, ctx->tok.lineno,
				 "a string array can only contain strings");
		}
		
		char** tmp = realloc(arr->val, (arr->nelem+1) * sizeof(*tmp));
		if (!tmp) outofmemory(ctx, FLINE);
		arr->val = tmp;
		
		if (0 == (val = strndup(val, vlen)))
		    outofmemory(ctx, FLINE);

		arr->val[arr->nelem++] = val;

		if (arr->nelem == 1) 
		    arr->type = valtype(arr->val[0]);
		else if (arr->type != valtype(val))
		    syntax_error(ctx, ctx->tok.lineno, "array type mismatch");
		    
		next_token(ctx, 0);
		break;
	    }

	case LBRACKET:
	    { /* [ [array], [array] ... ] */
		if (arr->kind == 0) arr->kind = 'a';
		if (arr->kind != 'a') {
		    syntax_error(ctx, ctx->tok.lineno, "array type mismatch");
		}
		parse_array(ctx, create_array_in_array(ctx, arr));
		break;
	    }

	case LBRACE:
	    { /* [ {table}, {table} ... ] */
		noimpl(ctx, "table in array");
	    }
	    
	default:
	    syntax_error(ctx, ctx->tok.lineno, "syntax error");
	}

	/* skip new lines */
	while (ctx->tok.tok == NEWLINE) next_token(ctx, 0);

	/* There can only be three valid cases here:
	 * 1. COMMA RBRACKET
	 * 2. COMMA non-RBRACKET
	 * 3. RBRACKET
	 */
	if (ctx->tok.tok == COMMA) {
	    /* case 1 and case 2 */
	    next_token(ctx, 0);
	    while (ctx->tok.tok == NEWLINE) next_token(ctx, 0);
	}
	else if (ctx->tok.tok == RBRACKET) {
	    /* case 3 */
	    ;
	} else {
	    syntax_error(ctx, ctx->tok.lineno, "syntax error");
	}
    }

    /* eat the ] */
    next_token(ctx, 0);
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

    token_t key = ctx->tok;

    if (next_token(ctx, 1) != EQUAL) 
	syntax_error(ctx, ctx->tok.lineno, "missing =");

    next_token(ctx, 0 /* dot is not special */ );

    switch (ctx->tok.tok) {
    case STRING:
    case MLSTRING:
	{ /* key = "value" */
	    toml_keyval_t* keyval = create_keyval_in_table(ctx, tab, key);
	    token_t val = ctx->tok;
	    assert(keyval->val == 0);
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
	    toml_array_t* arr = create_keyarray_in_table(ctx, tab, key, 0);
	    parse_array(ctx, arr);

	    if (ctx->tok.tok != NEWLINE)
		syntax_error(ctx, ctx->tok.lineno, "extra chars after array");
	    next_token(ctx, 1);	/* eat the NEWLINE */
	    return;
	}

    case LBRACE:
	{ /* key = { table } */
	    toml_table_t* nxttab = create_keytable_in_table(ctx, tab, key);
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
 * There will be at least one entry on return.
 */
static void fill_tabpath(context_t* ctx)
{
    int lineno = ctx->tok.lineno;
    int i;
    
    /* clear tpath */
    for (i = 0; i < ctx->tpath.top; i++) {
	char** p = &ctx->tpath.key[i];
	if (*p) free(*p);
	*p = 0;
    }
    ctx->tpath.top = 0;
    
    for (;;) {
	if (ctx->tpath.top >= 10)
	    syntax_error(ctx, lineno, "table path is too deep; max allowed is 10.");

	if (ctx->tok.tok != STRING)
	    syntax_error(ctx, lineno, "invalid or missing key");

	ctx->tpath.tok[ctx->tpath.top] = ctx->tok;
	ctx->tpath.key[ctx->tpath.top] = normalize_key(ctx, ctx->tok);
	ctx->tpath.top++;
	
	next_token(ctx, 1);

	if (ctx->tok.tok == RBRACKET) break;

	if (ctx->tok.tok != DOT)
	    syntax_error(ctx, lineno, "invalid key");

	next_token(ctx, 1);
    }

    if (ctx->tpath.top <= 0) {
	syntax_error(ctx, lineno, "empty table selector");
    }
}


/* Walk tabpath from the root, and create new tables on the way.
 * Sets ctx->curtab to the final table.
 */
static void walk_tabpath(context_t* ctx)
{
    /* start from root */
    toml_table_t* curtab = ctx->root;
    int i, j;

    for (i = 0; i < ctx->tpath.top; i++) {
	const char* key = ctx->tpath.key[i];

	toml_table_t* nexttab = 0;
	switch (check_key(curtab, key)) {
	case 't':
	    { /* key exists. look up table in curtab */
		for (j = 0; j < curtab->ntab && !nexttab; j++) {
		    if (0 == strcmp(curtab->tab[j]->key, key))
			nexttab = curtab->tab[j];
		}
		if (!nexttab)
		    key_exists(ctx, ctx->tpath.tok[i]);
	    }
	    break;

	case 'a':
	    { /* key exists. look up array in curtab */
		for (j = 0; j < curtab->narr && !nexttab; j++) {
		    if (0 == strcmp(curtab->arr[j]->key, key)) {
			toml_array_t* arr = curtab->arr[j];
			if (arr->kind != 't') internal_error(ctx, FLINE);
			if (arr->nelem == 0) internal_error(ctx, FLINE);
			nexttab = arr->tab[arr->nelem-1];
		    }
		}
		if (!nexttab) 
		    key_exists(ctx, ctx->tpath.tok[i]);
	    }
	    break;

	case 0:
	    {
		int n = curtab->ntab;
		toml_table_t** base = realloc(curtab->tab, (n+1) * sizeof(*base));
		if (0 == base) outofmemory(ctx, FLINE);
		curtab->tab = base;
		
		if (0 == (base[n] = calloc(1, sizeof(*base[n]))))
		    outofmemory(ctx, FLINE);
		
		if (0 == (base[n]->key = strdup(key)))
		    outofmemory(ctx, FLINE);
		
		nexttab = curtab->tab[curtab->ntab++];
		
		/* tabs created by walk_tabpath are considered implicit */
		nexttab->implicit = 1;
	    }
	    break;

	default:
	    abort();
	}

	/* switch to next tab */
	curtab = nexttab;
    }

    /* save it */
    ctx->curtab = curtab;
}

    
/* handle [x.y.z] or [[x.y.z]]*/
static void parse_select(context_t* ctx)
{
    int count_lbracket = 0;
    if (ctx->tok.tok != LBRACKET) internal_error(ctx, FLINE);
    count_lbracket++;
    next_token(ctx, 1);
    if (ctx->tok.tok == LBRACKET) {
	count_lbracket++;
	next_token(ctx, 1);
    }

    fill_tabpath(ctx);

    /* For [x.y.z] or [[x.y.z]], remove z from tpath. 
     */
    token_t z = ctx->tpath.tok[ctx->tpath.top-1];
    free(ctx->tpath.key[ctx->tpath.top-1]);
    ctx->tpath.top--;
    
    walk_tabpath(ctx);

    if (count_lbracket == 1) {
	/* [x.y.z] -> create z = {} in x.y */
	ctx->curtab = create_keytable_in_table(ctx, ctx->curtab, z);
    } else {
	/* [[x.y.z]] -> create z = [] in x.y.z */
	toml_array_t* arr = create_keyarray_in_table(ctx, ctx->curtab, z, 1);
	if (arr->kind == 0) arr->kind = 't';
	if (arr->kind != 't') syntax_error(ctx, z.lineno, "array mismatch");

	/* add to z=[] */
	toml_table_t* dest;
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

    if (ctx->tok.tok != RBRACKET) syntax_error(ctx, ctx->tok.lineno, "expects ]");
    next_token(ctx, 1); 
    if (count_lbracket == 2) {
	if (ctx->tok.tok != RBRACKET) syntax_error(ctx, ctx->tok.lineno, "expects ]]");
	next_token(ctx, 1);
    }
    if (ctx->tok.tok != NEWLINE) syntax_error(ctx, ctx->tok.lineno, "extra chars after ] or ]]");
}




toml_table_t* toml_parse(char* conf,
			 char* errbuf,
			 int errbufsz)
{
    int i;
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

	    case LBRACKET:  /* [ x.y.z ] or [[ x.y.z ]] */
		parse_select(ctx);
		break;

	    default:
		snprintf(ctx->errbuf, ctx->errbufsz, "line %d: syntax error", tok.lineno);
		longjmp(ctx->jmp, 1);
	    }
	}

	for (i = 0; i < ctx->tpath.top; i++) {
	    xfree(ctx->tpath.key[i]);
	}

	/* success */
	return ctx->root;
    }
    
    for (i = 0; i < ctx->tpath.top; i++) {
	xfree(ctx->tpath.key[i]);
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
	switch (p->kind) {
	case 'v': xfree(p->val[i]); break;
	case 'a': xfree_arr(p->arr[i]); break;
	case 't': xfree_tab(p->tab[i]); break;
	}
    }
    switch (p->kind) {
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
	int hexreq = 0;		/* #hex required */
	int escape = 0;
	int qcnt = 0;		/* count quote */
	for (p += 3; *p && qcnt < 3; p++) {
	    if (escape) {
		escape = 0;
		if (strchr("btnfr\"\\", *p)) continue;
		if (*p == 'u') { hexreq = 4; continue; }
		if (*p == 'U') { hexreq = 8; continue; }
		if (*p == '\n') continue; /* allow for line ending backslash */
		syntax_error(ctx, lineno, "bad escape char");
	    }
	    if (hexreq) {
		hexreq--;
		if (strchr("0123456789ABCDEF", *p)) continue;
		syntax_error(ctx, lineno, "expect hex char");
	    }
	    if (*p == '\\') { escape = 1; continue; }
	    qcnt = (*p == '"') ? qcnt + 1 : 0; 
	}
	if (qcnt != 3)
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
	int hexreq = 0;		/* #hex required */
	int escape = 0;
	for (p++; *p; p++) {
	    if (escape) {
		escape = 0;
		if (strchr("btnfr\"\\", *p)) continue;
		if (*p == 'u') { hexreq = 4; continue; }
		if (*p == 'U') { hexreq = 8; continue; }
		syntax_error(ctx, lineno, "bad escape char");
	    }
	    if (hexreq) {
		hexreq--;
		if (strchr("0123456789ABCDEF", *p)) continue;
		syntax_error(ctx, lineno, "expect hex char");
	    }
	    if (*p == '\\') { escape = 1; continue; }
	    if (*p == '\n') break;
	    if (*p == '"') break;
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
	if (strchr("+-_.:", ch)) continue;
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
	case '[': return ret_token(ctx, LBRACKET, lineno, p, 1);
	case ']': return ret_token(ctx, RBRACKET, lineno, p, 1);
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
    if (arr->kind != 'v')
	return 0;
    if (! (0 <= idx && idx < arr->nelem))
	return 0;
    return arr->val[idx];
}

char toml_array_kind(toml_array_t* arr)
{
    return arr->kind;
}



toml_array_t* toml_array_at(toml_array_t* arr, int idx)
{
    if (arr->kind != 'a')
	return 0;
    if (! (0 <= idx && idx < arr->nelem))
	return 0;
    return arr->arr[idx];
}

toml_table_t* toml_table_at(toml_array_t* arr, int idx)
{
    if (arr->kind != 't')
	return 0;
    if (! (0 <= idx && idx < arr->nelem))
	return 0;
    return arr->tab[idx];
}


int toml_rtots(const char* src_, toml_timestamp_t* ret)
{
    if (! src_) return -1;
    
    const char* p = src_;
    const char* q = src_ + strlen(src_);
    int64_t val;
    int i;
    
    memset(ret, 0, sizeof(*ret));

    /* parse date */
    val = 0;
    if (q - p > 4 && p[4] == '-') {
	for (i = 0; i < 10; i++, p++) {
	    int xx = *p;
	    if (xx == '-') {
		if (i == 4 || i == 7) continue; else return -1;
	    }
	    if (! ('0' <= xx && xx <= '9')) return -1;
	    val = val * 10 + (xx - '0');
	}
	ret->day   = &ret->__buffer.day;
	ret->month = &ret->__buffer.month;
	ret->year  = &ret->__buffer.year;
	
	*ret->day   = val % 100; val /= 100;
	*ret->month = val % 100; val /= 100;
	*ret->year  = val;
	
	if (*p) {
	    if (*p != 'T') return -1;
	    p++;
	}
    }
    if (q == p) return 0;
    if (q - p < 8) return -1;

    /* parse time */
    val = 0;
    for (i = 0; i < 8; i++, p++) {
	int xx = *p;
	if (xx == ':') {
	    if (i == 2 || i == 5) continue; else return -1;
	}
	if (! ('0' <= xx && xx <= '9')) return -1;
	val = val * 10 + (xx - '0');
    }
    ret->second = &ret->__buffer.second;
    ret->minute = &ret->__buffer.minute;
    ret->hour   = &ret->__buffer.hour;
    
    *ret->second = val % 100; val /= 100;
    *ret->minute = val % 100; val /= 100;
    *ret->hour   = val;
    
    /* skip fractional second */
    if (*p == '.') {
	for (p++; '0' <= *p && *p <= '9'; p++);
    }

    if (p == q) return 0;
    
    /* parse and copy Z */
    ret->z = ret->__buffer.z;
    char* z = ret->z;
    if (*p == 'Z') {
	*z++ = *p++;
	*z = 0;
	return (p == q) ? 0 : -1;
    }
    if (*p == '+' || *p == '-') {
	*z++ = *p++;
	if (('0' <= p[0] && p[0] <= '9') && ('0' <= p[1] && p[1] <= '9')) {
	    *z++ = *p++;
	    *z++ = *p++;
	    if (*p == ':') {
		*z++ = *p++;
		if (('0' <= p[0] && p[0] <= '9') && ('0' <= p[1] && p[1] <= '9')) {
		    *z++ = *p++;
		    *z++ = *p++;
		    *z++ = 0;
		}
	    }
	}
    }
    return (p == q) ? 0 : -1;
}


int toml_rtob(const char* src, int* ret_)
{
    if (!src) return -1;
    int dummy;
    int* ret = ret_ ? ret_ : &dummy;
    
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


int toml_rtoi(const char* src, int64_t* ret_)
{
    if (!src) return -1;
    
    char buf[100];
    char* p = buf;
    char* q = p + sizeof(buf);
    const char* s = src;
    int64_t dummy;
    int64_t* ret = ret_ ? ret_ : &dummy;
    
    if (*s == '+')
	*p++ = *s++;
    else if (*s == '-')
	*p++ = *s++;

    /* if 0 ... */
    if ('0' == s[0]) {
	/* ensure no other digits after it */
	if (s[1]) return -1;
	return *ret = 0, 0;
    }

    while (*s && p < q) {
	int ch = *s++;
	if ('0' <= ch && ch <= '9') *p++ = ch;
	else if (ch == '_') ;
	else return -1;
    }
    if (*s || p == q) return -1;
    *p++ = 0;
    
    errno = 0;
    *ret = strtoll(buf, 0, 0);
    return errno ? -1 : 0;
}


int toml_rtod(const char* src, double* ret_)
{
    if (!src) return -1;
    
    char buf[100];
    char* p = buf;
    char* q = p + sizeof(buf);
    const char* s = src;
    double dummy;
    double* ret = ret_ ? ret_ : &dummy;

    /* check for special cases */
    if (s[0] == '+' || s[0] == '-') *p++ = *s++;
    if (s[0] == '.') return -1;	/* no leading zero */

    /* just strip _ and pass to strtod */
    while (*s && p < q) {
	int ch = *s++;
	if (ch == '_') ; else *p++ = ch;
    }
    if (*s || p == q) return -1;

    if (p != buf && p[-1] == '.') 
	return -1; /* no trailing zero */
    
    *p++ = 0;


    char* endp;
    errno = 0;
    *ret = strtod(buf, &endp);
    if (*endp || errno)
	return -1;

    return 0;
}


static char* kill_line_ending_backslash(char* str)
{
    if (!str) return 0;
    
    /* first round: find (backslash, \n) */
    char* p = str;
    while (0 != (p = strstr(p, "\\\n"))) {
	char* q = (p + 1);
	q += strspn(q, " \t\r\n");
	memmove(p, q, strlen(q) + 1);
    }
    /* second round: find (backslash, \r, \n) */
    p = str;
    while (0 != (p = strstr(p, "\\\r\n"))) {
	char* q = (p + 1);
	q += strspn(q, " \t\r\n");
	memmove(p, q, strlen(q) + 1);
    }

    return str;
}


int toml_rtos(const char* src, char** ret)
{
    if (!src) return -1;
    if (*src != '\'' && *src != '"') return -1;

    *ret = 0;
    int srclen = strlen(src);
    if (*src == '\'') {
	if (0 == strncmp(src, "'''", 3)) {
	    const char* sp = src + 3;
	    const char* sq = src + srclen - 3;
	    if (! (sp <= sq && 0 == strcmp(sq, "'''")))
		return -1;
	    
	    /* skip first new line right after ''' */
	    if (*sp == '\n')
		sp++;
	    else if (sp[0] == '\r' && sp[1] == '\n')
		sp += 2;

	    *ret = kill_line_ending_backslash(strndup(sp, sq - sp));
	} else {
	    const char* sp = src + 1;
	    const char* sq = src + srclen - 1;
	    if (! (sp <= sq && *sq == '\''))
		return -1;
	    /* copy from sp to p */
	    *ret = strndup(sp, sq - sp);
	}
	return *ret ? 0 : -1;
    }

    const char* sp;
    const char* sq;
    if (0 == strncmp(src, "\"\"\"", 3)) {
	sp = src + 3;
	sq = src + srclen - 3;
	if (! (sp <= sq && 0 == strcmp(sq, "\"\"\"")))
	    return -1;
	
	/* skip first new line right after """ */
	if (*sp == '\n')
	    sp++;
	else if (sp[0] == '\r' && sp[1] == '\n')
	    sp += 2;
    } else {
	sp = src + 1;
	sq = src + srclen - 1;
	if (! (sp <= sq && *sq == '"'))
	    return -1;
    }

    *ret = normalize_string(sp, sq - sp, 1, 0, 0);
    return *ret ? 0 : -1;
}
