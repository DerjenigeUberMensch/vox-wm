# See LICENSE file for copyright and license details.

include config.mk

SRC = main.c xcb_trl.c util.c
SRCH= *.h
OBJ = ${SRC:.c=.o}
VERSION = XXX
EXE = dwm 
all: options default

options:
	@echo ${EXE} build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"
.c.o:
	${CC} -c ${CFLAGS} $<

default: ${OBJ}
	${CC} -o ${EXE} ${OBJ} ${LDFLAGS}
	rm -f -- *.o

release:
	rm -rf -f -- ${EXE}-${VERSION}
	cp -R . ${EXE}-${VERSION}
#	tar -cf ${EXE}-${VERSION}.tar ${EXE}-${VERSION}
#	gzip ${EXE}-${VERSION}.tar

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f ${EXE} ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/${EXE}

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/${EXE}

.PHONY: all options release dist install uninstall 
