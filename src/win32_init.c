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

#include <stdlib.h>

static const GUID _lum_GUID_DEVINTERFACE_HID =
    {0x4d1e55b2,0xf16f,0x11cf,{0x88,0xcb,0x00,0x11,0x11,0x00,0x00,0x30}};

#define GUID_DEVINTERFACE_HID _lum_GUID_DEVINTERFACE_HID

#if defined(_LUM_USE_HYBRID_HPG) || defined(_LUM_USE_OPTIMUS_HPG)

#if defined(_LUM_BUILD_DLL)
 #pragma message("These symbols must be exported by the executable and have no effect in a DLL")
#endif

// Executables (but not DLLs) exporting this symbol with this value will be
// automatically directed to the high-performance GPU on Nvidia Optimus systems
// with up-to-date drivers
//
__declspec(dllexport) DWORD NvOptimusEnablement = 1;

// Executables (but not DLLs) exporting this symbol with this value will be
// automatically directed to the high-performance GPU on AMD PowerXpress systems
// with up-to-date drivers
//
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

#endif // _LUM_USE_HYBRID_HPG

#if defined(_LUM_BUILD_DLL)

// Lumina DLL entry point
//
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    return TRUE;
}

#endif // _LUM_BUILD_DLL

// Load necessary libraries (DLLs)
//
static LUMbool loadLibraries(void)
{
    _lum.win32.user32.instance = _lumPlatformLoadModule("user32.dll");
    if (!_lum.win32.user32.instance)
    {
        _lumInputErrorWin32(LUM_PLATFORM_ERROR,
                             "Win32: Failed to load user32.dll");
        return LUM_FALSE;
    }

    _lum.win32.user32.SetProcessDPIAware_ = (PFN_SetProcessDPIAware)
        _lumPlatformGetModuleSymbol(_lum.win32.user32.instance, "SetProcessDPIAware");
    _lum.win32.user32.ChangeWindowMessageFilterEx_ = (PFN_ChangeWindowMessageFilterEx)
        _lumPlatformGetModuleSymbol(_lum.win32.user32.instance, "ChangeWindowMessageFilterEx");
    _lum.win32.user32.EnableNonClientDpiScaling_ = (PFN_EnableNonClientDpiScaling)
        _lumPlatformGetModuleSymbol(_lum.win32.user32.instance, "EnableNonClientDpiScaling");
    _lum.win32.user32.SetProcessDpiAwarenessContext_ = (PFN_SetProcessDpiAwarenessContext)
        _lumPlatformGetModuleSymbol(_lum.win32.user32.instance, "SetProcessDpiAwarenessContext");
    _lum.win32.user32.GetDpiForWindow_ = (PFN_GetDpiForWindow)
        _lumPlatformGetModuleSymbol(_lum.win32.user32.instance, "GetDpiForWindow");
    _lum.win32.user32.AdjustWindowRectExForDpi_ = (PFN_AdjustWindowRectExForDpi)
        _lumPlatformGetModuleSymbol(_lum.win32.user32.instance, "AdjustWindowRectExForDpi");

    _lum.win32.dinput8.instance = _lumPlatformLoadModule("dinput8.dll");
    if (_lum.win32.dinput8.instance)
    {
        _lum.win32.dinput8.Create = (PFN_DirectInput8Create)
            _lumPlatformGetModuleSymbol(_lum.win32.dinput8.instance, "DirectInput8Create");
    }

    {
        int i;
        const char* names[] =
        {
            "xinput1_4.dll",
            "xinput1_3.dll",
            "xinput9_1_0.dll",
            "xinput1_2.dll",
            "xinput1_1.dll",
            NULL
        };

        for (i = 0;  names[i];  i++)
        {
            _lum.win32.xinput.instance = _lumPlatformLoadModule(names[i]);
            if (_lum.win32.xinput.instance)
            {
                _lum.win32.xinput.GetCapabilities = (PFN_XInputGetCapabilities)
                    _lumPlatformGetModuleSymbol(_lum.win32.xinput.instance, "XInputGetCapabilities");
                _lum.win32.xinput.GetState = (PFN_XInputGetState)
                    _lumPlatformGetModuleSymbol(_lum.win32.xinput.instance, "XInputGetState");

                break;
            }
        }
    }

    _lum.win32.dwmapi.instance = _lumPlatformLoadModule("dwmapi.dll");
    if (_lum.win32.dwmapi.instance)
    {
        _lum.win32.dwmapi.IsCompositionEnabled = (PFN_DwmIsCompositionEnabled)
            _lumPlatformGetModuleSymbol(_lum.win32.dwmapi.instance, "DwmIsCompositionEnabled");
        _lum.win32.dwmapi.Flush = (PFN_DwmFlush)
            _lumPlatformGetModuleSymbol(_lum.win32.dwmapi.instance, "DwmFlush");
        _lum.win32.dwmapi.EnableBlurBehindWindow = (PFN_DwmEnableBlurBehindWindow)
            _lumPlatformGetModuleSymbol(_lum.win32.dwmapi.instance, "DwmEnableBlurBehindWindow");
        _lum.win32.dwmapi.GetColorizationColor = (PFN_DwmGetColorizationColor)
            _lumPlatformGetModuleSymbol(_lum.win32.dwmapi.instance, "DwmGetColorizationColor");
    }

    _lum.win32.shcore.instance = _lumPlatformLoadModule("shcore.dll");
    if (_lum.win32.shcore.instance)
    {
        _lum.win32.shcore.SetProcessDpiAwareness_ = (PFN_SetProcessDpiAwareness)
            _lumPlatformGetModuleSymbol(_lum.win32.shcore.instance, "SetProcessDpiAwareness");
        _lum.win32.shcore.GetDpiForMonitor_ = (PFN_GetDpiForMonitor)
            _lumPlatformGetModuleSymbol(_lum.win32.shcore.instance, "GetDpiForMonitor");
    }

    _lum.win32.ntdll.instance = _lumPlatformLoadModule("ntdll.dll");
    if (_lum.win32.ntdll.instance)
    {
        _lum.win32.ntdll.RtlVerifyVersionInfo_ = (PFN_RtlVerifyVersionInfo)
            _lumPlatformGetModuleSymbol(_lum.win32.ntdll.instance, "RtlVerifyVersionInfo");
    }

    return LUM_TRUE;
}

// Unload used libraries (DLLs)
//
static void freeLibraries(void)
{
    if (_lum.win32.xinput.instance)
        _lumPlatformFreeModule(_lum.win32.xinput.instance);

    if (_lum.win32.dinput8.instance)
        _lumPlatformFreeModule(_lum.win32.dinput8.instance);

    if (_lum.win32.user32.instance)
        _lumPlatformFreeModule(_lum.win32.user32.instance);

    if (_lum.win32.dwmapi.instance)
        _lumPlatformFreeModule(_lum.win32.dwmapi.instance);

    if (_lum.win32.shcore.instance)
        _lumPlatformFreeModule(_lum.win32.shcore.instance);

    if (_lum.win32.ntdll.instance)
        _lumPlatformFreeModule(_lum.win32.ntdll.instance);
}

// Create key code translation tables
//
static void createKeyTables(void)
{
    int scancode;

    memset(_lum.win32.keycodes, -1, sizeof(_lum.win32.keycodes));
    memset(_lum.win32.scancodes, -1, sizeof(_lum.win32.scancodes));

    _lum.win32.keycodes[0x00B] = LUM_KEY_0;
    _lum.win32.keycodes[0x002] = LUM_KEY_1;
    _lum.win32.keycodes[0x003] = LUM_KEY_2;
    _lum.win32.keycodes[0x004] = LUM_KEY_3;
    _lum.win32.keycodes[0x005] = LUM_KEY_4;
    _lum.win32.keycodes[0x006] = LUM_KEY_5;
    _lum.win32.keycodes[0x007] = LUM_KEY_6;
    _lum.win32.keycodes[0x008] = LUM_KEY_7;
    _lum.win32.keycodes[0x009] = LUM_KEY_8;
    _lum.win32.keycodes[0x00A] = LUM_KEY_9;
    _lum.win32.keycodes[0x01E] = LUM_KEY_A;
    _lum.win32.keycodes[0x030] = LUM_KEY_B;
    _lum.win32.keycodes[0x02E] = LUM_KEY_C;
    _lum.win32.keycodes[0x020] = LUM_KEY_D;
    _lum.win32.keycodes[0x012] = LUM_KEY_E;
    _lum.win32.keycodes[0x021] = LUM_KEY_F;
    _lum.win32.keycodes[0x022] = LUM_KEY_G;
    _lum.win32.keycodes[0x023] = LUM_KEY_H;
    _lum.win32.keycodes[0x017] = LUM_KEY_I;
    _lum.win32.keycodes[0x024] = LUM_KEY_J;
    _lum.win32.keycodes[0x025] = LUM_KEY_K;
    _lum.win32.keycodes[0x026] = LUM_KEY_L;
    _lum.win32.keycodes[0x032] = LUM_KEY_M;
    _lum.win32.keycodes[0x031] = LUM_KEY_N;
    _lum.win32.keycodes[0x018] = LUM_KEY_O;
    _lum.win32.keycodes[0x019] = LUM_KEY_P;
    _lum.win32.keycodes[0x010] = LUM_KEY_Q;
    _lum.win32.keycodes[0x013] = LUM_KEY_R;
    _lum.win32.keycodes[0x01F] = LUM_KEY_S;
    _lum.win32.keycodes[0x014] = LUM_KEY_T;
    _lum.win32.keycodes[0x016] = LUM_KEY_U;
    _lum.win32.keycodes[0x02F] = LUM_KEY_V;
    _lum.win32.keycodes[0x011] = LUM_KEY_W;
    _lum.win32.keycodes[0x02D] = LUM_KEY_X;
    _lum.win32.keycodes[0x015] = LUM_KEY_Y;
    _lum.win32.keycodes[0x02C] = LUM_KEY_Z;

    _lum.win32.keycodes[0x028] = LUM_KEY_APOSTROPHE;
    _lum.win32.keycodes[0x02B] = LUM_KEY_BACKSLASH;
    _lum.win32.keycodes[0x033] = LUM_KEY_COMMA;
    _lum.win32.keycodes[0x00D] = LUM_KEY_EQUAL;
    _lum.win32.keycodes[0x029] = LUM_KEY_GRAVE_ACCENT;
    _lum.win32.keycodes[0x01A] = LUM_KEY_LEFT_BRACKET;
    _lum.win32.keycodes[0x00C] = LUM_KEY_MINUS;
    _lum.win32.keycodes[0x034] = LUM_KEY_PERIOD;
    _lum.win32.keycodes[0x01B] = LUM_KEY_RIGHT_BRACKET;
    _lum.win32.keycodes[0x027] = LUM_KEY_SEMICOLON;
    _lum.win32.keycodes[0x035] = LUM_KEY_SLASH;
    _lum.win32.keycodes[0x056] = LUM_KEY_WORLD_2;

    _lum.win32.keycodes[0x00E] = LUM_KEY_BACKSPACE;
    _lum.win32.keycodes[0x153] = LUM_KEY_DELETE;
    _lum.win32.keycodes[0x14F] = LUM_KEY_END;
    _lum.win32.keycodes[0x01C] = LUM_KEY_ENTER;
    _lum.win32.keycodes[0x001] = LUM_KEY_ESCAPE;
    _lum.win32.keycodes[0x147] = LUM_KEY_HOME;
    _lum.win32.keycodes[0x152] = LUM_KEY_INSERT;
    _lum.win32.keycodes[0x15D] = LUM_KEY_MENU;
    _lum.win32.keycodes[0x151] = LUM_KEY_PAGE_DOWN;
    _lum.win32.keycodes[0x149] = LUM_KEY_PAGE_UP;
    _lum.win32.keycodes[0x045] = LUM_KEY_PAUSE;
    _lum.win32.keycodes[0x146] = LUM_KEY_PAUSE;
    _lum.win32.keycodes[0x039] = LUM_KEY_SPACE;
    _lum.win32.keycodes[0x00F] = LUM_KEY_TAB;
    _lum.win32.keycodes[0x03A] = LUM_KEY_CAPS_LOCK;
    _lum.win32.keycodes[0x145] = LUM_KEY_NUM_LOCK;
    _lum.win32.keycodes[0x046] = LUM_KEY_SCROLL_LOCK;
    _lum.win32.keycodes[0x03B] = LUM_KEY_F1;
    _lum.win32.keycodes[0x03C] = LUM_KEY_F2;
    _lum.win32.keycodes[0x03D] = LUM_KEY_F3;
    _lum.win32.keycodes[0x03E] = LUM_KEY_F4;
    _lum.win32.keycodes[0x03F] = LUM_KEY_F5;
    _lum.win32.keycodes[0x040] = LUM_KEY_F6;
    _lum.win32.keycodes[0x041] = LUM_KEY_F7;
    _lum.win32.keycodes[0x042] = LUM_KEY_F8;
    _lum.win32.keycodes[0x043] = LUM_KEY_F9;
    _lum.win32.keycodes[0x044] = LUM_KEY_F10;
    _lum.win32.keycodes[0x057] = LUM_KEY_F11;
    _lum.win32.keycodes[0x058] = LUM_KEY_F12;
    _lum.win32.keycodes[0x064] = LUM_KEY_F13;
    _lum.win32.keycodes[0x065] = LUM_KEY_F14;
    _lum.win32.keycodes[0x066] = LUM_KEY_F15;
    _lum.win32.keycodes[0x067] = LUM_KEY_F16;
    _lum.win32.keycodes[0x068] = LUM_KEY_F17;
    _lum.win32.keycodes[0x069] = LUM_KEY_F18;
    _lum.win32.keycodes[0x06A] = LUM_KEY_F19;
    _lum.win32.keycodes[0x06B] = LUM_KEY_F20;
    _lum.win32.keycodes[0x06C] = LUM_KEY_F21;
    _lum.win32.keycodes[0x06D] = LUM_KEY_F22;
    _lum.win32.keycodes[0x06E] = LUM_KEY_F23;
    _lum.win32.keycodes[0x076] = LUM_KEY_F24;
    _lum.win32.keycodes[0x038] = LUM_KEY_LEFT_ALT;
    _lum.win32.keycodes[0x01D] = LUM_KEY_LEFT_CONTROL;
    _lum.win32.keycodes[0x02A] = LUM_KEY_LEFT_SHIFT;
    _lum.win32.keycodes[0x15B] = LUM_KEY_LEFT_SUPER;
    _lum.win32.keycodes[0x137] = LUM_KEY_PRINT_SCREEN;
    _lum.win32.keycodes[0x138] = LUM_KEY_RIGHT_ALT;
    _lum.win32.keycodes[0x11D] = LUM_KEY_RIGHT_CONTROL;
    _lum.win32.keycodes[0x036] = LUM_KEY_RIGHT_SHIFT;
    _lum.win32.keycodes[0x15C] = LUM_KEY_RIGHT_SUPER;
    _lum.win32.keycodes[0x150] = LUM_KEY_DOWN;
    _lum.win32.keycodes[0x14B] = LUM_KEY_LEFT;
    _lum.win32.keycodes[0x14D] = LUM_KEY_RIGHT;
    _lum.win32.keycodes[0x148] = LUM_KEY_UP;

    _lum.win32.keycodes[0x052] = LUM_KEY_KP_0;
    _lum.win32.keycodes[0x04F] = LUM_KEY_KP_1;
    _lum.win32.keycodes[0x050] = LUM_KEY_KP_2;
    _lum.win32.keycodes[0x051] = LUM_KEY_KP_3;
    _lum.win32.keycodes[0x04B] = LUM_KEY_KP_4;
    _lum.win32.keycodes[0x04C] = LUM_KEY_KP_5;
    _lum.win32.keycodes[0x04D] = LUM_KEY_KP_6;
    _lum.win32.keycodes[0x047] = LUM_KEY_KP_7;
    _lum.win32.keycodes[0x048] = LUM_KEY_KP_8;
    _lum.win32.keycodes[0x049] = LUM_KEY_KP_9;
    _lum.win32.keycodes[0x04E] = LUM_KEY_KP_ADD;
    _lum.win32.keycodes[0x053] = LUM_KEY_KP_DECIMAL;
    _lum.win32.keycodes[0x135] = LUM_KEY_KP_DIVIDE;
    _lum.win32.keycodes[0x11C] = LUM_KEY_KP_ENTER;
    _lum.win32.keycodes[0x059] = LUM_KEY_KP_EQUAL;
    _lum.win32.keycodes[0x037] = LUM_KEY_KP_MULTIPLY;
    _lum.win32.keycodes[0x04A] = LUM_KEY_KP_SUBTRACT;

    for (scancode = 0;  scancode < 512;  scancode++)
    {
        if (_lum.win32.keycodes[scancode] > 0)
            _lum.win32.scancodes[_lum.win32.keycodes[scancode]] = scancode;
    }
}

// Creates a dummy window for behind-the-scenes work
//
static LUMbool createHelperWindow(void)
{
    MSG msg;

    _lum.win32.helperWindowHandle =
        CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
                        _LUM_WNDCLASSNAME,
                        L"Lumina message window",
                        WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                        0, 0, 1, 1,
                        NULL, NULL,
                        GetModuleHandleW(NULL),
                        NULL);

    if (!_lum.win32.helperWindowHandle)
    {
        _lumInputErrorWin32(LUM_PLATFORM_ERROR,
                             "Win32: Failed to create helper window");
        return LUM_FALSE;
    }

    // HACK: The command to the first ShowWindow call is ignored if the parent
    //       process passed along a STARTUPINFO, so clear that with a no-op call
    ShowWindow(_lum.win32.helperWindowHandle, SW_HIDE);

    // Register for HID device notifications
    {
        DEV_BROADCAST_DEVICEINTERFACE_W dbi;
        ZeroMemory(&dbi, sizeof(dbi));
        dbi.dbcc_size = sizeof(dbi);
        dbi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        dbi.dbcc_classguid = GUID_DEVINTERFACE_HID;

        _lum.win32.deviceNotificationHandle =
            RegisterDeviceNotificationW(_lum.win32.helperWindowHandle,
                                        (DEV_BROADCAST_HDR*) &dbi,
                                        DEVICE_NOTIFY_WINDOW_HANDLE);
    }

    while (PeekMessageW(&msg, _lum.win32.helperWindowHandle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

   return LUM_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                       Lumina internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Returns a wide string version of the specified UTF-8 string
//
WCHAR* _lumCreateWideStringFromUTF8Win32(const char* source)
{
    WCHAR* target;
    int count;

    count = MultiByteToWideChar(CP_UTF8, 0, source, -1, NULL, 0);
    if (!count)
    {
        _lumInputErrorWin32(LUM_PLATFORM_ERROR,
                             "Win32: Failed to convert string from UTF-8");
        return NULL;
    }

    target = _lum_calloc(count, sizeof(WCHAR));

    if (!MultiByteToWideChar(CP_UTF8, 0, source, -1, target, count))
    {
        _lumInputErrorWin32(LUM_PLATFORM_ERROR,
                             "Win32: Failed to convert string from UTF-8");
        _lum_free(target);
        return NULL;
    }

    return target;
}

// Returns a UTF-8 string version of the specified wide string
//
char* _lumCreateUTF8FromWideStringWin32(const WCHAR* source)
{
    char* target;
    int size;

    size = WideCharToMultiByte(CP_UTF8, 0, source, -1, NULL, 0, NULL, NULL);
    if (!size)
    {
        _lumInputErrorWin32(LUM_PLATFORM_ERROR,
                             "Win32: Failed to convert string to UTF-8");
        return NULL;
    }

    target = _lum_calloc(size, 1);

    if (!WideCharToMultiByte(CP_UTF8, 0, source, -1, target, size, NULL, NULL))
    {
        _lumInputErrorWin32(LUM_PLATFORM_ERROR,
                             "Win32: Failed to convert string to UTF-8");
        _lum_free(target);
        return NULL;
    }

    return target;
}

// Reports the specified error, appending information about the last Win32 error
//
void _lumInputErrorWin32(int error, const char* description)
{
    WCHAR buffer[_LUM_MESSAGE_SIZE] = L"";
    char message[_LUM_MESSAGE_SIZE] = "";

    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_IGNORE_INSERTS |
                       FORMAT_MESSAGE_MAX_WIDTH_MASK,
                   NULL,
                   GetLastError() & 0xffff,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   buffer,
                   sizeof(buffer) / sizeof(WCHAR),
                   NULL);
    WideCharToMultiByte(CP_UTF8, 0, buffer, -1, message, sizeof(message), NULL, NULL);

    _lumInputError(error, "%s: %s", description, message);
}

// Updates key names according to the current keyboard layout
//
void _lumUpdateKeyNamesWin32(void)
{
    int key;
    BYTE state[256] = {0};

    memset(_lum.win32.keynames, 0, sizeof(_lum.win32.keynames));

    for (key = LUM_KEY_SPACE;  key <= LUM_KEY_LAST;  key++)
    {
        UINT vk;
        int scancode, length;
        WCHAR chars[16];

        scancode = _lum.win32.scancodes[key];
        if (scancode == -1)
            continue;

        if (key >= LUM_KEY_KP_0 && key <= LUM_KEY_KP_ADD)
        {
            const UINT vks[] = {
                VK_NUMPAD0,  VK_NUMPAD1,  VK_NUMPAD2, VK_NUMPAD3,
                VK_NUMPAD4,  VK_NUMPAD5,  VK_NUMPAD6, VK_NUMPAD7,
                VK_NUMPAD8,  VK_NUMPAD9,  VK_DECIMAL, VK_DIVIDE,
                VK_MULTIPLY, VK_SUBTRACT, VK_ADD
            };

            vk = vks[key - LUM_KEY_KP_0];
        }
        else
            vk = MapVirtualKey(scancode, MAPVK_VSC_TO_VK);

        length = ToUnicode(vk, scancode, state,
                           chars, sizeof(chars) / sizeof(WCHAR),
                           0);

        if (length == -1)
        {
            length = ToUnicode(vk, scancode, state,
                               chars, sizeof(chars) / sizeof(WCHAR),
                               0);
        }

        if (length < 1)
            continue;

        WideCharToMultiByte(CP_UTF8, 0, chars, 1,
                            _lum.win32.keynames[key],
                            sizeof(_lum.win32.keynames[key]),
                            NULL, NULL);
    }
}

// Replacement for IsWindowsVersionOrGreater as MinGW lacks versionhelpers.h
//
BOOL _lumIsWindowsVersionOrGreaterWin32(WORD major, WORD minor, WORD sp)
{
    OSVERSIONINFOEXW osvi = { sizeof(osvi), major, minor, 0, 0, {0}, sp };
    DWORD mask = VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR;
    ULONGLONG cond = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_MINORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
    // HACK: Use RtlVerifyVersionInfo instead of VerifyVersionInfoW as the
    //       latter lies unless the user knew to embed a non-default manifest
    //       announcing support for Windows 10 via supportedOS GUID
    return RtlVerifyVersionInfo(&osvi, mask, cond) == 0;
}

// Checks whether we are on at least the specified build of Windows 10
//
BOOL _lumIsWindows10BuildOrGreaterWin32(WORD build)
{
    OSVERSIONINFOEXW osvi = { sizeof(osvi), 10, 0, build };
    DWORD mask = VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER;
    ULONGLONG cond = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_MINORVERSION, VER_GREATER_EQUAL);
    cond = VerSetConditionMask(cond, VER_BUILDNUMBER, VER_GREATER_EQUAL);
    // HACK: Use RtlVerifyVersionInfo instead of VerifyVersionInfoW as the
    //       latter lies unless the user knew to embed a non-default manifest
    //       announcing support for Windows 10 via supportedOS GUID
    return RtlVerifyVersionInfo(&osvi, mask, cond) == 0;
}

LUMbool _lumConnectWin32(int platformID, _LUMplatform* platform)
{
    const _LUMplatform win32 =
    {
        LUM_PLATFORM_WIN32,
        _lumInitWin32,
        _lumTerminateWin32,
        _lumGetCursorPosWin32,
        _lumSetCursorPosWin32,
        _lumSetCursorModeWin32,
        _lumSetRawMouseMotionWin32,
        _lumRawMouseMotionSupportedWin32,
        _lumCreateCursorWin32,
        _lumCreateStandardCursorWin32,
        _lumDestroyCursorWin32,
        _lumSetCursorWin32,
        _lumGetScancodeNameWin32,
        _lumGetKeyScancodeWin32,
        _lumSetClipboardStringWin32,
        _lumGetClipboardStringWin32,
        _lumInitJoysticksWin32,
        _lumTerminateJoysticksWin32,
        _lumPollJoystickWin32,
        _lumGetMappingNameWin32,
        _lumUpdateGamepadGUIDWin32,
        _lumFreeMonitorWin32,
        _lumGetMonitorPosWin32,
        _lumGetMonitorContentScaleWin32,
        _lumGetMonitorWorkareaWin32,
        _lumGetVideoModesWin32,
        _lumGetVideoModeWin32,
        _lumGetGammaRampWin32,
        _lumSetGammaRampWin32,
        _lumCreateWindowWin32,
        _lumDestroyWindowWin32,
        _lumSetWindowTitleWin32,
        _lumSetWindowIconWin32,
        _lumGetWindowPosWin32,
        _lumSetWindowPosWin32,
        _lumGetWindowSizeWin32,
        _lumSetWindowSizeWin32,
        _lumSetWindowSizeLimitsWin32,
        _lumSetWindowAspectRatioWin32,
        _lumGetFramebufferSizeWin32,
        _lumGetWindowFrameSizeWin32,
        _lumGetWindowContentScaleWin32,
        _lumIconifyWindowWin32,
        _lumRestoreWindowWin32,
        _lumMaximizeWindowWin32,
        _lumShowWindowWin32,
        _lumHideWindowWin32,
        _lumRequestWindowAttentionWin32,
        _lumFocusWindowWin32,
        _lumSetWindowMonitorWin32,
        _lumWindowFocusedWin32,
        _lumWindowIconifiedWin32,
        _lumWindowVisibleWin32,
        _lumWindowMaximizedWin32,
        _lumWindowHoveredWin32,
        _lumFramebufferTransparentWin32,
        _lumGetWindowOpacityWin32,
        _lumSetWindowResizableWin32,
        _lumSetWindowDecoratedWin32,
        _lumSetWindowFloatingWin32,
        _lumSetWindowOpacityWin32,
        _lumSetWindowMousePassthroughWin32,
        _lumPollEventsWin32,
        _lumWaitEventsWin32,
        _lumWaitEventsTimeoutWin32,
        _lumPostEmptyEventWin32,
        _lumSetWindowTitlebarWin32,
        _lumGetEGLPlatformWin32,
        _lumGetEGLNativeDisplayWin32,
        _lumGetEGLNativeWindowWin32,
        _lumGetRequiredInstanceExtensionsWin32,
        _lumGetPhysicalDevicePresentationSupportWin32,
        _lumCreateWindowSurfaceWin32,
    };

    *platform = win32;
    return LUM_TRUE;
}

int _lumInitWin32(void)
{
    if (!loadLibraries())
        return LUM_FALSE;

    createKeyTables();
    _lumUpdateKeyNamesWin32();

    if (_lumIsWindows10CreatorsUpdateOrGreaterWin32())
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    else if (IsWindows8Point1OrGreater())
        SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    else if (IsWindowsVistaOrGreater())
        SetProcessDPIAware();

    if (!_lumRegisterWindowClassWin32())
        return LUM_FALSE;

    if (!createHelperWindow())
        return LUM_FALSE;

    _lumPollMonitorsWin32();
    return LUM_TRUE;
}

void _lumTerminateWin32(void)
{
    if (_lum.win32.deviceNotificationHandle)
        UnregisterDeviceNotification(_lum.win32.deviceNotificationHandle);

    if (_lum.win32.helperWindowHandle)
        DestroyWindow(_lum.win32.helperWindowHandle);

    _lumUnregisterWindowClassWin32();

    _lum_free(_lum.win32.clipboardString);
    _lum_free(_lum.win32.rawInput);

    _lumTerminateWGL();
    _lumTerminateEGL();

    freeLibraries();
}

