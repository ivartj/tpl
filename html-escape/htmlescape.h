#ifndef HTMLESCAPE_H
#define HTMLESCAPE_H

#include <stdio.h>

typedef size_t (*htmlescape_iofunc)(void *, size_t, size_t, void *);

size_t htmlescape_fread(void *buf, size_t size, size_t nitems, void *in);
size_t htmlescape_fwrite(void *buf, size_t size, size_t nitems, void *in);

int htmlescape(void *in, htmlescape_iofunc read, void *out, htmlescape_iofunc write);

#endif
