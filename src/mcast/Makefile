

PIKE=pike

include pike_paths.d

CC=gcc
CFLAGS=-Wall -g  $(PIKE_CINCLUDES)
LDFLAGS=-shared

PIKE_OBJS=mcast.o
PIKE_SOBJ=mcast.so
PIKE_SMOD=$(PIKE_MODULES)/$(PIKE_SOBJ)
PIKE_PROG=MultiCastUDP.pike
PIKE_PMOD=$(PIKE_MODULES)/Stdio.pmod/$(PIKE_PROG)

all: $(PIKE_SOBJ)

clean:
	rm -f *.o *.so *.d

install: $(PIKE_PMOD) $(PIKE_SMOD)

pike_paths.d: get_paths.sh Makefile
	PIKE=$(PIKE) . get_paths.sh > pike_paths.d


$(PIKE_SOBJ): $(PIKE_OBJS)
	$(CC) $(LDFLAGS) -o $@ $^
	cp $@ $(PIKE_MODULES)
#	sudo cp $@ $(PIKE_MODULES)

$(PIKE_SMOD): $(PIKE_SOBJ)
	cp $^ $(PIKE_MODULES)

$(PIKE_PMOD): $(PIKE_PROG)
	cp $^ $@

mcast.o: mcast.c
	$(CC) $(CFLAGS) -c $^

