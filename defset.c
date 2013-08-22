#include "defset.h"
#include <string.h>
#include <stdlib.h>

static char *astrcpy(char *str);
static def *makedef(char *name, char *value);

void defset_set(defset *set, char *name, char *value)
{
	int i;
	def *f;

	for(i = 0; i < set->defs.len; i++) {
		f = (def *)(set->defs.els[i]);
		if(strcmp(f->name, name) == 0)
			break;
	}

	if(i == set->defs.len) {
		arrayadd(&(set->defs), makedef(name, value));
		return;
	}

	free(f->value);
	f->value = astrcpy(value);
}

char *defset_get(defset *set, char *name)
{
	int i;
	def *f;

	for(i = 0; i < set->defs.len; i++) {
		f = set->defs.els[i];
		if(strcmp(name, f->name) == 0)
			break;
	}

	if(i == set->defs.len)
		return NULL;

	return f->value;
}

void defset_merge(defset *a, defset *b)
{
	int i;
	def *f;

	for(i = 0; i < b->defs.len; i++) {
		f = b->defs.els[i];
		defset_set(a, f->name, f->value);
	}
}

char *astrcpy(char *str)
{

	char *cpy;

	cpy = malloc(strlen(str) + 1);
	strcpy(cpy, str);

	return cpy;
}

def *makedef(char *name, char *value)
{
	def *f;

	f = calloc(1, sizeof(def));
	f->name = astrcpy(name);
	f->value = astrcpy(value);

	return f;
}
