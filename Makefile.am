AM_CFLAGS = -I${abs_top_srcdir}/include
SUBDIRS = html-escape tests
noinst_LIBRARIES = libtpl.a
libtpl_a_SOURCES = tpl.h tpl.c array.h array.c tpl_doc.h tpl_doc.c defset.h defset.c parser.h parser.c merge.h merge.c filter.c filter.h print.h print.c msg.h
bin_PROGRAMS = tpl
tpl_SOURCES = main.c args.c args.h
tpl_LDADD = libtpl.a
