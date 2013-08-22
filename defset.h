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
};

void defset_set(defset *set, char *name, char *value);
char *defset_get(defset *set, char *name);
char *defset_getn(defset *set, char *name, size_t namelen);
void defset_merge(defset *a, defset *b);

#endif
