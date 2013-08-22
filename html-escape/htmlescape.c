#include "htmlescape.h"
#include <string.h>

typedef struct _ctx ctx;

struct _ctx {
	htmlescape_iofunc read;
	htmlescape_iofunc write;
	void *in;
	void *out;
};

static int escape(ctx *x);

int xgetc(ctx *x)
{
	unsigned char c;
	size_t len;

	len = x->read(&c, 1, 1, x->in);
	if(len == 0)
		return EOF;

	return c;
}

int xputs(char *str, ctx *x)
{
	return x->write(str, 1, strlen(str), x->out);
}

int xputc(int c, ctx *x)
{
	return x->write(&c, 1, 1, x->out);
}

size_t htmlescape_fread(void *buf, size_t size, size_t nitems, void *in)
{
	return fread(buf, size, nitems, (FILE *)in);
}

size_t htmlescape_fwrite(void *buf, size_t size, size_t nitems, void *in)
{
	return fwrite(buf, size, nitems, (FILE *)in);
}

int htmlescape(void *in, htmlescape_iofunc read, void *out, htmlescape_iofunc write)
{
	ctx x;

	x.in = in;
	x.read = read;
	x.out = out;
	x.write = write;

	return escape(&x);
}

int escape(ctx *x)
{
	int n;
	int c;

	n = 0;

	while((c = xgetc(x)) != EOF)
	switch(c) {
	case '<':
		n += xputs("&lt;", x);
		break;
	case '>':
		n += xputs("&gt;", x);
		break;
	case '"':
		n += xputs("&quot;", x);
		break;
	case '\'':
		n += xputs("&apos;", x);
		break;
	case '&':
		n += xputs("&amp;", x);
		break;
	default:
		n += xputc(c, x);
		break;
	}

	return n;
}
