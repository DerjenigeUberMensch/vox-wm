# compiler 
COMPILER ?= cc

# paths
PREFIX ?= /usr/local/

# includes and libs
# For more just do (GNU)
# cd /usr/lib/pkgconfig/
# grep -r xcb

# fallback
XCBLIST = xcb xcb-util xcb-aux xcb-xinerama xcb-event xcb-keysyms xcb-xinput xcb-image xcb-errors
INCLUDE_LIST = tools include ${XCBLIST}
INCS = $(foreach dir, ${INCLUDE_LIST}, -I${dir})
#${INCLUDE_INCS} ${TOOLS} 
#-lxcb-util -lxcb-icccm -lxcb-keysyms
LIBS = ${XCBLIST} x11 
#x11 xcb xcb-util xcb-aux xcb-xinerama xcb-event xcb-keysyms xcb-xinput xcb-image 
#${XCB_INCS} x11

CCVERSION = -std=c99
XNATIVE = -march=native -mtune=native
# Some libraries dont have static linking for some reason??
STATICLINK = -static
DYNAMICLINK= -ldl
SECTIONCODE= -ffunction-sections -fdata-sections
LINKMODE = ${DYNAMICLINK}
MEMFLAGSALWAYS = -fno-omit-frame-pointer
MEMFLAGSDEBUG = -fsanitize=address,pointer-compare,pointer-subtract,undefined,leak -fasynchronous-unwind-tables -ftrapv
#-fsanitize=undefined,thread -fasynchronous-unwind-tables -ftrapv -D_FORTIFY_SOURCE=2 
MEMFLAGS = ${MEMFLAGSALWAYS}
MEMFLAGS += ${MEMFLAGSDEBUG}

WARNINGDEFAULT = -pedantic -Wall -Wno-deprecated-declarations -Wshadow -Wuninitialized -Werror=format-security 
WARNINGEXTRAS = -Wunreachable-code -Waggregate-return -Wstrict-overflow=4 -Wpointer-arith -Wstrict-aliasing
WARNINGFLAGS = ${WARNINGDEFAULT} ${WARNINGEXTRAS}

LINKTIMEOPTIMIZATIONS = -flto -flto=auto

#SAFETYFLAGS = -fcf-protection=full -fharden-compares -fstack-protector-all -fstack-clash-protection -fharden-compares -fharden-conditional-branches -fharden-control-flow-redundancy  -fhardcfr-check-exceptions  -fhardcfr-check-returning-calls -fhardcfr-check-noreturn-calls=always 
PRELINKERFLAGS ?= -fpie -fstack-protector-strong -fstack-clash-protection ${LINKTIMEOPTIMIZATIONS} ${SECTIONCODE}

# can set higher but function overhead is pretty small so meh
INLINELIMIT ?= 15
# can conflict with adress sanatizer if used (clang)
NO_SANATIZE_FLAGS =  -Wl,-z,relro
X86 = -m32
X86_64 = -m64
BUILD_ARCHITECTURE = ${X86_64}
LINKLIBS = -lpthread -lm
LINKFLAGS = ${LINKMODE} -Wl,--as-needed,--relax,--gc-sections,-z,now,-z,noexecstack,-z,defs,-pie ${LINKTIMEOPTIMIZATIONS} ${LINKLIBS} ${BUILD_ARCHITECTURE}
LINKRELEASE = ${NO_SANATIZE_FLAGS} 
#-Wl,--strip-all 
LINKDEBUG = ${MEMFLAGS}

DEBUGFLAGS = -ggdb -g -fverbose-asm
CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_POSIX_C_SOURCE=200809L ${XINERAMAFLAGS}
CCFLAGS  = ${CCVERSION} ${WARNINGFLAGS} ${INCS} ${CPPFLAGS} ${PRELINKERFLAGS} ${BUILD_ARCHITECTURE}
RELEASEFLAGS = ${CCFLAGS} 

DEBUG 	= ${DEBUGFLAGS} ${MEMFLAGS} -O0

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
