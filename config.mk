# compiler 
# side note clang doesnt work with -flto for some reason
CC = cc

# paths
PREFIX = /usr/local/
MANPREFIX = ${PREFIX}/share/man

# includes and libs
# For more just do (GNU)
# cd /usr/lib/pkgconfig/
# grep -r xcb

# fallback
XCB_INCS_PKG = `pkg-config --cflags --libs xcb xcb-util xcb-aux xcb-xinerama xcb-event xcb-keysyms` 
XCB_INCS = -Ixcb -Ixcb-util -Ixcb-aux -Ixcb-xinerama -Ixcb-event -Ixcb-keysyms
TOOLS = -Itools
INCLUDE_INCS = -Iinclude
INCS = ${XCB_INCS} ${INCLUDE_INCS} ${TOOLS}
#-lxcb-util -lxcb-icccm -lxcb-keysyms
LIBS = ${XCB_INCS_PKG}

CCVERSION = -std=c99
XNATIVE = -march=native -mtune=native
# Some libraries dont have static linking for some reason??
STATICLINK = -static
DYNAMICLINK= -ldl
SECTIONCODE= -ffunction-sections -fdata-sections
LINKMODE = ${DYNAMICLINK}
MEMFLAGS = -fsanitize=address -fno-omit-frame-pointer
#MEMFLAGS = 
DEBUGFLAGS = -ggdb -g ${CCVERSION} ${WARNINGFLAGS} ${INCS} ${CPPFLAGS} ${BINARY} ${SECTIONCODE} ${MEMFLAGS}


WARNINGFLAGS = -pedantic -Wall -Wno-deprecated-declarations -Wshadow -Wuninitialized -Werror=format-security

LINKTIMEOPTIMIZATIONS = -flto -flto=auto

ifeq ($(CC), clang)
	LINKTIMEOPTIMIZATIONS = 
endif

PRELINKERFLAGS = -fstack-protector-strong -fstack-clash-protection -fpie ${LINKTIMEOPTIMIZATIONS} ${SECTIONCODE} ${LINKMODE} 

# can set higher but function overhead is pretty small so meh
INLINELIMIT = 15
LINKERFLAGS = ${LINKMODE} -Wl,--gc-sections,--as-needed,--relax,-z,relro,-z,now,-z,noexecstack,-z,defs,-pie -finline-limit=${INLINELIMIT}  ${LINKTIMEOPTIMIZATIONS} 

BINARY = ${X64}
CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_POSIX_C_SOURCE=200809L ${XINERAMAFLAGS}
CCFLAGS  = ${CCVERSION} ${WARNINGFLAGS} ${INCS} ${CPPFLAGS} ${BINARY} ${PRELINKERFLAGS} 
RELEASEFLAGS = ${CCFLAGS} 

DEBUG 	= ${DEBUGFLAGS} -O0

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
LDFLAGS = ${LIBS} ${LINKERFLAGS} ${BINARY} 
# Solaris
#CFLAGS  = -fast ${INCS} -DVERSION=\"${VERSION}\"
