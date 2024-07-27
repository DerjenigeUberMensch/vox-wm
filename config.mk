# compiler 
CC = cc

# paths
PREFIX = /usr/local/
MANPREFIX = ${PREFIX}/share/man

# includes and libs
# For more just do (GNU)
# cd /usr/lib/pkgconfig/
# grep -r xcb

# fallback

INCLUDE_LIST = tools include -Ixcb -Ixcb-util -Ixcb-aux -Ixcb-xinerama -Ixcb-event -Ixcb-keysyms -Ixcb-xinput -Ixcb-image
INCS = $(foreach dir, ${INCLUDE_LIST}, -I${dir}) 
#${INCLUDE_INCS} ${TOOLS} 
#-lxcb-util -lxcb-icccm -lxcb-keysyms
LIBS = xcb xcb-util xcb-aux xcb-xinerama xcb-event xcb-keysyms xcb-xinput xcb-image x11
#x11 xcb xcb-util xcb-aux xcb-xinerama xcb-event xcb-keysyms xcb-xinput xcb-image 
#${XCB_INCS} x11

CCVERSION = -std=c99
XNATIVE = -march=native -mtune=native
# Some libraries dont have static linking for some reason??
STATICLINK = -static
DYNAMICLINK= -ldl
SECTIONCODE= -ffunction-sections -fdata-sections
LINKMODE = ${DYNAMICLINK}
MEMFLAGS = -fsanitize=address -fno-omit-frame-pointer

WARNINGDEFAULT = -pedantic -Wall -Wno-deprecated-declarations -Wshadow -Wuninitialized -Werror=format-security 
WARNINGEXTRAS = -Wunreachable-code -Waggregate-return -Wstrict-overflow=4 -Wpointer-arith
WARNINGFLAGS = ${WARNINGDEFAULT} ${WARNINGEXTRAS}

LINKTIMEOPTIMIZATIONS = -flto -flto=auto

ifeq ($(CC), clang)
	LINKTIMEOPTIMIZATIONS = 
endif

PRELINKERFLAGS = -fstack-protector-strong -fstack-clash-protection -fpie ${LINKTIMEOPTIMIZATIONS} ${SECTIONCODE}

# can set higher but function overhead is pretty small so meh
INLINELIMIT = 15
# can conflict with adress sanatizer if used (clang)
NO_SANATIZE_FLAGS =  -Wl,-z,relro

LINKFLAGS = ${LINKMODE} -Wl,--as-needed,--relax,-z,now,-z,noexecstack,-z,defs,-pie -finline-limit=${INLINELIMIT}  ${LINKTIMEOPTIMIZATIONS} 
LINKRELEASE = ${NO_SANATIZE_FLAGS} -Wl,--strip-all 
LINKDEBUG = -Wl,--gc-sections ${MEMFLAGS}

CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_POSIX_C_SOURCE=200809L ${XINERAMAFLAGS}
CCFLAGS  = ${CCVERSION} ${WARNINGFLAGS} ${INCS} ${CPPFLAGS} ${PRELINKERFLAGS} 
RELEASEFLAGS = ${CCFLAGS} 

DEBUG 	= -ggdb -g ${SECTIONCODE} ${MEMFLAGS} -fverbose-asm -O0

SIZE  	= ${RELEASEFLAGS} -Os 

SIZEONLY= ${RELEASEFLAGS} -Oz -fno-ident -fno-asynchronous-unwind-tables

# Release Stable (-O2)
RELEASE = ${RELEASEFLAGS} -O2 
# Release Speed (-O3)
RELEASES= ${RELEASEFLAGS} -O3 
# Release Speed (-O3) (debug)
#RELEASES = ${RELEASEFLAGS} -O3 ${DEBUGFLAGS} -fno-inline -DENABLE_DEBUG -DXCB_TRL_ENABLE_DEBUG

# Build using cpu specific instruction set for more performance (Optional)
BUILDSELF = ${RELEASEFLAGS} ${XNATIVE} -O3

# Linker flags
LINKERFLAGS = ${LINKFLAGS} 
# Solaris
#CFLAGS  = -fast ${INCS} -DVERSION=\"${VERSION}\"
