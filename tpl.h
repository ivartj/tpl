#ifndef TPL_H
#define TPL_H

#include <sys/types.h>
#include <sys/param.h>

typedef struct _tpl_ctx tpl_ctx;

tpl_ctx *tpl_ctx_create(void);

typedef size_t (*tpl_writefunc)(const void *,size_t,size_t, void *);
void tpl_ctx_set_writefunc(tpl_ctx *ctx, tpl_writefunc write);
tpl_writefunc tpl_ctx_get_writefunc(tpl_ctx *ctx);

typedef size_t (*tpl_readfunc)(void *,size_t,size_t, void *);
void tpl_ctx_set_readfunc(tpl_ctx *ctx, tpl_readfunc read);
tpl_readfunc tpl_ctx_get_readfunc(tpl_ctx *ctx);

void tpl_ctx_set_definition(tpl_ctx *ctx, const char *name, const char *value);
char *tpl_ctx_get_definition(tpl_ctx *ctx, const char *name, size_t namelen);
void tpl_ctx_add_searchpath(tpl_ctx *ctx, const char *path);

int tpl_ctx_process(tpl_ctx *ctx, const char *inpath, void *in, void *out);

char *tpl_ctx_error(tpl_ctx *ctx);

int tpl_ctx_get_outpath(tpl_ctx *ctx, const char *inpath, char outpath[MAXPATHLEN]);

#endif
