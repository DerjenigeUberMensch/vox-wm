#ifndef __VX__SHAPES__H__
#define __VX__SHAPES__H__

#include "../bitlist/freelist.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX
#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#endif

#ifndef MIN
#define MIN(A, B)               ((A) < (B) ? (A) : (B))
#endif

#ifndef __intersect_area
#define __intersect_area(x1_start, y1_start, x1_end, y1_end, x2_start, y2_start, x2_end, y2_end) \
    ((MAX(0, MIN(x1_end, x2_end) - MAX(x1_start, x2_start))) * (MAX(0, MIN(y1_end, y2_end) - MAX(y1_start, y2_start))))
#endif

typedef struct VXRegion VXRegion;

struct
VXRegion
{
    uint32_t width;
    uint32_t height;

    uint32_t nx;
    uint32_t ny;

    FreeList grid;
};

VXRegion *VXRegionCreate(uint64_t width, uint64_t height, double SAMPLING_RATE);
int VXRegionCreateFilled(VXRegion *fill_return, uint32_t width, uint32_t height, double SAMPLING_RATE);
int VXRegionResize(VXRegion *region, uint32_t width, uint32_t height, double SAMPLING_RATE);
int VXRegionReserve(VXRegion *region, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
int VXRegionRelease(VXRegion *region, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
bool VXRegionIsUsed(VXRegion *region, uint32_t x, uint32_t y);
bool VXRegionAreaIsUsedAtAll(VXRegion *region, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
bool VXRegionAreaIsUsed(VXRegion *region, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void VXRegionClear(VXRegion *region);
void VXRegionDebugPrint(VXRegion *region);
void VXRegionDestroy(VXRegion *region);



#ifdef __cplusplus
}
#endif
#endif
