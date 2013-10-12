#include "defset.h"
#include <string.h>
#include <stdlib.h>

static def *makedef(const char *name, const char *value);
static def *makedef_len(const char *name, size_t namelen, const char *value, size_t valuelen);

void defset_set(defset *set, const char *name, const char *value)
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
	f->value = strdup(value);
	f->valuelen = strlen(value);
}

void defset_set_len(defset *set, const char *name, size_t namelen, const char *value, size_t valuelen)
{
	int i;
	def *f;

	for(i = 0; i < set->defs.len; i++) {
		f = (def *)(set->defs.els[i]);
		if(f->namelen == namelen)
		if(strncmp(f->name, name, namelen) == 0)
			break;
	}

	if(i == set->defs.len) {
		arrayadd(&(set->defs), makedef_len(name, namelen, value, valuelen));
		return;
	}

	free(f->value);
	f->value = strndup(value, valuelen);
	f->valuelen = valuelen;
}

char *defset_get(defset *set, const char *name, size_t namelen)
{
	int i;
	def *f;

	for(i = 0; i < set->defs.len; i++) {
		f = set->defs.els[i];
		if(f->namelen == namelen)
		if(strncmp(name, f->name, namelen) == 0)
			break;
	}

	if(i == set->defs.len)
		return NULL;

	return f->value;
}

void defset_transfer(defset *src, defset *dest)
{
	int i;
	def *f;

	for(i = 0; i < src->defs.len; i++) {
		f = src->defs.els[i];
		defset_set(dest, f->name, f->value);
	}
}

def *makedef(const char *name, const char *value)
{
	def *f;

	f = calloc(1, sizeof(def));
	f->name = strdup(name);
	f->namelen = strlen(name);
	f->value = strdup(value);
	f->valuelen = strlen(value);

	return f;
}

def *makedef_len(const char *name, size_t namelen, const char *value, size_t valuelen)
{
	def *f;

	f = calloc(1, sizeof(def));
	f->name = strndup(name, namelen);
	f->namelen = namelen;
	f->value = strndup(value, valuelen);
	f->valuelen = valuelen;

	return f;
}

void defset_clear(defset *set)
{
	int i;
	def *d;

	for(i = 0; i < set->defs.len; i++) {
		d = arrayget(&(set->defs), i);
		free(d->name);
		free(d->value);
		free(d);
	}

	arrayclear(&(set->defs));
		
}
