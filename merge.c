#include "merge.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

typedef struct _ctx ctx;

struct _ctx {
	astdoc *a, *b;
	tpldoc *m;
	unsigned char *src;
	size_t len;
	size_t cap;
	size_t off;
	tpl *t;
};

static int xprintf(ctx *x, const char *fmt, ...);
static size_t xwrite(void *buf, size_t size, size_t nitems, ctx *x);
static size_t xread(void *buf, size_t size, size_t nitems, ctx *x);
static size_t printmerge(ctx *x);
static size_t printdefset(ctx *x);
static size_t printadef(ctx *x, astdef *def);
static size_t printbdef(ctx *x, astdef *def);
static size_t printabody(ctx *x);
static size_t printbbody(ctx *x);
static size_t printaelem(ctx *x, astelem *el);
static size_t printbelem(ctx *x, astelem *el);

static void parsemerge(ctx *x);

size_t xwrite(void *buf, size_t size, size_t nitems, ctx *x)
{
	size_t len;
	int sizechange;

	sizechange = 0;
	len = size * nitems;
	while(x->len + len > x->cap) {
		sizechange = 1;
		if(x->cap == 0)
			x->cap = 256;
		else
			x->cap *= 2;
	}
	if(sizechange)
		x->src = realloc(x->src, x->cap);

	memcpy(x->src + x->len, buf, len);
	x->len += len;

	return len;
}

size_t xread(void *buf, size_t nitems, size_t size, ctx *x)
{
	size_t len;

	len = nitems * size;
	if(x->off + len > x->len)
		len = x->len - x->off;
	memcpy(buf, x->src + x->off, len);

	x->off += len;

	return len;
}

int xputc(ctx *x, int c)
{
	return xwrite(&c, 1, 1, x);
}

int xprintf(ctx *x, const char *fmt, ...)
{
	va_list ap;
	char *buf = 0;
	int len;

	va_start(ap, fmt);
	len = vasprintf(&buf, fmt, ap);
	len = xwrite(buf, 1, len, x);
	free(buf);
	va_end(ap);

	return len;
}

tpldoc *tpldoc_merge(tpldoc *a, tpldoc *b)
{
	ctx x = { 0 };

	x.a = a->ast;
	x.b = b->ast;
	x.t = b->t;

	printmerge(&x);
	parsemerge(&x);

	free(x.src);

	return x.m;
}

size_t printmerge(ctx *x)
{
	int i;
	size_t n;

	n = 0;

	n += printdefset(x);
	n += printabody(x);

	return n;
}

size_t printdefset(ctx *x)
{
	int i;
	astdefset *a, *b;

	a = x->a->defset;
	b = x->b->defset;

	if(b != NULL)
	for(i = 0; i < b->num; i++)
		printbdef(x, b->els[i]);
	if(a != NULL)
	for(i = 0; i < a->num; i++)
		printadef(x, a->els[i]);
	xputc(x, '\n');
}

size_t printbdef(ctx *x, astdef *def)
{
	if((x->b->src + def->off)[0] == '-')
		return 0;

	return xwrite(x->b->src + def->off, 1, def->len, x);
}

size_t printadef(ctx *x, astdef *adef)
{
	int i;
	astdefset *bset;
	astdef *bdef;

	bset = x->b->defset;

	if(bset != NULL) {
		for(i = 0; i < bset->num; i++) {
			bdef = bset->els[i];
			if(bdef->namelen == adef->namelen)
			if(memcmp(x->b->src + bdef->nameoff, x->a->src + adef->nameoff, adef->namelen) == 0)
			if((x->b->src + bdef->nameoff)[0] != '-')
				return 0;
		}
	}

	return xwrite(x->a->src + adef->off, 1, adef->len, x);
}

size_t printabody(ctx *x)
{
	int i;
	astelem *elem;
	astbody *body;
	size_t n;

	body = x->a->body;

	if(body == NULL)
		return 0;

	n = 0;

	for(i = 0; i < body->num; i++)
		n += printaelem(x, body->els[i]);
}

size_t printbbody(ctx *x)
{
	int i;
	astelem *elem;
	astbody *body;
	size_t n;

	body = x->b->body;

	if(body == NULL)
		return 0;

	n = 0;

	for(i = 0; i < body->num; i++)
		n += printbelem(x, body->els[i]);
}

size_t printaelem(ctx *x, astelem *el)
{
	switch(el->type) {
	case AST_EL_TEXT:
		return xwrite(x->a->src + el->text.off, 1, el->text.len, x);
	case AST_EL_FIELD:
		if(el->field.namelen == 3)
		if(memcmp(x->a->src + el->field.nameoff, "...", 3) == 0)
			return printbbody(x);
		return xwrite(x->a->src + el->field.off, 1, el->field.len, x);
	}

	return 0;
}

size_t printbelem(ctx *x, astelem *el)
{
	switch(el->type) {
	case AST_EL_TEXT:
		return xwrite(x->b->src + el->text.off, 1, el->text.len, x);
	case AST_EL_FIELD:
		return xwrite(x->b->src + el->field.off, 1, el->field.len, x);
	}

	return 0;
}

void parsemerge(ctx *x)
{
	x->m = tpldoc_parse_stream(x->t, (readfn)xread, x);
}
