#ifndef __VOX__WM__LUA__H__
#define __VOX__WM__LUA__H__

#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"

#include "safebool.h"

enum 
LuaInstrLimit
{ 
    LuaInstrLimitMax = 100000,
    LuaInstrLimitIncrement = 1000,
};


/* Initialize lua.
 *
 * NOTE: This function may block.
 */
int InitLua(void);
/*  Checks if the LUA thread is running. */
bool LuaIsThreadRunning(void);
bool LuaIsDead(void);
/* Attemptes to startup the the LUA thread for execution.
 *
 * NOTE: This function does not GURANTEE that the thread is running, must use LuaIsThreadRunning() to check.
 *
 * Returns EXIT_SUCCESS if the thread was started successfully pr already started.
 * Returns EXIT_FAILURE if the thread failed to start.
*/
int LuaRunThread(void);
/* Halt the lua thread (if its running).
 *
 * NOTE: This function may block.
 */
int LuaRequestHalt(void);
/* Resume the lua thread (if its running).
 *
 * NOTE: This function does NOT block.
*/
int LuaResumeAfterHalt(void);
/* Cleanup lua resources.
 *
 * NOTE: This function may block.
 */
void DestroyLua(void);


#endif
