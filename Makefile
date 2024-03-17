PREFIX=/usr/local
INCDIR=$(PREFIX)/include
LIBDIR=$(PREFIX)/lib
RANLIB = ranlib

TRG = libperfstat.a
OBJS = perfstat.o

-include config.mak

all: $(TRG)

clean:
	rm -f $(OBJS) $(TRG)

install: $(TRG) perfstat.h
	install -Dm 644 perfstat.h $(DESTDIR)$(INCDIR)/perfstat.h
	install -Dm 644 libperfstat.a $(DESTDIR)$(LIBDIR)/libperfstat.a

$(TRG): $(OBJS)
	rm -f $@
	$(AR) cru $@ $^
	$(RANLIB) $@

test: test.o $(OBJS)

.PHONY: all clean install

