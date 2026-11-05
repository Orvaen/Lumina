//========================================================================
// Lumina Wayland - fork of GLFW 3.4 (www.glfw.org)
//------------------------------------------------------------------------
// Copyright (c) 2014 Jonas Ådahl <jadahl@gmail.com>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
// It is fine to use C99 in this file because it will not be built with VS
//========================================================================

#include "internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "wayland-client-protocol.h"


static void outputHandleGeometry(void* data,
                                 struct wl_output* output,
                                 int32_t x,
                                 int32_t y,
                                 int32_t physicalWidth,
                                 int32_t physicalHeight,
                                 int32_t subpixel,
                                 const char* make,
                                 const char* model,
                                 int32_t transform)
{
    struct _LUMmonitor *monitor = data;

    monitor->wl.x = x;
    monitor->wl.y = y;
    monitor->widthMM = physicalWidth;
    monitor->heightMM = physicalHeight;

    snprintf(monitor->name, sizeof(monitor->name), "%s %s", make, model);
}

static void outputHandleMode(void* data,
                             struct wl_output* output,
                             uint32_t flags,
                             int32_t width,
                             int32_t height,
                             int32_t refresh)
{
    struct _LUMmonitor *monitor = data;
    LUMvidmode mode;

    mode.width = width;
    mode.height = height;
    mode.redBits = 8;
    mode.greenBits = 8;
    mode.blueBits = 8;
    mode.refreshRate = (int) round(refresh / 1000.0);

    monitor->modeCount++;
    monitor->modes =
        _lum_realloc(monitor->modes, monitor->modeCount * sizeof(LUMvidmode));
    monitor->modes[monitor->modeCount - 1] = mode;

    if (flags & WL_OUTPUT_MODE_CURRENT)
        monitor->wl.currentMode = monitor->modeCount - 1;
}

static void outputHandleDone(void* data, struct wl_output* output)
{
    struct _LUMmonitor *monitor = data;

    if (monitor->widthMM <= 0 || monitor->heightMM <= 0)
    {
        // If Wayland does not provide a physical size, assume the default 96 DPI
        const LUMvidmode* mode = &monitor->modes[monitor->wl.currentMode];
        monitor->widthMM  = (int) (mode->width * 25.4f / 96.f);
        monitor->heightMM = (int) (mode->height * 25.4f / 96.f);
    }

    _lumInputMonitor(monitor, LUM_CONNECTED, _LUM_INSERT_LAST);
}

static void outputHandleScale(void* data,
                              struct wl_output* output,
                              int32_t factor)
{
    struct _LUMmonitor *monitor = data;

    monitor->wl.scale = factor;
}

static const struct wl_output_listener outputListener = {
    outputHandleGeometry,
    outputHandleMode,
    outputHandleDone,
    outputHandleScale,
};


//////////////////////////////////////////////////////////////////////////
//////                       Lumina internal API                      //////
//////////////////////////////////////////////////////////////////////////

void _lumAddOutputWayland(uint32_t name, uint32_t version)
{
    _LUMmonitor *monitor;
    struct wl_output *output;

    if (version < 2)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Unsupported output interface version");
        return;
    }

    // The actual name of this output will be set in the geometry handler.
    monitor = _lumAllocMonitor("", 0, 0);

    output = wl_registry_bind(_lum.wl.registry,
                              name,
                              &wl_output_interface,
                              2);
    if (!output)
    {
        _lumFreeMonitor(monitor);
        return;
    }

    monitor->wl.scale = 1;
    monitor->wl.output = output;
    monitor->wl.name = name;

    wl_output_add_listener(output, &outputListener, monitor);
}


//////////////////////////////////////////////////////////////////////////
//////                       Lumina platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _lumFreeMonitorWayland(_LUMmonitor* monitor)
{
    if (monitor->wl.output)
        wl_output_destroy(monitor->wl.output);
}

void _lumGetMonitorPosWayland(_LUMmonitor* monitor, int* xpos, int* ypos)
{
    if (xpos)
        *xpos = monitor->wl.x;
    if (ypos)
        *ypos = monitor->wl.y;
}

void _lumGetMonitorContentScaleWayland(_LUMmonitor* monitor,
                                        float* xscale, float* yscale)
{
    if (xscale)
        *xscale = (float) monitor->wl.scale;
    if (yscale)
        *yscale = (float) monitor->wl.scale;
}

void _lumGetMonitorWorkareaWayland(_LUMmonitor* monitor,
                                    int* xpos, int* ypos,
                                    int* width, int* height)
{
    if (xpos)
        *xpos = monitor->wl.x;
    if (ypos)
        *ypos = monitor->wl.y;
    if (width)
        *width = monitor->modes[monitor->wl.currentMode].width;
    if (height)
        *height = monitor->modes[monitor->wl.currentMode].height;
}

LUMvidmode* _lumGetVideoModesWayland(_LUMmonitor* monitor, int* found)
{
    *found = monitor->modeCount;
    return monitor->modes;
}

void _lumGetVideoModeWayland(_LUMmonitor* monitor, LUMvidmode* mode)
{
    *mode = monitor->modes[monitor->wl.currentMode];
}

LUMbool _lumGetGammaRampWayland(_LUMmonitor* monitor, LUMgammaramp* ramp)
{
    _lumInputError(LUM_FEATURE_UNAVAILABLE,
                    "Wayland: Gamma ramp access is not available");
    return LUM_FALSE;
}

void _lumSetGammaRampWayland(_LUMmonitor* monitor, const LUMgammaramp* ramp)
{
    _lumInputError(LUM_FEATURE_UNAVAILABLE,
                    "Wayland: Gamma ramp access is not available");
}


//////////////////////////////////////////////////////////////////////////
//////                        Lumina native API                       //////
//////////////////////////////////////////////////////////////////////////

LUMAPI struct wl_output* lumGetWaylandMonitor(LUMmonitor* handle)
{
    _LUMmonitor* monitor = (_LUMmonitor*) handle;
    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    return monitor->wl.output;
}

