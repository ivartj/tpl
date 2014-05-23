#ifndef FILTER_H
#define FILTER_H

#include <stdio.h>
#include "tpl.h"

size_t filter_stream(const char *cmd, tpl_readfunc read, void *in, tpl_writefunc write, void *out);
size_t filter_buffer(const char *cmd, char *str, size_t len, tpl_writefunc writefn, void *out);

#endif
