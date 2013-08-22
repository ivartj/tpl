#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include "tpl.h"

char **inpaths = NULL;
char *destdir = "";
int inpathsnum = 0;
tpl *gctx = NULL;

void createcontext(void)
{
	gctx = tpl_create();
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

	tpl_setdef(gctx, name, value);
}

void usage(FILE *out)
{
	fprintf(out, "usage: tpl [ -T<template-dir> ... ] [ -D<name=value> ... ] <input-file> ...\n");
}

void parseargs(int argc, char *argv[])
{
	int c;
	int err;
	static struct option longopts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "tpl-dir", required_argument, NULL, 'T' },
		{ "define", required_argument, NULL, 'D' },
		{ "dest-dir", required_argument, NULL, 'd' },
		{ 0, 0, 0, 0 },
	};

	while((c = getopt_long(argc, argv, "hD:T:d:", longopts, NULL)) != -1)
	switch(c) {
	case 'h':
		usage(stdout);
		exit(EXIT_SUCCESS);
	case 'T':
		err = tpl_addpath(gctx, optarg);
		if(err) {
			fprintf(stderr, "tpl: %s.\n", tpl_errmsg(gctx));
			exit(EXIT_FAILURE);
		}
		break;
	case 'D':
		parsedef(optarg);
		break;
	case 'd':
		destdir = optarg;
		break;
	case ':':
	case '?':
		usage(stderr);
		exit(EXIT_FAILURE);
	}

	switch(argc - optind) {
	case 0:
		usage(stderr);
		exit(EXIT_FAILURE);
		break;
	default:
		inpaths = &(argv[optind]);
		inpathsnum = argc - optind;
		break;
	}
}

void processdocument(void)
{
	int i;
	int err;

	for(i = 0; i < inpathsnum; i++) {
		err = tpl_process(gctx, inpaths[i]);
		if(err)
			fprintf(stderr, "tpl: %s.\n", tpl_errmsg(gctx));
	}
}

int main(int argc, char *argv[])
{
	createcontext(); /* the context will be configured by the arguments */
	parseargs(argc, argv);
	processdocument();
	exit(EXIT_SUCCESS);
}
