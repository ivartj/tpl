#include "tpl.h"
#include "tpl_doc.h"
#include "parser.h"
#include "msg.h"
#include "defset.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

char *tpl_doc_get_definition(tpl_doc *tdoc, char *name)
{
	char *value;
	
	value = defset_get(&(tdoc->defs), name, strlen(name));
	return value;
}

void tpl_doc_destroy(tpl_doc *tdoc)
{
	astdefset_destroy(tdoc->ast->defset);
	astbody_destroy(tdoc->ast->body);
	free(tdoc->ast->src);
	free(tdoc->ast);
	defset_clear(&(tdoc->defs));
	free(tdoc);
}

tpl_doc *tpl_doc_parse(tpl_ctx *t, const char *tfile)
{
	FILE *in;
	tpl_doc *doc;

	in = fopen(tfile, "r");
	if(in == NULL) {
		seterrmsg(t, "Failed to open template file '%s' for reading: %s", tfile, strerror(errno));
		return NULL;
	}

	doc = tpl_doc_parse_stream(t, (tpl_readfunc)fread, in);
	fclose(in);

	return doc;
}

tpl_doc *tpl_doc_parse_stream(tpl_ctx *t, tpl_readfunc read, void *in)
{
	astdoc *ast;
	tpl_doc *doc;
	int i;
	astdef *def;

	ast = parsedoc(read, in);
	doc = calloc(1, sizeof(tpl_doc));
	doc->ast = ast;
	doc->t = t;

	if(ast->defset != NULL)
	for(i = 0; i < ast->defset->num; i++) {
		def = ast->defset->els[i];
		defset_set_len(&(doc->defs), ast->src + def->nameoff, def->namelen, ast->src + def->valueoff, def->valuelen);
	}


	return doc;
}
