#include <stdlib.h>

#include "GArray/garray.h"

#include "main.h"
#include "xerror.h"

/* 120s default */
const u64 TTL_MILLISECONDS = 120 * 1000;

enum { MAX_SKIP_IDS = 256 };

GArray skip_ids;
bool initialized = false;

typedef struct SkipIDStruct SkipIDStruct;

struct SkipIDStruct
{
    XCBWindow id;
    u64 expires_at_ms;
};

static bool
GET_TIME_MS(u64 *now)
{
    struct timespec ts;

    if(clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
    {   return false;
    }

    *now = (u64)ts.tv_sec * 1000 + (u64)ts.tv_nsec / 1000000;

    return true;
}

static void 
PURGE_EXPIRED_SKIP_IDS(void)
{
    if(!initialized)
    {   return;
    }

    u64 now;

    if(!GET_TIME_MS(&now))
    {   return;
    }

    garray_i i;

    for(i = GArrayStart(&skip_ids); i < GArrayEnd(&skip_ids); ++i)
    {
        SkipIDStruct *skipd = GArrayAt(&skip_ids, i);

        if(ASSERT(skipd))
        {
            if(skipd->id != XCBNone && now >= skipd->expires_at_ms)
            {   skipd->id = XCBNone;
            }
        }
    }
}

static bool 
IN_SKIP_IDS(XCBWindow resource)
{
    if(!initialized || resource == XCBNone)
    {   return false;
    }

    PURGE_EXPIRED_SKIP_IDS();

    garray_i i;

    for(i = GArrayStart(&skip_ids);i < GArrayEnd(&skip_ids); ++i)
    {
        SkipIDStruct *skipd = GArrayAt(&skip_ids, i);

        if(ASSERT(skipd) && skipd->id == resource)
        {   return true;
        }
    }

    return false;
}

void
INIT_XERROR(void)
{
    if(initialized)
    {   return;
    }

    XCBSetErrorHandler(xerror);

    int status;

    status = GArrayCreateFilled(&skip_ids, sizeof(SkipIDStruct), MAX_SKIP_IDS);

    if(status != EXIT_SUCCESS)
    {   
        DebugWarn("Failed to intialized XERROR skip id handler");
        return;
    }

    initialized = true;
}

void
DESTROY_XERROR(void)
{
    if(!initialized)
    {   return;
    }

    GArrayWipe(&skip_ids);

    initialized = false;
}

void
XERROR_ADD_SKIP_ID(XCBWindow id)
{
    if(!initialized || id == XCBNone)
    {   return;
    }

    PURGE_EXPIRED_SKIP_IDS();

    u64 now;

    if(!GET_TIME_MS(&now))
    {   
        DebugWarn("Failed to get MONOTONIC time for skip ID's ignoring...");
        return;
    }

    u64 expiration = now + TTL_MILLISECONDS;

    SkipIDStruct *free_slot = NULL;

    garray_i i;

    for(i = GArrayStart(&skip_ids); i < GArrayEnd(&skip_ids); ++i)
    {
        SkipIDStruct *skipd = GArrayAt(&skip_ids, i);

        if(ASSERT(skipd))
        {
            if(skipd->id == id)
            {
                skipd->expires_at_ms = expiration;
                return;
            }

            if(skipd->id == XCBNone && !free_slot)
            {   free_slot = skipd;
            }
        }
    }

    SkipIDStruct newskipd = { .id = id, .expires_at_ms = expiration };

    if(free_slot)
    {
        *free_slot = newskipd;
        return;
    }

    if(GArrayEnd(&skip_ids) - GArrayStart(&skip_ids) >= MAX_SKIP_IDS)
    {
        DebugWarn("Skip ID array is full");
        return;
    }

    GArrayPushBack(&skip_ids, &newskipd);
}

void
xerror(XCBDisplay *display, XCBGenericError *err)
{
    if(likely(err))
    {
        if(IN_SKIP_IDS(err->resource_id))
        {   
            DebugLog("Skipped XError for resource ID: %d", err->resource_id);
            return;
        }

        char *major_code = XCBGetErrorMajorCodeText(err->major_code);
        char *minor_code = XCBGetErrorMinorCodeText(err->minor_code);
        char *error_code = XCBGetFullErrorText(err->error_code);
        DebugError(
                "%s "
                "%s "
                "%s, "
                "(%d|%d)\n", 
                major_code ? major_code : "NULL", 
                minor_code ? minor_code : "NULL",
                error_code ? error_code : "NULL", 
                err->major_code, err->error_code
                );
#if NDEBUG
        DebugError("error_code: [%d], major_code: [%d], minor_code: [%d]\n"
              "sequence: [%d], response_type: [%d], resource_id: [%d]\n"
              "full_sequence: [%d]\n"
              ,
           err->error_code, err->major_code, err->minor_code, 
           err->sequence, err->response_type, err->resource_id, 
           err->full_sequence);
        XCBCookie id;
        id.sequence = err->sequence;
        (void)id;
#else
        XCBDefaultHandlerMsg(display, err);
#endif
    }
}