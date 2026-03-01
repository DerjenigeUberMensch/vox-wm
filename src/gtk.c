#include <stdlib.h>

#include "XCB-TRL/xcb_gtk.h"
#include "gtk.h"

extern XCBAtom gtkatom[];

void
X11GTKSetUseCSD(
        uint8_t enabled
        )
{
    /* TODO, probably wont work on changes, but only on window spawns
     * NOT TESTED YET.
     */
    const char *const ENV_VARIABLE = "GTK_CSD";
    const char *const VALUE_ENABLE = "1";
    const char *const VALUE_DISABLE = "0";
    const int REPLACE = 1;

    if(enabled)
    {   setenv(ENV_VARIABLE, VALUE_ENABLE, REPLACE);
    }
    else
    {   unsetenv(VALUE_DISABLE);
    }
}

void
X11GTK(void
        )
{
}
