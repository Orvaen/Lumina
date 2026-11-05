//========================================================================
// Lumina X11 - fork of GLFW 3.4 (www.glfw.org)
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
// It is fine to use C99 in this file because it will not be built with VS
//========================================================================

#include "internal.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


// Check whether the display mode should be included in enumeration
//
static LUMbool modeIsGood(const XRRModeInfo* mi)
{
    return (mi->modeFlags & RR_Interlace) == 0;
}

// Calculates the refresh rate, in Hz, from the specified RandR mode info
//
static int calculateRefreshRate(const XRRModeInfo* mi)
{
    if (mi->hTotal && mi->vTotal)
        return (int) round((double) mi->dotClock / ((double) mi->hTotal * (double) mi->vTotal));
    else
        return 0;
}

// Returns the mode info for a RandR mode XID
//
static const XRRModeInfo* getModeInfo(const XRRScreenResources* sr, RRMode id)
{
    for (int i = 0;  i < sr->nmode;  i++)
    {
        if (sr->modes[i].id == id)
            return sr->modes + i;
    }

    return NULL;
}

// Convert RandR mode info to Lumina video mode
//
static LUMvidmode vidmodeFromModeInfo(const XRRModeInfo* mi,
                                       const XRRCrtcInfo* ci)
{
    LUMvidmode mode;

    if (ci->rotation == RR_Rotate_90 || ci->rotation == RR_Rotate_270)
    {
        mode.width  = mi->height;
        mode.height = mi->width;
    }
    else
    {
        mode.width  = mi->width;
        mode.height = mi->height;
    }

    mode.refreshRate = calculateRefreshRate(mi);

    _lumSplitBPP(DefaultDepth(_lum.x11.display, _lum.x11.screen),
                  &mode.redBits, &mode.greenBits, &mode.blueBits);

    return mode;
}


//////////////////////////////////////////////////////////////////////////
//////                       Lumina internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Poll for changes in the set of connected monitors
//
void _lumPollMonitorsX11(void)
{
    if (_lum.x11.randr.available && !_lum.x11.randr.monitorBroken)
    {
        int disconnectedCount, screenCount = 0;
        _LUMmonitor** disconnected = NULL;
        XineramaScreenInfo* screens = NULL;
        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_lum.x11.display,
                                                              _lum.x11.root);
        RROutput primary = XRRGetOutputPrimary(_lum.x11.display,
                                               _lum.x11.root);

        if (_lum.x11.xinerama.available)
            screens = XineramaQueryScreens(_lum.x11.display, &screenCount);

        disconnectedCount = _lum.monitorCount;
        if (disconnectedCount)
        {
            disconnected = _lum_calloc(_lum.monitorCount, sizeof(_LUMmonitor*));
            memcpy(disconnected,
                   _lum.monitors,
                   _lum.monitorCount * sizeof(_LUMmonitor*));
        }

        for (int i = 0;  i < sr->noutput;  i++)
        {
            int j, type, widthMM, heightMM;

            XRROutputInfo* oi = XRRGetOutputInfo(_lum.x11.display, sr, sr->outputs[i]);
            if (oi->connection != RR_Connected || oi->crtc == None)
            {
                XRRFreeOutputInfo(oi);
                continue;
            }

            for (j = 0;  j < disconnectedCount;  j++)
            {
                if (disconnected[j] &&
                    disconnected[j]->x11.output == sr->outputs[i])
                {
                    disconnected[j] = NULL;
                    break;
                }
            }

            if (j < disconnectedCount)
            {
                XRRFreeOutputInfo(oi);
                continue;
            }

            XRRCrtcInfo* ci = XRRGetCrtcInfo(_lum.x11.display, sr, oi->crtc);
            if (ci->rotation == RR_Rotate_90 || ci->rotation == RR_Rotate_270)
            {
                widthMM  = oi->mm_height;
                heightMM = oi->mm_width;
            }
            else
            {
                widthMM  = oi->mm_width;
                heightMM = oi->mm_height;
            }

            if (widthMM <= 0 || heightMM <= 0)
            {
                // HACK: If RandR does not provide a physical size, assume the
                //       X11 default 96 DPI and calculate from the CRTC viewport
                // NOTE: These members are affected by rotation, unlike the mode
                //       info and output info members
                widthMM  = (int) (ci->width * 25.4f / 96.f);
                heightMM = (int) (ci->height * 25.4f / 96.f);
            }

            _LUMmonitor* monitor = _lumAllocMonitor(oi->name, widthMM, heightMM);
            monitor->x11.output = sr->outputs[i];
            monitor->x11.crtc   = oi->crtc;

            for (j = 0;  j < screenCount;  j++)
            {
                if (screens[j].x_org == ci->x &&
                    screens[j].y_org == ci->y &&
                    screens[j].width == ci->width &&
                    screens[j].height == ci->height)
                {
                    monitor->x11.index = j;
                    break;
                }
            }

            if (monitor->x11.output == primary)
                type = _LUM_INSERT_FIRST;
            else
                type = _LUM_INSERT_LAST;

            _lumInputMonitor(monitor, LUM_CONNECTED, type);

            XRRFreeOutputInfo(oi);
            XRRFreeCrtcInfo(ci);
        }

        XRRFreeScreenResources(sr);

        if (screens)
            XFree(screens);

        for (int i = 0;  i < disconnectedCount;  i++)
        {
            if (disconnected[i])
                _lumInputMonitor(disconnected[i], LUM_DISCONNECTED, 0);
        }

        _lum_free(disconnected);
    }
    else
    {
        const int widthMM = DisplayWidthMM(_lum.x11.display, _lum.x11.screen);
        const int heightMM = DisplayHeightMM(_lum.x11.display, _lum.x11.screen);

        _lumInputMonitor(_lumAllocMonitor("Display", widthMM, heightMM),
                          LUM_CONNECTED,
                          _LUM_INSERT_FIRST);
    }
}

// Set the current video mode for the specified monitor
//
void _lumSetVideoModeX11(_LUMmonitor* monitor, const LUMvidmode* desired)
{
    if (_lum.x11.randr.available && !_lum.x11.randr.monitorBroken)
    {
        LUMvidmode current;
        RRMode native = None;

        const LUMvidmode* best = _lumChooseVideoMode(monitor, desired);
        _lumGetVideoModeX11(monitor, &current);
        if (_lumCompareVideoModes(&current, best) == 0)
            return;

        XRRScreenResources* sr =
            XRRGetScreenResourcesCurrent(_lum.x11.display, _lum.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_lum.x11.display, sr, monitor->x11.crtc);
        XRROutputInfo* oi = XRRGetOutputInfo(_lum.x11.display, sr, monitor->x11.output);

        for (int i = 0;  i < oi->nmode;  i++)
        {
            const XRRModeInfo* mi = getModeInfo(sr, oi->modes[i]);
            if (!modeIsGood(mi))
                continue;

            const LUMvidmode mode = vidmodeFromModeInfo(mi, ci);
            if (_lumCompareVideoModes(best, &mode) == 0)
            {
                native = mi->id;
                break;
            }
        }

        if (native)
        {
            if (monitor->x11.oldMode == None)
                monitor->x11.oldMode = ci->mode;

            XRRSetCrtcConfig(_lum.x11.display,
                             sr, monitor->x11.crtc,
                             CurrentTime,
                             ci->x, ci->y,
                             native,
                             ci->rotation,
                             ci->outputs,
                             ci->noutput);
        }

        XRRFreeOutputInfo(oi);
        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);
    }
}

// Restore the saved (original) video mode for the specified monitor
//
void _lumRestoreVideoModeX11(_LUMmonitor* monitor)
{
    if (_lum.x11.randr.available && !_lum.x11.randr.monitorBroken)
    {
        if (monitor->x11.oldMode == None)
            return;

        XRRScreenResources* sr =
            XRRGetScreenResourcesCurrent(_lum.x11.display, _lum.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_lum.x11.display, sr, monitor->x11.crtc);

        XRRSetCrtcConfig(_lum.x11.display,
                         sr, monitor->x11.crtc,
                         CurrentTime,
                         ci->x, ci->y,
                         monitor->x11.oldMode,
                         ci->rotation,
                         ci->outputs,
                         ci->noutput);

        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);

        monitor->x11.oldMode = None;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       Lumina platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _lumFreeMonitorX11(_LUMmonitor* monitor)
{
}

void _lumGetMonitorPosX11(_LUMmonitor* monitor, int* xpos, int* ypos)
{
    if (_lum.x11.randr.available && !_lum.x11.randr.monitorBroken)
    {
        XRRScreenResources* sr =
            XRRGetScreenResourcesCurrent(_lum.x11.display, _lum.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_lum.x11.display, sr, monitor->x11.crtc);

        if (ci)
        {
            if (xpos)
                *xpos = ci->x;
            if (ypos)
                *ypos = ci->y;

            XRRFreeCrtcInfo(ci);
        }

        XRRFreeScreenResources(sr);
    }
}

void _lumGetMonitorContentScaleX11(_LUMmonitor* monitor,
                                    float* xscale, float* yscale)
{
    if (xscale)
        *xscale = _lum.x11.contentScaleX;
    if (yscale)
        *yscale = _lum.x11.contentScaleY;
}

void _lumGetMonitorWorkareaX11(_LUMmonitor* monitor,
                                int* xpos, int* ypos,
                                int* width, int* height)
{
    int areaX = 0, areaY = 0, areaWidth = 0, areaHeight = 0;

    if (_lum.x11.randr.available && !_lum.x11.randr.monitorBroken)
    {
        XRRScreenResources* sr =
            XRRGetScreenResourcesCurrent(_lum.x11.display, _lum.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_lum.x11.display, sr, monitor->x11.crtc);

        areaX = ci->x;
        areaY = ci->y;

        const XRRModeInfo* mi = getModeInfo(sr, ci->mode);

        if (ci->rotation == RR_Rotate_90 || ci->rotation == RR_Rotate_270)
        {
            areaWidth  = mi->height;
            areaHeight = mi->width;
        }
        else
        {
            areaWidth  = mi->width;
            areaHeight = mi->height;
        }

        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);
    }
    else
    {
        areaWidth  = DisplayWidth(_lum.x11.display, _lum.x11.screen);
        areaHeight = DisplayHeight(_lum.x11.display, _lum.x11.screen);
    }

    if (_lum.x11.NET_WORKAREA && _lum.x11.NET_CURRENT_DESKTOP)
    {
        Atom* extents = NULL;
        Atom* desktop = NULL;
        const unsigned long extentCount =
            _lumGetWindowPropertyX11(_lum.x11.root,
                                      _lum.x11.NET_WORKAREA,
                                      XA_CARDINAL,
                                      (unsigned char**) &extents);

        if (_lumGetWindowPropertyX11(_lum.x11.root,
                                      _lum.x11.NET_CURRENT_DESKTOP,
                                      XA_CARDINAL,
                                      (unsigned char**) &desktop) > 0)
        {
            if (extentCount >= 4 && *desktop < extentCount / 4)
            {
                const int globalX = extents[*desktop * 4 + 0];
                const int globalY = extents[*desktop * 4 + 1];
                const int globalWidth  = extents[*desktop * 4 + 2];
                const int globalHeight = extents[*desktop * 4 + 3];

                if (areaX < globalX)
                {
                    areaWidth -= globalX - areaX;
                    areaX = globalX;
                }

                if (areaY < globalY)
                {
                    areaHeight -= globalY - areaY;
                    areaY = globalY;
                }

                if (areaX + areaWidth > globalX + globalWidth)
                    areaWidth = globalX - areaX + globalWidth;
                if (areaY + areaHeight > globalY + globalHeight)
                    areaHeight = globalY - areaY + globalHeight;
            }
        }

        if (extents)
            XFree(extents);
        if (desktop)
            XFree(desktop);
    }

    if (xpos)
        *xpos = areaX;
    if (ypos)
        *ypos = areaY;
    if (width)
        *width = areaWidth;
    if (height)
        *height = areaHeight;
}

LUMvidmode* _lumGetVideoModesX11(_LUMmonitor* monitor, int* count)
{
    LUMvidmode* result;

    *count = 0;

    if (_lum.x11.randr.available && !_lum.x11.randr.monitorBroken)
    {
        XRRScreenResources* sr =
            XRRGetScreenResourcesCurrent(_lum.x11.display, _lum.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_lum.x11.display, sr, monitor->x11.crtc);
        XRROutputInfo* oi = XRRGetOutputInfo(_lum.x11.display, sr, monitor->x11.output);

        result = _lum_calloc(oi->nmode, sizeof(LUMvidmode));

        for (int i = 0;  i < oi->nmode;  i++)
        {
            const XRRModeInfo* mi = getModeInfo(sr, oi->modes[i]);
            if (!modeIsGood(mi))
                continue;

            const LUMvidmode mode = vidmodeFromModeInfo(mi, ci);
            int j;

            for (j = 0;  j < *count;  j++)
            {
                if (_lumCompareVideoModes(result + j, &mode) == 0)
                    break;
            }

            // Skip duplicate modes
            if (j < *count)
                continue;

            (*count)++;
            result[*count - 1] = mode;
        }

        XRRFreeOutputInfo(oi);
        XRRFreeCrtcInfo(ci);
        XRRFreeScreenResources(sr);
    }
    else
    {
        *count = 1;
        result = _lum_calloc(1, sizeof(LUMvidmode));
        _lumGetVideoModeX11(monitor, result);
    }

    return result;
}

void _lumGetVideoModeX11(_LUMmonitor* monitor, LUMvidmode* mode)
{
    if (_lum.x11.randr.available && !_lum.x11.randr.monitorBroken)
    {
        XRRScreenResources* sr =
            XRRGetScreenResourcesCurrent(_lum.x11.display, _lum.x11.root);
        XRRCrtcInfo* ci = XRRGetCrtcInfo(_lum.x11.display, sr, monitor->x11.crtc);

        if (ci)
        {
            const XRRModeInfo* mi = getModeInfo(sr, ci->mode);
            if (mi)  // mi can be NULL if the monitor has been disconnected
                *mode = vidmodeFromModeInfo(mi, ci);

            XRRFreeCrtcInfo(ci);
        }

        XRRFreeScreenResources(sr);
    }
    else
    {
        mode->width = DisplayWidth(_lum.x11.display, _lum.x11.screen);
        mode->height = DisplayHeight(_lum.x11.display, _lum.x11.screen);
        mode->refreshRate = 0;

        _lumSplitBPP(DefaultDepth(_lum.x11.display, _lum.x11.screen),
                      &mode->redBits, &mode->greenBits, &mode->blueBits);
    }
}

LUMbool _lumGetGammaRampX11(_LUMmonitor* monitor, LUMgammaramp* ramp)
{
    if (_lum.x11.randr.available && !_lum.x11.randr.gammaBroken)
    {
        const size_t size = XRRGetCrtcGammaSize(_lum.x11.display,
                                                monitor->x11.crtc);
        XRRCrtcGamma* gamma = XRRGetCrtcGamma(_lum.x11.display,
                                              monitor->x11.crtc);

        _lumAllocGammaArrays(ramp, size);

        memcpy(ramp->red,   gamma->red,   size * sizeof(unsigned short));
        memcpy(ramp->green, gamma->green, size * sizeof(unsigned short));
        memcpy(ramp->blue,  gamma->blue,  size * sizeof(unsigned short));

        XRRFreeGamma(gamma);
        return LUM_TRUE;
    }
    else if (_lum.x11.vidmode.available)
    {
        int size;
        XF86VidModeGetGammaRampSize(_lum.x11.display, _lum.x11.screen, &size);

        _lumAllocGammaArrays(ramp, size);

        XF86VidModeGetGammaRamp(_lum.x11.display,
                                _lum.x11.screen,
                                ramp->size, ramp->red, ramp->green, ramp->blue);
        return LUM_TRUE;
    }
    else
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "X11: Gamma ramp access not supported by server");
        return LUM_FALSE;
    }
}

void _lumSetGammaRampX11(_LUMmonitor* monitor, const LUMgammaramp* ramp)
{
    if (_lum.x11.randr.available && !_lum.x11.randr.gammaBroken)
    {
        if (XRRGetCrtcGammaSize(_lum.x11.display, monitor->x11.crtc) != ramp->size)
        {
            _lumInputError(LUM_PLATFORM_ERROR,
                            "X11: Gamma ramp size must match current ramp size");
            return;
        }

        XRRCrtcGamma* gamma = XRRAllocGamma(ramp->size);

        memcpy(gamma->red,   ramp->red,   ramp->size * sizeof(unsigned short));
        memcpy(gamma->green, ramp->green, ramp->size * sizeof(unsigned short));
        memcpy(gamma->blue,  ramp->blue,  ramp->size * sizeof(unsigned short));

        XRRSetCrtcGamma(_lum.x11.display, monitor->x11.crtc, gamma);
        XRRFreeGamma(gamma);
    }
    else if (_lum.x11.vidmode.available)
    {
        XF86VidModeSetGammaRamp(_lum.x11.display,
                                _lum.x11.screen,
                                ramp->size,
                                (unsigned short*) ramp->red,
                                (unsigned short*) ramp->green,
                                (unsigned short*) ramp->blue);
    }
    else
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "X11: Gamma ramp access not supported by server");
    }
}


//////////////////////////////////////////////////////////////////////////
//////                        Lumina native API                       //////
//////////////////////////////////////////////////////////////////////////

LUMAPI RRCrtc lumGetX11Adapter(LUMmonitor* handle)
{
    _LUMmonitor* monitor = (_LUMmonitor*) handle;
    _LUM_REQUIRE_INIT_OR_RETURN(None);
    return monitor->x11.crtc;
}

LUMAPI RROutput lumGetX11Monitor(LUMmonitor* handle)
{
    _LUMmonitor* monitor = (_LUMmonitor*) handle;
    _LUM_REQUIRE_INIT_OR_RETURN(None);
    return monitor->x11.output;
}

