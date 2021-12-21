all:
	$(CC) $(CPPFLAGS) $(CFLAGS) -o src/webif-page src/webif-page.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o src/bstrip src/bstrip.c

clean:
	$(RM) -f src/webif-page src/bstrip

install:
	mkdir -p $(DESTDIR)/usr/bin
	mkdir -p $(DESTDIR)/usr/lib/webif
	mkdir -p $(DESTDIR)/var/www/cgi-bin/webif
#	install -s -m0755 src/webif-page $(DESTDIR)/usr/bin/
#	install -s -m0755 src/bstrip $(DESTDIR)/usr/bin/
	cp src/webif-page $(DESTDIR)/usr/bin/
	cp src/bstrip $(DESTDIR)/usr/bin/
	cp -dpfr lib/* $(DESTDIR)/usr/lib/webif/
	cp -dpfr www/* $(DESTDIR)/var/www/

uninstall:
	$(RM) -r $(DESTDIR)/var/www/cgi-bin/webif*
	$(RM) -r $(DESTDIR)/var/www/webif.*
	$(RM) $(DESTDIR)/usr/lib/webif
	$(RM) $(DESTDIR)/usr/bin/bstrip
	$(RM) $(DESTDIR)/usr/bin/webif-page
