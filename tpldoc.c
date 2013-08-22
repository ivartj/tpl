#include "tpldoc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *tpldoc_get(tpldoc *tdoc, char *name)
{
	int i;
	astdefset *defset;
	astdef *def;
	unsigned char *src;
	size_t namelen;
	char *val;

	src = tdoc->ast->src;
	defset = tdoc->ast->defset;
	namelen = strlen(name);

	if(defset == NULL)
		return NULL;

	for(i = 0; i < defset->num; i++) {
		def = defset->els[i];
		if(def->namelen != namelen)
			continue;
		if(memcmp(src + def->nameoff, name, namelen) != 0)
			continue;
		val = malloc(def->valuelen + 1);
		memcpy(val, src + def->valueoff, def->valuelen);
		val[def->valuelen] = '\0';
		return val;
	}

	return NULL;
}

void tpldoc_destroy(tpldoc *tdoc)
{
	astdefset_destroy(tdoc->ast->defset);
	astbody_destroy(tdoc->ast->body);
	free(tdoc->ast->src);
	free(tdoc->ast);
	free(tdoc);
}
