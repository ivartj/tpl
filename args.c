#include "args.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <setjmp.h>

#define WRAP 80

#define ARGMAXLEN 256
#define ERRMAXLEN 256

#define xenter(ctx)                \
	ctx->entered++;            \
	if(ctx->entered == 1)      \
	if(setjmp(ctx->entry))     \
		return ARGS_ERROR; \


struct args_context {
	int argc;
	char **argv;

	int entered;
	jmp_buf entry;

	int idx; /* current argument being parsed */

	int off; /* offset when traversing a collection of short options
                       * (such as -abc) */

	int no_more_options;

	char arg[ARGMAXLEN];
	char error[ERRMAXLEN];

};

static const char *nextword(const char *(*text));
static int xprintf(FILE *, const char *fmt, ...);
static int print_option(FILE *, const args_option *opt);
static int parselongopt(args_context *ctx, int argc, char **argv, char **optarg);
static int parseshortopt(args_context *ctx, int argc, char **argv, char **optarg);
static void xpanic(args_context *ctx, const char *fmt, ...);

const args_option *args_get_option_by_flag(const args_option *opts, const char *flag)
{
	int i, j;

	for(i = 0; opts[i].code != 0; i++) {
		for(j = 0; j < ARGS_MAX_OPTION_FLAGS; j++) {
			if(opts[i].flags[j] == NULL)
				break;
			if(strcmp(flag, opts[i].flags[j]) == 0)
				return &(opts[i]);
		}
	}

	return NULL;
}

args_context *args_create(int argc, char **argv)
{
	args_context *ctx = calloc(1, sizeof(args_context));
	if(ctx == NULL)
		return NULL;
	ctx->argc = argc;
	ctx->argv = argv;
	return ctx;
}

void xleave(args_context *ctx)
{
	if(ctx->entered == 0)
		xpanic(ctx, "Duplicate exit from args_context");
	ctx->entered--;
}

void xpanic(args_context *ctx, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	vsnprintf(ctx->error, ERRMAXLEN, fmt, ap);

	va_end(ap);

	ctx->entered = 0;
	longjmp(ctx->entry, -1);
}

void args_destroy(args_context *ctx)
{
	free(ctx);
}

int args_next(args_context *ctx, const char **argp)
{
	char *arg;
	int retval;

	xenter(ctx);

	if(ctx->idx == 0)
		ctx->idx++;
	
	if(ctx->idx >= ctx->argc) {
		retval = ARGS_END;
		goto ret;
	}
	
	arg = ctx->argv[ctx->idx];
	
	/* Plain option */
	if(ctx->no_more_options || strlen(arg) <= 1 || arg[0] != '-') {
		ctx->idx++;
		*argp = arg;
		retval = ARGS_PLAIN;
		goto ret;
	}

	/* "--" argument, signifying end of any further options */
	if(strcmp(arg, "--") == 0) {
		ctx->idx++;
		ctx->no_more_options = 1;
		return args_next(ctx, argp);
	}

	/* Parameter */
	if(ctx->off != 0 && arg[ctx->off] == '=')
		xpanic(ctx, "Unexpected parameter");

	/* Long options */
	if(strncmp(arg, "--", 2) == 0) {
		
		/* With parameter */
		if(strchr(arg, '=') != NULL) {
			/* TODO: This is probably flawed */
			ctx->off = (int)(strchr(arg, '=') - arg);
			strncpy(ctx->arg, arg, ctx->off); /* potential overflow */
			ctx->arg[ctx->off] = '\0';
			*argp = ctx->arg;
			retval = ARGS_OPTION;
			goto ret;
		}

		/* Without parameter */
		ctx->idx++;
		*argp = arg;
		retval = ARGS_OPTION;
		goto ret;
	}

	/* Short options */
	if(arg[0] == '-') {
		int c;
		if(ctx->off == 0)
			ctx->off++;
		c = arg[ctx->off];
		ctx->off++;

		if(ctx->off == strlen(arg)) {
			ctx->idx++;
			ctx->off = 0;
		}

		sprintf(ctx->arg, "-%c", c);
		*argp = ctx->arg;
		retval = ARGS_OPTION;
		goto ret;
	}

	/* Unreachable */
	xpanic(ctx, "Unreachable code reached in args_next");
	return ARGS_ERROR;

ret:
	xleave(ctx);
	return retval;
}

int args_take_parameter(args_context *ctx, const char **argp)
{
	const char *arg;

	xenter(ctx);
	
	if(ctx->idx == ctx->argc)
		xpanic(ctx, "End of arguments when looking for option parameter");

	arg = ctx->argv[ctx->idx];

	if(ctx->off == 0) {
		ctx->idx++;
		*argp = arg;
		xleave(ctx);
		return ARGS_OK;
	}

	if(arg[ctx->off] == '=') {
		arg += ctx->off + 1;
		ctx->off = 0;
		ctx->idx++;
		*argp = arg;
		xleave(ctx);
		return ARGS_OK;
	}

	arg += ctx->off;
	ctx->off = 0;
	ctx->idx++;

	*argp = arg;

	xleave(ctx);
	return ARGS_OK;
}

const char *args_error(args_context *ctx)
{
	return ctx->error;
}

int args_parse(args_context *ctx, const args_option *opts, const char **optarg)
{
	const char *arg;
	int argtype;
	int err;
	char *optarg_new = NULL;
	int retval;

	xenter(ctx);

	argtype = args_next(ctx, &arg);
	switch(argtype) {
	case ARGS_END:
	case ARGS_ERROR:
		retval = argtype;
		goto ret;
	case ARGS_PLAIN:
		*optarg = arg;
		retval = ARGS_PLAIN;
		goto ret;
	case ARGS_OPTION:
		{
			const args_option *opt = args_get_option_by_flag(opts, arg);
			if(opt == NULL)
				xpanic(ctx, "Unrecognized flag '%s'", arg);

			if(opt->parameter != NULL) {
				err = args_take_parameter(ctx, optarg);
				if(err)
					xpanic(ctx, "Missing parameter to '%s'", arg);
			}

			retval = opt->code;
			goto ret;
		}
	}

	xpanic(ctx, "Unreachable code reached in args_parse");

	return ARGS_ERROR;
ret:
	xleave(ctx);
	return retval;
}

int xprintf(FILE *file, const char *fmt, ...)
{
	va_list ap;
	int val;

	va_start(ap, fmt);
	if(file == NULL)
		val = vsnprintf(NULL, 0, fmt, ap);
	else
		val = vfprintf(file, fmt, ap);
	va_end(ap);

	return val;

}

int print_option(FILE *out, const args_option *opt)
{
	/* Cases:
	 *   -h  |
	 *   -i FILENAME  |
	 *   -h, --help  |
	 *   --version  |
	 *   --format=FORMAT  |
	 *   -o, --output=FILENAME  |
	 */
	unsigned off = 0;
	int last_was_long = 0;
	int i;

	off += xprintf(out, "  ");
	for(i = 0; opt->flags[i] != NULL; i++) {
		if(i != 0)
			off += xprintf(out, ", ");
		off += xprintf(out, "%s", opt->flags[i]);
		last_was_long = strlen(opt->flags[i]) > 2;
	}
	if(opt->parameter != NULL)
		if(last_was_long)
			off += xprintf(out, "=%s", opt->parameter);
		else
			off += xprintf(out, " %s", opt->parameter);

	off += xprintf(out, "  ");

	return off;
}

void args_print_options(FILE *out, const args_option *opts)
{
	unsigned maxoff = 0;
	unsigned off = 0;
	const args_option *opt;
	int i;

	/* First find a common offset long enough for every option.
	 */
	for(i = 0; opts[i].code != 0; i++) {
		opt = &(opts[i]);

		off = print_option(NULL, opt);

		if(off > maxoff)
			maxoff = off;
	}


	/* Write options with option help at given offset. */

	for(i = 0; opts[i].code != 0; i++) {
		opt = &(opts[i]);

		off = print_option(out, opt);

		args_print_wrap_and_indent(out, maxoff, off, opt->help);
	}
}

const char *args_get_command(const char *argv0)
{
	const char *sepcheck = NULL;

	for(sepcheck = argv0; *sepcheck != '\0'; sepcheck++)
		if(*sepcheck == '/')
			argv0 = sepcheck + 1;

	return argv0;
}

const char *nextword(const char *(*text))
{
	static char word[WRAP + 1] = { 0 };
	int i;

	while(**text && !isgraph(**text))
		(*text)++;

	if(**text == '\0')
		return NULL;

	for(i = 0; i < WRAP && isgraph(**text); i++) {
		word[i] = **text;
		(*text)++;
	}

	word[i] = '\0';


	return word;
}

void args_print_wrap_and_indent(FILE *out, int ind, int initind, const char *text)
{
	int i;
	int textoff = 0;
	int curoff;
	int maxoff = WRAP;
	const char *word;
	int wordlen;
	int newline;

	if(initind < ind) {
		for(i = 0; i < ind - initind; i++)
			fputc(' ', out);
		curoff = ind;
	} else
		curoff = initind;

	newline = 1;

	while((word = nextword(&text)) != NULL) {

		wordlen = (int)strlen(word);
		if(curoff + wordlen > WRAP && !newline) {
			fputc('\n', out);
			for(i = 0; i < ind; i++)
				fputc(' ', out);
			curoff = ind;
			newline = 1;
		}

		if(!newline) {
			fputc(' ', out);
			curoff++;
		}

		curoff += fprintf(out, "%s", word);

		newline = 0;
	}

	fputc('\n', out);
}

int args_parse_definition(char *def, char **namep, char **valuep)
{
	int i;
	char c;


	while((c = def[i++]) != '\0')
		if(c == '=')
			break;

	if(c == '\0')
		return -1;

	def[i] = '\0';
	*namep = def;
	*valuep = def + i + 1;

	return 0;
}

