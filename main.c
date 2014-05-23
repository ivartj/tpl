#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include "tpl.h"
#include "args.h"
#include "main.h"

#define PROGRAM_NAME "tpl"

const char *main_name = PROGRAM_NAME;

void processdocument(main_config *conf)
{
	int i;
	int err;
	FILE *in = NULL;
	FILE *out = NULL;
	char opath[MAXPATHLEN];

	for(i = 0; i < conf->inpathsnum; i++) {
		if(conf->outpath == NULL) {
			err = tpl_ctx_get_outpath(conf->tpl, conf->inpaths[i], opath);
			if(err) {
				fprintf(stderr, "Failed to decide output path for '%s': %s\n", conf->inpaths[i], tpl_ctx_error(conf->tpl));
				exit(EXIT_FAILURE);
			}
			conf->outpath = opath;
		}

		in = fopen(conf->inpaths[i], "r");
		if(in == NULL) {
			fprintf(stderr, "Failed to open file '%s' for reading: %s.\n", conf->inpaths[i], strerror(errno));
			exit(EXIT_FAILURE);
		}

		out = fopen(conf->outpath, "w");
		if(out == NULL) {
			fprintf(stderr, "Failed to open file '%s' for writing: %s.\n", conf->outpath, strerror(errno));
			exit(EXIT_FAILURE);
		}

		err = tpl_ctx_process(conf->tpl, conf->inpaths[i], in, out);
		if(err) {
			fprintf(stderr, "tpl: %s.\n", tpl_ctx_error(conf->tpl));
			exit(EXIT_FAILURE);
		}

		fclose(in);
		fclose(out);
	}
}

int main(int argc, char *argv[])
{
	main_config conf = { 0 };
	main_name = args_get_cmd(argv[0]);
	conf.tpl = tpl_ctx_create(); /* the context will be configured by the arguments */
	args_process(&conf, argc, argv);
	processdocument(&conf);
	exit(EXIT_SUCCESS);
}
