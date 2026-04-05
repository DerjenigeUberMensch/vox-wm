#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "shapes.h"

static bool 
rect_intersects(
    int64_t x1, int64_t y1, int64_t w1, int64_t h1,
    int64_t x2, int64_t y2, int64_t w2, int64_t h2
) {
    return x1 < x2 + w2 &&
           x1 + w1 > x2 &&
           y1 < y2 + h2 &&
           y1 + h1 > y2;
}

VXRegion *
VXRegionCreate(uint64_t width, uint64_t height, double SAMPLING_RATE)
{
    VXRegion *grid = malloc(sizeof(VXRegion));

    if(grid)
    {
        int status;

        status = VXRegionCreateFilled(grid, width, height, SAMPLING_RATE);

        if(status == EXIT_FAILURE)
        {
            free(grid);
            grid = NULL;
        }
    }

    return grid;
}

int
VXRegionCreateFilled(VXRegion *fill_return, uint32_t width, uint32_t height, double SAMPLING_RATE)
{
    if(!fill_return)
    {   return EXIT_FAILURE;
    }

    if(SAMPLING_RATE <= 0.0)
    {   SAMPLING_RATE = 1.0;
    }

    uint32_t nx = (uint32_t)ceil((double)width * SAMPLING_RATE) + 1;
    uint32_t ny = (uint32_t)ceil((double)height * SAMPLING_RATE) + 1;

    int status;

    fill_return->nx = nx;
    fill_return->ny = ny;
    fill_return->width = width;
    fill_return->height = height;

    status = FreeListCreateFill(&fill_return->grid, (fill_return->nx * fill_return->ny) / 8 + 1);

    if(status == EXIT_FAILURE)
    {   return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int
VXRegionResize(VXRegion *region, uint32_t width, uint32_t height, double SAMPLING_RATE)
{
    if(!region)
    {   return EXIT_FAILURE;
    }

    VXRegion cpy;
    int status;

    status = VXRegionCreateFilled(&cpy, width, height, SAMPLING_RATE);

    if(status == EXIT_SUCCESS)
    {   
        VXRegionDestroy(region);
        memcpy(region, &cpy, sizeof(VXRegion));
    }

    return status;
}

static int
VX_REGION_RESERVE_RELEASE(bool is_reserve, VXRegion *region, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    if(!region)
    {   return EXIT_FAILURE;
    }

    uint32_t cx = 0;
    uint32_t cy = 0;
    uint32_t cw = region->width;
    uint32_t ch = region->height;

    if(!rect_intersects(x, y, width, height, cx, cy, cw, ch))
    {   return EXIT_SUCCESS;
    }

    width  = MIN(width, cw);
    height = MIN(height, ch);

    uint32_t rx = x;
    uint32_t ry = y;
    uint32_t rw = MAX(MIN(width,  cw - rx), 1);
    uint32_t rh = MAX(MIN(height, ch - ry), 1);

    /* scale pixels to grid */
    double sx = (double)region->nx / region->width;
    double sy = (double)region->ny / region->height;

    uint32_t gx = (uint32_t)(rx * sx);
    uint32_t gy = (uint32_t)(ry * sy);

    uint32_t gw = (uint32_t)ceil(rw * sx);
    uint32_t gh = (uint32_t)ceil(rh * sy);

    /* clamp to grid */
    uint32_t maxx = MIN(gx + gw, region->nx);
    uint32_t maxy = MIN(gy + gh, region->ny);

    for(uint32_t cordy = gy; cordy < maxy; ++cordy)
    {
        uint64_t row_start = (uint64_t)cordy * region->nx;

        for(uint32_t cordx = gx; cordx < maxx; ++cordx)
        {
            uint64_t i = row_start + cordx;

            if(is_reserve)
            {   FreeListSetUsed(&region->grid, i);
            }
            else
            {   FreeListSetFree(&region->grid, i);
            }
        }
    }

    return EXIT_SUCCESS;
}

int
VXRegionReserve(VXRegion *region, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{   return VX_REGION_RESERVE_RELEASE(true, region, x, y, width, height);
}

int
VXRegionRelease(VXRegion *region, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{   return VX_REGION_RESERVE_RELEASE(false, region, x, y, width, height);
}

bool 
VXRegionIsUsed(VXRegion *region, uint32_t x, uint32_t y)
{   return VXRegionAreaIsUsed(region, x, y, 1, 1);
}

bool
VXRegionAreaIsUsedAtAll(VXRegion *region, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    if(!region)
    {   return false;
    }

    double sx = (double)region->nx / region->width;
    double sy = (double)region->ny / region->height;

    uint32_t gx = (uint32_t)(x * sx);
    uint32_t gy = (uint32_t)(y * sy);
    uint32_t gw = (uint32_t)ceil(width * sx);
    uint32_t gh = (uint32_t)ceil(height * sy);

    uint32_t maxx = MIN(gx + gw, region->nx);
    uint32_t maxy = MIN(gy + gh, region->ny);

    for(uint32_t cy = gy; cy < maxy; ++cy)
    {
        uint64_t row = (uint64_t)cy * region->nx;

        for(uint32_t cx = gx; cx < maxx; ++cx)
        {
            if(!FreeListIsFree(&region->grid, row + cx))
            {   return true;
            }
        }
    }

    return false;
}

bool
VXRegionAreaIsUsed(VXRegion *region, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    if(!region)
    {   return false;
    }

    double sx = (double)region->nx / region->width;
    double sy = (double)region->ny / region->height;

    uint32_t gx = (uint32_t)(x * sx);
    uint32_t gy = (uint32_t)(y * sy);
    uint32_t gw = (uint32_t)ceil(width * sx);
    uint32_t gh = (uint32_t)ceil(height * sy);

    uint32_t maxx = MIN(gx + gw, region->nx);
    uint32_t maxy = MIN(gy + gh, region->ny);

    for(uint32_t cy = gy; cy < maxy; ++cy)
    {
        uint64_t row = (uint64_t)cy * region->nx;

         uint64_t rowStart = (uint64_t)cy * region->nx;
        uint64_t startBit = rowStart + gx;
        uint64_t endBit   = rowStart + maxx;

        // Check the whole row segment using FreeList
        if (!FreeListIsFreeRange(&region->grid, startBit, endBit)) {
            return false;
        }
        for(uint32_t cx = gx; cx < maxx; ++cx)
        {
            if(FreeListIsFree(&region->grid, row + cx))
            {   return false;
            }
        }
    }

    return true;
}


void
VXRegionClear(VXRegion *region)
{
    if(!region)
    {   return;
    }

    FreeListSetFreeLen(&region->grid, 0, region->nx * region->ny);
}

void
VXRegionDebugPrint(VXRegion *region)
{
    for(uint32_t y = 0; y < region->ny; ++y)
    {
        for(uint32_t x = 0; x < region->nx; ++x)
        {
            uint64_t i = (uint64_t)y * region->nx + x;
            printf("%c", FreeListIsFree(&region->grid, i) ? '.'  : '#');
        }
        printf("\n");
    }
}

void
VXRegionDestroy(VXRegion *region)
{
    if(!region)
    {   return;
    }

    FreeListDestroyFilled(&region->grid);
}
