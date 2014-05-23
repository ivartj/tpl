#include "print.h"
#include <stdlib.h>
#include <string.h>
#include "filter.h"

typedef struct _ctx ctx;

struct _ctx {
	tpl_writefunc write;
	void *out;
	tpl_doc *tdoc;
	tpl_doc *doc;
	char *tsrc;
	char *src;
};

static int xprintf(ctx *x, const char *fmt, ...);
static size_t printtext(ctx *x, asttext *el);
static size_t printfield(ctx *x, astfield *el);
static int xgetdef(ctx *x, char *name, size_t namelen, char **value, size_t *valuelen);

size_t xwrite(void *buf, size_t size, size_t nitems, ctx *x)
{
	return x->write(buf, size, nitems, x->out);
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

size_t tpl_doc_process(tpl_doc *tdoc, void *in, void *out)
{
	int i;
	size_t n;
	ctx x = { 0 };
	tpl_doc *doc;
	astbody *body;

	doc = tpl_doc_parse_stream(tdoc->t, tpl_ctx_get_readfunc(tdoc->t), in);
	if(doc == NULL)
		return 0;

	x.write = tpl_ctx_get_writefunc(tdoc->t);
	x.out = out;
	x.tdoc = tdoc;
	x.doc = doc;
	x.tsrc = tdoc->ast->src;
	x.src = doc->ast->src;

	body = tdoc->ast->body;
	if(body == 0)
		return 0;

	n = 0;
	for(i = 0; i < body->num; i++)
	switch(body->els[i]->type) {
	case AST_EL_TEXT:
		n += printtext(&x, &(body->els[i]->text));
		break;
	case AST_EL_FIELD:
		n += printfield(&x, &(body->els[i]->field));
		break;
	}

	return n;
}

size_t printtext(ctx *x, asttext *el)
{
	return xwrite(x->tsrc + el->off, 1, el->len, x);
}

size_t printfield(ctx *x, astfield *el)
{
	char *name;
	size_t namelen;
	char *value;
	size_t valuelen;
	char *filter;
	size_t n;

	name = x->tsrc + el->nameoff;
	namelen = el->namelen;

	if(!xgetdef(x, name, namelen, &value, &valuelen))
		return 0;

	if(!el->hasfilter)
		return xwrite(value, 1, valuelen, x);

	filter = malloc(el->filterlen + 1);
	memcpy(filter, x->tsrc + el->filteroff, el->filterlen);
	filter[el->filterlen] = '\0';

	n = filter_buffer(filter, value, valuelen, (tpl_writefunc)xwrite, x);

	free(filter);
	return n;
}

int xgetdef(ctx *x, char *name, size_t namelen, char **value, size_t *valuelen)
{
	int i;
	astdefset *defset;
	astdef *def;

	if((*value = tpl_ctx_get_definition(x->doc->t, name, namelen)) != NULL) {
		*valuelen = strlen(*value);
		return 1;
	}

	if(namelen == 3)
	if(name[0] == '.' && name[1] == '.' && name[2] == '.') {
		*value = x->src + x->doc->ast->body->off;
		*valuelen = x->doc->ast->body->len;
		return 1;
	}

	defset = x->doc->ast->defset;
	if(defset != NULL)
		for(i = 0; i < defset->num; i++) {
			def = defset->els[i];
			if(def->namelen == namelen)
			if(memcmp(x->src + def->nameoff, name, namelen) == 0) {
				*value = x->doc->ast->src + def->valueoff;
				*valuelen = def->valuelen;
				return 1;
			}
		}

	defset = x->tdoc->ast->defset;
	if(defset != NULL)
		for(i = 0; i < defset->num; i++) {
			def = defset->els[i];
			if(def->namelen == namelen)
			if(memcmp(x->tsrc + def->nameoff, name, namelen) == 0) {
				*value = x->tsrc + def->valueoff;
				*valuelen = def->valuelen;
				return 1;
			}
		}

	return 0;
}
