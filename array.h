#ifndef ARRAY_H
#define ARRAY_H

#include <stdio.h>

typedef struct _array array;

struct _array {
	void **els;
	size_t len;
	size_t cap;
};

array *makearray(void);
void arrayadd(array *a, void *el);
void *arrayget(array *a, int idx);
void arrayclear(array *a);


#endif
