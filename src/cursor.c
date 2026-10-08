#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <xcb/xcb_cursor.h>

#include "cursor.h"
#include "XCB-TRL/xcb_trl.h"

extern WM _wm;

XCBCursor x11_cursors[XC_num_glyphs];
XCBCursor x11_modern_cursors[XC_num_glyphs];
XCBCursor wm_cursors[WM_cursor_LAST - WM_cursor_START];

static const char *const xc_cursor_names[XC_num_glyphs] = 
{
    [XC_X_cursor]             = "X_cursor",
    [XC_arrow]                = "arrow",
    [XC_based_arrow_down]     = "based_arrow_down",
    [XC_based_arrow_up]       = "based_arrow_up",
    [XC_boat]                 = "boat",
    [XC_bogosity]             = "bogosity",
    [XC_bottom_left_corner]   = "bottom_left_corner",
    [XC_bottom_right_corner]  = "bottom_right_corner",
    [XC_bottom_side]          = "bottom_side",
    [XC_bottom_tee]           = "bottom_tee",
    [XC_box_spiral]           = "box_spiral",
    [XC_center_ptr]           = "center_ptr",
    [XC_circle]               = "circle",
    [XC_clock]                = "clock",
    [XC_coffee_mug]           = "coffee_mug",
    [XC_cross]                = "cross",
    [XC_cross_reverse]        = "cross_reverse",
    [XC_crosshair]            = "crosshair",
    [XC_diamond_cross]        = "diamond_cross",
    [XC_dot]                  = "dot",
    [XC_dotbox]               = "dotbox",
    [XC_double_arrow]         = "double_arrow",
    [XC_draft_large]          = "draft_large",
    [XC_draft_small]          = "draft_small",
    [XC_draped_box]           = "draped_box",
    [XC_exchange]             = "exchange",
    [XC_fleur]                = "fleur",
    [XC_gobbler]              = "gobbler",
    [XC_gumby]                = "gumby",
    [XC_hand1]                = "hand1",
    [XC_hand2]                = "hand2",
    [XC_heart]                = "heart",
    [XC_icon]                 = "icon",
    [XC_iron_cross]           = "iron_cross",
    [XC_left_ptr]             = "left_ptr",
    [XC_left_side]            = "left_side",
    [XC_left_tee]             = "left_tee",
    [XC_leftbutton]           = "leftbutton",
    [XC_ll_angle]             = "ll_angle",
    [XC_lr_angle]             = "lr_angle",
    [XC_man]                  = "man",
    [XC_middlebutton]         = "middlebutton",
    [XC_mouse]                = "mouse",
    [XC_pencil]               = "pencil",
    [XC_pirate]               = "pirate",
    [XC_plus]                 = "plus",
    [XC_question_arrow]       = "question_arrow",
    [XC_right_ptr]            = "right_ptr",
    [XC_right_side]           = "right_side",
    [XC_right_tee]            = "right_tee",
    [XC_rightbutton]          = "rightbutton",
    [XC_rtl_logo]             = "rtl_logo",
    [XC_sailboat]             = "sailboat",
    [XC_sb_down_arrow]        = "sb_down_arrow",
    [XC_sb_h_double_arrow]    = "sb_h_double_arrow",
    [XC_sb_left_arrow]        = "sb_left_arrow",
    [XC_sb_right_arrow]       = "sb_right_arrow",
    [XC_sb_up_arrow]          = "sb_up_arrow",
    [XC_sb_v_double_arrow]    = "sb_v_double_arrow",
    [XC_shuttle]              = "shuttle",
    [XC_sizing]               = "sizing",
    [XC_spider]               = "spider",
    [XC_spraycan]             = "spraycan",
    [XC_star]                 = "star",
    [XC_target]               = "target",
    [XC_tcross]               = "tcross",
    [XC_top_left_arrow]       = "top_left_arrow",
    [XC_top_left_corner]      = "top_left_corner",
    [XC_top_right_corner]     = "top_right_corner",
    [XC_top_side]             = "top_side",
    [XC_top_tee]              = "top_tee",
    [XC_trek]                 = "trek",
    [XC_ul_angle]             = "ul_angle",
    [XC_umbrella]             = "umbrella",
    [XC_ur_angle]             = "ur_angle",
    [XC_watch]                = "watch",
    [XC_xterm]                = "xterm",
};

static const char *const wm_cursor_names[WM_cursor_LAST - WM_cursor_START] =
{
    [WM_cursor_none                         - WM_cursor_START]  = NULL,
    [WM_cursor_default                      - WM_cursor_START]  = "default",
    [WM_cursor_pointer                      - WM_cursor_START]  = "pointer",
    [WM_cursor_text                         - WM_cursor_START]  = "text",
    [WM_cursor_text_vert                    - WM_cursor_START]  = "vertical-text",
    [WM_cursor_help                         - WM_cursor_START]  = "help",
    [WM_cursor_context_menu                 - WM_cursor_START]  = "context-menu",
    [WM_cursor_progress                     - WM_cursor_START]  = "progress",
    [WM_cursor_not_allowed                  - WM_cursor_START]  = "not-allowed",
    [WM_cursor_grab                         - WM_cursor_START]  = "grab",
    [WM_cursor_grabbing                     - WM_cursor_START]  = "grabbing",
    [WM_cursor_copy                         - WM_cursor_START]  = "copy",
    [WM_cursor_alias                        - WM_cursor_START]  = "alias",
    [WM_cursor_cell                         - WM_cursor_START]  = "cell",
    [WM_cursor_move                         - WM_cursor_START]  = "move",
    [WM_cursor_all_scroll                   - WM_cursor_START]  = "all-scroll",
    [WM_cursor_zoom_in                      - WM_cursor_START]  = "zoom-in",
    [WM_cursor_zoom_out                     - WM_cursor_START]  = "zoom-out",
    [WM_cursor_column_resize                - WM_cursor_START]  = "col-resize",
    [WM_cursor_row_resize                   - WM_cursor_START]  = "row-resize",
    [WM_cursor_resize_horz                  - WM_cursor_START]  = "ew-resize",          /* east -> west */
    [WM_cursor_resize_vert                  - WM_cursor_START]  = "ns-resize",          /* north -> south*/
    [WM_cursor_resize_diagonal_left         - WM_cursor_START]  = "nwse-resize",        /* north west -> south east */
    [WM_cursor_resize_diagonal_right        - WM_cursor_START]  = "nesw-resize",        /* north east -> south west */
    [WM_cursor_resize_edge_left             - WM_cursor_START]  = "w-resize",           /* west */  
    [WM_cursor_resize_edge_right            - WM_cursor_START]  = "e-resize",           /* east */
    [WM_cursor_resize_edge_top              - WM_cursor_START]  = "n-resize",           /* north */
    [WM_cursor_resize_edge_bottom           - WM_cursor_START]  = "s-resize",           /* south */
    [WM_cursor_resize_corner_top_left       - WM_cursor_START]  = "nw-resize",          /* north west */
    [WM_cursor_resize_corner_top_right      - WM_cursor_START]  = "ne-resize",          /* north east */
    [WM_cursor_resize_corner_bottom_left    - WM_cursor_START]  = "sw-resize",          /* south west */
    [WM_cursor_resize_corner_bottom_right   - WM_cursor_START]  = "se-resize"           /* south east */
};

enum { CURSOR_KICK_UP_MAX = 4 };

static const int wm_cursor_kick_up[WM_cursor_LAST - WM_cursor_START][CURSOR_KICK_UP_MAX] =
{
    [WM_cursor_default                      - WM_cursor_START]  = { XC_left_ptr, -1 },
    [WM_cursor_pointer                      - WM_cursor_START]  = { XC_hand2, XC_left_ptr, -1 },
    [WM_cursor_text                         - WM_cursor_START]  = { XC_xterm, -1 },
    [WM_cursor_text_vert                    - WM_cursor_START]  = { XC_xterm, -1 },
    [WM_cursor_help                         - WM_cursor_START]  = { XC_question_arrow, XC_left_ptr, -1 },
    [WM_cursor_context_menu                 - WM_cursor_START]  = { XC_left_ptr, -1 },
    [WM_cursor_progress                     - WM_cursor_START]  = { XC_watch, XC_clock, -1 },
    [WM_cursor_not_allowed                  - WM_cursor_START]  = { XC_circle, -1 },
    [WM_cursor_grab                         - WM_cursor_START]  = { XC_hand1, XC_hand2, -1 },
    [WM_cursor_grabbing                     - WM_cursor_START]  = { XC_hand2, XC_hand1, -1 },
    [WM_cursor_copy                         - WM_cursor_START]  = { XC_left_ptr, -1},
    [WM_cursor_alias                        - WM_cursor_START]  = { XC_left_ptr, -1 },
    [WM_cursor_cell                         - WM_cursor_START]  = { XC_crosshair, XC_cross, -1 },
    [WM_cursor_move                         - WM_cursor_START]  = { XC_fleur, XC_sizing, -1, },
    [WM_cursor_all_scroll                   - WM_cursor_START]  = { XC_fleur, -1, },
    [WM_cursor_zoom_in                      - WM_cursor_START]  = { XC_crosshair, XC_target, -1 },
    [WM_cursor_zoom_out                     - WM_cursor_START]  = { XC_crosshair, XC_target, -1 },
    [WM_cursor_column_resize                - WM_cursor_START]  = { XC_sb_h_double_arrow, XC_sizing, -1 },
    [WM_cursor_row_resize                   - WM_cursor_START]  = { XC_sb_v_double_arrow, XC_sizing, -1 },
    [WM_cursor_resize_horz                  - WM_cursor_START]  = { XC_sb_h_double_arrow, XC_sizing, -1 },
    [WM_cursor_resize_vert                  - WM_cursor_START]  = { XC_sb_v_double_arrow, XC_sizing, -1 },
    [WM_cursor_resize_diagonal_left         - WM_cursor_START]  = { XC_bottom_right_corner, XC_top_left_corner, -1 },
    [WM_cursor_resize_diagonal_right        - WM_cursor_START]  = { XC_top_right_corner, XC_bottom_left_corner, -1 },
    [WM_cursor_resize_edge_left             - WM_cursor_START]  = { XC_left_side, XC_sb_h_double_arrow, -1 },
    [WM_cursor_resize_edge_right            - WM_cursor_START]  = { XC_right_side, XC_sb_h_double_arrow, -1 },
    [WM_cursor_resize_edge_top              - WM_cursor_START]  = { XC_top_side, XC_sb_v_double_arrow, -1 },
    [WM_cursor_resize_edge_bottom           - WM_cursor_START]  = { XC_bottom_side, XC_sb_v_double_arrow, -1 },
    [WM_cursor_resize_corner_top_left       - WM_cursor_START]  = { XC_top_left_corner, XC_sizing, -1 },
    [WM_cursor_resize_corner_top_right      - WM_cursor_START]  = { XC_top_right_corner, XC_sizing, -1 },
    [WM_cursor_resize_corner_bottom_left    - WM_cursor_START]  = { XC_bottom_left_corner, XC_sizing, -1 },
    [WM_cursor_resize_corner_bottom_right   - WM_cursor_START]  = { XC_bottom_right_corner, XC_sizing, -1 }
};

void
InitCursors(void)
{
    int i;
    int status;

    XCBCursorContext *cursorctx = NULL;

    status = XCBCursorContextNew(_wm.dpy, _wm.screen, &cursorctx);

    /* havent actualy read up on this code of xcb so here just in case. */
    if(status)
    {   cursorctx = NULL;
    }

    for(i = 0; i < XC_num_glyphs; ++i)
    {
        /* prevent using the mask cursors */
        x11_cursors[i] = 0;
        x11_modern_cursors[i] = 0;

        /* X has only even glpys
         * odd numberbed gylps ie XC_arrow + 1 or 3 are simply the mask for the previous gylph in this case it would mask for XC_arrow
         */
        if(i & 1)
        {   continue;
        }

        /* since XCB already handles that with cursors we skip the mask*/
        x11_cursors[i] = XCBCreateFontCursor(_wm.dpy, i);

        if(cursorctx)
        {   x11_modern_cursors[i] = XCBCursorLoadCursor(_wm.dpy, cursorctx, xc_cursor_names[i]);
        }
    }

    for(i = 0; i < LENGTH(wm_cursor_names); ++i)
    {
        wm_cursors[i] = 0;

        if(cursorctx && wm_cursor_names[i])
        {   wm_cursors[i] = XCBCursorLoadCursor(_wm.dpy, cursorctx, wm_cursor_names[i]);
        }
    }

    if(cursorctx)
    {   XCBCursorContextFree(cursorctx);
    }
}


XCBCursor 
TryGetCursor(int cursor_index)
{
    if(cursor_index == WM_cursor_none)
    {   return XCBNone;
    }

    if(cursor_index < 0)
    {   
        DebugWarn("Invalid cursor index: %d", cursor_index);
        return XCBNone;
    }

    if(cursor_index > WM_cursor_none)
    {
        if(cursor_index >= WM_cursor_LAST)
        {   
            DebugWarn("Cursor index out of bounds: %d", cursor_index);
            return XCBNone;
        }

        if(wm_cursors[cursor_index - WM_cursor_START])
        {   return wm_cursors[cursor_index - WM_cursor_START];
        }
        else
        {
            int i;
            const int *cursor_check = wm_cursor_kick_up[cursor_index - WM_cursor_START];
            XCBCursor cur;

            for(i = 0; i < CURSOR_KICK_UP_MAX; i++)
            {
                if(cursor_check[i] == -1)
                {   break;
                }

                cur = TryGetCursor(cursor_check[i]);

                if(cur != XCBNone)
                {   
                    if(cur > 0)
                    {   DebugWarn("Failed to kick up %d times", i);
                    }

                    DebugWarn("Using fallback cursor: %d", cursor_check[i]);
                    return cur;
                }
            }
        }


        return XCBNone;
    }

    if(x11_modern_cursors[cursor_index])
    {   return x11_modern_cursors[cursor_index];
    }

    if(x11_cursors[cursor_index])
    {   
        if(xc_cursor_names[cursor_index])
        {   DebugWarn("Using legacy fallback cursor for %s", xc_cursor_names[cursor_index]);
        }

        return x11_cursors[cursor_index];
    }

    DebugError("Failed to get cursor for index %d", cursor_index);

    return XCBNone;
}

void
CursorsDestroy(void)
{
    int i;

    (void)ASSERT(LENGTH(x11_cursors) == XC_num_glyphs);
    (void)ASSERT(LENGTH(x11_modern_cursors) == XC_num_glyphs);
    (void)ASSERT(LENGTH(wm_cursors) == WM_cursor_LAST - WM_cursor_START);

    for(i = 0; i < XC_num_glyphs; ++i) 
    {   
        if(x11_cursors[i] != 0)
        {   XCBFreeCursor(_wm.dpy, x11_cursors[i]); 
        }
        if(x11_modern_cursors[i] != 0)
        {   XCBFreeCursor(_wm.dpy, x11_modern_cursors[i]); 
        }
    }

    for(i = 0; i < LENGTH(wm_cursors); ++i)
    {
        if(wm_cursors[i] != 0)
        {   XCBFreeCursor(_wm.dpy, wm_cursors[i]); 
        }
    }
}
