#include "array.h"
#include <stdlib.h>

array *makearray(void)
{
	array *a;

	a = calloc(1, sizeof(array));

	return a;
}

void arrayadd(array *a, void *el)
{
	if(a->len == a->cap) {
		if(a->cap == 0)
			a->cap = 256;
		else
			a->cap *= 2;
		a->els = realloc(a->els, a->cap * sizeof(void *));
	}
	a->els[a->len++] = el;
}

void *arrayget(array *a, int idx)
{
	return a->els[idx];
}
