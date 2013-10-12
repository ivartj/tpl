#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "tpl.h"

typedef struct _astdoc astdoc;
typedef struct _astdefset astdefset;
typedef struct _astdef astdef;
typedef struct _astbody astbody;
typedef union _astelem astelem;
typedef struct _asttext asttext;
typedef struct _astfield astfield;

#define AST_EL_TEXT 1
#define AST_EL_FIELD 2

struct _astdoc {
	char *src;
	size_t srclen;
	astdefset *defset;
	astbody *body;
};

struct _astdefset {
	size_t off, len;
	size_t num;
	size_t cap;
	astdef **els;
};

struct _astdef {
	size_t off, len;
	size_t nameoff;
	size_t namelen;
	size_t valueoff;
	size_t valuelen;
};

struct _astbody {
	size_t off, len;
	size_t num;
	size_t cap;
	astelem **els;
};

struct _astfield {
	int type;
	size_t off, len;
	size_t nameoff;
	size_t namelen;
	int hasfilter;
	size_t filteroff;
	size_t filterlen;
};

struct _asttext {
	int type;
	size_t off, len;
};

union _astelem {
	int type;
	astfield field;
	asttext text;
};

static size_t parser_fread(void *buf, size_t size, size_t nitems, void *in);
typedef size_t (*readfn)(void *, size_t, size_t, void *);
typedef size_t (*writefn)(void *, size_t, size_t, void *);

astdoc *parsedoc(tpl_readfunc read, void *in);
void astdefset_destroy(astdefset *defset);
void astbody_destroy(astbody *body);

#endif
