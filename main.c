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

#define OPT_HELP			1
#define OPT_VERSION			2
#define OPT_TEMPLATE_DIRECTORY		3
#define OPT_DEFINE			4
#define OPT_DESTINATION_DIRECTORY	5
#define OPT_OUTPUT			6

args_option main_opts[] = {

	{ OPT_HELP,
		{ "-h", "--help" }, NULL,
		"Prints help message." },

	{ OPT_VERSION,
		{ "--version" }, NULL,
		"Prints version." },

	{ OPT_TEMPLATE_DIRECTORY,
		{ "-T", "--template-directory" }, "DIRECTORY",
		"Adds search path for templates." },

	{ OPT_DEFINE,
		{ "-D", "--define" }, "DEFINITION",
		"Adds definition for use in templates." },

	{ OPT_DESTINATION_DIRECTORY,
		{ "-d", "--destination-directory" }, "DIRECTORY",
		"Specifies destination of produced files." },

	{ OPT_OUTPUT,
		{ "-o", "--output" }, "FILE",
		"Specifies output path in case of a single output file." },

	{ 0 },
};

void main_usage(FILE *out)
{
	fprintf(out,
		"Usage: %s\n"
		"   [ -d <destination-directory> ]\n"
		"   [ -o <output-path> ]\n"
		"   [ -T<template-directory> ... ]\n"
		"   [ -D<name=value> ... ]\n"
		"   <input-file> ...\n"
		"\n", main_name);

	fprintf(out,
		"Description:\n");
	args_print_wrap_and_indent(out, 2, 0,
		"The -o and -d options are mutually exclusive.");
	fprintf(out,
		"\n");

	fprintf(out,
		"Options:\n");
	args_print_options(out, main_opts);
	fprintf(out,
		"\n");
}

void main_args(main_config *conf, int argc, char *argv[])
{

	int code;
	int err;
	char *defname, *defvalue;

	args_context *args = args_create(argc, argv);

	char *optarg;
	while((code = args_parse(args, main_opts, (const char **)&optarg)) != -1)
	switch(code) {

	case OPT_HELP:
		main_usage(stdout);
		exit(EXIT_SUCCESS);

	case OPT_TEMPLATE_DIRECTORY:
		tpl_ctx_add_searchpath(conf->tpl, optarg);
		break;

	case OPT_DEFINE:
		err = args_parse_definition(optarg, &defname, &defvalue);
		if(err) {
			fprintf(stderr, "Failed to parse definition '%s'.\n", optarg);
			exit(EXIT_FAILURE);
		}
		tpl_ctx_set_definition(conf->tpl, defname, defvalue);
		break;

	case OPT_OUTPUT:
		conf->outpath = optarg;
		break;

	case OPT_DESTINATION_DIRECTORY:
		conf->destdir = optarg;
		break;

	case ARGS_PLAIN:
		conf->inpaths = realloc(conf->inpaths, sizeof(void*) * (++conf->inpathsnum));
		conf->inpaths[conf->inpathsnum - 1] = optarg;
		break;

	case ARGS_ERROR:
		fprintf(stderr, "Error occurred on processing command-line arguments: %s.\n", args_error(args));
		exit(EXIT_FAILURE);

	}

	if(conf->inpathsnum > 1 && conf->outpath != NULL) {
		fprintf(stderr, "Can't use -o option with multiple input files.\n");
		main_usage(stderr);
		exit(EXIT_FAILURE);
	}
}

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
	main_name = args_get_command(argv[0]);
	conf.tpl = tpl_ctx_create(); /* the context will be configured by the arguments */
	main_args(&conf, argc, argv);
	processdocument(&conf);
	exit(EXIT_SUCCESS);
}
