#ifndef TPL_H
#define TPL_H

#include <stdio.h>

typedef struct _tpl tpl;

tpl *tpl_create(void);
void tpl_setfield(tpl *t, char *name, char *value);
char *tpl_getdef(tpl *t, char *name, size_t namelen);
int tpl_addpath(tpl *t, char *path);
int tpl_process(tpl *t, char *docpath);
char *tpl_errmsg(tpl *t);

extern tpl *gctx;

#endif
