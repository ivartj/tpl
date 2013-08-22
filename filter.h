#ifndef FILTER_H
#define FILTER_H

#include <stdio.h>

typedef size_t (*filter_iofunc)(void *, size_t, size_t, void *);

size_t filter_stream(char *cmd, filter_iofunc read, void *in, filter_iofunc write, void *out);
size_t filter_buffer(char *cmd, char *str, size_t len, filter_iofunc writefn, void *out);

#endif
