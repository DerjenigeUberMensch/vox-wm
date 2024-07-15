
#include "hashing.h"
#include "client.h"
#include "khash.h"
#include "util.h"

/* hashing */
KHASH_MAP_INIT_INT(__CLIENTS__, Client *)
static khash_t(__CLIENTS__) *hashedclients = NULL;

void
addclienthash(Client *c)
{
    /* no SIGSEV protection in khash so we must check ourselvs */
    if(!hashedclients)
    {   
        setupclienthash();
        /* malloc probably failed */
        if(!hashedclients)
        {   
            DEBUG0("Failed to reinitialize hashedclients");
            return;
        }
    }
    int err;
    khint_t k = kh_put(__CLIENTS__, hashedclients, c->win, &err);

    enum
    {
        __KHASH_BAD_OPERATION = -1,
        __KHASH_ALREADY_PRESENT = 0,
        __KHASH_FIRST_HASH = 1,
        __KHASH_PREVIOUSLY_DELETED = 2,
    };
    /* set value to client */
    switch(err)
    {
        case __KHASH_BAD_OPERATION:
            /* likely malloc() failed. */
            DEBUG0("Failed to alloc memory for hash.");
            return;
        case __KHASH_ALREADY_PRESENT:
            DEBUG0("Item already present in khash. FIXME");
            return;
        case __KHASH_FIRST_HASH:
            break;
        /* XCB overflows and reuses ids when ever it can, or should I say, the XServer does that. */
        case __KHASH_PREVIOUSLY_DELETED:
            break;
    }
    /* bounds check */
    if(kh_end(hashedclients) > k)
    {   kh_value(hashedclients, k) = c;
    }
}

void
cleanupclienthash(void)
{   kh_destroy(__CLIENTS__, hashedclients);
}

Client *
getclienthash(XCBWindow win)
{
    /* no SIGSEV protection in khash so we must check ourselvs */
    if(!hashedclients)
    {   return NULL;
    }
    khint_t k = kh_get(__CLIENTS__, hashedclients, win);
    if(k != kh_end(hashedclients))
    {   return kh_val(hashedclients, k);
    }
    return NULL;
}

void
delclienthash(Client *c)
{
    /* no SIGSEV protection in khash so we must check ourselvs */
    if(!hashedclients)
    {   return;
    }
    khint_t k = kh_get(__CLIENTS__, hashedclients, c->win);
    if(k != kh_end(hashedclients))
    {   kh_del(__CLIENTS__, hashedclients, k);
    }
}

void
setupclienthash(void)
{   hashedclients = kh_init(__CLIENTS__);
}
