#ifndef MAIN_H
#define MAIN_H

#include "tpl.h"

typedef struct _main_config main_config;

extern const char *main_name;

struct _main_config {
	tpl_ctx *tpl;
	char *outpath;
	char *destdir;
	char **inpaths;
	int inpathsnum;
};

#endif
