#
#
#
SUBDIRS = imap src

	
spotless: 
	-for s in $(SUBDIRS); do \
	    (cd $$s; $(MAKE) spotless); \
	done
	-$(MAKE) distclean
	-rm Makefile Makefile.in configure 
	-rm -rf autom4te.cache
	-rm -rf config.guess config.sub depcomp install-sh missing mkinstalldirs
