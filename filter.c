#include "filter.h"
#include <unistd.h>
#include <string.h>

typedef struct _buffer buffer;

struct _buffer {
	char *data;
	size_t len;
	size_t off;
};

static size_t bread(void *str, size_t size, size_t nitems, buffer *buf);

size_t filter_stream(const char *cmd, tpl_readfunc readfn, void *in, tpl_writefunc writefn, void *out)
{
	FILE *pout, *pin;
	int wr[2];
	int rd[2];
	size_t cap, len;
	char buffer[256];
	size_t n;

	pipe(wr);
	pipe(rd);

	if(fork() == 0) {
		close(wr[1]);
		close(rd[0]);

		dup2(wr[0], 0);
		dup2(rd[1], 1);

		return execlp(cmd, cmd, (char *)NULL);
	}

	close(wr[0]);
	close(rd[1]);

	cap = sizeof(buffer);
	pout = fdopen(wr[1], "w");
	pin = fdopen(rd[0], "r");

	while((len = readfn(buffer, 1, cap, in)) != 0)
		fwrite(buffer, 1, len, pout);

	fflush(pout);
	fclose(pout);

	n = 0;

	while((len = fread(buffer, 1, cap, pin)) != 0) {
		n += writefn(buffer, 1, len, out);
	}

	return n;
}

size_t filter_buffer(const char *cmd, char *str, size_t len, tpl_writefunc writefn, void *out)
{
	buffer buf = { 0 };

	buf.data = str;
	buf.len = len;

	len = filter_stream(cmd, (tpl_readfunc)bread, &buf, writefn, out);

	return len;
}

size_t bread(void *str, size_t size, size_t nitems, buffer *buf)
{
	size_t len;

	len = size * nitems;
	if(buf->off == buf->len)
		return 0;
	if(buf->off + len > buf->len)
		len = buf->len - buf->off;
	memcpy(str, buf->data + buf->off, len);
	buf->off += len;

	return len;
}
