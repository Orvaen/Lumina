//========================================================================
// Lumina - fork of GLFW 3.4 (www.glfw.org)
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2018 Camilla Löwy <elmindreda@glfw.org>
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>


// NOTE: The global variables below comprise all mutable global data in Lumina
//       Any other mutable global variable is a bug

// This contains all mutable state shared between compilation units of Lumina
//
_LUMlibrary _lum = { LUM_FALSE };

// These are outside of _lum so they can be used before initialization and
// after termination without special handling when _lum is cleared to zero
//
static _LUMerror _lumMainThreadError;
static LUMerrorfun _lumErrorCallback;
static LUMallocator _lumInitAllocator;
static _LUMinitconfig _lumInitHints =
{
    LUM_TRUE,      // hat buttons
    LUM_ANGLE_PLATFORM_TYPE_NONE, // ANGLE backend
    LUM_ANY_PLATFORM, // preferred platform
    NULL,           // vkGetInstanceProcAddr function
    {
        LUM_TRUE,  // macOS menu bar
        LUM_TRUE   // macOS bundle chdir
    },
    {
        LUM_TRUE,  // X11 XCB Vulkan surface
    },
};

// The allocation function used when no custom allocator is set
//
static void* defaultAllocate(size_t size, void* user)
{
    return malloc(size);
}

// The deallocation function used when no custom allocator is set
//
static void defaultDeallocate(void* block, void* user)
{
    free(block);
}

// The reallocation function used when no custom allocator is set
//
static void* defaultReallocate(void* block, size_t size, void* user)
{
    return realloc(block, size);
}

// Terminate the library
//
static void terminate(void)
{
    int i;

    memset(&_lum.callbacks, 0, sizeof(_lum.callbacks));

    while (_lum.windowListHead)
        lumDestroyWindow((LUMwindow*) _lum.windowListHead);

    while (_lum.cursorListHead)
        lumDestroyCursor((LUMcursor*) _lum.cursorListHead);

    for (i = 0;  i < _lum.monitorCount;  i++)
    {
        _LUMmonitor* monitor = _lum.monitors[i];
        if (monitor->originalRamp.size)
            _lum.platform.setGammaRamp(monitor, &monitor->originalRamp);
        _lumFreeMonitor(monitor);
    }

    _lum_free(_lum.monitors);
    _lum.monitors = NULL;
    _lum.monitorCount = 0;

    _lum_free(_lum.mappings);
    _lum.mappings = NULL;
    _lum.mappingCount = 0;

    _lumTerminateVulkan();
    _lum.platform.terminateJoysticks();
    _lum.platform.terminate();

    _lum.initialized = LUM_FALSE;

    while (_lum.errorListHead)
    {
        _LUMerror* error = _lum.errorListHead;
        _lum.errorListHead = error->next;
        _lum_free(error);
    }

    _lumPlatformDestroyTls(&_lum.contextSlot);
    _lumPlatformDestroyTls(&_lum.errorSlot);
    _lumPlatformDestroyMutex(&_lum.errorLock);

    memset(&_lum, 0, sizeof(_lum));
}


//////////////////////////////////////////////////////////////////////////
//////                       Lumina internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Encode a Unicode code point to a UTF-8 stream
// Based on cutef8 by Jeff Bezanson (Public Domain)
//
size_t _lumEncodeUTF8(char* s, uint32_t codepoint)
{
    size_t count = 0;

    if (codepoint < 0x80)
        s[count++] = (char) codepoint;
    else if (codepoint < 0x800)
    {
        s[count++] = (codepoint >> 6) | 0xc0;
        s[count++] = (codepoint & 0x3f) | 0x80;
    }
    else if (codepoint < 0x10000)
    {
        s[count++] = (codepoint >> 12) | 0xe0;
        s[count++] = ((codepoint >> 6) & 0x3f) | 0x80;
        s[count++] = (codepoint & 0x3f) | 0x80;
    }
    else if (codepoint < 0x110000)
    {
        s[count++] = (codepoint >> 18) | 0xf0;
        s[count++] = ((codepoint >> 12) & 0x3f) | 0x80;
        s[count++] = ((codepoint >> 6) & 0x3f) | 0x80;
        s[count++] = (codepoint & 0x3f) | 0x80;
    }

    return count;
}

char* _lum_strdup(const char* source)
{
    const size_t length = strlen(source);
    char* result = _lum_calloc(length + 1, 1);
    strcpy(result, source);
    return result;
}

float _lum_fminf(float a, float b)
{
    if (a != a)
        return b;
    else if (b != b)
        return a;
    else if (a < b)
        return a;
    else
        return b;
}

float _lum_fmaxf(float a, float b)
{
    if (a != a)
        return b;
    else if (b != b)
        return a;
    else if (a > b)
        return a;
    else
        return b;
}

void* _lum_calloc(size_t count, size_t size)
{
    if (count && size)
    {
        void* block;

        if (count > SIZE_MAX / size)
        {
            _lumInputError(LUM_INVALID_VALUE, "Allocation size overflow");
            return NULL;
        }

        block = _lum.allocator.allocate(count * size, _lum.allocator.user);
        if (block)
            return memset(block, 0, count * size);
        else
        {
            _lumInputError(LUM_OUT_OF_MEMORY, NULL);
            return NULL;
        }
    }
    else
        return NULL;
}

void* _lum_realloc(void* block, size_t size)
{
    if (block && size)
    {
        void* resized = _lum.allocator.reallocate(block, size, _lum.allocator.user);
        if (resized)
            return resized;
        else
        {
            _lumInputError(LUM_OUT_OF_MEMORY, NULL);
            return NULL;
        }
    }
    else if (block)
    {
        _lum_free(block);
        return NULL;
    }
    else
        return _lum_calloc(1, size);
}

void _lum_free(void* block)
{
    if (block)
        _lum.allocator.deallocate(block, _lum.allocator.user);
}


//////////////////////////////////////////////////////////////////////////
//////                         Lumina event API                       //////
//////////////////////////////////////////////////////////////////////////

// Notifies shared code of an error
//
void _lumInputError(int code, const char* format, ...)
{
    _LUMerror* error;
    char description[_LUM_MESSAGE_SIZE];

    if (format)
    {
        va_list vl;

        va_start(vl, format);
        vsnprintf(description, sizeof(description), format, vl);
        va_end(vl);

        description[sizeof(description) - 1] = '\0';
    }
    else
    {
        if (code == LUM_NOT_INITIALIZED)
            strcpy(description, "The Lumina library is not initialized");
        else if (code == LUM_NO_CURRENT_CONTEXT)
            strcpy(description, "There is no current context");
        else if (code == LUM_INVALID_ENUM)
            strcpy(description, "Invalid argument for enum parameter");
        else if (code == LUM_INVALID_VALUE)
            strcpy(description, "Invalid value for parameter");
        else if (code == LUM_OUT_OF_MEMORY)
            strcpy(description, "Out of memory");
        else if (code == LUM_API_UNAVAILABLE)
            strcpy(description, "The requested API is unavailable");
        else if (code == LUM_VERSION_UNAVAILABLE)
            strcpy(description, "The requested API version is unavailable");
        else if (code == LUM_PLATFORM_ERROR)
            strcpy(description, "A platform-specific error occurred");
        else if (code == LUM_FORMAT_UNAVAILABLE)
            strcpy(description, "The requested format is unavailable");
        else if (code == LUM_NO_WINDOW_CONTEXT)
            strcpy(description, "The specified window has no context");
        else if (code == LUM_CURSOR_UNAVAILABLE)
            strcpy(description, "The specified cursor shape is unavailable");
        else if (code == LUM_FEATURE_UNAVAILABLE)
            strcpy(description, "The requested feature cannot be implemented for this platform");
        else if (code == LUM_FEATURE_UNIMPLEMENTED)
            strcpy(description, "The requested feature has not yet been implemented for this platform");
        else if (code == LUM_PLATFORM_UNAVAILABLE)
            strcpy(description, "The requested platform is unavailable");
        else
            strcpy(description, "ERROR: UNKNOWN Lumina ERROR");
    }

    if (_lum.initialized)
    {
        error = _lumPlatformGetTls(&_lum.errorSlot);
        if (!error)
        {
            error = _lum_calloc(1, sizeof(_LUMerror));
            _lumPlatformSetTls(&_lum.errorSlot, error);
            _lumPlatformLockMutex(&_lum.errorLock);
            error->next = _lum.errorListHead;
            _lum.errorListHead = error;
            _lumPlatformUnlockMutex(&_lum.errorLock);
        }
    }
    else
        error = &_lumMainThreadError;

    error->code = code;
    strcpy(error->description, description);

    if (_lumErrorCallback)
        _lumErrorCallback(code, description);
}


//////////////////////////////////////////////////////////////////////////
//////                        Lumina public API                       //////
//////////////////////////////////////////////////////////////////////////

LUMAPI int lumInit(void)
{
    if (_lum.initialized)
        return LUM_TRUE;

    memset(&_lum, 0, sizeof(_lum));
    _lum.hints.init = _lumInitHints;

    _lum.allocator = _lumInitAllocator;
    if (!_lum.allocator.allocate)
    {
        _lum.allocator.allocate   = defaultAllocate;
        _lum.allocator.reallocate = defaultReallocate;
        _lum.allocator.deallocate = defaultDeallocate;
    }

    if (!_lumSelectPlatform(_lum.hints.init.platformID, &_lum.platform))
        return LUM_FALSE;

    if (!_lum.platform.init())
    {
        terminate();
        return LUM_FALSE;
    }

    if (!_lumPlatformCreateMutex(&_lum.errorLock) ||
        !_lumPlatformCreateTls(&_lum.errorSlot) ||
        !_lumPlatformCreateTls(&_lum.contextSlot))
    {
        terminate();
        return LUM_FALSE;
    }

    _lumPlatformSetTls(&_lum.errorSlot, &_lumMainThreadError);

    _lumInitGamepadMappings();

    _lumPlatformInitTimer();
    _lum.timer.offset = _lumPlatformGetTimerValue();

    _lum.initialized = LUM_TRUE;

    lumDefaultWindowHints();
    return LUM_TRUE;
}

LUMAPI void lumTerminate(void)
{
    if (!_lum.initialized)
        return;

    terminate();
}

LUMAPI void lumInitHint(int hint, int value)
{
    switch (hint)
    {
        case LUM_JOYSTICK_HAT_BUTTONS:
            _lumInitHints.hatButtons = value;
            return;
        case LUM_ANGLE_PLATFORM_TYPE:
            _lumInitHints.angleType = value;
            return;
        case LUM_PLATFORM:
            _lumInitHints.platformID = value;
            return;
        case LUM_COCOA_CHDIR_RESOURCES:
            _lumInitHints.ns.chdir = value;
            return;
        case LUM_COCOA_MENUBAR:
            _lumInitHints.ns.menubar = value;
            return;
        case LUM_X11_XCB_VULKAN_SURFACE:
            _lumInitHints.x11.xcbVulkanSurface = value;
            return;
    }

    _lumInputError(LUM_INVALID_ENUM,
                    "Invalid init hint 0x%08X", hint);
}

LUMAPI void lumInitAllocator(const LUMallocator* allocator)
{
    if (allocator)
    {
        if (allocator->allocate && allocator->reallocate && allocator->deallocate)
            _lumInitAllocator = *allocator;
        else
            _lumInputError(LUM_INVALID_VALUE, "Missing function in allocator");
    }
    else
        memset(&_lumInitAllocator, 0, sizeof(LUMallocator));
}

LUMAPI void lumInitVulkanLoader(PFN_vkGetInstanceProcAddr loader)
{
    _lumInitHints.vulkanLoader = loader;
}

LUMAPI void lumGetVersion(int* major, int* minor, int* rev)
{
    if (major != NULL)
        *major = LUM_VERSION_MAJOR;
    if (minor != NULL)
        *minor = LUM_VERSION_MINOR;
    if (rev != NULL)
        *rev = LUM_VERSION_REVISION;
}

LUMAPI int lumGetError(const char** description)
{
    _LUMerror* error;
    int code = LUM_NO_ERROR;

    if (description)
        *description = NULL;

    if (_lum.initialized)
        error = _lumPlatformGetTls(&_lum.errorSlot);
    else
        error = &_lumMainThreadError;

    if (error)
    {
        code = error->code;
        error->code = LUM_NO_ERROR;
        if (description && code)
            *description = error->description;
    }

    return code;
}

LUMAPI LUMerrorfun lumSetErrorCallback(LUMerrorfun cbfun)
{
    _LUM_SWAP(LUMerrorfun, _lumErrorCallback, cbfun);
    return cbfun;
}

