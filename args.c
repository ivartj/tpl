#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "args.h"
#include "main.h"

void args_usage(FILE *out)
{
	fprintf(out,
		"usage: %s\n"
		"           [ -d <destination-directory> ]\n"
		"           [ -o <output-path> ]\n"
		"           [ -T<template-directory> ... ]\n"
		"           [ -D<name=value> ... ]\n"
		"           <input-file> ...\n"
		"\n"
		"The -o and -d options are mutually exclusive.\n");
}


void parsedef(main_config *conf, char *def)
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

	tpl_ctx_set_definition(conf->tpl, name, value);
}


void args_process(main_config *conf, int argc, char *argv[])
{
	int c;
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
		args_usage(stdout);
		exit(EXIT_SUCCESS);
	case 'T':
		tpl_ctx_add_searchpath(conf->tpl, optarg);
		break;
	case 'D':
		parsedef(conf, optarg);
		break;
	case 'o':
		conf->outpath = optarg;
		break;
	case 'd':
		conf->destdir = optarg;
		break;
	case ':':
	case '?':
		args_usage(stderr);
		exit(EXIT_FAILURE);
	}

	switch(argc - optind) {
	case 0:
		args_usage(stderr);
		exit(EXIT_FAILURE);
	default:
		conf->inpaths = &(argv[optind]);
		conf->inpathsnum = argc - optind;
		if(conf->inpathsnum != 1 && conf->outpath != NULL) {
			fprintf(stderr, "Can't use -o option with multiple input files.\n");
			args_usage(stderr);
			exit(EXIT_FAILURE);
		}
		break;
	}
}

const char *args_get_cmd(const char *argv0)
{
	const char *sepcheck = NULL;

	if(argv0 == NULL)
		return main_name;

	for(sepcheck = argv0; *sepcheck != '\0'; sepcheck++)
		if(*sepcheck == '/')
			argv0 = sepcheck + 1;

	return argv0;
}
