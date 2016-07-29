

PREFIX=/usr/local

LIBS = libvive.so libvive.a
OBJS = libvive.o

all: $(LIBS)

libvive.so: $(OBJS)
	$(CC) $< -shared -o $@

libvive.a: $(OBJS)
	ar rcs $@ $<

libvive.o: libvive.c
	$(CC) -c -fPIC $^ -o $@

install: $(LIBS)
	mkdir -p $(PREFIX)/lib
	cp $^ $(PREFIX)/lib

clean:
	rm -f $(LIBS) $(OBJS)
