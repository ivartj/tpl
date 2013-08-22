#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include "htmlescape.h"

char *outpath = NULL;
FILE *outfile = NULL;
char *inpath = NULL;
FILE *infile = NULL;

void usage(FILE *out)
{
	fprintf(out, "usage: html-escape [ -o <output-file> ] [ <input-file> ]\n");
}

void parseargs(int argc, char *argv[])
{
	int c;
	static struct option longopts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "out", required_argument, NULL, 'o' },
		{ 0, 0, 0, 0 },
	};

	while((c = getopt_long(argc, argv, "ho:", longopts, NULL)) != -1)
	switch(c) {
	case 'h':
		usage(stdout);
		exit(EXIT_SUCCESS);
	case 'o':
		outpath = optarg;
		break;
	case ':':
	case '?':
		usage(stderr);
		exit(EXIT_FAILURE);
	}

	switch(argc - optind) {
	case 0:
		break;
	case 1:
		inpath = argv[optind];
		break;
	default:
		usage(stderr);
		exit(EXIT_FAILURE);
	}
}

void openfiles(void)
{
	if(outpath != NULL) {
		outfile = fopen(outpath, "w");
		if(outfile == NULL) {
			fprintf(stderr, "Failed to open '%s' for writing: .\n", outpath, strerror(errno));
			exit(EXIT_FAILURE);
		}
	} else
		outfile = stdout;

	if(inpath != NULL) {
		infile = fopen(inpath, "r");
		if(infile == NULL) {
			fprintf(stderr, "Failed to open '%s' for reading: .\n", inpath, strerror(errno));
			exit(EXIT_FAILURE);
		}
	} else
		infile = stdin;
}

void filter(void)
{
	htmlescape(infile, htmlescape_fread, outfile, htmlescape_fwrite);
}

int main(int argc, char *argv[])
{
	parseargs(argc, argv);
	openfiles();
	filter();
	exit(EXIT_SUCCESS);
}
