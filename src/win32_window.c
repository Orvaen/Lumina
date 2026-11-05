//========================================================================
// Lumina Win32 - fork of GLFW 3.4 (www.glfw.org)
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

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <windowsx.h>
#include <shellapi.h>
#include <uxtheme.h>

// Returns the window style for the specified window
//
static DWORD getWindowStyle(const _LUMwindow* window)
{
    DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    if (window->monitor)
        style |= WS_POPUP;
    else
    {
        style |= WS_SYSMENU | WS_MINIMIZEBOX;

        if (window->decorated)
        {
            style |= WS_CAPTION;

            if (window->resizable)
                style |= WS_MAXIMIZEBOX | WS_THICKFRAME;
        }
        else
            style |= WS_POPUP;
    }

    return style;
}

// Returns the extended window style for the specified window
//
static DWORD getWindowExStyle(const _LUMwindow* window)
{
    DWORD style = WS_EX_APPWINDOW;

    if (window->monitor || window->floating)
        style |= WS_EX_TOPMOST;

    return style;
}

// Returns the image whose area most closely matches the desired one
//
static const LUMimage* chooseImage(int count, const LUMimage* images,
                                    int width, int height)
{
    int i, leastDiff = INT_MAX;
    const LUMimage* closest = NULL;

    for (i = 0;  i < count;  i++)
    {
        const int currDiff = abs(images[i].width * images[i].height -
                                 width * height);
        if (currDiff < leastDiff)
        {
            closest = images + i;
            leastDiff = currDiff;
        }
    }

    return closest;
}

// Creates an RGBA icon or cursor
//
static HICON createIcon(const LUMimage* image,
                        int xhot, int yhot, LUMbool icon)
{
    int i;
    HDC dc;
    HICON handle;
    HBITMAP color, mask;
    BITMAPV5HEADER bi;
    ICONINFO ii;
    unsigned char* target = NULL;
    unsigned char* source = image->pixels;

    ZeroMemory(&bi, sizeof(bi));
    bi.bV5Size        = sizeof(bi);
    bi.bV5Width       = image->width;
    bi.bV5Height      = -image->height;
    bi.bV5Planes      = 1;
    bi.bV5BitCount    = 32;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5RedMask     = 0x00ff0000;
    bi.bV5GreenMask   = 0x0000ff00;
    bi.bV5BlueMask    = 0x000000ff;
    bi.bV5AlphaMask   = 0xff000000;

    dc = GetDC(NULL);
    color = CreateDIBSection(dc,
                             (BITMAPINFO*) &bi,
                             DIB_RGB_COLORS,
                             (void**) &target,
                             NULL,
                             (DWORD) 0);
    ReleaseDC(NULL, dc);

    if (!color)
    {
        _lumInputErrorWin32(LUM_PLATFORM_ERROR,
                             "Win32: Failed to create RGBA bitmap");
        return NULL;
    }

    mask = CreateBitmap(image->width, image->height, 1, 1, NULL);
    if (!mask)
    {
        _lumInputErrorWin32(LUM_PLATFORM_ERROR,
                             "Win32: Failed to create mask bitmap");
        DeleteObject(color);
        return NULL;
    }

    for (i = 0;  i < image->width * image->height;  i++)
    {
        target[0] = source[2];
        target[1] = source[1];
        target[2] = source[0];
        target[3] = source[3];
        target += 4;
        source += 4;
    }

    ZeroMemory(&ii, sizeof(ii));
    ii.fIcon    = icon;
    ii.xHotspot = xhot;
    ii.yHotspot = yhot;
    ii.hbmMask  = mask;
    ii.hbmColor = color;

    handle = CreateIconIndirect(&ii);

    DeleteObject(color);
    DeleteObject(mask);

    if (!handle)
    {
        if (icon)
        {
            _lumInputErrorWin32(LUM_PLATFORM_ERROR,
                                 "Win32: Failed to create icon");
        }
        else
        {
            _lumInputErrorWin32(LUM_PLATFORM_ERROR,
                                 "Win32: Failed to create cursor");
        }
    }

    return handle;
}

// Translate content area size to full window size according to styles and DPI
//
static void getFullWindowSize(DWORD style, DWORD exStyle,
                              int contentWidth, int contentHeight,
                              int* fullWidth, int* fullHeight,
                              UINT dpi)
{
    RECT rect = { 0, 0, contentWidth, contentHeight };

    if (_lumIsWindows10AnniversaryUpdateOrGreaterWin32())
        AdjustWindowRectExForDpi(&rect, style, FALSE, exStyle, dpi);
    else
        AdjustWindowRectEx(&rect, style, FALSE, exStyle);

    *fullWidth = rect.right - rect.left;
    *fullHeight = rect.bottom - rect.top;
}

// Enforce the content area aspect ratio based on which edge is being dragged
//
static void applyAspectRatio(_LUMwindow* window, int edge, RECT* area)
{
    int xoff, yoff;
    UINT dpi = USER_DEFAULT_SCREEN_DPI;
    const float ratio = (float) window->numer / (float) window->denom;

    if (_lumIsWindows10AnniversaryUpdateOrGreaterWin32())
        dpi = GetDpiForWindow(window->win32.handle);

    getFullWindowSize(getWindowStyle(window), getWindowExStyle(window),
                      0, 0, &xoff, &yoff, dpi);

    if (edge == WMSZ_LEFT  || edge == WMSZ_BOTTOMLEFT ||
        edge == WMSZ_RIGHT || edge == WMSZ_BOTTOMRIGHT)
    {
        area->bottom = area->top + yoff +
            (int) ((area->right - area->left - xoff) / ratio);
    }
    else if (edge == WMSZ_TOPLEFT || edge == WMSZ_TOPRIGHT)
    {
        area->top = area->bottom - yoff -
            (int) ((area->right - area->left - xoff) / ratio);
    }
    else if (edge == WMSZ_TOP || edge == WMSZ_BOTTOM)
    {
        area->right = area->left + xoff +
            (int) ((area->bottom - area->top - yoff) * ratio);
    }
}

// Updates the cursor image according to its cursor mode
//
static void updateCursorImage(_LUMwindow* window)
{
    if (window->cursorMode == LUM_CURSOR_NORMAL)
    {
        if (window->cursor)
            SetCursor(window->cursor->win32.handle);
        else
            SetCursor(LoadCursorW(NULL, IDC_ARROW));
    }
    else
        SetCursor(NULL);
}

// Updates the cursor clip rect
//
static void updateClipRect(_LUMwindow* window)
{
    if (window)
    {
        RECT clipRect;
        GetClientRect(window->win32.handle, &clipRect);
        ClientToScreen(window->win32.handle, (POINT*) &clipRect.left);
        ClientToScreen(window->win32.handle, (POINT*) &clipRect.right);
        ClipCursor(&clipRect);
    }
    else
        ClipCursor(NULL);
}

// Enables WM_INPUT messages for the mouse for the specified window
//
static void enableRawMouseMotion(_LUMwindow* window)
{
    const RAWINPUTDEVICE rid = { 0x01, 0x02, 0, window->win32.handle };

    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
    {
        _lumInputErrorWin32(LUM_PLATFORM_ERROR,
                             "Win32: Failed to register raw input device");
    }
}

// Disables WM_INPUT messages for the mouse
//
static void disableRawMouseMotion(_LUMwindow* window)
{
    const RAWINPUTDEVICE rid = { 0x01, 0x02, RIDEV_REMOVE, NULL };

    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
    {
        _lumInputErrorWin32(LUM_PLATFORM_ERROR,
                             "Win32: Failed to remove raw input device");
    }
}

// Apply disabled cursor mode to a focused window
//
static void disableCursor(_LUMwindow* window)
{
    _lum.win32.disabledCursorWindow = window;
    _lumGetCursorPosWin32(window,
                           &_lum.win32.restoreCursorPosX,
                           &_lum.win32.restoreCursorPosY);
    updateCursorImage(window);
    _lumCenterCursorInContentArea(window);
    updateClipRect(window);

    if (window->rawMouseMotion)
        enableRawMouseMotion(window);
}

// Exit disabled cursor mode for the specified window
//
static void enableCursor(_LUMwindow* window)
{
    if (window->rawMouseMotion)
        disableRawMouseMotion(window);

    _lum.win32.disabledCursorWindow = NULL;
    updateClipRect(NULL);
    _lumSetCursorPosWin32(window,
                           _lum.win32.restoreCursorPosX,
                           _lum.win32.restoreCursorPosY);
    updateCursorImage(window);
}

// Returns whether the cursor is in the content area of the specified window
//
static LUMbool cursorInContentArea(_LUMwindow* window)
{
    RECT area;
    POINT pos;

    if (!GetCursorPos(&pos))
        return LUM_FALSE;

    if (WindowFromPoint(pos) != window->win32.handle)
        return LUM_FALSE;

    GetClientRect(window->win32.handle, &area);
    ClientToScreen(window->win32.handle, (POINT*) &area.left);
    ClientToScreen(window->win32.handle, (POINT*) &area.right);

    return PtInRect(&area, pos);
}

// Update native window styles to match attributes
//
static void updateWindowStyles(const _LUMwindow* window)
{
    RECT rect;
    DWORD style = GetWindowLongW(window->win32.handle, GWL_STYLE);
    style &= ~(WS_OVERLAPPEDWINDOW | WS_POPUP);
    style |= getWindowStyle(window);

    GetClientRect(window->win32.handle, &rect);

    if (_lumIsWindows10AnniversaryUpdateOrGreaterWin32())
    {
        AdjustWindowRectExForDpi(&rect, style, FALSE,
                                 getWindowExStyle(window),
                                 GetDpiForWindow(window->win32.handle));
    }
    else
        AdjustWindowRectEx(&rect, style, FALSE, getWindowExStyle(window));

    ClientToScreen(window->win32.handle, (POINT*) &rect.left);
    ClientToScreen(window->win32.handle, (POINT*) &rect.right);
    SetWindowLongW(window->win32.handle, GWL_STYLE, style);
    SetWindowPos(window->win32.handle, HWND_TOP,
                 rect.left, rect.top,
                 rect.right - rect.left, rect.bottom - rect.top,
                 SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOZORDER);
}

// Update window framebuffer transparency
//
static void updateFramebufferTransparency(const _LUMwindow* window)
{
    BOOL composition, opaque;
    DWORD color;

    if (!IsWindowsVistaOrGreater())
        return;

    if (FAILED(DwmIsCompositionEnabled(&composition)) || !composition)
       return;

    if (IsWindows8OrGreater() ||
        (SUCCEEDED(DwmGetColorizationColor(&color, &opaque)) && !opaque))
    {
        HRGN region = CreateRectRgn(0, 0, -1, -1);
        DWM_BLURBEHIND bb = {0};
        bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        bb.hRgnBlur = region;
        bb.fEnable = TRUE;

        DwmEnableBlurBehindWindow(window->win32.handle, &bb);
        DeleteObject(region);
    }
    else
    {
        // HACK: Disable framebuffer transparency on Windows 7 when the
        //       colorization color is opaque, because otherwise the window
        //       contents is blended additively with the previous frame instead
        //       of replacing it
        DWM_BLURBEHIND bb = {0};
        bb.dwFlags = DWM_BB_ENABLE;
        DwmEnableBlurBehindWindow(window->win32.handle, &bb);
    }
}

// Retrieves and translates modifier keys
//
static int getKeyMods(void)
{
    int mods = 0;

    if (GetKeyState(VK_SHIFT) & 0x8000)
        mods |= LUM_MOD_SHIFT;
    if (GetKeyState(VK_CONTROL) & 0x8000)
        mods |= LUM_MOD_CONTROL;
    if (GetKeyState(VK_MENU) & 0x8000)
        mods |= LUM_MOD_ALT;
    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
        mods |= LUM_MOD_SUPER;
    if (GetKeyState(VK_CAPITAL) & 1)
        mods |= LUM_MOD_CAPS_LOCK;
    if (GetKeyState(VK_NUMLOCK) & 1)
        mods |= LUM_MOD_NUM_LOCK;

    return mods;
}

static void fitToMonitor(_LUMwindow* window)
{
    MONITORINFO mi = { sizeof(mi) };
    GetMonitorInfo(window->monitor->win32.handle, &mi);
    SetWindowPos(window->win32.handle, HWND_TOPMOST,
                 mi.rcMonitor.left,
                 mi.rcMonitor.top,
                 mi.rcMonitor.right - mi.rcMonitor.left,
                 mi.rcMonitor.bottom - mi.rcMonitor.top,
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);
}

// Make the specified window and its video mode active on its monitor
//
static void acquireMonitor(_LUMwindow* window)
{
    if (!_lum.win32.acquiredMonitorCount)
    {
        SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);

        // HACK: When mouse trails are enabled the cursor becomes invisible when
        //       the OpenGL ICD switches to page flipping
        SystemParametersInfo(SPI_GETMOUSETRAILS, 0, &_lum.win32.mouseTrailSize, 0);
        SystemParametersInfo(SPI_SETMOUSETRAILS, 0, 0, 0);
    }

    if (!window->monitor->window)
        _lum.win32.acquiredMonitorCount++;

    _lumSetVideoModeWin32(window->monitor, &window->videoMode);
    _lumInputMonitorWindow(window->monitor, window);
}

// Remove the window and restore the original video mode
//
static void releaseMonitor(_LUMwindow* window)
{
    if (window->monitor->window != window)
        return;

    _lum.win32.acquiredMonitorCount--;
    if (!_lum.win32.acquiredMonitorCount)
    {
        SetThreadExecutionState(ES_CONTINUOUS);

        // HACK: Restore mouse trail length saved in acquireMonitor
        SystemParametersInfo(SPI_SETMOUSETRAILS, _lum.win32.mouseTrailSize, 0, 0);
    }

    _lumInputMonitorWindow(window->monitor, NULL);
    _lumRestoreVideoModeWin32(window->monitor);
}

// Window callback function (handles window messages)
//
static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg,
                                   WPARAM wParam, LPARAM lParam)
{
	static RECT border_thickness = { 4, 4, 4, 4 };
	BOOL hasThickFrame = GetWindowLongPtr(hWnd, GWL_STYLE) & WS_THICKFRAME;

    _LUMwindow* window = GetPropW(hWnd, L"Lumina");
    if (!window)
    {
        // This is the message handling for the hidden helper window
        // and for a regular window during its initial creation

        switch (uMsg)
        {
            case WM_NCCREATE:
            {
                if (_lumIsWindows10AnniversaryUpdateOrGreaterWin32())
                {
                    const CREATESTRUCTW* cs = (const CREATESTRUCTW*) lParam;
                    const _LUMwndconfig* wndconfig = cs->lpCreateParams;

                    // On per-monitor DPI aware V1 systems, only enable
                    // non-client scaling for windows that scale the client area
                    // We need WM_GETDPISCALEDSIZE from V2 to keep the client
                    // area static when the non-client area is scaled
                    if (wndconfig && wndconfig->scaleToMonitor)
                        EnableNonClientDpiScaling(hWnd);
                }

                break;
            }

            case WM_DISPLAYCHANGE:
                _lumPollMonitorsWin32();
                break;

            case WM_DEVICECHANGE:
            {
                if (!_lum.joysticksInitialized)
                    break;

                if (wParam == DBT_DEVICEARRIVAL)
                {
                    DEV_BROADCAST_HDR* dbh = (DEV_BROADCAST_HDR*) lParam;
                    if (dbh && dbh->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
                        _lumDetectJoystickConnectionWin32();
                }
                else if (wParam == DBT_DEVICEREMOVECOMPLETE)
                {
                    DEV_BROADCAST_HDR* dbh = (DEV_BROADCAST_HDR*) lParam;
                    if (dbh && dbh->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
                        _lumDetectJoystickDisconnectionWin32();
                }

                break;
            }

            case WM_CREATE:
            {
                if (_lum.hints.window.titlebar)
                    break;

                if (hasThickFrame)
                {
                    RECT size_rect;
                    GetWindowRect(hWnd, &size_rect);

                    // Inform the application of the frame change to force redrawing with the new
                    // client area that is extended into the title bar
                    SetWindowPos(
                        hWnd, NULL,
                        size_rect.left, size_rect.top,
                        size_rect.right - size_rect.left, size_rect.bottom - size_rect.top,
                        SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE
                    );
                    break;
                }

                break;
            }

            case WM_ACTIVATE:
            {
                if (_lum.hints.window.titlebar)
                    break;

				RECT title_bar_rect = {0};
				InvalidateRect(hWnd, &title_bar_rect, FALSE);
                break;
            }
        }

        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    switch (uMsg)
    {
        case WM_MOUSEACTIVATE:
        {
            // HACK: Postpone cursor disabling when the window was activated by
            //       clicking a caption button
            if (HIWORD(lParam) == WM_LBUTTONDOWN)
            {
                if (LOWORD(lParam) != HTCLIENT)
                    window->win32.frameAction = LUM_TRUE;
            }

            break;
        }

        case WM_CAPTURECHANGED:
        {
            // HACK: Disable the cursor once the caption button action has been
            //       completed or cancelled
            if (lParam == 0 && window->win32.frameAction)
            {
                if (window->cursorMode == LUM_CURSOR_DISABLED)
                    disableCursor(window);

                window->win32.frameAction = LUM_FALSE;
            }

            break;
        }

        case WM_SETFOCUS:
        {
            _lumInputWindowFocus(window, LUM_TRUE);

            // HACK: Do not disable cursor while the user is interacting with
            //       a caption button
            if (window->win32.frameAction)
                break;

            if (window->cursorMode == LUM_CURSOR_DISABLED)
                disableCursor(window);

            return 0;
        }

        case WM_KILLFOCUS:
        {
            if (window->cursorMode == LUM_CURSOR_DISABLED)
                enableCursor(window);

            if (window->monitor && window->autoIconify)
                _lumIconifyWindowWin32(window);

            _lumInputWindowFocus(window, LUM_FALSE);
            return 0;
        }

        case WM_SYSCOMMAND:
        {
            switch (wParam & 0xfff0)
            {
                case SC_SCREENSAVE:
                case SC_MONITORPOWER:
                {
                    if (window->monitor)
                    {
                        // We are running in full screen mode, so disallow
                        // screen saver and screen blanking
                        return 0;
                    }
                    else
                        break;
                }

                // User trying to access application menu using ALT?
                case SC_KEYMENU:
                {
                    if (!window->win32.keymenu)
                        return 0;

                    break;
                }
            }
            break;
        }

        case WM_CLOSE:
        {
            _lumInputWindowCloseRequest(window);
            return 0;
        }

        case WM_INPUTLANGCHANGE:
        {
            _lumUpdateKeyNamesWin32();
            break;
        }

        case WM_CHAR:
        case WM_SYSCHAR:
        {
            if (wParam >= 0xd800 && wParam <= 0xdbff)
                window->win32.highSurrogate = (WCHAR) wParam;
            else
            {
                uint32_t codepoint = 0;

                if (wParam >= 0xdc00 && wParam <= 0xdfff)
                {
                    if (window->win32.highSurrogate)
                    {
                        codepoint += (window->win32.highSurrogate - 0xd800) << 10;
                        codepoint += (WCHAR) wParam - 0xdc00;
                        codepoint += 0x10000;
                    }
                }
                else
                    codepoint = (WCHAR) wParam;

                window->win32.highSurrogate = 0;
                _lumInputChar(window, codepoint, getKeyMods(), uMsg != WM_SYSCHAR);
            }

            if (uMsg == WM_SYSCHAR && window->win32.keymenu)
                break;

            return 0;
        }

        case WM_UNICHAR:
        {
            if (wParam == UNICODE_NOCHAR)
            {
                // WM_UNICHAR is not sent by Windows, but is sent by some
                // third-party input method engine
                // Returning TRUE here announces support for this message
                return TRUE;
            }

            _lumInputChar(window, (uint32_t) wParam, getKeyMods(), LUM_TRUE);
            return 0;
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            int key, scancode;
            const int action = (HIWORD(lParam) & KF_UP) ? LUM_RELEASE : LUM_PRESS;
            const int mods = getKeyMods();

            scancode = (HIWORD(lParam) & (KF_EXTENDED | 0xff));
            if (!scancode)
            {
                // NOTE: Some synthetic key messages have a scancode of zero
                // HACK: Map the virtual key back to a usable scancode
                scancode = MapVirtualKeyW((UINT) wParam, MAPVK_VK_TO_VSC);
            }

            key = _lum.win32.keycodes[scancode];

            // The Ctrl keys require special handling
            if (wParam == VK_CONTROL)
            {
                if (HIWORD(lParam) & KF_EXTENDED)
                {
                    // Right side keys have the extended key bit set
                    key = LUM_KEY_RIGHT_CONTROL;
                }
                else
                {
                    // NOTE: Alt Gr sends Left Ctrl followed by Right Alt
                    // HACK: We only want one event for Alt Gr, so if we detect
                    //       this sequence we discard this Left Ctrl message now
                    //       and later report Right Alt normally
                    MSG next;
                    const DWORD time = GetMessageTime();

                    if (PeekMessageW(&next, NULL, 0, 0, PM_NOREMOVE))
                    {
                        if (next.message == WM_KEYDOWN ||
                            next.message == WM_SYSKEYDOWN ||
                            next.message == WM_KEYUP ||
                            next.message == WM_SYSKEYUP)
                        {
                            if (next.wParam == VK_MENU &&
                                (HIWORD(next.lParam) & KF_EXTENDED) &&
                                next.time == time)
                            {
                                // Next message is Right Alt down so discard this
                                break;
                            }
                        }
                    }

                    // This is a regular Left Ctrl message
                    key = LUM_KEY_LEFT_CONTROL;
                }
            }
            else if (wParam == VK_PROCESSKEY)
            {
                // IME notifies that keys have been filtered by setting the
                // virtual key-code to VK_PROCESSKEY
                break;
            }

            if (action == LUM_RELEASE && wParam == VK_SHIFT)
            {
                // HACK: Release both Shift keys on Shift up event, as when both
                //       are pressed the first release does not emit any event
                // NOTE: The other half of this is in _lumPollEventsWin32
                _lumInputKey(window, LUM_KEY_LEFT_SHIFT, scancode, action, mods);
                _lumInputKey(window, LUM_KEY_RIGHT_SHIFT, scancode, action, mods);
            }
            else if (wParam == VK_SNAPSHOT)
            {
                // HACK: Key down is not reported for the Print Screen key
                _lumInputKey(window, key, scancode, LUM_PRESS, mods);
                _lumInputKey(window, key, scancode, LUM_RELEASE, mods);
            }
            else
                _lumInputKey(window, key, scancode, action, mods);

            break;
        }

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_XBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP:
        {
            int i, button, action;

            if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP)
                button = LUM_MOUSE_BUTTON_LEFT;
            else if (uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP)
                button = LUM_MOUSE_BUTTON_RIGHT;
            else if (uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONUP)
                button = LUM_MOUSE_BUTTON_MIDDLE;
            else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)
                button = LUM_MOUSE_BUTTON_4;
            else
                button = LUM_MOUSE_BUTTON_5;

            if (uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN ||
                uMsg == WM_MBUTTONDOWN || uMsg == WM_XBUTTONDOWN)
            {
                action = LUM_PRESS;
            }
            else
                action = LUM_RELEASE;

            for (i = 0;  i <= LUM_MOUSE_BUTTON_LAST;  i++)
            {
                if (window->mouseButtons[i] == LUM_PRESS)
                    break;
            }

            if (i > LUM_MOUSE_BUTTON_LAST)
                SetCapture(hWnd);

            _lumInputMouseClick(window, button, action, getKeyMods());

            for (i = 0;  i <= LUM_MOUSE_BUTTON_LAST;  i++)
            {
                if (window->mouseButtons[i] == LUM_PRESS)
                    break;
            }

            if (i > LUM_MOUSE_BUTTON_LAST)
                ReleaseCapture();

            if (uMsg == WM_XBUTTONDOWN || uMsg == WM_XBUTTONUP)
                return TRUE;

            return 0;
        }

        case WM_MOUSEMOVE:
        {
            const int x = GET_X_LPARAM(lParam);
            const int y = GET_Y_LPARAM(lParam);

            if (!window->win32.cursorTracked)
            {
                TRACKMOUSEEVENT tme;
                ZeroMemory(&tme, sizeof(tme));
                tme.cbSize = sizeof(tme);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = window->win32.handle;
                TrackMouseEvent(&tme);

                window->win32.cursorTracked = LUM_TRUE;
                _lumInputCursorEnter(window, LUM_TRUE);
            }

            if (window->cursorMode == LUM_CURSOR_DISABLED)
            {
                const int dx = x - window->win32.lastCursorPosX;
                const int dy = y - window->win32.lastCursorPosY;

                if (_lum.win32.disabledCursorWindow != window)
                    break;
                if (window->rawMouseMotion)
                    break;

                _lumInputCursorPos(window,
                                    window->virtualCursorPosX + dx,
                                    window->virtualCursorPosY + dy);
            }
            else
                _lumInputCursorPos(window, x, y);

            window->win32.lastCursorPosX = x;
            window->win32.lastCursorPosY = y;

            return 0;
        }

        case WM_INPUT:
        {
            UINT size = 0;
            HRAWINPUT ri = (HRAWINPUT) lParam;
            RAWINPUT* data = NULL;
            int dx, dy;

            if (_lum.win32.disabledCursorWindow != window)
                break;
            if (!window->rawMouseMotion)
                break;

            GetRawInputData(ri, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
            if (size > (UINT) _lum.win32.rawInputSize)
            {
                _lum_free(_lum.win32.rawInput);
                _lum.win32.rawInput = _lum_calloc(size, 1);
                _lum.win32.rawInputSize = size;
            }

            size = _lum.win32.rawInputSize;
            if (GetRawInputData(ri, RID_INPUT,
                                _lum.win32.rawInput, &size,
                                sizeof(RAWINPUTHEADER)) == (UINT) -1)
            {
                _lumInputError(LUM_PLATFORM_ERROR,
                                "Win32: Failed to retrieve raw input data");
                break;
            }

            data = _lum.win32.rawInput;
            if (data->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
            {
                dx = data->data.mouse.lLastX - window->win32.lastCursorPosX;
                dy = data->data.mouse.lLastY - window->win32.lastCursorPosY;
            }
            else
            {
                dx = data->data.mouse.lLastX;
                dy = data->data.mouse.lLastY;
            }

            _lumInputCursorPos(window,
                                window->virtualCursorPosX + dx,
                                window->virtualCursorPosY + dy);

            window->win32.lastCursorPosX += dx;
            window->win32.lastCursorPosY += dy;
            break;
        }

        case WM_MOUSELEAVE:
        {
            window->win32.cursorTracked = LUM_FALSE;
            _lumInputCursorEnter(window, LUM_FALSE);
            return 0;
        }

        case WM_MOUSEWHEEL:
        {
            _lumInputScroll(window, 0.0, (SHORT) HIWORD(wParam) / (double) WHEEL_DELTA);
            return 0;
        }

        case WM_MOUSEHWHEEL:
        {
            // This message is only sent on Windows Vista and later
            // NOTE: The X-axis is inverted for consistency with macOS and X11
            _lumInputScroll(window, -((SHORT) HIWORD(wParam) / (double) WHEEL_DELTA), 0.0);
            return 0;
        }

        case WM_ENTERSIZEMOVE:
        case WM_ENTERMENULOOP:
        {
            if (window->win32.frameAction)
                break;

            // HACK: Enable the cursor while the user is moving or
            //       resizing the window or using the window menu
            if (window->cursorMode == LUM_CURSOR_DISABLED)
                enableCursor(window);

            break;
        }

        case WM_EXITSIZEMOVE:
        case WM_EXITMENULOOP:
        {
            if (window->win32.frameAction)
                break;

            // HACK: Disable the cursor once the user is done moving or
            //       resizing the window or using the menu
            if (window->cursorMode == LUM_CURSOR_DISABLED)
                disableCursor(window);

            break;
        }

		case WM_NCCALCSIZE:
		{
			if (_lum.hints.window.titlebar || !hasThickFrame || !wParam)
				break;

            // For custom frames

            // Shrink client area by border thickness so we can
            // resize window and see borders
			const int resizeBorderX = GetSystemMetrics(SM_CXFRAME);
			const int resizeBorderY = GetSystemMetrics(SM_CYFRAME);

			NCCALCSIZE_PARAMS* params = (NCCALCSIZE_PARAMS*)lParam;
			RECT* requestedClientRect = params->rgrc;

			requestedClientRect->right -= resizeBorderX;
			requestedClientRect->left += resizeBorderX;
			requestedClientRect->bottom -= resizeBorderY;

            //
            // NOTE(Yan):
            // 
            // Top borders seem to be handled differently.
            // 
            // Contracting by 1 on Win 11 seems to give a small area
            // for resizing whilst not showing a white border.
            // 
			// But this doesn't seem to work on Win 10, instead showing
			// a general white titlebar on top of the custom one...
			// to be continued.
            // 
            // Not changing the top (i.e. 0) means we don't see the
            // mouse icon change to a resize handle, but resizing still
            // works once you click and drag. This works on both
            // Windows 10 & 11, so we'll keep that for now.
			requestedClientRect->top += 0; 

            // NOTE(Yan): seems to make no difference what we return here,
            //            was originally 0
			return WVR_ALIGNTOP | WVR_ALIGNLEFT;
		}

        case WM_SIZE:
        {
            const int width = LOWORD(lParam);
            const int height = HIWORD(lParam);
            const LUMbool iconified = wParam == SIZE_MINIMIZED;
            const LUMbool maximized = wParam == SIZE_MAXIMIZED ||
                                       (window->win32.maximized &&
                                        wParam != SIZE_RESTORED);

            if (_lum.win32.disabledCursorWindow == window)
                updateClipRect(window);

            if (window->win32.iconified != iconified)
                _lumInputWindowIconify(window, iconified);

            if (window->win32.maximized != maximized)
                _lumInputWindowMaximize(window, maximized);

            if (width != window->win32.width || height != window->win32.height)
            {
                window->win32.width = width;
                window->win32.height = height;

                _lumInputFramebufferSize(window, width, height);
                _lumInputWindowSize(window, width, height);
            }

            if (window->monitor && window->win32.iconified != iconified)
            {
                if (iconified)
                    releaseMonitor(window);
                else
                {
                    acquireMonitor(window);
                    fitToMonitor(window);
                }
            }

            window->win32.iconified = iconified;
            window->win32.maximized = maximized;

			RECT size_rect;
			GetWindowRect(hWnd, &size_rect);

			// Inform the application of the frame change to force redrawing with the new
			// client area that is extended into the title bar
			SetWindowPos(
				hWnd, NULL,
				size_rect.left, size_rect.top,
				size_rect.right - size_rect.left, size_rect.bottom - size_rect.top,
				SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE
			);
            return 0;
        }

        case WM_MOVE:
        {
            if (_lum.win32.disabledCursorWindow == window)
                updateClipRect(window);

            // NOTE: This cannot use LOWORD/HIWORD recommended by MSDN, as
            // those macros do not handle negative window positions correctly
            _lumInputWindowPos(window,
                                GET_X_LPARAM(lParam),
                                GET_Y_LPARAM(lParam));
            return 0;
        }

        case WM_SIZING:
        {
            if (window->numer == LUM_DONT_CARE ||
                window->denom == LUM_DONT_CARE)
            {
                break;
            }

            applyAspectRatio(window, (int) wParam, (RECT*) lParam);
            return TRUE;
        }

        case WM_GETMINMAXINFO:
        {
            int xoff, yoff;
            UINT dpi = USER_DEFAULT_SCREEN_DPI;
            MINMAXINFO* mmi = (MINMAXINFO*) lParam;

            if (window->monitor)
                break;

            if (_lumIsWindows10AnniversaryUpdateOrGreaterWin32())
                dpi = GetDpiForWindow(window->win32.handle);

            getFullWindowSize(getWindowStyle(window), getWindowExStyle(window),
                              0, 0, &xoff, &yoff, dpi);

            if (window->minwidth != LUM_DONT_CARE &&
                window->minheight != LUM_DONT_CARE)
            {
                mmi->ptMinTrackSize.x = window->minwidth + xoff;
                mmi->ptMinTrackSize.y = window->minheight + yoff;
            }

            if (window->maxwidth != LUM_DONT_CARE &&
                window->maxheight != LUM_DONT_CARE)
            {
                mmi->ptMaxTrackSize.x = window->maxwidth + xoff;
                mmi->ptMaxTrackSize.y = window->maxheight + yoff;
            }

            if (!window->decorated)
            {
                MONITORINFO mi;
                const HMONITOR mh = MonitorFromWindow(window->win32.handle,
                                                      MONITOR_DEFAULTTONEAREST);

                ZeroMemory(&mi, sizeof(mi));
                mi.cbSize = sizeof(mi);
                GetMonitorInfo(mh, &mi);

                mmi->ptMaxPosition.x = mi.rcWork.left - mi.rcMonitor.left;
                mmi->ptMaxPosition.y = mi.rcWork.top - mi.rcMonitor.top;
                mmi->ptMaxSize.x = mi.rcWork.right - mi.rcWork.left;
                mmi->ptMaxSize.y = mi.rcWork.bottom - mi.rcWork.top;
            }

            return 0;
        }

        case WM_PAINT:
        {
            _lumInputWindowDamage(window);
            break;
        }

        case WM_ERASEBKGND:
        {
            return TRUE;
        }

        case WM_NCACTIVATE:
        case WM_NCPAINT:
        {
            // Prevent title bar from being drawn after restoring a minimized
            // undecorated window
            if (!window->decorated)
                return TRUE;

            break;
        }

        case WM_DWMCOMPOSITIONCHANGED:
        case WM_DWMCOLORIZATIONCOLORCHANGED:
        {
            if (window->win32.transparent)
                updateFramebufferTransparency(window);
            return 0;
        }

        case WM_GETDPISCALEDSIZE:
        {
            if (window->win32.scaleToMonitor)
                break;

            // Adjust the window size to keep the content area size constant
            if (_lumIsWindows10CreatorsUpdateOrGreaterWin32())
            {
                RECT source = {0}, target = {0};
                SIZE* size = (SIZE*) lParam;

                AdjustWindowRectExForDpi(&source, getWindowStyle(window),
                                         FALSE, getWindowExStyle(window),
                                         GetDpiForWindow(window->win32.handle));
                AdjustWindowRectExForDpi(&target, getWindowStyle(window),
                                         FALSE, getWindowExStyle(window),
                                         LOWORD(wParam));

                size->cx += (target.right - target.left) -
                            (source.right - source.left);
                size->cy += (target.bottom - target.top) -
                            (source.bottom - source.top);
                return TRUE;
            }

            break;
        }

        case WM_DPICHANGED:
        {
            const float xscale = HIWORD(wParam) / (float) USER_DEFAULT_SCREEN_DPI;
            const float yscale = LOWORD(wParam) / (float) USER_DEFAULT_SCREEN_DPI;

            // Resize windowed mode windows that either permit rescaling or that
            // need it to compensate for non-client area scaling
            if (!window->monitor &&
                (window->win32.scaleToMonitor ||
                 _lumIsWindows10CreatorsUpdateOrGreaterWin32()))
            {
                RECT* suggested = (RECT*) lParam;
                SetWindowPos(window->win32.handle, HWND_TOP,
                             suggested->left,
                             suggested->top,
                             suggested->right - suggested->left,
                             suggested->bottom - suggested->top,
                             SWP_NOACTIVATE | SWP_NOZORDER);
            }

            _lumInputWindowContentScale(window, xscale, yscale);
            break;
        }

        case WM_SETCURSOR:
        {
            if (LOWORD(lParam) == HTCLIENT)
            {
                updateCursorImage(window);
                return TRUE;
            }

            break;
        }

        case WM_DROPFILES:
        {
            HDROP drop = (HDROP) wParam;
            POINT pt;
            int i;

            const int count = DragQueryFileW(drop, 0xffffffff, NULL, 0);
            char** paths = _lum_calloc(count, sizeof(char*));

            // Move the mouse to the position of the drop
            DragQueryPoint(drop, &pt);
            _lumInputCursorPos(window, pt.x, pt.y);

            for (i = 0;  i < count;  i++)
            {
                const UINT length = DragQueryFileW(drop, i, NULL, 0);
                WCHAR* buffer = _lum_calloc((size_t) length + 1, sizeof(WCHAR));

                DragQueryFileW(drop, i, buffer, length + 1);
                paths[i] = _lumCreateUTF8FromWideStringWin32(buffer);

                _lum_free(buffer);
            }

            _lumInputDrop(window, count, (const char**) paths);

            for (i = 0;  i < count;  i++)
                _lum_free(paths[i]);
            _lum_free(paths);

            DragFinish(drop);
            return 0;
        }

        case WM_ACTIVATE:
        {
            if (_lum.hints.window.titlebar)
                break;

			RECT title_bar_rect = { 0 };
			InvalidateRect(hWnd, &title_bar_rect, FALSE);
        }
        case WM_NCHITTEST:
        {
            if (_lum.hints.window.titlebar || !hasThickFrame)
                break;

            //
            // Hit test for custom frames
            //
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			ScreenToClient(hWnd, &pt);

            // Check borders first
			if (!window->win32.maximized)
			{
				RECT rc;
				GetClientRect(hWnd, &rc);

				const int verticalBorderSize = GetSystemMetrics(SM_CYFRAME);

				enum { left = 1, top = 2, right = 4, bottom = 8 };
				int hit = 0;
				if (pt.x <= border_thickness.left)
                    hit |= left;
				if (pt.x >= rc.right - border_thickness.right)
                    hit |= right;
				if (pt.y <= border_thickness.top || pt.y < verticalBorderSize)
                    hit |= top;
				if (pt.y >= rc.bottom - border_thickness.bottom)
                    hit |= bottom;

				if (hit & top && hit & left)        return HTTOPLEFT;
				if (hit & top && hit & right)       return HTTOPRIGHT;
				if (hit & bottom && hit & left)     return HTBOTTOMLEFT;
				if (hit & bottom && hit & right)    return HTBOTTOMRIGHT;
				if (hit & left)                     return HTLEFT;
                if (hit & top)                      return HTTOP;
				if (hit & right)                    return HTRIGHT;
				if (hit & bottom)                   return HTBOTTOM;
			}

            // Then do client-side test which should determine titlebar bounds
			int titlebarHittest = 0;
			_lumInputTitleBarHitTest(window, pt.x, pt.y, &titlebarHittest);
			if (titlebarHittest)
				return HTCAPTION;

            // In client area
			return HTCLIENT;
        }
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

// Creates the Lumina window
//
static int createNativeWindow(_LUMwindow* window,
                              const _LUMwndconfig* wndconfig,
                              const _LUMfbconfig* fbconfig)
{
    int xpos, ypos, fullWidth, fullHeight;
    WCHAR* wideTitle;
    DWORD style = getWindowStyle(window);
    DWORD exStyle = getWindowExStyle(window);

    if (window->monitor)
    {
        LUMvidmode mode;

        // NOTE: This window placement is temporary and approximate, as the
        //       correct position and size cannot be known until the monitor
        //       video mode has been picked in _lumSetVideoModeWin32
        _lumGetMonitorPosWin32(window->monitor, &xpos, &ypos);
        _lumGetVideoModeWin32(window->monitor, &mode);
        fullWidth  = mode.width;
        fullHeight = mode.height;
    }
    else
    {
        xpos = CW_USEDEFAULT;
        ypos = CW_USEDEFAULT;

        window->win32.maximized = wndconfig->maximized;
        if (wndconfig->maximized)
            style |= WS_MAXIMIZE;

        getFullWindowSize(style, exStyle,
                          wndconfig->width, wndconfig->height,
                          &fullWidth, &fullHeight,
                          USER_DEFAULT_SCREEN_DPI);
    }

    wideTitle = _lumCreateWideStringFromUTF8Win32(wndconfig->title);
    if (!wideTitle)
        return LUM_FALSE;

    window->win32.handle = CreateWindowExW(exStyle,
                                           _LUM_WNDCLASSNAME,
                                           wideTitle,
                                           style,
                                           xpos, ypos,
                                           fullWidth, fullHeight,
                                           NULL, // No parent window
                                           NULL, // No window menu
                                           GetModuleHandleW(NULL),
                                           (LPVOID) wndconfig);

    _lum_free(wideTitle);

    if (!window->win32.handle)
    {
        _lumInputErrorWin32(LUM_PLATFORM_ERROR,
                             "Win32: Failed to create window");
        return LUM_FALSE;
    }

    SetPropW(window->win32.handle, L"Lumina", window);

    if (IsWindows7OrGreater())
    {
        ChangeWindowMessageFilterEx(window->win32.handle,
                                    WM_DROPFILES, MSGFLT_ALLOW, NULL);
        ChangeWindowMessageFilterEx(window->win32.handle,
                                    WM_COPYDATA, MSGFLT_ALLOW, NULL);
        ChangeWindowMessageFilterEx(window->win32.handle,
                                    WM_COPYGLOBALDATA, MSGFLT_ALLOW, NULL);
    }

    window->win32.scaleToMonitor = wndconfig->scaleToMonitor;
    window->win32.keymenu = wndconfig->win32.keymenu;

    // Adjust window rect to account for DPI scaling of the window frame and
    // (if enabled) DPI scaling of the content area
    // This cannot be done until we know what monitor the window was placed on
    if (!window->monitor)
    {
        RECT rect = { 0, 0, wndconfig->width, wndconfig->height };
        WINDOWPLACEMENT wp = { sizeof(wp) };

        if (wndconfig->scaleToMonitor)
        {
            float xscale, yscale;
            _lumGetWindowContentScaleWin32(window, &xscale, &yscale);

            if (xscale > 0.f && yscale > 0.f)
            {
                rect.right = (int) (rect.right * xscale);
                rect.bottom = (int) (rect.bottom * yscale);
            }
        }

        ClientToScreen(window->win32.handle, (POINT*) &rect.left);
        ClientToScreen(window->win32.handle, (POINT*) &rect.right);

        if (_lumIsWindows10AnniversaryUpdateOrGreaterWin32())
        {
            AdjustWindowRectExForDpi(&rect, style, FALSE, exStyle,
                                     GetDpiForWindow(window->win32.handle));
        }
        else
            AdjustWindowRectEx(&rect, style, FALSE, exStyle);

        // Only update the restored window rect as the window may be maximized
        GetWindowPlacement(window->win32.handle, &wp);
        wp.rcNormalPosition = rect;
        wp.showCmd = SW_HIDE;
        SetWindowPlacement(window->win32.handle, &wp);
    }

    DragAcceptFiles(window->win32.handle, TRUE);

    if (fbconfig->transparent)
    {
        updateFramebufferTransparency(window);
        window->win32.transparent = LUM_TRUE;
    }

    _lumGetWindowSizeWin32(window, &window->win32.width, &window->win32.height);

    return LUM_TRUE;
}

// Registers the Lumina window class
//
LUMbool _lumRegisterWindowClassWin32(void)
{
    WNDCLASSEXW wc;

    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc   = (WNDPROC) windowProc;
    wc.hInstance     = GetModuleHandleW(NULL);
    wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
    wc.lpszClassName = _LUM_WNDCLASSNAME;

    // Load user-provided icon if available
    wc.hIcon = LoadImageW(GetModuleHandleW(NULL),
                          L"LUM_ICON", IMAGE_ICON,
                          0, 0, LR_DEFAULTSIZE | LR_SHARED);
    if (!wc.hIcon)
    {
        // No user-provided icon found, load default icon
        wc.hIcon = LoadImageW(NULL,
                              IDI_APPLICATION, IMAGE_ICON,
                              0, 0, LR_DEFAULTSIZE | LR_SHARED);
    }

    if (!RegisterClassExW(&wc))
    {
        _lumInputErrorWin32(LUM_PLATFORM_ERROR,
                             "Win32: Failed to register window class");
        return LUM_FALSE;
    }

    return LUM_TRUE;
}

// Unregisters the Lumina window class
//
void _lumUnregisterWindowClassWin32(void)
{
    UnregisterClassW(_LUM_WNDCLASSNAME, GetModuleHandleW(NULL));
}

int _lumCreateWindowWin32(_LUMwindow* window,
                           const _LUMwndconfig* wndconfig,
                           const _LUMctxconfig* ctxconfig,
                           const _LUMfbconfig* fbconfig)
{
    if (!createNativeWindow(window, wndconfig, fbconfig))
        return LUM_FALSE;

    if (ctxconfig->client != LUM_NO_API)
    {
        if (ctxconfig->source == LUM_NATIVE_CONTEXT_API)
        {
            if (!_lumInitWGL())
                return LUM_FALSE;
            if (!_lumCreateContextWGL(window, ctxconfig, fbconfig))
                return LUM_FALSE;
        }
        else if (ctxconfig->source == LUM_EGL_CONTEXT_API)
        {
            if (!_lumInitEGL())
                return LUM_FALSE;
            if (!_lumCreateContextEGL(window, ctxconfig, fbconfig))
                return LUM_FALSE;
        }
        else if (ctxconfig->source == LUM_OSMESA_CONTEXT_API)
        {
            if (!_lumInitOSMesa())
                return LUM_FALSE;
            if (!_lumCreateContextOSMesa(window, ctxconfig, fbconfig))
                return LUM_FALSE;
        }
    }

    if (window->monitor)
    {
        _lumShowWindowWin32(window);
        _lumFocusWindowWin32(window);
        acquireMonitor(window);
        fitToMonitor(window);
    }

    return LUM_TRUE;
}

void _lumDestroyWindowWin32(_LUMwindow* window)
{
    if (window->monitor)
        releaseMonitor(window);

    if (window->context.destroy)
        window->context.destroy(window);

    if (_lum.win32.disabledCursorWindow == window)
        _lum.win32.disabledCursorWindow = NULL;

    if (window->win32.handle)
    {
        RemovePropW(window->win32.handle, L"Lumina");
        DestroyWindow(window->win32.handle);
        window->win32.handle = NULL;
    }

    if (window->win32.bigIcon)
        DestroyIcon(window->win32.bigIcon);

    if (window->win32.smallIcon)
        DestroyIcon(window->win32.smallIcon);
}

void _lumSetWindowTitleWin32(_LUMwindow* window, const char* title)
{
    WCHAR* wideTitle = _lumCreateWideStringFromUTF8Win32(title);
    if (!wideTitle)
        return;

    SetWindowTextW(window->win32.handle, wideTitle);
    _lum_free(wideTitle);
}

void _lumSetWindowIconWin32(_LUMwindow* window,
                             int count, const LUMimage* images)
{
    HICON bigIcon = NULL, smallIcon = NULL;

    if (count)
    {
        const LUMimage* bigImage = chooseImage(count, images,
                                                GetSystemMetrics(SM_CXICON),
                                                GetSystemMetrics(SM_CYICON));
        const LUMimage* smallImage = chooseImage(count, images,
                                                  GetSystemMetrics(SM_CXSMICON),
                                                  GetSystemMetrics(SM_CYSMICON));

        bigIcon = createIcon(bigImage, 0, 0, LUM_TRUE);
        smallIcon = createIcon(smallImage, 0, 0, LUM_TRUE);
    }
    else
    {
        bigIcon = (HICON) GetClassLongPtrW(window->win32.handle, GCLP_HICON);
        smallIcon = (HICON) GetClassLongPtrW(window->win32.handle, GCLP_HICONSM);
    }

    SendMessage(window->win32.handle, WM_SETICON, ICON_BIG, (LPARAM) bigIcon);
    SendMessage(window->win32.handle, WM_SETICON, ICON_SMALL, (LPARAM) smallIcon);

    if (window->win32.bigIcon)
        DestroyIcon(window->win32.bigIcon);

    if (window->win32.smallIcon)
        DestroyIcon(window->win32.smallIcon);

    if (count)
    {
        window->win32.bigIcon = bigIcon;
        window->win32.smallIcon = smallIcon;
    }
}

void _lumGetWindowPosWin32(_LUMwindow* window, int* xpos, int* ypos)
{
    POINT pos = { 0, 0 };
    ClientToScreen(window->win32.handle, &pos);

    if (xpos)
        *xpos = pos.x;
    if (ypos)
        *ypos = pos.y;
}

void _lumSetWindowPosWin32(_LUMwindow* window, int xpos, int ypos)
{
    RECT rect = { xpos, ypos, xpos, ypos };

    if (_lumIsWindows10AnniversaryUpdateOrGreaterWin32())
    {
        AdjustWindowRectExForDpi(&rect, getWindowStyle(window),
                                 FALSE, getWindowExStyle(window),
                                 GetDpiForWindow(window->win32.handle));
    }
    else
    {
        AdjustWindowRectEx(&rect, getWindowStyle(window),
                           FALSE, getWindowExStyle(window));
    }

    SetWindowPos(window->win32.handle, NULL, rect.left, rect.top, 0, 0,
                 SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}

void _lumGetWindowSizeWin32(_LUMwindow* window, int* width, int* height)
{
    RECT area;
    GetClientRect(window->win32.handle, &area);

    if (width)
        *width = area.right;
    if (height)
        *height = area.bottom;
}

void _lumSetWindowSizeWin32(_LUMwindow* window, int width, int height)
{
    if (window->monitor)
    {
        if (window->monitor->window == window)
        {
            acquireMonitor(window);
            fitToMonitor(window);
        }
    }
    else
    {
        RECT rect = { 0, 0, width, height };

        if (_lumIsWindows10AnniversaryUpdateOrGreaterWin32())
        {
            AdjustWindowRectExForDpi(&rect, getWindowStyle(window),
                                     FALSE, getWindowExStyle(window),
                                     GetDpiForWindow(window->win32.handle));
        }
        else
        {
            AdjustWindowRectEx(&rect, getWindowStyle(window),
                               FALSE, getWindowExStyle(window));
        }

        SetWindowPos(window->win32.handle, HWND_TOP,
                     0, 0, rect.right - rect.left, rect.bottom - rect.top,
                     SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
    }
}

void _lumSetWindowSizeLimitsWin32(_LUMwindow* window,
                                           int minwidth, int minheight,
                                           int maxwidth, int maxheight)
{
    RECT area;

    if ((minwidth == LUM_DONT_CARE || minheight == LUM_DONT_CARE) &&
        (maxwidth == LUM_DONT_CARE || maxheight == LUM_DONT_CARE))
    {
        return;
    }

    GetWindowRect(window->win32.handle, &area);
    MoveWindow(window->win32.handle,
               area.left, area.top,
               area.right - area.left,
               area.bottom - area.top, TRUE);
}

void _lumSetWindowAspectRatioWin32(_LUMwindow* window, int numer, int denom)
{
    RECT area;

    if (numer == LUM_DONT_CARE || denom == LUM_DONT_CARE)
        return;

    GetWindowRect(window->win32.handle, &area);
    applyAspectRatio(window, WMSZ_BOTTOMRIGHT, &area);
    MoveWindow(window->win32.handle,
               area.left, area.top,
               area.right - area.left,
               area.bottom - area.top, TRUE);
}

void _lumGetFramebufferSizeWin32(_LUMwindow* window, int* width, int* height)
{
    _lumGetWindowSizeWin32(window, width, height);
}

void _lumGetWindowFrameSizeWin32(_LUMwindow* window,
                                          int* left, int* top,
                                          int* right, int* bottom)
{
    RECT rect;
    int width, height;

    _lumGetWindowSizeWin32(window, &width, &height);
    SetRect(&rect, 0, 0, width, height);

    if (_lumIsWindows10AnniversaryUpdateOrGreaterWin32())
    {
        AdjustWindowRectExForDpi(&rect, getWindowStyle(window),
                                 FALSE, getWindowExStyle(window),
                                 GetDpiForWindow(window->win32.handle));
    }
    else
    {
        AdjustWindowRectEx(&rect, getWindowStyle(window),
                           FALSE, getWindowExStyle(window));
    }

    if (left)
        *left = -rect.left;
    if (top)
        *top = -rect.top;
    if (right)
        *right = rect.right - width;
    if (bottom)
        *bottom = rect.bottom - height;
}

void _lumGetWindowContentScaleWin32(_LUMwindow* window, float* xscale, float* yscale)
{
    const HANDLE handle = MonitorFromWindow(window->win32.handle,
                                            MONITOR_DEFAULTTONEAREST);
    _lumGetHMONITORContentScaleWin32(handle, xscale, yscale);
}

void _lumIconifyWindowWin32(_LUMwindow* window)
{
    ShowWindow(window->win32.handle, SW_MINIMIZE);
}

void _lumRestoreWindowWin32(_LUMwindow* window)
{
    ShowWindow(window->win32.handle, SW_RESTORE);
}

void _lumMaximizeWindowWin32(_LUMwindow* window)
{
    ShowWindow(window->win32.handle, SW_MAXIMIZE);
}

void _lumShowWindowWin32(_LUMwindow* window)
{
    ShowWindow(window->win32.handle, SW_SHOWNA);
}

void _lumHideWindowWin32(_LUMwindow* window)
{
    ShowWindow(window->win32.handle, SW_HIDE);
}

void _lumRequestWindowAttentionWin32(_LUMwindow* window)
{
    FlashWindow(window->win32.handle, TRUE);
}

void _lumFocusWindowWin32(_LUMwindow* window)
{
    BringWindowToTop(window->win32.handle);
    SetForegroundWindow(window->win32.handle);
    SetFocus(window->win32.handle);
}

void _lumSetWindowMonitorWin32(_LUMwindow* window,
                                _LUMmonitor* monitor,
                                int xpos, int ypos,
                                int width, int height,
                                int refreshRate)
{
    if (window->monitor == monitor)
    {
        if (monitor)
        {
            if (monitor->window == window)
            {
                acquireMonitor(window);
                fitToMonitor(window);
            }
        }
        else
        {
            RECT rect = { xpos, ypos, xpos + width, ypos + height };

            if (_lumIsWindows10AnniversaryUpdateOrGreaterWin32())
            {
                AdjustWindowRectExForDpi(&rect, getWindowStyle(window),
                                         FALSE, getWindowExStyle(window),
                                         GetDpiForWindow(window->win32.handle));
            }
            else
            {
                AdjustWindowRectEx(&rect, getWindowStyle(window),
                                   FALSE, getWindowExStyle(window));
            }

            SetWindowPos(window->win32.handle, HWND_TOP,
                         rect.left, rect.top,
                         rect.right - rect.left, rect.bottom - rect.top,
                         SWP_NOCOPYBITS | SWP_NOACTIVATE | SWP_NOZORDER);
        }

        return;
    }

    if (window->monitor)
        releaseMonitor(window);

    _lumInputWindowMonitor(window, monitor);

    if (window->monitor)
    {
        MONITORINFO mi = { sizeof(mi) };
        UINT flags = SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOCOPYBITS;

        if (window->decorated)
        {
            DWORD style = GetWindowLongW(window->win32.handle, GWL_STYLE);
            style &= ~WS_OVERLAPPEDWINDOW;
            style |= getWindowStyle(window);
            SetWindowLongW(window->win32.handle, GWL_STYLE, style);
            flags |= SWP_FRAMECHANGED;
        }

        acquireMonitor(window);

        GetMonitorInfo(window->monitor->win32.handle, &mi);
        SetWindowPos(window->win32.handle, HWND_TOPMOST,
                     mi.rcMonitor.left,
                     mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left,
                     mi.rcMonitor.bottom - mi.rcMonitor.top,
                     flags);
    }
    else
    {
        HWND after;
        RECT rect = { xpos, ypos, xpos + width, ypos + height };
        DWORD style = GetWindowLongW(window->win32.handle, GWL_STYLE);
        UINT flags = SWP_NOACTIVATE | SWP_NOCOPYBITS;

        if (window->decorated)
        {
            style &= ~WS_POPUP;
            style |= getWindowStyle(window);
            SetWindowLongW(window->win32.handle, GWL_STYLE, style);

            flags |= SWP_FRAMECHANGED;
        }

        if (window->floating)
            after = HWND_TOPMOST;
        else
            after = HWND_NOTOPMOST;

        if (_lumIsWindows10AnniversaryUpdateOrGreaterWin32())
        {
            AdjustWindowRectExForDpi(&rect, getWindowStyle(window),
                                     FALSE, getWindowExStyle(window),
                                     GetDpiForWindow(window->win32.handle));
        }
        else
        {
            AdjustWindowRectEx(&rect, getWindowStyle(window),
                               FALSE, getWindowExStyle(window));
        }

        SetWindowPos(window->win32.handle, after,
                     rect.left, rect.top,
                     rect.right - rect.left, rect.bottom - rect.top,
                     flags);
    }
}

int _lumWindowFocusedWin32(_LUMwindow* window)
{
    return window->win32.handle == GetActiveWindow();
}

int _lumWindowIconifiedWin32(_LUMwindow* window)
{
    return IsIconic(window->win32.handle);
}

int _lumWindowVisibleWin32(_LUMwindow* window)
{
    return IsWindowVisible(window->win32.handle);
}

int _lumWindowMaximizedWin32(_LUMwindow* window)
{
    return IsZoomed(window->win32.handle);
}

int _lumWindowHoveredWin32(_LUMwindow* window)
{
    return cursorInContentArea(window);
}

int _lumFramebufferTransparentWin32(_LUMwindow* window)
{
    BOOL composition, opaque;
    DWORD color;

    if (!window->win32.transparent)
        return LUM_FALSE;

    if (!IsWindowsVistaOrGreater())
        return LUM_FALSE;

    if (FAILED(DwmIsCompositionEnabled(&composition)) || !composition)
        return LUM_FALSE;

    if (!IsWindows8OrGreater())
    {
        // HACK: Disable framebuffer transparency on Windows 7 when the
        //       colorization color is opaque, because otherwise the window
        //       contents is blended additively with the previous frame instead
        //       of replacing it
        if (FAILED(DwmGetColorizationColor(&color, &opaque)) || opaque)
            return LUM_FALSE;
    }

    return LUM_TRUE;
}

void _lumSetWindowResizableWin32(_LUMwindow* window, LUMbool enabled)
{
    updateWindowStyles(window);
}

void _lumSetWindowDecoratedWin32(_LUMwindow* window, LUMbool enabled)
{
    updateWindowStyles(window);
}

void _lumSetWindowTitlebarWin32(_LUMwindow* window, LUMbool enabled)
{
    updateWindowStyles(window);
}

void _lumSetWindowFloatingWin32(_LUMwindow* window, LUMbool enabled)
{
    const HWND after = enabled ? HWND_TOPMOST : HWND_NOTOPMOST;
    SetWindowPos(window->win32.handle, after, 0, 0, 0, 0,
                 SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
}

void _lumSetWindowMousePassthroughWin32(_LUMwindow* window, LUMbool enabled)
{
    COLORREF key = 0;
    BYTE alpha = 0;
    DWORD flags = 0;
    DWORD exStyle = GetWindowLongW(window->win32.handle, GWL_EXSTYLE);

    if (exStyle & WS_EX_LAYERED)
        GetLayeredWindowAttributes(window->win32.handle, &key, &alpha, &flags);

    if (enabled)
        exStyle |= (WS_EX_TRANSPARENT | WS_EX_LAYERED);
    else
    {
        exStyle &= ~WS_EX_TRANSPARENT;
        // NOTE: Window opacity also needs the layered window style so do not
        //       remove it if the window is alpha blended
        if (exStyle & WS_EX_LAYERED)
        {
            if (!(flags & LWA_ALPHA))
                exStyle &= ~WS_EX_LAYERED;
        }
    }

    SetWindowLongW(window->win32.handle, GWL_EXSTYLE, exStyle);

    if (enabled)
        SetLayeredWindowAttributes(window->win32.handle, key, alpha, flags);
}

float _lumGetWindowOpacityWin32(_LUMwindow* window)
{
    BYTE alpha;
    DWORD flags;

    if ((GetWindowLongW(window->win32.handle, GWL_EXSTYLE) & WS_EX_LAYERED) &&
        GetLayeredWindowAttributes(window->win32.handle, NULL, &alpha, &flags))
    {
        if (flags & LWA_ALPHA)
            return alpha / 255.f;
    }

    return 1.f;
}

void _lumSetWindowOpacityWin32(_LUMwindow* window, float opacity)
{
    LONG exStyle = GetWindowLongW(window->win32.handle, GWL_EXSTYLE);
    if (opacity < 1.f || (exStyle & WS_EX_TRANSPARENT))
    {
        const BYTE alpha = (BYTE) (255 * opacity);
        exStyle |= WS_EX_LAYERED;
        SetWindowLongW(window->win32.handle, GWL_EXSTYLE, exStyle);
        SetLayeredWindowAttributes(window->win32.handle, 0, alpha, LWA_ALPHA);
    }
    else if (exStyle & WS_EX_TRANSPARENT)
    {
        SetLayeredWindowAttributes(window->win32.handle, 0, 0, 0);
    }
    else
    {
        exStyle &= ~WS_EX_LAYERED;
        SetWindowLongW(window->win32.handle, GWL_EXSTYLE, exStyle);
    }
}

void _lumSetRawMouseMotionWin32(_LUMwindow *window, LUMbool enabled)
{
    if (_lum.win32.disabledCursorWindow != window)
        return;

    if (enabled)
        enableRawMouseMotion(window);
    else
        disableRawMouseMotion(window);
}

LUMbool _lumRawMouseMotionSupportedWin32(void)
{
    return LUM_TRUE;
}

void _lumPollEventsWin32(void)
{
    MSG msg;
    HWND handle;
    _LUMwindow* window;

    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            // NOTE: While Lumina does not itself post WM_QUIT, other processes
            //       may post it to this one, for example Task Manager
            // HACK: Treat WM_QUIT as a close on all windows

            window = _lum.windowListHead;
            while (window)
            {
                _lumInputWindowCloseRequest(window);
                window = window->next;
            }
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    // HACK: Release modifier keys that the system did not emit KEYUP for
    // NOTE: Shift keys on Windows tend to "stick" when both are pressed as
    //       no key up message is generated by the first key release
    // NOTE: Windows key is not reported as released by the Win+V hotkey
    //       Other Win hotkeys are handled implicitly by _lumInputWindowFocus
    //       because they change the input focus
    // NOTE: The other half of this is in the WM_*KEY* handler in windowProc
    handle = GetActiveWindow();
    if (handle)
    {
        window = GetPropW(handle, L"Lumina");
        if (window)
        {
            int i;
            const int keys[4][2] =
            {
                { VK_LSHIFT, LUM_KEY_LEFT_SHIFT },
                { VK_RSHIFT, LUM_KEY_RIGHT_SHIFT },
                { VK_LWIN, LUM_KEY_LEFT_SUPER },
                { VK_RWIN, LUM_KEY_RIGHT_SUPER }
            };

            for (i = 0;  i < 4;  i++)
            {
                const int vk = keys[i][0];
                const int key = keys[i][1];
                const int scancode = _lum.win32.scancodes[key];

                if ((GetKeyState(vk) & 0x8000))
                    continue;
                if (window->keys[key] != LUM_PRESS)
                    continue;

                _lumInputKey(window, key, scancode, LUM_RELEASE, getKeyMods());
            }
        }
    }

    window = _lum.win32.disabledCursorWindow;
    if (window)
    {
        int width, height;
        _lumGetWindowSizeWin32(window, &width, &height);

        // NOTE: Re-center the cursor only if it has moved since the last call,
        //       to avoid breaking lumWaitEvents with WM_MOUSEMOVE
        if (window->win32.lastCursorPosX != width / 2 ||
            window->win32.lastCursorPosY != height / 2)
        {
            _lumSetCursorPosWin32(window, width / 2, height / 2);
        }
    }
}

void _lumWaitEventsWin32(void)
{
    WaitMessage();

    _lumPollEventsWin32();
}

void _lumWaitEventsTimeoutWin32(double timeout)
{
    MsgWaitForMultipleObjects(0, NULL, FALSE, (DWORD) (timeout * 1e3), QS_ALLEVENTS);

    _lumPollEventsWin32();
}

void _lumPostEmptyEventWin32(void)
{
    PostMessage(_lum.win32.helperWindowHandle, WM_NULL, 0, 0);
}

void _lumGetCursorPosWin32(_LUMwindow* window, double* xpos, double* ypos)
{
    POINT pos;

    if (GetCursorPos(&pos))
    {
        ScreenToClient(window->win32.handle, &pos);

        if (xpos)
            *xpos = pos.x;
        if (ypos)
            *ypos = pos.y;
    }
}

void _lumSetCursorPosWin32(_LUMwindow* window, double xpos, double ypos)
{
    POINT pos = { (int) xpos, (int) ypos };

    // Store the new position so it can be recognized later
    window->win32.lastCursorPosX = pos.x;
    window->win32.lastCursorPosY = pos.y;

    ClientToScreen(window->win32.handle, &pos);
    SetCursorPos(pos.x, pos.y);
}

void _lumSetCursorModeWin32(_LUMwindow* window, int mode)
{
    if (mode == LUM_CURSOR_DISABLED)
    {
        if (_lumWindowFocusedWin32(window))
            disableCursor(window);
    }
    else if (_lum.win32.disabledCursorWindow == window)
        enableCursor(window);
    else if (cursorInContentArea(window))
        updateCursorImage(window);
}

const char* _lumGetScancodeNameWin32(int scancode)
{
    if (scancode < 0 || scancode > (KF_EXTENDED | 0xff) ||
        _lum.win32.keycodes[scancode] == LUM_KEY_UNKNOWN)
    {
        _lumInputError(LUM_INVALID_VALUE, "Invalid scancode %i", scancode);
        return NULL;
    }

    return _lum.win32.keynames[_lum.win32.keycodes[scancode]];
}

int _lumGetKeyScancodeWin32(int key)
{
    return _lum.win32.scancodes[key];
}

int _lumCreateCursorWin32(_LUMcursor* cursor,
                                   const LUMimage* image,
                                   int xhot, int yhot)
{
    cursor->win32.handle = (HCURSOR) createIcon(image, xhot, yhot, LUM_FALSE);
    if (!cursor->win32.handle)
        return LUM_FALSE;

    return LUM_TRUE;
}

int _lumCreateStandardCursorWin32(_LUMcursor* cursor, int shape)
{
    int id = 0;

    switch (shape)
    {
        case LUM_ARROW_CURSOR:
            id = OCR_NORMAL;
            break;
        case LUM_IBEAM_CURSOR:
            id = OCR_IBEAM;
            break;
        case LUM_CROSSHAIR_CURSOR:
            id = OCR_CROSS;
            break;
        case LUM_POINTING_HAND_CURSOR:
            id = OCR_HAND;
            break;
        case LUM_RESIZE_EW_CURSOR:
            id = OCR_SIZEWE;
            break;
        case LUM_RESIZE_NS_CURSOR:
            id = OCR_SIZENS;
            break;
        case LUM_RESIZE_NWSE_CURSOR:
            id = OCR_SIZENWSE;
            break;
        case LUM_RESIZE_NESW_CURSOR:
            id = OCR_SIZENESW;
            break;
        case LUM_RESIZE_ALL_CURSOR:
            id = OCR_SIZEALL;
            break;
        case LUM_NOT_ALLOWED_CURSOR:
            id = OCR_NO;
            break;
        default:
            _lumInputError(LUM_PLATFORM_ERROR, "Win32: Unknown standard cursor");
            return LUM_FALSE;
    }

    cursor->win32.handle = LoadImageW(NULL,
                                      MAKEINTRESOURCEW(id), IMAGE_CURSOR, 0, 0,
                                      LR_DEFAULTSIZE | LR_SHARED);
    if (!cursor->win32.handle)
    {
        _lumInputErrorWin32(LUM_PLATFORM_ERROR,
                             "Win32: Failed to create standard cursor");
        return LUM_FALSE;
    }

    return LUM_TRUE;
}

void _lumDestroyCursorWin32(_LUMcursor* cursor)
{
    if (cursor->win32.handle)
        DestroyIcon((HICON) cursor->win32.handle);
}

void _lumSetCursorWin32(_LUMwindow* window, _LUMcursor* cursor)
{
    if (cursorInContentArea(window))
        updateCursorImage(window);
}

void _lumSetClipboardStringWin32(const char* string)
{
    int characterCount;
    HANDLE object;
    WCHAR* buffer;

    characterCount = MultiByteToWideChar(CP_UTF8, 0, string, -1, NULL, 0);
    if (!characterCount)
        return;

    object = GlobalAlloc(GMEM_MOVEABLE, characterCount * sizeof(WCHAR));
    if (!object)
    {
        _lumInputErrorWin32(LUM_PLATFORM_ERROR,
                             "Win32: Failed to allocate global handle for clipboard");
        return;
    }

    buffer = GlobalLock(object);
    if (!buffer)
    {
        _lumInputErrorWin32(LUM_PLATFORM_ERROR,
                             "Win32: Failed to lock global handle");
        GlobalFree(object);
        return;
    }

    MultiByteToWideChar(CP_UTF8, 0, string, -1, buffer, characterCount);
    GlobalUnlock(object);

    if (!OpenClipboard(_lum.win32.helperWindowHandle))
    {
        _lumInputErrorWin32(LUM_PLATFORM_ERROR,
                             "Win32: Failed to open clipboard");
        GlobalFree(object);
        return;
    }

    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, object);
    CloseClipboard();
}

const char* _lumGetClipboardStringWin32(void)
{
    HANDLE object;
    WCHAR* buffer;

    if (!OpenClipboard(_lum.win32.helperWindowHandle))
    {
        _lumInputErrorWin32(LUM_PLATFORM_ERROR,
                             "Win32: Failed to open clipboard");
        return NULL;
    }

    object = GetClipboardData(CF_UNICODETEXT);
    if (!object)
    {
        _lumInputErrorWin32(LUM_FORMAT_UNAVAILABLE,
                             "Win32: Failed to convert clipboard to string");
        CloseClipboard();
        return NULL;
    }

    buffer = GlobalLock(object);
    if (!buffer)
    {
        _lumInputErrorWin32(LUM_PLATFORM_ERROR,
                             "Win32: Failed to lock global handle");
        CloseClipboard();
        return NULL;
    }

    _lum_free(_lum.win32.clipboardString);
    _lum.win32.clipboardString = _lumCreateUTF8FromWideStringWin32(buffer);

    GlobalUnlock(object);
    CloseClipboard();

    return _lum.win32.clipboardString;
}

EGLenum _lumGetEGLPlatformWin32(EGLint** attribs)
{
    if (_lum.egl.ANGLE_platform_angle)
    {
        int type = 0;

        if (_lum.egl.ANGLE_platform_angle_opengl)
        {
            if (_lum.hints.init.angleType == LUM_ANGLE_PLATFORM_TYPE_OPENGL)
                type = EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE;
            else if (_lum.hints.init.angleType == LUM_ANGLE_PLATFORM_TYPE_OPENGLES)
                type = EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE;
        }

        if (_lum.egl.ANGLE_platform_angle_d3d)
        {
            if (_lum.hints.init.angleType == LUM_ANGLE_PLATFORM_TYPE_D3D9)
                type = EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE;
            else if (_lum.hints.init.angleType == LUM_ANGLE_PLATFORM_TYPE_D3D11)
                type = EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE;
        }

        if (_lum.egl.ANGLE_platform_angle_vulkan)
        {
            if (_lum.hints.init.angleType == LUM_ANGLE_PLATFORM_TYPE_VULKAN)
                type = EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE;
        }

        if (type)
        {
            *attribs = _lum_calloc(3, sizeof(EGLint));
            (*attribs)[0] = EGL_PLATFORM_ANGLE_TYPE_ANGLE;
            (*attribs)[1] = type;
            (*attribs)[2] = EGL_NONE;
            return EGL_PLATFORM_ANGLE_ANGLE;
        }
    }

    return 0;
}

EGLNativeDisplayType _lumGetEGLNativeDisplayWin32(void)
{
    return GetDC(_lum.win32.helperWindowHandle);
}

EGLNativeWindowType _lumGetEGLNativeWindowWin32(_LUMwindow* window)
{
    return window->win32.handle;
}

void _lumGetRequiredInstanceExtensionsWin32(char** extensions)
{
    if (!_lum.vk.KHR_surface || !_lum.vk.KHR_win32_surface)
        return;

    extensions[0] = "VK_KHR_surface";
    extensions[1] = "VK_KHR_win32_surface";
}

int _lumGetPhysicalDevicePresentationSupportWin32(VkInstance instance,
                                                   VkPhysicalDevice device,
                                                   uint32_t queuefamily)
{
    PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR
        vkGetPhysicalDeviceWin32PresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)
        vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
    if (!vkGetPhysicalDeviceWin32PresentationSupportKHR)
    {
        _lumInputError(LUM_API_UNAVAILABLE,
                        "Win32: Vulkan instance missing VK_KHR_win32_surface extension");
        return LUM_FALSE;
    }

    return vkGetPhysicalDeviceWin32PresentationSupportKHR(device, queuefamily);
}

VkResult _lumCreateWindowSurfaceWin32(VkInstance instance,
                                       _LUMwindow* window,
                                       const VkAllocationCallbacks* allocator,
                                       VkSurfaceKHR* surface)
{
    VkResult err;
    VkWin32SurfaceCreateInfoKHR sci;
    PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;

    vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)
        vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
    if (!vkCreateWin32SurfaceKHR)
    {
        _lumInputError(LUM_API_UNAVAILABLE,
                        "Win32: Vulkan instance missing VK_KHR_win32_surface extension");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    memset(&sci, 0, sizeof(sci));
    sci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    sci.hinstance = GetModuleHandle(NULL);
    sci.hwnd = window->win32.handle;

    err = vkCreateWin32SurfaceKHR(instance, &sci, allocator, surface);
    if (err)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Win32: Failed to create Vulkan surface: %s",
                        _lumGetVulkanResultString(err));
    }

    return err;
}

LUMAPI HWND lumGetWin32Window(LUMwindow* handle)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    if (_lum.platform.platformID != LUM_PLATFORM_WIN32)
    {
        _lumInputError(LUM_PLATFORM_UNAVAILABLE,
                        "Win32: Platform not initialized");
        return NULL;
    }

    return window->win32.handle;
}

