
PREFIX=/usr/local

vivehid: vivehid.c Makefile
	$(CC) -g -O0 $< -o $@ -lhidapi-hidraw

install: vivehid
	mkdir -p $(PREFIX)/bin
	cp $^ $(PREFIX)/bin

clean:
	rm -f vivehid
