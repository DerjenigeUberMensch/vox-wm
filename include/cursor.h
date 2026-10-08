#ifndef __WM__CURSOR__H__
#define __WM__CURSOR__H__

#include <X11/cursorfont.h>

#include "main.h"

typedef enum WMCursorExtra WMCursorExtra;

enum WMCursorExtra
{
    WM_cursor_none = XC_num_glyphs,
    WM_cursor_START = WM_cursor_none,
    WM_cursor_default,
    WM_cursor_pointer,
    WM_cursor_text,
    WM_cursor_text_vert,
    WM_cursor_help,
    WM_cursor_context_menu,
    WM_cursor_progress,
    WM_cursor_not_allowed,
    WM_cursor_grab,
    WM_cursor_grabbing,
    WM_cursor_copy,
    WM_cursor_alias,
    WM_cursor_cell,
    WM_cursor_move,
    WM_cursor_all_scroll,
    WM_cursor_zoom_in,
    WM_cursor_zoom_out,
    WM_cursor_column_resize,
    WM_cursor_row_resize,
    WM_cursor_resize_horz,
    WM_cursor_resize_vert,
    WM_cursor_resize_diagonal_left,
    WM_cursor_resize_diagonal_right,
    WM_cursor_resize_edge_left,
    WM_cursor_resize_edge_right,
    WM_cursor_resize_edge_top,
    WM_cursor_resize_edge_bottom,
    WM_cursor_resize_corner_top_left,
    WM_cursor_resize_corner_top_right,
    WM_cursor_resize_corner_bottom_left,
    WM_cursor_resize_corner_bottom_right,
    WM_cursor_LAST,
};

/* init cursor data */
void InitCursors(void);
/* Get Cursor ID using x11 XC_glyps or WM cursor extra */
XCBCursor TryGetCursor(int cursor_index);
/* destroy cursor data */
void CursorsDestroy(void);

#endif