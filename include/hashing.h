#ifndef _HASHING_HELPER_H
#define _HASHING_HELPER_H

#include "../tools/XCB-TRL/xcb_trl.h"

struct Client;

/* Add Client to hashmap */
void addclienthash(struct Client *c);
/* Delete Client hashmap */
void cleanupclienthash(void);
/* Get Client * from window */
struct Client *getclienthash(XCBWindow window);
/* Remove Client from hashmap */
void delclienthash(struct Client *c);
/* Setup Client hashmap */
void setupclienthash(void);

#endif
