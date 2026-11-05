//========================================================================
// Lumina Win32 - fork of GLFW 3.4 (www.glfw.org)
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2017 Camilla Löwy <elmindreda@glfw.org>
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


//////////////////////////////////////////////////////////////////////////
//////                       Lumina platform API                      //////
//////////////////////////////////////////////////////////////////////////

LUMbool _lumPlatformCreateTls(_LUMtls* tls)
{
    assert(tls->win32.allocated == LUM_FALSE);

    tls->win32.index = TlsAlloc();
    if (tls->win32.index == TLS_OUT_OF_INDEXES)
    {
        _lumInputError(LUM_PLATFORM_ERROR, "Win32: Failed to allocate TLS index");
        return LUM_FALSE;
    }

    tls->win32.allocated = LUM_TRUE;
    return LUM_TRUE;
}

void _lumPlatformDestroyTls(_LUMtls* tls)
{
    if (tls->win32.allocated)
        TlsFree(tls->win32.index);
    memset(tls, 0, sizeof(_LUMtls));
}

void* _lumPlatformGetTls(_LUMtls* tls)
{
    assert(tls->win32.allocated == LUM_TRUE);
    return TlsGetValue(tls->win32.index);
}

void _lumPlatformSetTls(_LUMtls* tls, void* value)
{
    assert(tls->win32.allocated == LUM_TRUE);
    TlsSetValue(tls->win32.index, value);
}

LUMbool _lumPlatformCreateMutex(_LUMmutex* mutex)
{
    assert(mutex->win32.allocated == LUM_FALSE);
    InitializeCriticalSection(&mutex->win32.section);
    return mutex->win32.allocated = LUM_TRUE;
}

void _lumPlatformDestroyMutex(_LUMmutex* mutex)
{
    if (mutex->win32.allocated)
        DeleteCriticalSection(&mutex->win32.section);
    memset(mutex, 0, sizeof(_LUMmutex));
}

void _lumPlatformLockMutex(_LUMmutex* mutex)
{
    assert(mutex->win32.allocated == LUM_TRUE);
    EnterCriticalSection(&mutex->win32.section);
}

void _lumPlatformUnlockMutex(_LUMmutex* mutex)
{
    assert(mutex->win32.allocated == LUM_TRUE);
    LeaveCriticalSection(&mutex->win32.section);
}

