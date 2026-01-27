

#include <pthread.h>

#include "khash.h"
#include "xcb_trl.h"
#include "__private__xcb__utils__.h"
#include "__private__xcb__err__.h"
#include "safebool.h"

#ifdef XCB_TRL_ENABLE_DEBUG

KHASH_SET_INIT_STR(__KHASH__ERROR__FUNCTIONS__)
KHASH_MAP_INIT_INT(__KHASH__ERROR__LOGS__, khint_t)

static khash_t(__KHASH__ERROR__FUNCTIONS__) *_hashed_functions = NULL;
static khash_t(__KHASH__ERROR__LOGS__) *_hashed_logs = NULL;

/* khash is not thread safe */
static pthread_mutex_t _khash_mutex = PTHREAD_MUTEX_INITIALIZER;

#endif

void _xcb_trl_err_handler(xcb_connection_t *display, xcb_generic_error_t *error);
void  _xcb_handler(xcb_connection_t *display, xcb_generic_error_t *error);

static void (*_xcb_trl_handler)(xcb_connection_t *, xcb_generic_error_t *) = _xcb_handler;

/* error handling */

bool 
__XValidFormat(
        uint8_t x
        ) 
{
    return likely((x == 32 || x == 16 || x == 8)); 
}

bool 
__XValidSize(
        uint32_t value_len
        ) 
{
    enum { X_SERVER_SECURITY_CHECK_MAX_LEN = INT32_MAX >> 4 };

    return likely(value_len <= X_SERVER_SECURITY_CHECK_MAX_LEN); 
}

bool
__XValidReply(
        xcb_get_property_reply_t *reply
        )
{
    /*
     * Yes these will cause performance hicups if your program causes errors.
     * Yes this is intended.
     */

    /* Check is this a valid error? */
    if(unlikely(reply->sequence == XCB_NONE))
    {   return false;
    }


    if(unlikely(!__XValidFormat(reply->format)))
    {   return false;  
    }
    
    if(unlikely(!__XValidSize(reply->length)))
    {   return false;
    }

    return true;
}

bool
__XIsEmptyReply(
        xcb_get_property_reply_t *reply
	)
{
    /* Check is this a valid 'Empty' reply? */
    return unlikely(reply->type == XCB_NONE && reply->value_len == 0);
}

void *
__XValidateReply(
        xcb_connection_t *display,
        xcb_get_property_reply_t *reply,
        xcb_generic_error_t *error
        )
{
    if(reply)
    {
	/* Empty replies are allowed */
	if(__XIsEmptyReply(reply))
	{   return reply;
	}
	if(__XValidReply(reply))
	{   return reply;
	}
    }

    /* suprisingly both can return NULL, 
     * further investigation is required to determine this behaviour though. 
     */
    if(error)
    {   _xcb_trl_err_handler(display, (void *)error);
    }
    else if(reply)
    {   _xcb_trl_err_handler(display, (void *)reply);
    }

    return NULL;
}

void *
__XCheckReply(
        xcb_connection_t *display,
        xcb_get_property_reply_t *reply,
        xcb_generic_error_t *error
        )
{
    if(reply && __XValidReply(reply))
    {   return reply;
    }

    /* suprisingly both can return NULL, 
     * further investigation is required to determine this behaviour though. 
     */

    if(error)
    {   _xcb_trl_handler(display, error);
    }
    else if(reply)
    {   _xcb_trl_handler(display, (xcb_generic_error_t *)reply);
    }

    return NULL;
}

void
__XCBThrowError(
        xcb_connection_t *display,
        xcb_void_cookie_t cookie, 
        uint8_t error, 
        uint8_t major_code, 
        uint16_t minor_code
        )
{
    xcb_generic_error_t _err;
    memset(&_err, 0, sizeof(xcb_generic_error_t));
    _err.error_code = error;
    _err.major_code = major_code;
    _err.minor_code = minor_code;
    _err.response_type = XCB_NONE;
    _err.full_sequence = cookie.sequence;
    _err.sequence = cookie.sequence;

    _xcb_trl_handler(display, &_err);
}

void
__XCBSetErrorHandler(
        void (*error_handler)(
            XCBDisplay *, 
            XCBGenericError *
            )
        )
{
    if(error_handler)
    {   _xcb_trl_handler = error_handler;
    }
    else
    {   _xcb_trl_handler = _xcb_handler;
    }
}

/* atexit() is not multithreaded */
void 
__atexit_xcb_handler(void)
{
#ifdef XCB_TRL_ENABLE_DEBUG

    if(_hashed_logs)
    {   kh_destroy(__KHASH__ERROR__LOGS__, _hashed_logs);
    }

    if(_hashed_functions)
    {   kh_destroy(__KHASH__ERROR__FUNCTIONS__, _hashed_functions);
    }

    _hashed_logs = NULL;
    _hashed_functions = NULL;

    /* ensure no deadlocks occur */

    int ret = pthread_mutex_trylock(&_khash_mutex);
    pthread_mutex_unlock(&_khash_mutex);

    (void)ret;
#endif
}

void 
_xcb_handler(
        xcb_connection_t *display, 
        xcb_generic_error_t *error
        )
{
#ifdef XCB_TRL_ENABLE_DEBUG

    uint32_t sequence = error->sequence;
    khint_t k;

    pthread_mutex_lock(&_khash_mutex);

    if(!_hashed_logs || !_hashed_functions)
    {   goto UNLOCK;
    }

    k = kh_get(__KHASH__ERROR__LOGS__, _hashed_logs, sequence);

    if(unlikely(k == kh_end(_hashed_logs)))
    {   goto UNLOCK;
    }

    k = kh_val(_hashed_logs, k);

    _XCB_MANUAL_DEBUG("%s", kh_key(_hashed_functions, k));

UNLOCK:
    pthread_mutex_unlock(&_khash_mutex);
#endif
    XCBBreakPoint();
}

void
_xcb_trl_err_handler(
        xcb_connection_t *display, 
        xcb_generic_error_t *error
        )
{
    _xcb_trl_handler(display, error);

    free(error);
}


void XCBBreakPoint(void) { volatile int *e = 0; if(e != (volatile int *)1) { e = (volatile int *)3; } (void)e; }

void
XCBDefaultHandlerMsg(
        XCBDisplay *display,
        XCBGenericError *error
        )
{
    _xcb_handler(display, error);
}

void 
XCBDebugPushID(
        const char *const function_name, 
        uint32_t sequence
        )
{
#ifdef XCB_TRL_ENABLE_DEBUG

    /* Due to this being an implementation detail we can push this later down development,
     * Anayways we ignore 0 sequences as they are technically not implemented yet,
     * if they were implemented then we would just use the function_name as a call stack order
     * rather than the sequence of 0, but we removed call stack backtrace since last major release.
     */
    if(!sequence)
    {   return;
    }

    pthread_mutex_lock(&_khash_mutex);

    if(!_hashed_logs)
    {   
        _hashed_logs = kh_init(__KHASH__ERROR__LOGS__);
        if(_hashed_logs)
        {   
            /* this could fail but would only result in memory leak which isnt that important */
            int status = atexit(__atexit_xcb_handler);

            if(unlikely(status))
            {   fprintf(stderr, "XCB error handler failed to set atexit(), this may result in a MEMORY_LEAK\n");
            }
        }
    }

    if(!_hashed_logs)
    {   goto UNLOCK;
    }

    if(!_hashed_functions)
    {   _hashed_functions = kh_init(__KHASH__ERROR__FUNCTIONS__);
    }

    if(!_hashed_functions)
    {   goto UNLOCK;
    }

    enum
    {
        __KHASH_BAD_OPERATION = -1,
        __KHASH_ALREADY_PRESENT = 0,
        __KHASH_FIRST_HASH = 1,
        __KHASH_PREVIOUSLY_DELETED = 2,
    };

    khint_t k = kh_get(__KHASH__ERROR__FUNCTIONS__, _hashed_functions, function_name);
    int err = 0;

    /* Add function if not found. */
    if(k == kh_end(_hashed_functions))
    {   
        k = kh_put(__KHASH__ERROR__FUNCTIONS__, _hashed_functions, function_name, &err);
        /* Failed to add function */
        if(err == __KHASH_BAD_OPERATION)
        {   goto UNLOCK;
        }
    }

    khint_t func_it = k;

    k = kh_put(__KHASH__ERROR__LOGS__, _hashed_logs, sequence, &err);

    if(unlikely(err == __KHASH_BAD_OPERATION))
    {   goto UNLOCK;
    }

    /* bounds check */
    if(likely(kh_end(_hashed_logs) > k))
    {   kh_value(_hashed_logs, k) = func_it;
    }
UNLOCK:
    pthread_mutex_unlock(&_khash_mutex);
#endif
}
