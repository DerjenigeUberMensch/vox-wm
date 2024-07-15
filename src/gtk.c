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
    const char *const VALUE = "amogus";     /* value only has to be filled, not checked if 1 */
    const int REPLACE = 1;

    if(enabled)
    {   setenv(ENV_VARIABLE, VALUE, REPLACE);
    }
    else
    {   unsetenv(ENV_VARIABLE);
    }
}

void
X11GTK(
        )
{
}
