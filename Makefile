
# Various install locations for glib:
GLIB_INSTALL_DIRS := \
	/home/nmiller/build/meson-glib \
	/home/nmiller/build/glib

GLIB_PKGCONFIG_DIRS := $(foreach dir,$(GLIB_INSTALL_DIRS),${dir}/lib/pkgconfig)

GLIB_PKGCONFIG_DIR := \
$(and ${DEBUG_GLIB},\
	$(or $(wildcard ${DEBUG_GLIB}),\
	     $(wildcard ${GLIB_PKGCONFIG_DIRS})))

# Invoke pkg-config with the appropriate environment variables:
pkg-config = $(shell ${PKG_CONFIG_ENV} pkg-config $1 $2)

ifdef GLIB_PKGCONFIG_DIR
PKG_CONFIG_ENV := PKG_CONFIG_PATH=${GLIB_PKGCONFIG_DIR}
rpath = -Wl,-rpath=$(call pkg-config,--variable=libdir,$1)
endif

PROGS = termsplice raw
CFLAGS = -g -O0

all: ${PROGS}

raw: raw.c
	${CC} ${CFLAGS} -o $@ $^

termsplice: termsplice.c
	${CC} \
	  $(call pkg-config,--cflags,glib-2.0) \
	  ${CFLAGS} $(call rpath,glib-2.0) -o $@ $^ \
	  $(call pkg-config,--libs,glib-2.0)

.PHONY: clean
clean:
	rm -f ${PROGS} *.o
