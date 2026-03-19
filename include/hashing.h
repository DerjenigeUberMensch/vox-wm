#ifndef _HASHING_HELPER_H
#define _HASHING_HELPER_H

#include "../tools/XCB-TRL/xcb_trl.h"
#include "util.h"

struct Client;

/* Add Client to hashmap 
 *
 * RETURN: EXIT_SUCCESS on Success,
 * RETURN: EXIT_FAILURE on Failure.
 */
int NonNull addclienthash(struct Client *c, XCBWindow key);
/* Delete Client hashmap */
void cleanupclienthash(void);
/* Get Client * from window */
struct Client *getclienthash(XCBWindow window);
/* Remove Client from hashmap */
void delclienthash(XCBWindow key);
/* Setup Client hashmap */
void setupclienthash(void);

#endif
