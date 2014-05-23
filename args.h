#ifndef ARGPARSE_H
#define ARGPARSE_H

#include "main.h"

const char *args_get_cmd(const char *argv0);
void args_process(main_config *conf, int argc, char *argv[]);

#endif
