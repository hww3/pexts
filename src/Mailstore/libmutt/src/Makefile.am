#
#
#
INCLUDES = -I../imap

lib_LTLIBRARIES = libmutt.la

#EXTRA_DIST = configure muttconfig.h.in mutt_ssl.c mutt_ssl.h

if USE_SSL
SSLSOURCES = mutt_ssl.c
endif

if USE_SASL
SASLSOURCES = mutt_sasl.c
endif

if USE_NSS
NSSSOURCES = mutt_ssl_nss.c
endif

libmutt_la_LDFLAGS = -version-fino `echo $(VERSION) | sed -e 's/\./:/g'`
libmutt_la_LIBADD = $(top_builddir)/imap/libimap.a
libmutt_la_CFLAGS = -fexceptions -fPIC -Wall \
	-DSHAREDIR=\"$(sharedir)\" \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBMUTT


libmutt_la_SOURCES = \
	account.c	\
	ascii.c		\
	attach.c 	\
	base64.c	\
	browser.h	\
	buffy.c 	\
	charset.c 	\
	copy.c 		\
	date.c 		\
	dotlock.c	\
	flags.c 	\
	from.c 		\
	getdomain.c	\
	globals.c 	\
	handler.c 	\
	hash.c 		\
	init.c 		\
	lib.c 		\
	mbox.c		\
	md5c.c		\
	mh.c 		\
	mx.c 		\
	mutt_socket.c	\
	muttlib.c	\
	parse.c 	\
	pattern.c 	\
	recvattach.c	\
	rfc2047.c 	\
	rfc2231.c	\
	rfc822.c 	\
	sendlib.c 	\
	signal.c 	\
	sort.c 		\
	url.c $(SSLSOURCES) $(SASLSOURCES) $(NSSSOURCES)

include_HEADERS =
	mutt.h		\
	mutt_regex.h	\
	rfc822.h	\
	mailbox.h


