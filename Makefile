# See LICENSE file for copyright and license details.

include config.mk

BIN = binary
SRC = $(wildcard *.c)
SRCH= $(wildcard *.h)
OBJ = $(patsubst %.c,${BIN}/%.o,$(SRC))
VERSION = 1.0.0
EXE = dwm
EXEPATH = ${BIN}/${EXE}
CMACROS += -DVERSION=\"${VERSION}\" -DNAME=\"${EXE}\" 
CFLAGS += ${CMACROS}

all: options default

${BIN}:
	mkdir -p ${BIN}
options:
	@echo ${EXE} build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LDFLAGS = ${LDFLAGS}"
	@echo "CC      = ${CC}"

${BIN}/%.o: %.c | ${BIN}
	@${CC} ${CFLAGS} -c $< -o $@
	@echo "Building:" $< 

default: ${OBJ}
	@echo "_Linking:" ${OBJ}
	@${CC} -o ${BIN}/${EXE} ${OBJ} ${LDFLAGS}
	@echo "Done."


__CLEANARGS = ${RM} ${BIN}/*.o ${EXEPATH}

clean:
	${__CLEANARGS}
claen:
	${__CLEANARGS}
clen:
	${__CLEANARGS}
clena:
	${__CLEANARGS}
clane:
	${__CLEANARGS}
clear: clean


rebuild: clean default




release:
	rm -rf -f -- ${EXE}-${VERSION}
	cp -R . ${EXE}-${VERSION}
#	tar -cf ${EXE}-${VERSION}.tar ${EXE}-${VERSION}
#	gzip ${EXE}-${VERSION}.tar

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f ${EXEPATH} ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/${EXE}

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/${EXE}

.PHONY: all options clean clear release dist install uninstall 
