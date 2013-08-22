#include "tpl.h"
#include "array.h"
#include "tpldoc.h"
#include "defset.h"
#include "print.h"
#include <sys/stat.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

struct _tpl {
	array spaths;
	defset set;
	char *errmsg;
};

static void seterrmsg(tpl *t, const char *fmt, ...);
static void *datacopy(void *data, size_t len);
static char *findtpl(char *ext, char **spaths, int *npaths);

tpl *tpl_create(void)
{
	tpl *t;

	t = calloc(1, sizeof(tpl));

	return t;
}

int tpl_addpath(tpl *t, char *path)
{
	struct stat st;
	int err;

	err = stat(path, &st);
	if(err) {
		seterrmsg(t, "Failed to stat template search path '%s', '%s'", path, strerror(errno));
		return 1;
	}

	if((st.st_mode & S_IFDIR) == 0) {
		seterrmsg(t, "Template search path '%s' is not a directory", path);
		return 1;
	}

	if(access(path, R_OK | W_OK) == -1) {
		seterrmsg(t, "Insufficient read access on template search path '%s'", path);
		return 1;
	}

	arrayadd(&(t->spaths), path);

	return 0;
}

void tpl_setdef(tpl *t, char *name, char *value)
{
	defset_set(&(t->set), name, value);
}

char *tpl_getdef(tpl *t, char *name, size_t namelen)
{

	char *val;
	char *str;

	str = malloc(namelen);
	str[namelen] = '\0';
	memcpy(str, name, namelen);
	val = defset_get(&(t->set), str);
	free(str);

	return val;
}

char *tpl_errmsg(tpl *t)
{
	return t->errmsg;
}

void seterrmsg(tpl *t, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	free(t->errmsg);
	vasprintf(&(t->errmsg), fmt, ap);

	va_end(ap);
}

void *datacopy(void *data, size_t len)
{
	void *copy;

	copy = malloc(len);
	memcpy(copy, data, len);

	return copy;
}

char *findtpl(char *ext, char **spaths, int *npaths)
{
	int i;
	char *spath;
	char *path = NULL;

	for(i = 0; i < *npaths; i++) {
		spath = spaths[i];
		if(path != NULL)
			free(path);
		asprintf(&path, "%s/%s.tpl", spath, ext);
		if(access(path, R_OK | W_OK) != -1)
			break;
	}

	if(i == *npaths) {
		if(path != NULL)
			free(path);
		return NULL;
	}

	*npaths -= i + 1;

	return path;
}

char *getext(char *docpath)
{
	while(*docpath != '.' && *docpath != '\0')
		docpath++;
	if(*docpath == '\0')
		return NULL;

	return docpath + 1;
}

tpldoc *tpl_get(tpl *t, char *docpath)
{
	char *ext;
	char **spaths;
	int npaths;
	char *tpath;
	tpldoc *tdoc = NULL;
	tpldoc *ndoc = NULL;
	tpldoc *mdoc = NULL;
	char *next = NULL;

	ext = getext(docpath);

	spaths = (char **)(t->spaths.els);
	npaths = t->spaths.len;

	tpath = findtpl(ext, spaths, &npaths);
	if(tpath == NULL)
		return 0;

	tdoc = tpldoc_parse(t, tpath);
	free(tpath);
	tpath = NULL;

	while((next = tpldoc_get(tdoc, "-template")) != NULL) {
		if(strcmp(next, ext) == 0)
			tpath = findtpl(next, &(spaths[t->spaths.len - npaths]), &npaths);
		else {
			npaths = t->spaths.len;
			tpath = findtpl(next, spaths, &npaths);
		}


		if(tpath == NULL)
			goto abort;
		ext = next; // TODO: memory leak

		ndoc = tpldoc_parse(t, tpath);
		free(tpath);
		tpath = NULL;

		mdoc = tpldoc_merge(ndoc, tdoc);
		tpldoc_destroy(tdoc);
		tpldoc_destroy(ndoc);
		tdoc = mdoc;
		mdoc = NULL;
		ndoc = NULL;
	}

	return tdoc;
abort:
	if(tdoc)
		tpldoc_destroy(tdoc);
	if(ndoc)
		tpldoc_destroy(ndoc);
	if(tpath)
		free(tpath);
	if(next)
		free(next);
}

int tpl_process(tpl *t, char *docpath)
{
	tpldoc *tdoc;
	size_t len;
	char *destpath;
	FILE *out;

	tdoc = tpl_get(t, docpath);
	if(tdoc == NULL)
		return -1;

	len = tpldoc_process(tdoc, docpath, (writefn)fwrite, stdout);
	if(len == 0)
		return -1;

	return 0;
}
