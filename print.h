#ifndef PRINT_H
#define PRINT_H

#include <stdio.h>
#include "tpldoc.h"

size_t tpldoc_process(tpldoc *tdoc, const char *docpath, writefn write, void *out);

#endif
