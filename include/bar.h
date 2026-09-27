#ifndef __BAR__H__
#define __BAR__H__

#include <stdint.h>

#include "desktop.h"
#include "util.h"


enum 
BarSides
{
    BarSideLeft, 
    BarSideRight, 
    BarSideTop, 
    BarSideBottom,
};



struct Client;
struct Desktop;
struct Monitor;


/* checks if a client is a bar */
uint32_t NonNull ISBAR(struct Client *c);
/* Gets the bar side in which it is relative to the screen */
enum BarSides NonNullAll GETBARSIDE(struct Monitor *m, struct Client *bar, uint8_t get_prev_side);
/* Sets up special data. */
void NonNullAll setupbar(struct Monitor *m, struct Client *bar);
/* updates the bar geometry from the given monitor */
void NonNull updatebargeom(struct Desktop *desk);
/* updates the Status Bar Position from given monitor */
void NonNull updatebarpos(struct Desktop *desk);
/* Updates all bar related stuff */
void NonNull updatebars(struct Desktop *desk);
/* Updates workarea for the dekstop mon*/
void NonNull updateworkarea(struct Desktop *desk);

#endif
