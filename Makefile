# Generated automatically from Makefile.in by configure.
# Generated automatically from Makefile.in by configure.
SRCDIR=/home/neotron/src/pexts
VPATH=/home/neotron/src/pexts
prefix=/usr/local
DESTDIR=



INSTALL=/usr/bin/install -c
CC=/home/neotron/src/pexts/tools/smartlink gcc
CPPFLAGS= -I/usr/local/include -I/usr/local/include -I/usr/X11/include -I/usr/X11R6/include
LDFLAGS= -R/usr/local/lib -L/usr/local/lib -R/usr/X11/lib -L/usr/X11/lib -R/usr/X11R6/lib -L/usr/X11R6/lib
PIKE=/usr/local/pike/7.1.6/bin/pike
PIKE_VERISON=7.1.6
# Used to avoid make compatibility problems.
BIN_TRUE=":"
#extern tools
all:	src 
	-@$(BIN_TRUE)

src: force
	@(cd src;$(MAKE) "prefix=$(prefix)" "CC=$(CC)" all)

force:
	-@$(BIN_TRUE)

depend:
	-@(cd src/cmods/;$(MAKE) "prefix=$(prefix)" "CC=$(CC)" "LDFLAGS=$(LDFLAGS)" depend)

clean:
	@(cd src;$(MAKE) "prefix=$(prefix)" "CC=$(CC)" "LDFLAGS=$(LDFLAGS)" clean)
install: all install_only 
	-@$(BIN_TRUE)

install_only: 
	@(cd src;$(MAKE) "prefix=$(prefix)" "CC=$(CC)" "LDFLAGS=$(LDFLAGS)" install)
	@echo "Pike Extension modules installed."

spotless:  clean
	@(cd src/;$(MAKE) "prefix=$(prefix)" "CC=$(CC)" "LDFLAGS=$(LDFLAGS)" spotless)
	rm -f tools/caudium-rc_script
	find . '(' -name '#*#' -o -name '*~' -o -name '.*~' -o -name '*.bak'\
        -o -name '.pine-debug*' -o -name '.*.bak' -o -name core -o -name \
	config.cache -o -name config.status -o -name config.log -o -name \
	"*.a" -o -name configure ')'  -print -exec /bin/rm '{}'  ';'
	rm -rf server/logs
	rm -rf logs

./Makefile: $(SRCDIR)/Makefile.in config.status
	CONFIG_FILES=Makefile CONFIG_HEADERS="" ./config.status
	@echo "Run make again"
	@exit 1

bump_version.stamp:
	@touch bump_version.stamp

bump_version: bump_version.stamp
	@if test -f $(SRCDIR)/server/base_server/caudium.pike.new; then \
	  echo Deleting old $(SRCDIR)/server/base_server/caudium.pike.new...; \
	  rm $(SRCDIR)/server/base_server/caudium.pike.new || exit 1; \
	else : ; fi

$(SRCDIR)/ChangeLog.gz:
	cd $(SRCDIR); pike tools/make_changelog.pike | gzip -9 > ChangeLog.gz

$(SRCDIR)/ChangeLog.rxml.gz:
	cd $(SRCDIR); pike tools/make_changelog.pike --rxml | gzip -9 > ChangeLog.rxml.gz


