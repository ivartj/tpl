#ifndef DEFSET_H
#define DEFSET_H

#include "array.h"

typedef struct _defset defset;
typedef struct _def def;

struct _defset {
	array defs;
};

struct _def {
	char *name;
	char *value;
	size_t namelen;
	size_t valuelen;
};

void defset_set(defset *set, const char *name, const char *value);
void defset_set_len(defset *set, const char *name, size_t namelen, const char *value, size_t valuelen);
char *defset_get(defset *set, const char *name, size_t namelen);
void defset_transfer(defset *src, defset *dest);
void defset_clear(defset *set);

#endif
