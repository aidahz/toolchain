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

static int bad_string(context_t* ctx, int lineno)
{
    snprintf(ctx->errbuf, ctx->errbufsz, "line %d: bad string", lineno);
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


static char* quote_string(const char* src, int srclen)
{
    char* dst = 0;
    int   max = 0;
    int   off = 0;
    const char* sp = src;
    const char* sq = src + srclen;

    for ( ; sp < sq; sp++){
	if (off >=  max - 10) { /* have some slack for misc stuff */
	    char* x = realloc(dst, max += 100);
	    if (!x) {
		if (dst) free(dst);
		return 0;
	    }
	    dst = x;
	}
	
	int ch = *sp;
	if (strchr("\b\t\n\f\r\"\\", ch)) {
	    dst[off++] = '\\';
	    switch (ch) {
	    case '\b': ch = 'b'; break;
	    case '\t': ch = 't'; break;
	    case '\n': ch = 'n'; break;
	    case '\f': ch = 'f'; break;
	    case '\r': ch = 'r'; break;
	    case '"':  ch = '"'; break;
	    case '\\': ch = '\\'; break;
	    }
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

    const char* end;
    if (0 == strncmp(sp, "'''", 3)) {
	sp += 3;
	end = sq - 3;
    } else if (sp[0] == '\'') {
	sp++;
	end = sq - 1;
    } else if (0 == strncmp(sp, "\"\"\"", 3)) {
	sp += 3;
	end = sq - 3;
    } else if (sp[0] == '"') {
	sp++;
	end = sq - 1;
    } else {
	end = sq;
	/* for barekey allow only this regex: [A-Za-z0-9_-]+ */
	const char* xp;
	for (xp = sp; xp != sq; xp++) {
	    int ch = *xp;
	    if ('A' <= ch && ch <= 'Z') continue;
	    if ('a' <= ch && ch <= 'z') continue;
	    if ('0' <= ch && ch <= '9') continue;
	    if (ch == '_' || ch == '-') continue;
	    bad_string(ctx, lineno);
	}
    }

    char* ret = quote_string(sp, end - sp);
    if (! ret)
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

		next_token(ctx, 0);
		break;
	    }

	case LBRACKET:
	    { /* [ [array], [array] ... ] */
		if (arr->typ == 0) arr->typ = 'a';
		if (arr->typ != 'a') {
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
			if (arr->typ != 't') internal_error(ctx, FLINE);
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
	if (arr->typ == 0) arr->typ = 't';
	if (arr->typ != 't') syntax_error(ctx, z.lineno, "array mismatch");

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
    for (i = 0; i < 10 && *p; i++) {
	int ch = *p++;
	*z++ = ch;
	if (ch == 'Z') {
	    if (i == 0) break; else return -1;
	}
	if (ch == '+' || ch == '-') {
	    if (i == 0) continue; else return -1;
	}
	if (ch == ':') {
	    if (i == 3) continue; else return -1;
	}
	if (! ('0' <= ch && ch <= '9')) return -1;
    }
    return (p == q) ? 0 : -1;
}


int toml_rtob(const char* src, int* ret)
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


int toml_rtoi(const char* src, int64_t* ret)
{
    if (!src) return -1;
    
    char buf[100];
    char* p = buf;
    char* q = p + sizeof(buf);
    const char* s = src;
    
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


int toml_rtod(const char* src, double* ret)
{
    if (!src) return -1;
    
    char buf[100];
    char* p = buf;
    char* q = p + sizeof(buf);
    const char* s = src;

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


int toml_rtos(const char* src, char** ret)
{
    if (!src) return -1;
    if (*src != '\'' && *src != '"') return -1;
    
    char* buf = malloc(strlen(src) + 1);
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

    abort();
    return 0;
}
