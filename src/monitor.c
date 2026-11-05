//========================================================================
// Lumina - fork of GLFW 3.4 (www.glfw.org)
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2019 Camilla Löwy <elmindreda@glfw.org>
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
// Please use C89 style variable declarations in this file because VS 2010
//========================================================================

#include "internal.h"

#include <assert.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>


// Lexically compare video modes, used by qsort
//
static int compareVideoModes(const void* fp, const void* sp)
{
    const LUMvidmode* fm = fp;
    const LUMvidmode* sm = sp;
    const int fbpp = fm->redBits + fm->greenBits + fm->blueBits;
    const int sbpp = sm->redBits + sm->greenBits + sm->blueBits;
    const int farea = fm->width * fm->height;
    const int sarea = sm->width * sm->height;

    // First sort on color bits per pixel
    if (fbpp != sbpp)
        return fbpp - sbpp;

    // Then sort on screen area
    if (farea != sarea)
        return farea - sarea;

    // Then sort on width
    if (fm->width != sm->width)
        return fm->width - sm->width;

    // Lastly sort on refresh rate
    return fm->refreshRate - sm->refreshRate;
}

// Retrieves the available modes for the specified monitor
//
static LUMbool refreshVideoModes(_LUMmonitor* monitor)
{
    int modeCount;
    LUMvidmode* modes;

    if (monitor->modes)
        return LUM_TRUE;

    modes = _lum.platform.getVideoModes(monitor, &modeCount);
    if (!modes)
        return LUM_FALSE;

    qsort(modes, modeCount, sizeof(LUMvidmode), compareVideoModes);

    _lum_free(monitor->modes);
    monitor->modes = modes;
    monitor->modeCount = modeCount;

    return LUM_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                         Lumina event API                       //////
//////////////////////////////////////////////////////////////////////////

// Notifies shared code of a monitor connection or disconnection
//
void _lumInputMonitor(_LUMmonitor* monitor, int action, int placement)
{
    if (action == LUM_CONNECTED)
    {
        _lum.monitorCount++;
        _lum.monitors =
            _lum_realloc(_lum.monitors,
                          sizeof(_LUMmonitor*) * _lum.monitorCount);

        if (placement == _LUM_INSERT_FIRST)
        {
            memmove(_lum.monitors + 1,
                    _lum.monitors,
                    ((size_t) _lum.monitorCount - 1) * sizeof(_LUMmonitor*));
            _lum.monitors[0] = monitor;
        }
        else
            _lum.monitors[_lum.monitorCount - 1] = monitor;
    }
    else if (action == LUM_DISCONNECTED)
    {
        int i;
        _LUMwindow* window;

        for (window = _lum.windowListHead;  window;  window = window->next)
        {
            if (window->monitor == monitor)
            {
                int width, height, xoff, yoff;
                _lum.platform.getWindowSize(window, &width, &height);
                _lum.platform.setWindowMonitor(window, NULL, 0, 0, width, height, 0);
                _lum.platform.getWindowFrameSize(window, &xoff, &yoff, NULL, NULL);
                _lum.platform.setWindowPos(window, xoff, yoff);
            }
        }

        for (i = 0;  i < _lum.monitorCount;  i++)
        {
            if (_lum.monitors[i] == monitor)
            {
                _lum.monitorCount--;
                memmove(_lum.monitors + i,
                        _lum.monitors + i + 1,
                        ((size_t) _lum.monitorCount - i) * sizeof(_LUMmonitor*));
                break;
            }
        }
    }

    if (_lum.callbacks.monitor)
        _lum.callbacks.monitor((LUMmonitor*) monitor, action);

    if (action == LUM_DISCONNECTED)
        _lumFreeMonitor(monitor);
}

// Notifies shared code that a full screen window has acquired or released
// a monitor
//
void _lumInputMonitorWindow(_LUMmonitor* monitor, _LUMwindow* window)
{
    monitor->window = window;
}


//////////////////////////////////////////////////////////////////////////
//////                       Lumina internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Allocates and returns a monitor object with the specified name and dimensions
//
_LUMmonitor* _lumAllocMonitor(const char* name, int widthMM, int heightMM)
{
    _LUMmonitor* monitor = _lum_calloc(1, sizeof(_LUMmonitor));
    monitor->widthMM = widthMM;
    monitor->heightMM = heightMM;

    strncpy(monitor->name, name, sizeof(monitor->name) - 1);

    return monitor;
}

// Frees a monitor object and any data associated with it
//
void _lumFreeMonitor(_LUMmonitor* monitor)
{
    if (monitor == NULL)
        return;

    _lum.platform.freeMonitor(monitor);

    _lumFreeGammaArrays(&monitor->originalRamp);
    _lumFreeGammaArrays(&monitor->currentRamp);

    _lum_free(monitor->modes);
    _lum_free(monitor);
}

// Allocates red, green and blue value arrays of the specified size
//
void _lumAllocGammaArrays(LUMgammaramp* ramp, unsigned int size)
{
    ramp->red = _lum_calloc(size, sizeof(unsigned short));
    ramp->green = _lum_calloc(size, sizeof(unsigned short));
    ramp->blue = _lum_calloc(size, sizeof(unsigned short));
    ramp->size = size;
}

// Frees the red, green and blue value arrays and clears the struct
//
void _lumFreeGammaArrays(LUMgammaramp* ramp)
{
    _lum_free(ramp->red);
    _lum_free(ramp->green);
    _lum_free(ramp->blue);

    memset(ramp, 0, sizeof(LUMgammaramp));
}

// Chooses the video mode most closely matching the desired one
//
const LUMvidmode* _lumChooseVideoMode(_LUMmonitor* monitor,
                                        const LUMvidmode* desired)
{
    int i;
    unsigned int sizeDiff, leastSizeDiff = UINT_MAX;
    unsigned int rateDiff, leastRateDiff = UINT_MAX;
    unsigned int colorDiff, leastColorDiff = UINT_MAX;
    const LUMvidmode* current;
    const LUMvidmode* closest = NULL;

    if (!refreshVideoModes(monitor))
        return NULL;

    for (i = 0;  i < monitor->modeCount;  i++)
    {
        current = monitor->modes + i;

        colorDiff = 0;

        if (desired->redBits != LUM_DONT_CARE)
            colorDiff += abs(current->redBits - desired->redBits);
        if (desired->greenBits != LUM_DONT_CARE)
            colorDiff += abs(current->greenBits - desired->greenBits);
        if (desired->blueBits != LUM_DONT_CARE)
            colorDiff += abs(current->blueBits - desired->blueBits);

        sizeDiff = abs((current->width - desired->width) *
                       (current->width - desired->width) +
                       (current->height - desired->height) *
                       (current->height - desired->height));

        if (desired->refreshRate != LUM_DONT_CARE)
            rateDiff = abs(current->refreshRate - desired->refreshRate);
        else
            rateDiff = UINT_MAX - current->refreshRate;

        if ((colorDiff < leastColorDiff) ||
            (colorDiff == leastColorDiff && sizeDiff < leastSizeDiff) ||
            (colorDiff == leastColorDiff && sizeDiff == leastSizeDiff && rateDiff < leastRateDiff))
        {
            closest = current;
            leastSizeDiff = sizeDiff;
            leastRateDiff = rateDiff;
            leastColorDiff = colorDiff;
        }
    }

    return closest;
}

// Performs lexical comparison between two @ref LUMvidmode structures
//
int _lumCompareVideoModes(const LUMvidmode* fm, const LUMvidmode* sm)
{
    return compareVideoModes(fm, sm);
}

// Splits a color depth into red, green and blue bit depths
//
void _lumSplitBPP(int bpp, int* red, int* green, int* blue)
{
    int delta;

    // We assume that by 32 the user really meant 24
    if (bpp == 32)
        bpp = 24;

    // Convert "bits per pixel" to red, green & blue sizes

    *red = *green = *blue = bpp / 3;
    delta = bpp - (*red * 3);
    if (delta >= 1)
        *green = *green + 1;

    if (delta == 2)
        *red = *red + 1;
}


//////////////////////////////////////////////////////////////////////////
//////                        Lumina public API                       //////
//////////////////////////////////////////////////////////////////////////

LUMAPI LUMmonitor** lumGetMonitors(int* count)
{
    assert(count != NULL);

    *count = 0;

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    *count = _lum.monitorCount;
    return (LUMmonitor**) _lum.monitors;
}

LUMAPI LUMmonitor* lumGetPrimaryMonitor(void)
{
    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    if (!_lum.monitorCount)
        return NULL;

    return (LUMmonitor*) _lum.monitors[0];
}

LUMAPI void lumGetMonitorPos(LUMmonitor* handle, int* xpos, int* ypos)
{
    _LUMmonitor* monitor = (_LUMmonitor*) handle;
    assert(monitor != NULL);

    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 0;

    _LUM_REQUIRE_INIT();

    _lum.platform.getMonitorPos(monitor, xpos, ypos);
}

LUMAPI void lumGetMonitorWorkarea(LUMmonitor* handle,
                                    int* xpos, int* ypos,
                                    int* width, int* height)
{
    _LUMmonitor* monitor = (_LUMmonitor*) handle;
    assert(monitor != NULL);

    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 0;
    if (width)
        *width = 0;
    if (height)
        *height = 0;

    _LUM_REQUIRE_INIT();

    _lum.platform.getMonitorWorkarea(monitor, xpos, ypos, width, height);
}

LUMAPI void lumGetMonitorPhysicalSize(LUMmonitor* handle, int* widthMM, int* heightMM)
{
    _LUMmonitor* monitor = (_LUMmonitor*) handle;
    assert(monitor != NULL);

    if (widthMM)
        *widthMM = 0;
    if (heightMM)
        *heightMM = 0;

    _LUM_REQUIRE_INIT();

    if (widthMM)
        *widthMM = monitor->widthMM;
    if (heightMM)
        *heightMM = monitor->heightMM;
}

LUMAPI void lumGetMonitorContentScale(LUMmonitor* handle,
                                        float* xscale, float* yscale)
{
    _LUMmonitor* monitor = (_LUMmonitor*) handle;
    assert(monitor != NULL);

    if (xscale)
        *xscale = 0.f;
    if (yscale)
        *yscale = 0.f;

    _LUM_REQUIRE_INIT();
    _lum.platform.getMonitorContentScale(monitor, xscale, yscale);
}

LUMAPI const char* lumGetMonitorName(LUMmonitor* handle)
{
    _LUMmonitor* monitor = (_LUMmonitor*) handle;
    assert(monitor != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    return monitor->name;
}

LUMAPI void lumSetMonitorUserPointer(LUMmonitor* handle, void* pointer)
{
    _LUMmonitor* monitor = (_LUMmonitor*) handle;
    assert(monitor != NULL);

    _LUM_REQUIRE_INIT();
    monitor->userPointer = pointer;
}

LUMAPI void* lumGetMonitorUserPointer(LUMmonitor* handle)
{
    _LUMmonitor* monitor = (_LUMmonitor*) handle;
    assert(monitor != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    return monitor->userPointer;
}

LUMAPI LUMmonitorfun lumSetMonitorCallback(LUMmonitorfun cbfun)
{
    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    _LUM_SWAP(LUMmonitorfun, _lum.callbacks.monitor, cbfun);
    return cbfun;
}

LUMAPI const LUMvidmode* lumGetVideoModes(LUMmonitor* handle, int* count)
{
    _LUMmonitor* monitor = (_LUMmonitor*) handle;
    assert(monitor != NULL);
    assert(count != NULL);

    *count = 0;

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    if (!refreshVideoModes(monitor))
        return NULL;

    *count = monitor->modeCount;
    return monitor->modes;
}

LUMAPI const LUMvidmode* lumGetVideoMode(LUMmonitor* handle)
{
    _LUMmonitor* monitor = (_LUMmonitor*) handle;
    assert(monitor != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    _lum.platform.getVideoMode(monitor, &monitor->currentMode);
    return &monitor->currentMode;
}

LUMAPI void lumSetGamma(LUMmonitor* handle, float gamma)
{
    unsigned int i;
    unsigned short* values;
    LUMgammaramp ramp;
    const LUMgammaramp* original;
    assert(handle != NULL);
    assert(gamma > 0.f);
    assert(gamma <= FLT_MAX);

    _LUM_REQUIRE_INIT();

    if (gamma != gamma || gamma <= 0.f || gamma > FLT_MAX)
    {
        _lumInputError(LUM_INVALID_VALUE, "Invalid gamma value %f", gamma);
        return;
    }

    original = lumGetGammaRamp(handle);
    if (!original)
        return;

    values = _lum_calloc(original->size, sizeof(unsigned short));

    for (i = 0;  i < original->size;  i++)
    {
        float value;

        // Calculate intensity
        value = i / (float) (original->size - 1);
        // Apply gamma curve
        value = powf(value, 1.f / gamma) * 65535.f + 0.5f;
        // Clamp to value range
        value = _lum_fminf(value, 65535.f);

        values[i] = (unsigned short) value;
    }

    ramp.red = values;
    ramp.green = values;
    ramp.blue = values;
    ramp.size = original->size;

    lumSetGammaRamp(handle, &ramp);
    _lum_free(values);
}

LUMAPI const LUMgammaramp* lumGetGammaRamp(LUMmonitor* handle)
{
    _LUMmonitor* monitor = (_LUMmonitor*) handle;
    assert(monitor != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    _lumFreeGammaArrays(&monitor->currentRamp);
    if (!_lum.platform.getGammaRamp(monitor, &monitor->currentRamp))
        return NULL;

    return &monitor->currentRamp;
}

LUMAPI void lumSetGammaRamp(LUMmonitor* handle, const LUMgammaramp* ramp)
{
    _LUMmonitor* monitor = (_LUMmonitor*) handle;
    assert(monitor != NULL);
    assert(ramp != NULL);
    assert(ramp->size > 0);
    assert(ramp->red != NULL);
    assert(ramp->green != NULL);
    assert(ramp->blue != NULL);

    if (ramp->size <= 0)
    {
        _lumInputError(LUM_INVALID_VALUE,
                        "Invalid gamma ramp size %i",
                        ramp->size);
        return;
    }

    _LUM_REQUIRE_INIT();

    if (!monitor->originalRamp.size)
    {
        if (!_lum.platform.getGammaRamp(monitor, &monitor->originalRamp))
            return;
    }

    _lum.platform.setGammaRamp(monitor, ramp);
}

