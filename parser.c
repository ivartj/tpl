#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "parser.h"
#include "tpldoc.h"

typedef struct _ctx ctx;

struct _ctx {
	void *in;
	readfn read;
	char *src;
	size_t off;
	size_t len;
	size_t cap;
};

static void astdefset_add(astdefset *defset, astdef *def);
static void astbody_add(astbody *body, astelem *el);
static void astbody_text(astbody *body, size_t off, size_t len);

static astdoc *parsedoc(ctx *x);
static astdefset *parsedefset(ctx *x);
static astdef *parsedef(ctx *x);
static astbody *parsebody(ctx *x);
static astfield *parsefield(ctx *x);

static size_t parsews(ctx *x);
static size_t parsenl(ctx *x);

static size_t parsechar(ctx *x, int c);
static size_t parsename(ctx *x, size_t *off, size_t *len);
static size_t parsevalue(ctx *x, astdef *def);
static size_t parseellipsis(ctx *x, size_t *off, size_t *len);

static size_t xread(void *buf, size_t size, size_t nitems, ctx *x);

tpldoc *tpldoc_parse(tpl *t, const char *tfile)
{
	ctx x = { 0 };
	FILE *in;
	astdoc *ast;
	tpldoc *doc;

	in = fopen(tfile, "r");
	if(in == NULL)
		return NULL;

	x.read = parser_fread;
	x.in = in;

	ast = parsedoc(&x);
	doc = calloc(1, sizeof(tpldoc));
	doc->t = t;
	doc->ast = ast;

	return doc;
}

tpldoc *tpldoc_parse_stream(tpl *t, size_t (*readcb)(void *, size_t, size_t, void *), void *in)
{
	ctx x = { 0 };
	astdoc *ast;
	tpldoc *doc;

	x.read = readcb;
	x.in = in;

	ast = parsedoc(&x);
	doc = calloc(1, sizeof(tpldoc));
	doc->ast = ast;
	doc->t = t;

	return doc;
}

void xsrc(ctx *x, char *src, size_t len)
{
	int sizechange;

	sizechange = 0;
	while(x->cap <= x->len + len) {
		sizechange = 1;
		if(x->cap == 0)
			x->cap = 256;
		else
			x->cap *= 2;
	}
	if(sizechange)
		x->src = realloc(x->src, x->cap);

	memcpy(x->src + x->len, src, len);
	x->len += len;
	x->off = x->len;
}

size_t xread(void *buf, size_t size, size_t nitems, ctx *x)
{
	size_t len;
	int i;
	size_t inmem;
	size_t outmem;

	len = size * nitems;
	inmem = 0;
	outmem = 0;

	len * size * nitems;
	if(x->off < x->len) {
		inmem = x->len - x->off;
		if(inmem > len)
			inmem = len;
		memcpy(buf, x->src + x->off, inmem);
		x->off += inmem;

	}
	if(x->off + len >= x->len) {
		outmem = len - inmem;
		outmem = x->read(buf + inmem, 1, outmem, x->in);
		if(outmem != 0)
			xsrc(x, buf, outmem);
	}

	return inmem + outmem;
}

int xgetc(ctx *x)
{
	size_t len;
	unsigned char c;

	len = xread(&c, 1, 1, x);
	if(len == 0)
		return EOF;

	return c;
}

astdoc *parsedoc(ctx *x)
{
	astdoc *doc;
	size_t off;

	doc = calloc(1, sizeof(astdoc));
	off = x->off;

	doc->defset = parsedefset(x);
	doc->body = parsebody(x);

	doc->src = x->src;
	doc->srclen = x->len;

	return doc;
}

astdefset *parsedefset(ctx *x)
{
	astdefset *defset;
	astdef *def;
	size_t off;

	defset = calloc(1, sizeof(astdefset));
	defset->off = off = x->off;

	while((def = parsedef(x)) != NULL) {
		astdefset_add(defset, def);
		defset->len += def->len;
	}

	if(defset->num == 0) {
		astdefset_destroy(defset);
		x->off = off;
		return NULL;
	}

	if(parsenl(x) == 0) {
		astdefset_destroy(defset);
		x->off = off;
		return NULL;
	}

	defset->len = x->off - defset->off;

	return defset;
}

astdef *parsedef(ctx *x)
{
	astdef *def;

	def = calloc(1, sizeof(astdef));
	def->off = x->off;

	parsews(x);
	if(parsename(x, &(def->nameoff), &(def->namelen)) == 0)
		goto abort;
	parsews(x);
	if(parsechar(x, ':') == 0)
		goto abort;
	parsews(x);
	if(parsevalue(x, def) == 0)
		goto abort;
	if(parsenl(x) == 0)
		goto abort;

	def->len = x->off - def->off;

	return def;
abort:
	x->off = def->off;
	free(def);
	return NULL;
}

size_t parsenl(ctx *x)
{
	int c;
	size_t off;
	size_t len;

	off = x->off;

	while((c = xgetc(x)) != EOF) {
		if(isspace(c) == 0) {
			x->off = off;
			return 0;
		}
		if(c == '\n')
			break;
	}
	len = x->off - off;

	return len;
}

size_t parsews(ctx *x)
{
	int c;
	size_t off;

	off = x->off;

	while(isspace(c = xgetc(x)));
	if(c != EOF)
		x->off--;

	return x->off - off;
}

size_t parsename(ctx *x, size_t *noff, size_t *nlen)
{
	size_t off;
	int c;
	int ok;

	off = x->off;
	*noff = off;
	*nlen = 0;

	while((c = xgetc(x)) != EOF) {
		ok = 0;
		switch(c) {
		case '-':
			ok = 1;
		default:
			if(ok || isalnum(c)) {
				*nlen = x->off - off;
				continue;
			}

			if(isspace(c) && c != '\n')
				continue;

			x->off = off + *nlen;
			return *nlen;
		}
	}

	x->off = off + *nlen;

	return *nlen;
}

size_t parsevalue(ctx *x, astdef *def)
{
	size_t off;
	int c;
	int ok;

	off = x->off;
	def->valueoff = off;
	def->valuelen = 0;

	while((c = xgetc(x)) != EOF) {
		if(isgraph(c)) {
			def->valuelen = x->off - off;
			continue;
		}

		if(isspace(c) && c != '\n')
			continue;

		x->off = off + def->valuelen;
		return def->valuelen;
	}

	x->off = off + def->valuelen;

	return def->valuelen;
}

size_t parsechar(ctx *x, int c)
{
	size_t off;

	off = x->off;
	if(xgetc(x) == c)
		return 1;
	x->off = off;
	return 0;
}

void astdefset_add(astdefset *defset, astdef *def)
{
	if(defset->num == defset->cap) {
		if(defset->cap == 0)
			defset->cap = 256;
		else
			defset->cap *= 2;
		defset->els = realloc(defset->els, defset->cap * sizeof(astdef *));
	}
	defset->els[defset->num++] = def;
}

void astdefset_destroy(astdefset *defset)
{
	int i;

	if(defset->els) {
		for(i = 0; i < defset->num; i++)
			free(defset->els[i]);
		free(defset->els);
	}
	free(defset);
}

void astbody_add(astbody *body, astelem *el)
{
	if(body->num == body->cap) {
		if(body->cap == 0)
			body->cap = 256;
		else
			body->cap *= 2;
		body->els = realloc(body->els, body->cap * sizeof(astelem *));
	}
	body->els[body->num++] = el;
}

void astbody_text(astbody *body, size_t off, size_t len)
{
	asttext *text;

	text = calloc(1, sizeof(asttext));
	text->type = AST_EL_TEXT;
	text->off = off;
	text->len = len;

	astbody_add(body, (astelem *)text);
}

void astbody_destroy(astbody *body)
{
	int i;

	if(body->els) {
		for(i = 0; i < body->num; i++)
			free(body->els[i]);
		free(body->els);
	}
	free(body);
}

astbody *parsebody(ctx *x)
{
	astbody *body;
	astfield *field;
	int c;
	size_t len;
	size_t toff;
	enum {
		start, lt,
	} state;

	body = calloc(1, sizeof(astbody));
	body->off = x->off;

	state = start;
	toff = x->off;

	while((c = xgetc(x)) != EOF)
	switch(state) {
	case start:
		switch(c) {
		case '<':
			state = lt;
		}
		break;
	case lt:
		switch(c) {
		case '[':
			len = x->off;
			x->off -= 2;
			field = parsefield(x);
			if(field != NULL) {
				astbody_text(body, toff, len - 2 - toff);
				astbody_add(body, (astelem *)field);
				toff = x->off;
			} else
				x->off = len;
			state = start;
			break;
		default:
			state = start;
		}
		break;
	}
	astbody_text(body, toff, x->off - toff);

	body->len = x->off - body->off;
	return body;
abort:
	x->off = body->off;
	astbody_destroy(body);
}

astfield *parsefield(ctx *x)
{
	astfield *field;
	size_t len;

	field = calloc(1, sizeof(astfield));
	field->off = x->off;
	field->type = AST_EL_FIELD;

	if(parsechar(x, '<') == 0)
		goto abort;
	if(parsechar(x, '[') == 0)
		goto abort;
	parsews(x);
	if(parsename(x, &(field->nameoff), &(field->namelen)) == 0)
	if(parseellipsis(x, &(field->nameoff), &(field->namelen)) == 0)
		goto abort;
	parsews(x);

	len = x->off;
	if(parsechar(x, '|'))
	if(parsename(x, &(field->filteroff), &(field->filterlen)) != 0) {
		field->hasfilter = 1;
		parsews(x);
	} else
		x->off = len;

	if(parsechar(x, ']') == 0)
		goto abort;
	if(parsechar(x, '>') == 0)
		goto abort;

	field->len = x->off - field->off;

	return field;
abort:
	x->off = field->off;
	free(field);

	return NULL;
}

size_t parseellipsis(ctx *x, size_t *off, size_t *len)
{
	*off = x->off;
	if(parsechar(x, '.'))
	if(parsechar(x, '.'))
	if(parsechar(x, '.')) {
		*len = x->off - *off;
		return *len;
	}
abort:
	x->off = *off;
	return 0;
}

size_t parser_fread(void *buf, size_t size, size_t nitems, void *in)
{
	return fread(buf, size, nitems, (FILE *)in);
}
