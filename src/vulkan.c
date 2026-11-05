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

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define _LUM_FIND_LOADER    1
#define _LUM_REQUIRE_LOADER 2


//////////////////////////////////////////////////////////////////////////
//////                       Lumina internal API                      //////
//////////////////////////////////////////////////////////////////////////

LUMbool _lumInitVulkan(int mode)
{
    VkResult err;
    VkExtensionProperties* ep;
    PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
    uint32_t i, count;

    if (_lum.vk.available)
        return LUM_TRUE;

    if (_lum.hints.init.vulkanLoader)
        _lum.vk.GetInstanceProcAddr = _lum.hints.init.vulkanLoader;
    else
    {
#if defined(_LUM_VULKAN_LIBRARY)
        _lum.vk.handle = _lumPlatformLoadModule(_LUM_VULKAN_LIBRARY);
#elif defined(_LUM_WIN32)
        _lum.vk.handle = _lumPlatformLoadModule("vulkan-1.dll");
#elif defined(_LUM_COCOA)
        _lum.vk.handle = _lumPlatformLoadModule("libvulkan.1.dylib");
        if (!_lum.vk.handle)
            _lum.vk.handle = _lumLoadLocalVulkanLoaderCocoa();
#elif defined(__OpenBSD__)
        _lum.vk.handle = _lumPlatformLoadModule("libvulkan.so");
#else
        _lum.vk.handle = _lumPlatformLoadModule("libvulkan.so.1");
#endif
        if (!_lum.vk.handle)
        {
            if (mode == _LUM_REQUIRE_LOADER)
                _lumInputError(LUM_API_UNAVAILABLE, "Vulkan: Loader not found");

            return LUM_FALSE;
        }

        _lum.vk.GetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)
            _lumPlatformGetModuleSymbol(_lum.vk.handle, "vkGetInstanceProcAddr");
        if (!_lum.vk.GetInstanceProcAddr)
        {
            _lumInputError(LUM_API_UNAVAILABLE,
                            "Vulkan: Loader does not export vkGetInstanceProcAddr");

            _lumTerminateVulkan();
            return LUM_FALSE;
        }
    }

    vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)
        vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceExtensionProperties");
    if (!vkEnumerateInstanceExtensionProperties)
    {
        _lumInputError(LUM_API_UNAVAILABLE,
                        "Vulkan: Failed to retrieve vkEnumerateInstanceExtensionProperties");

        _lumTerminateVulkan();
        return LUM_FALSE;
    }

    err = vkEnumerateInstanceExtensionProperties(NULL, &count, NULL);
    if (err)
    {
        // NOTE: This happens on systems with a loader but without any Vulkan ICD
        if (mode == _LUM_REQUIRE_LOADER)
        {
            _lumInputError(LUM_API_UNAVAILABLE,
                            "Vulkan: Failed to query instance extension count: %s",
                            _lumGetVulkanResultString(err));
        }

        _lumTerminateVulkan();
        return LUM_FALSE;
    }

    ep = _lum_calloc(count, sizeof(VkExtensionProperties));

    err = vkEnumerateInstanceExtensionProperties(NULL, &count, ep);
    if (err)
    {
        _lumInputError(LUM_API_UNAVAILABLE,
                        "Vulkan: Failed to query instance extensions: %s",
                        _lumGetVulkanResultString(err));

        _lum_free(ep);
        _lumTerminateVulkan();
        return LUM_FALSE;
    }

    for (i = 0;  i < count;  i++)
    {
        if (strcmp(ep[i].extensionName, "VK_KHR_surface") == 0)
            _lum.vk.KHR_surface = LUM_TRUE;
        else if (strcmp(ep[i].extensionName, "VK_KHR_win32_surface") == 0)
            _lum.vk.KHR_win32_surface = LUM_TRUE;
        else if (strcmp(ep[i].extensionName, "VK_MVK_macos_surface") == 0)
            _lum.vk.MVK_macos_surface = LUM_TRUE;
        else if (strcmp(ep[i].extensionName, "VK_EXT_metal_surface") == 0)
            _lum.vk.EXT_metal_surface = LUM_TRUE;
        else if (strcmp(ep[i].extensionName, "VK_KHR_xlib_surface") == 0)
            _lum.vk.KHR_xlib_surface = LUM_TRUE;
        else if (strcmp(ep[i].extensionName, "VK_KHR_xcb_surface") == 0)
            _lum.vk.KHR_xcb_surface = LUM_TRUE;
        else if (strcmp(ep[i].extensionName, "VK_KHR_wayland_surface") == 0)
            _lum.vk.KHR_wayland_surface = LUM_TRUE;
    }

    _lum_free(ep);

    _lum.vk.available = LUM_TRUE;

    _lum.platform.getRequiredInstanceExtensions(_lum.vk.extensions);

    return LUM_TRUE;
}

void _lumTerminateVulkan(void)
{
    if (_lum.vk.handle)
        _lumPlatformFreeModule(_lum.vk.handle);
}

const char* _lumGetVulkanResultString(VkResult result)
{
    switch (result)
    {
        case VK_SUCCESS:
            return "Success";
        case VK_NOT_READY:
            return "A fence or query has not yet completed";
        case VK_TIMEOUT:
            return "A wait operation has not completed in the specified time";
        case VK_EVENT_SET:
            return "An event is signaled";
        case VK_EVENT_RESET:
            return "An event is unsignaled";
        case VK_INCOMPLETE:
            return "A return array was too small for the result";
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "A host memory allocation has failed";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "A device memory allocation has failed";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "Initialization of an object could not be completed for implementation-specific reasons";
        case VK_ERROR_DEVICE_LOST:
            return "The logical or physical device has been lost";
        case VK_ERROR_MEMORY_MAP_FAILED:
            return "Mapping of a memory object has failed";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "A requested layer is not present or could not be loaded";
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "A requested extension is not supported";
        case VK_ERROR_FEATURE_NOT_PRESENT:
            return "A requested feature is not supported";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible";
        case VK_ERROR_TOO_MANY_OBJECTS:
            return "Too many objects of the type have already been created";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            return "A requested format is not supported on this device";
        case VK_ERROR_SURFACE_LOST_KHR:
            return "A surface is no longer available";
        case VK_SUBOPTIMAL_KHR:
            return "A swapchain no longer matches the surface properties exactly, but can still be used";
        case VK_ERROR_OUT_OF_DATE_KHR:
            return "A surface has changed in such a way that it is no longer compatible with the swapchain";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            return "The display used by a swapchain does not use the same presentable image layout";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            return "The requested window is already connected to a VkSurfaceKHR, or to some other non-Vulkan API";
        case VK_ERROR_VALIDATION_FAILED_EXT:
            return "A validation layer found an error";
        default:
            return "ERROR: UNKNOWN VULKAN ERROR";
    }
}


//////////////////////////////////////////////////////////////////////////
//////                        Lumina public API                       //////
//////////////////////////////////////////////////////////////////////////

LUMAPI int lumVulkanSupported(void)
{
    _LUM_REQUIRE_INIT_OR_RETURN(LUM_FALSE);
    return _lumInitVulkan(_LUM_FIND_LOADER);
}

LUMAPI const char** lumGetRequiredInstanceExtensions(uint32_t* count)
{
    assert(count != NULL);

    *count = 0;

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    if (!_lumInitVulkan(_LUM_REQUIRE_LOADER))
        return NULL;

    if (!_lum.vk.extensions[0])
        return NULL;

    *count = 2;
    return (const char**) _lum.vk.extensions;
}

LUMAPI LUMvkproc lumGetInstanceProcAddress(VkInstance instance,
                                              const char* procname)
{
    LUMvkproc proc;
    assert(procname != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    if (!_lumInitVulkan(_LUM_REQUIRE_LOADER))
        return NULL;

    // NOTE: Vulkan 1.0 and 1.1 vkGetInstanceProcAddr cannot return itself
    if (strcmp(procname, "vkGetInstanceProcAddr") == 0)
        return (LUMvkproc) vkGetInstanceProcAddr;

    proc = (LUMvkproc) vkGetInstanceProcAddr(instance, procname);
    if (!proc)
    {
        if (_lum.vk.handle)
            proc = (LUMvkproc) _lumPlatformGetModuleSymbol(_lum.vk.handle, procname);
    }

    return proc;
}

LUMAPI int lumGetPhysicalDevicePresentationSupport(VkInstance instance,
                                                     VkPhysicalDevice device,
                                                     uint32_t queuefamily)
{
    assert(instance != VK_NULL_HANDLE);
    assert(device != VK_NULL_HANDLE);

    _LUM_REQUIRE_INIT_OR_RETURN(LUM_FALSE);

    if (!_lumInitVulkan(_LUM_REQUIRE_LOADER))
        return LUM_FALSE;

    if (!_lum.vk.extensions[0])
    {
        _lumInputError(LUM_API_UNAVAILABLE,
                        "Vulkan: Window surface creation extensions not found");
        return LUM_FALSE;
    }

    return _lum.platform.getPhysicalDevicePresentationSupport(instance,
                                                               device,
                                                               queuefamily);
}

LUMAPI VkResult lumCreateWindowSurface(VkInstance instance,
                                         LUMwindow* handle,
                                         const VkAllocationCallbacks* allocator,
                                         VkSurfaceKHR* surface)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(instance != VK_NULL_HANDLE);
    assert(window != NULL);
    assert(surface != NULL);

    *surface = VK_NULL_HANDLE;

    _LUM_REQUIRE_INIT_OR_RETURN(VK_ERROR_INITIALIZATION_FAILED);

    if (!_lumInitVulkan(_LUM_REQUIRE_LOADER))
        return VK_ERROR_INITIALIZATION_FAILED;

    if (!_lum.vk.extensions[0])
    {
        _lumInputError(LUM_API_UNAVAILABLE,
                        "Vulkan: Window surface creation extensions not found");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    if (window->context.client != LUM_NO_API)
    {
        _lumInputError(LUM_INVALID_VALUE,
                        "Vulkan: Window surface creation requires the window to have the client API set to LUM_NO_API");
        return VK_ERROR_NATIVE_WINDOW_IN_USE_KHR;
    }

    return _lum.platform.createWindowSurface(instance, window, allocator, surface);
}

