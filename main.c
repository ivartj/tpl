#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include "tpl.h"

tpl_ctx *ctx = NULL;
char *outpath = NULL;
char *destdir = NULL;
char **inpaths = NULL;
int inpathsnum = 0;

void createcontext(void)
{
	ctx = tpl_ctx_create();
}

void parsedef(char *def)
{
	char *i;
	char *name;
	size_t namelen;
	char *value;
	char c;

	i = def;

	while((c = *(i++)) != '\0')
		if(c == '=')
			break;

	if(c == '\0') {
		fprintf(stderr, "tpl: '%s' is not a valid definition.\n", def);
		exit(EXIT_FAILURE);
	}

	name = def;
	namelen = i - def - 1;
	name[namelen] = '\0';

	value = i;

	tpl_ctx_set_definition(ctx, name, value);
}

void usage(FILE *out)
{
	fprintf(out,
		"usage: tpl [ -d <destination-directory> ]\n"
		"           [ -o <output-path> ]\n"
		"           [ -T<template-directory> ... ]\n"
		"           [ -D<name=value> ... ]\n"
		"           <input-file> ...\n"
		"\n"
		"The -o and -d options are mutually exclusive.\n");
}

void parseargs(int argc, char *argv[])
{
	int c;
	int err;
	static struct option longopts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "template-directory", required_argument, NULL, 'T' },
		{ "define", required_argument, NULL, 'D' },
		{ "destination-directory", required_argument, NULL, 'd' },
		{ "output-path", required_argument, NULL, 'o' },
		{ 0, 0, 0, 0 },
	};

	while((c = getopt_long(argc, argv, "hD:T:d:o:", longopts, NULL)) != -1)
	switch(c) {
	case 'h':
		usage(stdout);
		exit(EXIT_SUCCESS);
	case 'T':
		tpl_ctx_add_searchpath(ctx, optarg);
		break;
	case 'D':
		parsedef(optarg);
		break;
	case 'd':
		destdir = optarg;
		break;
	case 'o':
		outpath = optarg;
		break;
	case ':':
	case '?':
		usage(stderr);
		exit(EXIT_FAILURE);
	}

	if(destdir != NULL && outpath != NULL) {
		fprintf(stderr, "ERROR: options -d and -o are mutually exclusive.\n");
		usage(stderr);
		exit(EXIT_FAILURE);
	}

	if(destdir == NULL && outpath == NULL)
		destdir = ".";

	switch(argc - optind) {
	case 0:
		usage(stderr);
		exit(EXIT_FAILURE);
		break;
	default:
		inpaths = &(argv[optind]);
		inpathsnum = argc - optind;
		if(outpath != NULL && inpathsnum != 1) {
			fprintf(stderr, "ERROR: Can't use -o option with multiple input files.\n");
			usage(stderr);
			exit(EXIT_FAILURE);
		}

		break;
	}
}

void processdocument(void)
{
	int i;
	int err;
	FILE *in = NULL;
	FILE *out = NULL;
	char opath[MAXPATHLEN];

	for(i = 0; i < inpathsnum; i++) {
		if(outpath == NULL) {
			err = tpl_ctx_get_outpath(ctx, inpaths[i], opath);
			if(err) {
				fprintf(stderr, "Failed to decide output path for '%s': %s\n", inpaths[i], tpl_ctx_error(ctx));
				exit(EXIT_FAILURE);
			}
			outpath = opath;
		}

		in = fopen(inpaths[i], "r");
		if(in == NULL) {
			fprintf(stderr, "Failed to open file '%s' for reading: %s.\n", inpaths[i], strerror(errno));
			exit(EXIT_FAILURE);
		}

		out = fopen(outpath, "w");
		if(out == NULL) {
			fprintf(stderr, "Failed to open file '%s' for writing: %s.\n", outpath, strerror(errno));
			exit(EXIT_FAILURE);
		}

		err = tpl_ctx_process(ctx, inpaths[i], in, out);
		if(err) {
			fprintf(stderr, "tpl: %s.\n", tpl_ctx_error(ctx));
			exit(EXIT_FAILURE);
		}

		fclose(in);
		fclose(out);
	}
}

int main(int argc, char *argv[])
{
	createcontext(); /* the context will be configured by the arguments */
	parseargs(argc, argv);
	processdocument();
	exit(EXIT_SUCCESS);
}
