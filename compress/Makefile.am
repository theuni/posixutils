
bin_PROGRAMS = compress

compress_SOURCES = zopen.cc zopen.h compress.cc

EXTRA_DIST = NOTES README revision.log

install-exec-hook:
	ln $(DESTDIR)$(bindir)/compress$(EXEEXT) \
	   $(DESTDIR)$(bindir)/uncompress$(EXEEXT)
	ln $(DESTDIR)$(bindir)/compress$(EXEEXT) \
	   $(DESTDIR)$(bindir)/zcat$(EXEEXT)

uninstall-hook:
	rm $(DESTDIR)$(bindir)/zcat$(EXEEXT) \
	   $(DESTDIR)$(bindir)/uncompress$(EXEEXT)

