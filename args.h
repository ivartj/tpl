#ifndef ARGS_H
#define ARGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define ARGS_MAX_OPTION_FLAGS 10

/* Declare an array of args_option, terminated by a zeroed args_option. This is
 * passed to args_parse. */

typedef struct args_option args_option;
struct args_option {
	int code;          /* Value returned by args_parse upon encountering
			    * option. */

	const char *flags[ARGS_MAX_OPTION_FLAGS];
	                   /* Flags associated with the option. */

	const char *parameter;   /* Specifies that the option has a parameter and the
			    * name of the parameter for use in help message. */

	const char *help;        /* Help string for option. */
};


const args_option *args_get_option_by_flag(const args_option *opts, const char *flag);


typedef struct args_context args_context;

#define ARGS_ERROR		-1
#define ARGS_OK			0
#define ARGS_END		0
#define ARGS_PLAIN		'_'
#define ARGS_OPTION		'o'

args_context *args_create(int argc, char *argv[]);
void args_destroy(args_context *ctx);
int args_next(args_context *ctx, const char **argp);
const char *args_error(args_context *ctx);
int args_take_parameter(args_context *ctx, const char **argp);

/* Returns code specified in the encountered args_option or:
 *    0 (ARGS_END)   : Upon end of parsing
 *   -1 (ARGS_ERROR) : Upon encountering errors, such as missing parameter to option
 *  '_' (ARGS_PLAIN) : Upon encountering what does not appear to be an option
 *                     (not starting with hyphen, "-", or after "--" option).
 */

int args_parse(args_context *ctx, const args_option *opts, const char **optarg);


/* Prints formatted help for the given options. */

void args_print_options(FILE *out, const args_option *opts);


/* Retrieves what appears to be the command in the execution path. */
const char *args_get_command(const char *argv0);


/* Writes the given text with the given indentation and wraps it around 80 characters. */

void args_print_wrap_and_indent(FILE *out, int indentation, int initial_indentation, const char *text);


/* Parses definitions of the style name=value. A null-byte is placed on the
 * equal sign. Returns non-zero on error. */

int args_parse_definition(char *def, char **namep, char **valuep);

#endif

#ifdef __cplusplus
}
#endif
