#ifndef TPLDOC_H
#define TPLDOC_H

#include "defset.h"
#include "tpl.h"
#include "parser.h"

typedef struct _tpl_doc tpl_doc;

struct _tpl_doc {
	astdoc *ast;
	defset defs;
	tpl_ctx *t;
};

char *tpl_doc_get_definition(tpl_doc *td, char *name);

/* defined in parser.c */
tpl_doc *tpl_doc_parse(tpl_ctx *ctx, const char *tfile);

tpl_doc *tpl_doc_parse_stream(tpl_ctx *ctx, size_t (*readcb)(void *, size_t, size_t, void *), void *in);

size_t tpl_doc_process(tpl_doc *doc, void *in, void *out);

#include "merge.h" /* tpl_doc_merge */
void tpl_doc_destroy(tpl_doc *tdoc);

#endif
