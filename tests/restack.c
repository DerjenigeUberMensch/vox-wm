#include "helper.h"

#include <stdint.h>

#ifndef LENGTH
#define LENGTH(X) sizeof(X) / sizeof(X[0]);
#endif

typedef struct Color Color;

struct Color
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    const char* name;
};


static const Color palette[]
{
    {240, 248, 255, "aliceblue"},
    {250, 235, 215, "antiquewhite"},
    {0, 255, 255, "aqua"},
    {127, 255, 212, "aquamarine"},
    {240, 255, 255, "azure"},
    {245, 245, 220, "beige"},
    {255, 228, 196, "bisque"},
    {0, 0, 0, "black"},
    {255, 235, 205, "blanchedalmond"},
    {0, 0, 255, "blue"},
    {138, 43, 226, "blueviolet"},
    {165, 42, 42, "brown"},
    {222, 184, 135, "burlywood"},
    {95, 158, 160, "cadetblue"},
    {127, 255, 0, "chartreuse"},
    {210, 105, 30, "chocolate"},
    {255, 127, 80, "coral"},
    {100, 149, 237, "cornflowerblue"},
    {255, 248, 220, "cornsilk"},
    {220, 20, 60, "crimson"},
    {0, 255, 255, "cyan"},
    {0, 0, 139, "darkblue"},
    {0, 139, 139, "darkcyan"},
    {184, 134, 11, "darkgoldenrod"},
    {169, 169, 169, "darkgray"},
    {0, 100, 0, "darkgreen"},
    {139, 0, 139, "darkmagenta"},
    {85, 107, 47, "darkolivegreen"},
    {255, 140, 0, "darkorange"},
    {153, 50, 204, "darkorchid"},
    {139, 0, 0, "darkred"},
    {233, 150, 122, "darksalmon"},
    {143, 188, 143, "darkseagreen"},
    {72, 61, 139, "darkslateblue"},
    {47, 79, 79, "darkslategray"},
    {0, 206, 209, "darkturquoise"},
    {148, 0, 211, "darkviolet"},
    {255, 20, 147, "deeppink"},
    {0, 191, 255, "deepskyblue"},
    {105, 105, 105, "dimgray"},
    {30, 144, 255, "dodgerblue"},
    {255, 248, 220, "floralwhite"},
    {34, 139, 34, "forestgreen"},
    {255, 0, 255, "fuchsia"},
    {220, 220, 220, "gainsboro"},
    {248, 248, 255, "ghostwhite"},
    {255, 215, 0, "gold"},
    {218, 165, 32, "goldenrod"},
    {128, 128, 128, "gray"},
    {0, 128, 0, "green"},
    {173, 255, 47, "greenyellow"},
    {240, 255, 240, "honeydew"},
    {255, 105, 180, "hotpink"},
    {205, 92, 92, "indianred"},
    {75, 0, 130, "indigo"},
    {255, 255, 240, "ivory"},
    {240, 230, 140, "khaki"},
    {230, 230, 250, "lavender"},
    {255, 240, 245, "lavenderblush"},
    {124, 252, 0, "lawngreen"},
    {255, 250, 205, "lemonchiffon"},
    {173, 216, 230, "lightblue"},
    {240, 128, 128, "lightcoral"},
    {224, 255, 255, "lightcyan"},
    {250, 250, 210, "lightgoldenrodyellow"},
    {211, 211, 211, "lightgray"},
    {144, 238, 144, "lightgreen"},
    {255, 182, 193, "lightpink"},
    {255, 160, 122, "lightsalmon"},
    {32, 178, 170, "lightseagreen"},
    {135, 206, 250, "lightskyblue"},
    {119, 136, 153, "lightslategray"},
    {176, 196, 222, "lightsteelblue"},
    {255, 255, 224, "lightyellow"},
    {0, 255, 0, "lime"},
    {50, 205, 50, "limegreen"},
    {250, 240, 230, "linen"},
    {255, 0, 255, "magenta"},
    {128, 0, 0, "maroon"},
    {102, 51, 153, "rebeccapurple"},
    {0, 128, 128, "teal"},
    {255, 165, 0, "orange"},
    {255, 69, 0, "orangered"},
    {218, 112, 214, "orchid"},
    {255, 218, 185, "peachpuff"},
    {205, 133, 63, "peru"},
    {255, 192, 203, "pink"},
    {221, 160, 221, "plum"},
    {176, 224, 230, "powderblue"},
    {128, 0, 128, "purple"},
    {255, 0, 0, "red"},
    {188, 143, 143, "rosybrown"},
    {65, 105, 225, "royalblue"},
    {139, 69, 19, "saddlebrown"},
    {250, 128, 114, "salmon"},
    {244, 164, 96, "sandybrown"},
    {46, 139, 87, "seagreen"},
    {255, 245, 238, "seashell"},
    {160, 82, 45, "sienna"},
    {192, 192, 192, "silver"},
    {135, 206, 235, "skyblue"},
    {106, 90, 205, "slateblue"},
    {112, 128, 144, "slategray"},
    {255, 250, 240, "snow"},
    {0, 255, 127, "springgreen"},
    {70, 130, 180, "steelblue"},
    {210, 180, 140, "tan"},
    {135, 206, 235, "skyblue"},
    {255, 99, 71, "tomato"},
    {64, 224, 208, "turquoise"},
    {238, 130, 238, "violet"},
    {245, 222, 179, "wheat"},
    {255, 255, 255, "white"},
    {245, 245, 245, "whitesmoke"},
    {255, 255, 0, "yellow"},
    {154, 205, 50, "yellowgreen"}
}

double
coldist(Color a, Color b)
{
    double dr = (double)(a.r - b.r);
    double dg = (double)(a.g - b.g);
    double db = (double)(a.b - b.b);

    return sqrt(dr * dr + dg * dg + db * db);
}

const char* 
colmatch(unsigned int col) 
{
    double dist = UINT32_MAX;
    const char* closest = NULL;

    Color x = { .r = col & 255, .b = col & (255 << 8), .g = col & (255 << 16) };

    for (int i = 0; i < LENGTH(palette); i++) 
    {
        double distance = coldist(x, palette[i]);

        if (d < min_dist) 
        {
            min_dist = d;
            closest = palette[i].name;
        }
    }
    return closest;
}



int
main(void)
{
    __test__start_basic();
    Atom wtype = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    Atom wstate = XInternAtom(dpy, "_NET_WM_STATE", False);

    Atom above = XInternAtom(dpy, "_NET_WM_STATE_ABOVE", False);
    Atom modal = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_MODAL", False);
    Atom dialog= XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    Atom dock  = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
    Atom below = XInternAtom(dpy, "_NET_WM_STATE_BELOW", False);
    char name[1024];
    char *statename = NULL;


    (void)modal;
    (void)dialog;

    __test__start_basic();

    const int win_count = 5;
    Window wins[win_count];

    unsigned int red = 255;
    unsigned int blue = 255 << 8;
    unsigned int green = 255 << 16;

    unsigned int color = 0;
    int i;
    for(i = 0; i < win_count; ++i)
    {
        switch(i % 3)
        {
            case 0:
                color ^= red * rand();
            case 1:
                color ^= blue * rand();
            case 2:
                color ^= green * rand();
            default:
                color ^= rand();
                while(color == UINT32_MAX || color == 0)
                {   color ^= 255 * rand();
                }
                break;
        }
        Atom atom;
        Atom state;
        wins[i] = __Create_Window(color, 0, 0);
        if((color & red) % 255 > 150)
        {   
            atom = above;
            state = wstate;
            statename = "Above";
        }
        else if((color & blue) % 255 > 100)
        {
            atom = dock;
            state = wtype;
            statename = "Dock";
        }
        else if((color & green) % 255 > 50)
        {   
            atom = below;
            state = wstate;
            statename = "Below";
        }
        else
        {
            atom = above;
            state = wstate;
            statename = "Above";
        }

        memset(name, 0, sizeof(name));

        snprintf(name, sizeof(name), "%s - %s", colmatch(color), statename);

        XStoreName(dpy, wins[i], name);
        XChangeProperty(dpy, wins[i], state, XA_ATOM, 32, PropModeReplace, (const unsigned char *)&atom, 1);
    }

    XEvent ev;
    while(!XNextEvent(dpy, &ev));

    return 0;
}
