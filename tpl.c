#include <sys/stat.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "tpl.h"
#include "array.h"
#include "tpl_doc.h"
#include "defset.h"
#include "print.h"
#include "msg.h"

#define ERRMSGLEN 256

struct _tpl_ctx {
	array searchpaths;
	defset set;
	char errmsg[ERRMSGLEN];
	tpl_readfunc read;
	tpl_writefunc write;
};

static char *findtpl(char *ext, char **spaths, int *npaths);
static const char *getext(const char *path);
static tpl_doc *tpl_ctx_get_tpl_doc(tpl_ctx *t, const char *inpath);

tpl_ctx *tpl_ctx_create(void)
{
	tpl_ctx *t;

	t = calloc(1, sizeof(tpl_ctx));
	t->read = (tpl_readfunc)fread;
	t->write = (tpl_writefunc)fwrite;

	return t;
}

void tpl_ctx_add_searchpath(tpl_ctx *t, const char *path)
{
	arrayadd(&(t->searchpaths), strdup(path));
}

void tpl_ctx_set_definition(tpl_ctx *t, const char *name, const char *value)
{
	defset_set(&(t->set), name, value);
}

char *tpl_ctx_get_definition(tpl_ctx *t, const char *name, size_t namelen)
{
	return defset_get(&(t->set), name, namelen);
}

char *tpl_ctx_error(tpl_ctx *t)
{
	return t->errmsg;
}

tpl_writefunc tpl_ctx_get_writefunc(tpl_ctx *ctx)
{
	return ctx->write;
}

void tpl_ctx_set_writefunc(tpl_ctx *ctx, tpl_writefunc write)
{
	ctx->write = write;
}

tpl_readfunc tpl_ctx_get_readfunc(tpl_ctx *ctx)
{
	return ctx->read;
}

void tpl_ctx_set_readfunc(tpl_ctx *ctx, tpl_readfunc read)
{
	ctx->read = read;
}

int tpl_ctx_get_outpath(tpl_ctx *ctx, const char *inpath, char outpath[MAXPATHLEN])
{
	tpl_doc *t;
	char *outext;
	char *inext;

	t = tpl_ctx_get_tpl_doc(ctx, inpath);
	if(t == NULL) {
		seterrmsg(ctx, "Did not find templates for file '%s': %s", inpath, tpl_ctx_error(ctx));
		return -1;
	}

	outext = tpl_doc_get_definition(t, "-output");
	if(outext == NULL) {
		seterrmsg(ctx, "Template output file extension not defined for '%s'", inpath);
		return -1;
	}

	strncpy(outpath, inpath, MAXPATHLEN);
	inext = (char *)getext(outpath);
	strncpy(inext, outext, MAXPATHLEN - strlen(inpath));
	
	return 0;
}

int get_tpl_path(tpl_ctx *ctx, const char *ext, char path[MAXPATHLEN], int *offp)
{
	int i;
	char *spath;

	if(offp == NULL)
		i = 0;
	else
		i = *offp;

	for(; i < ctx->searchpaths.len; i++) {
		spath = ctx->searchpaths.els[i];
		snprintf(path, MAXPATHLEN, "%s/%s.tpl", spath, ext);
		if(access(path, R_OK) != -1)
			break;
	}

	if(i >= ctx->searchpaths.len)
		return -1;

	if(offp != NULL)
		*offp = i + 1;

	return 0;
}

const char *getext(const char *path)
{
	while(*path != '.' && *path != '\0')
		path++;
	if(*path == '\0')
		return NULL;

	return path + 1;
}

tpl_doc *tpl_ctx_get_tpl_doc(tpl_ctx *t, const char *inpath)
{
	char tpath[MAXPATHLEN] = { 0 };
	int err;
	tpl_doc *tdoc = NULL;
	tpl_doc *ndoc;
	tpl_doc *mdoc;
	const char *ext = NULL;
	char pext[MAXPATHLEN] = { 0 };
	const char *next = NULL;
	int off = 0;

	ext = getext(inpath);
	if(ext == NULL) {
		seterrmsg(t, "File path has no extensions");
		return NULL;
	}

	do {
		if(next != NULL) {
			if(strcmp(next, pext) != 0)
				off = 0;
			ext = next;
		}

		err = get_tpl_path(t, ext, tpath, &off);
		if(err) {
			if(tdoc != NULL)
				return tdoc;
			seterrmsg(t, "Did not find template for file extension '%s'", ext);
			return NULL;
		}

		ndoc = tpl_doc_parse(t, tpath);
		if(ndoc == NULL) {
			seterrmsg(t, "Failed to parse template file '%s': %s", tpath, tpl_ctx_error(t));
			return NULL;
		}

		if(tdoc == NULL)
			tdoc = ndoc;
		else {
			mdoc = tpl_doc_merge(ndoc, tdoc);
			strncpy(pext, ext, sizeof(pext));
			tpl_doc_destroy(tdoc);
			tpl_doc_destroy(ndoc);
			tdoc = mdoc;
		}

	} while((next = tpl_doc_get_definition(tdoc, "-template")) != NULL);

	return tdoc;
}

int tpl_ctx_process(tpl_ctx *ctx, const char *inpath, void *in, void *out)
{
	tpl_doc *tdoc;

	tdoc = tpl_ctx_get_tpl_doc(ctx, inpath);
	if(tdoc == NULL) {
		seterrmsg(ctx, "Failed to gather template for file '%s': %s", inpath, tpl_ctx_error(ctx));
		return -1;
	}

	tpl_doc_process(tdoc, in, out);

	return 0;
}

void seterrmsg(tpl_ctx *t, const char *fmt, ...)
{
	va_list ap;
	char tmp[ERRMSGLEN];

	va_start(ap, fmt);
	vsnprintf(tmp, ERRMSGLEN, fmt, ap);
	memcpy(t->errmsg, tmp, ERRMSGLEN);
	va_end(ap);
}
