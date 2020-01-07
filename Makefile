
CFLAGS = `pkg-config --cflags glib-2.0` -g -O0
LDFLAGS = `pkg-config --libs glib-2.0`

.PHONY: clean

PROGS = termsplice raw

all: ${PROGS}

raw: raw.c
	${CC} -o $@ $^

termsplice: termsplice.c
	${CC} ${CFLAGS} -o $@ $^ ${LDFLAGS}

clean:
	rm -f ${PROGS} *.o
