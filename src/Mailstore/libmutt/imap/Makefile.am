#
#
#

if USE_GSS
GSSSOURCES = auth_gss.c
endif

if USE_SASL
AUTHENTICATORS = auth_sasl.c
else
AUTHENTICATORS = auth_anon.c auth_cram.c
endif

libdir = .

noinst_LIBRARIES = libimap.a
noinst_HEADERS = auth.h imap_private.h message.h

DEFS = \
	-DSHAREDIR=\"$(sharedir)\" \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBMUTT

#libmutt_a_CFLAGS = -I../src/
INCLUDES = -I$(srcdir)/../src/
AM_CFLAGS = -I$(srcdir)/../src/
CFLAGS += $(INCLUDES)

libimap_a_SOURCES = auth.c \
	auth_login.c \
	browse.c \
	command.c \
	imap.c \
        message.c \
	utf7.c \
	util.c $(AUTHENTICATORS) $(GSSSOURCES)

spotless: distclean
	@rm -rf Makefile Makefile.in
