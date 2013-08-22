#ifndef TPLDOC_H
#define TPLDOC_H

#include "parser.h"
#include "tpl.h"

typedef struct _tpldoc tpldoc;

struct _tpldoc {
	astdoc *ast;
	tpl *t;
};

char *tpldoc_get(tpldoc *td, char *name);

/* defined in parser.c */
tpldoc *tpldoc_parse(tpl *t, const char *tfile);
tpldoc *tpldoc_parse_stream(tpl *t, size_t (*readcb)(void *, size_t, size_t, void *), void *in);
tpldoc *tpldoc_merge(tpldoc *a, tpldoc *b);
void tpldoc_destroy(tpldoc *tdoc);

#endif
