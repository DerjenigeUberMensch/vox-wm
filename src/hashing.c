#include <pthread.h>

#include "hashing.h"
#include "client.h"
#include "khash.h"
#include "util.h"

/* hashing */
KHASH_MAP_INIT_INT(__CLIENTS__, Client *)
static khash_t(__CLIENTS__) *hashedclients = NULL;
pthread_mutex_t hashing_mutex = PTHREAD_MUTEX_INITIALIZER;

static int
HashingLock(void)
{   return pthread_mutex_lock(&hashing_mutex);
}

static int
HashingUnlock()
{   return pthread_mutex_unlock(&hashing_mutex);
}

int
addclienthash(Client *c, XCBWindow key)
{
    HashingLock();
    /* no SIGSEV protection in khash so we must check ourselvs */
    if(!hashedclients)
    {   
        setupclienthash();
        /* malloc probably failed */
        if(!hashedclients)
        {   
            DebugWarn("Failed to reinitialize hashedclients");
            HashingUnlock();
            return EXIT_FAILURE;
        }
    }

    int err;
    khint_t k = kh_put(__CLIENTS__, hashedclients, key, &err);

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
            HashingUnlock();
            /* likely malloc() failed. */
            DebugWarn("Failed to alloc memory for hash.");
            return EXIT_FAILURE;
        case __KHASH_ALREADY_PRESENT:
            DebugWarn("Item already present in khash. FIXME");
            HashingUnlock();
            return EXIT_SUCCESS;
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

    HashingUnlock();

    return EXIT_SUCCESS;
}

void
cleanupclienthash(void)
{   
    HashingLock();
    kh_destroy(__CLIENTS__, hashedclients);
    hashedclients = NULL;
     HashingUnlock();
}

Client *
getclienthash(XCBWindow win)
{
    Client *ret = NULL;

    HashingLock();

    /* no SIGSEV protection in khash so we must check ourselvs */
    if(!hashedclients)
    {   goto UNLOCK;
    }

    khint_t k = kh_get(__CLIENTS__, hashedclients, win);

    if(k != kh_end(hashedclients))
    {   
        ret = kh_val(hashedclients, k);
    }

UNLOCK:
    HashingUnlock();

    return ret;
}

void
delclienthash(XCBWindow key)
{
    HashingLock();

    /* no SIGSEV protection in khash so we must check ourselvs */
    if(!hashedclients)
    {   goto UNLOCK;
    }

    khint_t k = kh_get(__CLIENTS__, hashedclients, key);

    if(k != kh_end(hashedclients))
    {   kh_del(__CLIENTS__, hashedclients, k);
    }
UNLOCK:
    HashingUnlock();
}

void
setupclienthash(void)
{   
    hashedclients = kh_init(__CLIENTS__);
}
