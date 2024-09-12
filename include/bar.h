#ifndef BAR_H_
#define BAR_H_

#include <stdint.h>

/* struct externs */
struct Client;
struct Desktop;
struct Monitor;

enum 
BarSides
{
    BarSideLeft, 
    BarSideRight, 
    BarSideTop, 
    BarSideBottom,
    BarSideLAST,
};

/* Calculates which side the bar is currently on, in regards to geometry.
 * NOTE: get_prev, gets the previous side in which the bar was on, previously.
 */
enum BarSides calculatebarside(struct Monitor *m, struct Client *bar, uint8_t get_prev);
/* Checks given the provided information if a window is eligible to be a new bar.
 * if it is then it becomes the new bar.
 * RETURN: 0 on Success.
 * RETURN: 1 on no new bar (Failure).
*/
uint8_t checknewbar(struct Monitor *m, struct Client *c, uint8_t has_strut_or_strutp);
/* Sets up special data. */
void setupbar(struct Monitor *m, struct Client *c);
/* updates the bar geometry from the given monitor */
void updatebargeom(struct Monitor *m);
/* updates the Status Bar Position from given monitor */
void updatebarpos(struct Monitor *m);


#endif
