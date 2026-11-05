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

#define _GNU_SOURCE

#include "internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/timerfd.h>
#include <poll.h>

#include "wayland-client-protocol.h"
#include "wayland-xdg-shell-client-protocol.h"
#include "wayland-xdg-decoration-client-protocol.h"
#include "wayland-viewporter-client-protocol.h"
#include "wayland-relative-pointer-unstable-v1-client-protocol.h"
#include "wayland-pointer-constraints-unstable-v1-client-protocol.h"
#include "wayland-idle-inhibit-unstable-v1-client-protocol.h"


static int createTmpfileCloexec(char* tmpname)
{
    int fd;

    fd = mkostemp(tmpname, O_CLOEXEC);
    if (fd >= 0)
        unlink(tmpname);

    return fd;
}

/*
 * Create a new, unique, anonymous file of the given size, and
 * return the file descriptor for it. The file descriptor is set
 * CLOEXEC. The file is immediately suitable for mmap()'ing
 * the given size at offset zero.
 *
 * The file should not have a permanent backing store like a disk,
 * but may have if XDG_RUNTIME_DIR is not properly implemented in OS.
 *
 * The file name is deleted from the file system.
 *
 * The file is suitable for buffer sharing between processes by
 * transmitting the file descriptor over Unix sockets using the
 * SCM_RIGHTS methods.
 *
 * posix_fallocate() is used to guarantee that disk space is available
 * for the file at the given size. If disk space is insufficient, errno
 * is set to ENOSPC. If posix_fallocate() is not supported, program may
 * receive SIGBUS on accessing mmap()'ed file contents instead.
 */
static int createAnonymousFile(off_t size)
{
    static const char template[] = "/lumina-shared-XXXXXX";
    const char* path;
    char* name;
    int fd;
    int ret;

#ifdef HAVE_MEMFD_CREATE
    fd = memfd_create("lumina-shared", MFD_CLOEXEC | MFD_ALLOW_SEALING);
    if (fd >= 0)
    {
        // We can add this seal before calling posix_fallocate(), as the file
        // is currently zero-sized anyway.
        //
        // There is also no need to check for the return value, we couldn’t do
        // anything with it anyway.
        fcntl(fd, F_ADD_SEALS, F_SEAL_SHRINK | F_SEAL_SEAL);
    }
    else
#elif defined(SHM_ANON)
    fd = shm_open(SHM_ANON, O_RDWR | O_CLOEXEC, 0600);
    if (fd < 0)
#endif
    {
        path = getenv("XDG_RUNTIME_DIR");
        if (!path)
        {
            errno = ENOENT;
            return -1;
        }

        name = _lum_calloc(strlen(path) + sizeof(template), 1);
        strcpy(name, path);
        strcat(name, template);

        fd = createTmpfileCloexec(name);
        _lum_free(name);
        if (fd < 0)
            return -1;
    }

#if defined(SHM_ANON)
    // posix_fallocate does not work on SHM descriptors
    ret = ftruncate(fd, size);
#else
    ret = posix_fallocate(fd, 0, size);
#endif
    if (ret != 0)
    {
        close(fd);
        errno = ret;
        return -1;
    }
    return fd;
}

static struct wl_buffer* createShmBuffer(const LUMimage* image)
{
    struct wl_shm_pool* pool;
    struct wl_buffer* buffer;
    int stride = image->width * 4;
    int length = image->width * image->height * 4;
    void* data;

    const int fd = createAnonymousFile(length);
    if (fd < 0)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Creating a buffer file for %d B failed: %s",
                        length, strerror(errno));
        return NULL;
    }

    data = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: mmap failed: %s", strerror(errno));
        close(fd);
        return NULL;
    }

    pool = wl_shm_create_pool(_lum.wl.shm, fd, length);

    close(fd);
    unsigned char* source = (unsigned char*) image->pixels;
    unsigned char* target = data;
    for (int i = 0;  i < image->width * image->height;  i++, source += 4)
    {
        unsigned int alpha = source[3];

        *target++ = (unsigned char) ((source[2] * alpha) / 255);
        *target++ = (unsigned char) ((source[1] * alpha) / 255);
        *target++ = (unsigned char) ((source[0] * alpha) / 255);
        *target++ = (unsigned char) alpha;
    }

    buffer =
        wl_shm_pool_create_buffer(pool, 0,
                                  image->width,
                                  image->height,
                                  stride, WL_SHM_FORMAT_ARGB8888);
    munmap(data, length);
    wl_shm_pool_destroy(pool);

    return buffer;
}

static void createDecoration(_LUMdecorationWayland* decoration,
                             struct wl_surface* parent,
                             struct wl_buffer* buffer, LUMbool opaque,
                             int x, int y,
                             int width, int height)
{
    struct wl_region* region;

    decoration->surface = wl_compositor_create_surface(_lum.wl.compositor);
    decoration->subsurface =
        wl_subcompositor_get_subsurface(_lum.wl.subcompositor,
                                        decoration->surface, parent);
    wl_subsurface_set_position(decoration->subsurface, x, y);
    decoration->viewport = wp_viewporter_get_viewport(_lum.wl.viewporter,
                                                      decoration->surface);
    wp_viewport_set_destination(decoration->viewport, width, height);
    wl_surface_attach(decoration->surface, buffer, 0, 0);

    if (opaque)
    {
        region = wl_compositor_create_region(_lum.wl.compositor);
        wl_region_add(region, 0, 0, width, height);
        wl_surface_set_opaque_region(decoration->surface, region);
        wl_surface_commit(decoration->surface);
        wl_region_destroy(region);
    }
    else
        wl_surface_commit(decoration->surface);
}

static void createDecorations(_LUMwindow* window)
{
    unsigned char data[] = { 224, 224, 224, 255 };
    const LUMimage image = { 1, 1, data };
    LUMbool opaque = (data[3] == 255);

    if (!_lum.wl.viewporter || !window->decorated || window->wl.decorations.serverSide)
        return;

    if (!window->wl.decorations.buffer)
        window->wl.decorations.buffer = createShmBuffer(&image);
    if (!window->wl.decorations.buffer)
        return;

    createDecoration(&window->wl.decorations.top, window->wl.surface,
                     window->wl.decorations.buffer, opaque,
                     0, -_LUM_DECORATION_TOP,
                     window->wl.width, _LUM_DECORATION_TOP);
    createDecoration(&window->wl.decorations.left, window->wl.surface,
                     window->wl.decorations.buffer, opaque,
                     -_LUM_DECORATION_WIDTH, -_LUM_DECORATION_TOP,
                     _LUM_DECORATION_WIDTH, window->wl.height + _LUM_DECORATION_TOP);
    createDecoration(&window->wl.decorations.right, window->wl.surface,
                     window->wl.decorations.buffer, opaque,
                     window->wl.width, -_LUM_DECORATION_TOP,
                     _LUM_DECORATION_WIDTH, window->wl.height + _LUM_DECORATION_TOP);
    createDecoration(&window->wl.decorations.bottom, window->wl.surface,
                     window->wl.decorations.buffer, opaque,
                     -_LUM_DECORATION_WIDTH, window->wl.height,
                     window->wl.width + _LUM_DECORATION_HORIZONTAL, _LUM_DECORATION_WIDTH);
}

static void destroyDecoration(_LUMdecorationWayland* decoration)
{
    if (decoration->subsurface)
        wl_subsurface_destroy(decoration->subsurface);
    if (decoration->surface)
        wl_surface_destroy(decoration->surface);
    if (decoration->viewport)
        wp_viewport_destroy(decoration->viewport);
    decoration->surface = NULL;
    decoration->subsurface = NULL;
    decoration->viewport = NULL;
}

static void destroyDecorations(_LUMwindow* window)
{
    destroyDecoration(&window->wl.decorations.top);
    destroyDecoration(&window->wl.decorations.left);
    destroyDecoration(&window->wl.decorations.right);
    destroyDecoration(&window->wl.decorations.bottom);
}

static void xdgDecorationHandleConfigure(void* data,
                                         struct zxdg_toplevel_decoration_v1* decoration,
                                         uint32_t mode)
{
    _LUMwindow* window = data;

    window->wl.decorations.serverSide = (mode == ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);

    if (!window->wl.decorations.serverSide)
        createDecorations(window);
}

static const struct zxdg_toplevel_decoration_v1_listener xdgDecorationListener = {
    xdgDecorationHandleConfigure,
};

// Makes the surface considered as XRGB instead of ARGB.
static void setOpaqueRegion(_LUMwindow* window)
{
    struct wl_region* region;

    region = wl_compositor_create_region(_lum.wl.compositor);
    if (!region)
        return;

    wl_region_add(region, 0, 0, window->wl.width, window->wl.height);
    wl_surface_set_opaque_region(window->wl.surface, region);
    wl_surface_commit(window->wl.surface);
    wl_region_destroy(region);
}


static void resizeWindow(_LUMwindow* window)
{
    int scale = window->wl.scale;
    int scaledWidth = window->wl.width * scale;
    int scaledHeight = window->wl.height * scale;
    wl_egl_window_resize(window->wl.native, scaledWidth, scaledHeight, 0, 0);
    if (!window->wl.transparent)
        setOpaqueRegion(window);
    _lumInputFramebufferSize(window, scaledWidth, scaledHeight);
    _lumInputWindowContentScale(window, scale, scale);

    if (!window->wl.decorations.top.surface)
        return;

    // Top decoration.
    wp_viewport_set_destination(window->wl.decorations.top.viewport,
                                window->wl.width, _LUM_DECORATION_TOP);
    wl_surface_commit(window->wl.decorations.top.surface);

    // Left decoration.
    wp_viewport_set_destination(window->wl.decorations.left.viewport,
                                _LUM_DECORATION_WIDTH, window->wl.height + _LUM_DECORATION_TOP);
    wl_surface_commit(window->wl.decorations.left.surface);

    // Right decoration.
    wl_subsurface_set_position(window->wl.decorations.right.subsurface,
                               window->wl.width, -_LUM_DECORATION_TOP);
    wp_viewport_set_destination(window->wl.decorations.right.viewport,
                                _LUM_DECORATION_WIDTH, window->wl.height + _LUM_DECORATION_TOP);
    wl_surface_commit(window->wl.decorations.right.surface);

    // Bottom decoration.
    wl_subsurface_set_position(window->wl.decorations.bottom.subsurface,
                               -_LUM_DECORATION_WIDTH, window->wl.height);
    wp_viewport_set_destination(window->wl.decorations.bottom.viewport,
                                window->wl.width + _LUM_DECORATION_HORIZONTAL, _LUM_DECORATION_WIDTH);
    wl_surface_commit(window->wl.decorations.bottom.surface);
}

static void checkScaleChange(_LUMwindow* window)
{
    int scale = 1;
    int monitorScale;

    // Check if we will be able to set the buffer scale or not.
    if (_lum.wl.compositorVersion < 3)
        return;

    // Get the scale factor from the highest scale monitor.
    for (int i = 0; i < window->wl.monitorsCount; ++i)
    {
        monitorScale = window->wl.monitors[i]->wl.scale;
        if (scale < monitorScale)
            scale = monitorScale;
    }

    // Only change the framebuffer size if the scale changed.
    if (scale != window->wl.scale)
    {
        window->wl.scale = scale;
        wl_surface_set_buffer_scale(window->wl.surface, scale);
        resizeWindow(window);
    }
}

static void surfaceHandleEnter(void *data,
                               struct wl_surface *surface,
                               struct wl_output *output)
{
    _LUMwindow* window = data;
    _LUMmonitor* monitor = wl_output_get_user_data(output);

    if (window->wl.monitorsCount + 1 > window->wl.monitorsSize)
    {
        ++window->wl.monitorsSize;
        window->wl.monitors =
            _lum_realloc(window->wl.monitors,
                          window->wl.monitorsSize * sizeof(_LUMmonitor*));
    }

    window->wl.monitors[window->wl.monitorsCount++] = monitor;

    checkScaleChange(window);
}

static void surfaceHandleLeave(void *data,
                               struct wl_surface *surface,
                               struct wl_output *output)
{
    _LUMwindow* window = data;
    _LUMmonitor* monitor = wl_output_get_user_data(output);
    LUMbool found = LUM_FALSE;

    for (int i = 0; i < window->wl.monitorsCount - 1; ++i)
    {
        if (monitor == window->wl.monitors[i])
            found = LUM_TRUE;
        if (found)
            window->wl.monitors[i] = window->wl.monitors[i + 1];
    }
    window->wl.monitors[--window->wl.monitorsCount] = NULL;

    checkScaleChange(window);
}

static const struct wl_surface_listener surfaceListener = {
    surfaceHandleEnter,
    surfaceHandleLeave
};

static void setIdleInhibitor(_LUMwindow* window, LUMbool enable)
{
    if (enable && !window->wl.idleInhibitor && _lum.wl.idleInhibitManager)
    {
        window->wl.idleInhibitor =
            zwp_idle_inhibit_manager_v1_create_inhibitor(
                _lum.wl.idleInhibitManager, window->wl.surface);
        if (!window->wl.idleInhibitor)
            _lumInputError(LUM_PLATFORM_ERROR,
                            "Wayland: Idle inhibitor creation failed");
    }
    else if (!enable && window->wl.idleInhibitor)
    {
        zwp_idle_inhibitor_v1_destroy(window->wl.idleInhibitor);
        window->wl.idleInhibitor = NULL;
    }
}

static void setFullscreen(_LUMwindow* window, _LUMmonitor* monitor,
                          int refreshRate)
{
    if (window->wl.xdg.toplevel)
    {
        xdg_toplevel_set_fullscreen(
            window->wl.xdg.toplevel,
            monitor->wl.output);
    }
    setIdleInhibitor(window, LUM_TRUE);
    if (!window->wl.decorations.serverSide)
        destroyDecorations(window);
}

static void xdgToplevelHandleConfigure(void* data,
                                       struct xdg_toplevel* toplevel,
                                       int32_t width,
                                       int32_t height,
                                       struct wl_array* states)
{
    _LUMwindow* window = data;
    float aspectRatio;
    float targetRatio;
    uint32_t* state;
    LUMbool maximized = LUM_FALSE;
    LUMbool fullscreen = LUM_FALSE;
    LUMbool activated = LUM_FALSE;

    wl_array_for_each(state, states)
    {
        switch (*state)
        {
            case XDG_TOPLEVEL_STATE_MAXIMIZED:
                maximized = LUM_TRUE;
                break;
            case XDG_TOPLEVEL_STATE_FULLSCREEN:
                fullscreen = LUM_TRUE;
                break;
            case XDG_TOPLEVEL_STATE_RESIZING:
                break;
            case XDG_TOPLEVEL_STATE_ACTIVATED:
                activated = LUM_TRUE;
                break;
        }
    }

    if (width != 0 && height != 0)
    {
        if (!maximized && !fullscreen)
        {
            if (window->numer != LUM_DONT_CARE && window->denom != LUM_DONT_CARE)
            {
                aspectRatio = (float)width / (float)height;
                targetRatio = (float)window->numer / (float)window->denom;
                if (aspectRatio < targetRatio)
                    height = width / targetRatio;
                else if (aspectRatio > targetRatio)
                    width = height * targetRatio;
            }
        }

        _lumInputWindowSize(window, width, height);
        _lumSetWindowSizeWayland(window, width, height);
        _lumInputWindowDamage(window);
    }

    if (window->wl.wasFullscreen && window->autoIconify)
    {
        if (!activated || !fullscreen)
        {
            _lumIconifyWindowWayland(window);
            window->wl.wasFullscreen = LUM_FALSE;
        }
    }
    if (fullscreen && activated)
        window->wl.wasFullscreen = LUM_TRUE;
}

static void xdgToplevelHandleClose(void* data,
                                   struct xdg_toplevel* toplevel)
{
    _LUMwindow* window = data;
    _lumInputWindowCloseRequest(window);
}

static const struct xdg_toplevel_listener xdgToplevelListener = {
    xdgToplevelHandleConfigure,
    xdgToplevelHandleClose
};

static void xdgSurfaceHandleConfigure(void* data,
                                      struct xdg_surface* surface,
                                      uint32_t serial)
{
    xdg_surface_ack_configure(surface, serial);
}

static const struct xdg_surface_listener xdgSurfaceListener = {
    xdgSurfaceHandleConfigure
};

static void setXdgDecorations(_LUMwindow* window)
{
    if (_lum.wl.decorationManager)
    {
        window->wl.xdg.decoration =
            zxdg_decoration_manager_v1_get_toplevel_decoration(
                _lum.wl.decorationManager, window->wl.xdg.toplevel);
        zxdg_toplevel_decoration_v1_add_listener(window->wl.xdg.decoration,
                                                 &xdgDecorationListener,
                                                 window);
        zxdg_toplevel_decoration_v1_set_mode(
            window->wl.xdg.decoration,
            ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }
    else
    {
        window->wl.decorations.serverSide = LUM_FALSE;
        createDecorations(window);
    }
}

static LUMbool createXdgSurface(_LUMwindow* window)
{
    window->wl.xdg.surface = xdg_wm_base_get_xdg_surface(_lum.wl.wmBase,
                                                         window->wl.surface);
    if (!window->wl.xdg.surface)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: xdg-surface creation failed");
        return LUM_FALSE;
    }

    xdg_surface_add_listener(window->wl.xdg.surface,
                             &xdgSurfaceListener,
                             window);

    window->wl.xdg.toplevel = xdg_surface_get_toplevel(window->wl.xdg.surface);
    if (!window->wl.xdg.toplevel)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: xdg-toplevel creation failed");
        return LUM_FALSE;
    }

    xdg_toplevel_add_listener(window->wl.xdg.toplevel,
                              &xdgToplevelListener,
                              window);

    if (window->wl.title)
        xdg_toplevel_set_title(window->wl.xdg.toplevel, window->wl.title);

    if (window->minwidth != LUM_DONT_CARE && window->minheight != LUM_DONT_CARE)
        xdg_toplevel_set_min_size(window->wl.xdg.toplevel,
                                  window->minwidth, window->minheight);
    if (window->maxwidth != LUM_DONT_CARE && window->maxheight != LUM_DONT_CARE)
        xdg_toplevel_set_max_size(window->wl.xdg.toplevel,
                                  window->maxwidth, window->maxheight);

    if (window->monitor)
    {
        xdg_toplevel_set_fullscreen(window->wl.xdg.toplevel,
                                    window->monitor->wl.output);
        setIdleInhibitor(window, LUM_TRUE);
    }
    else if (window->wl.maximized)
    {
        xdg_toplevel_set_maximized(window->wl.xdg.toplevel);
        setIdleInhibitor(window, LUM_FALSE);
        setXdgDecorations(window);
    }
    else
    {
        setIdleInhibitor(window, LUM_FALSE);
        setXdgDecorations(window);
    }

    wl_surface_commit(window->wl.surface);
    wl_display_roundtrip(_lum.wl.display);

    return LUM_TRUE;
}

static LUMbool createSurface(_LUMwindow* window,
                              const _LUMwndconfig* wndconfig,
                              const _LUMfbconfig* fbconfig)
{
    window->wl.surface = wl_compositor_create_surface(_lum.wl.compositor);
    if (!window->wl.surface)
        return LUM_FALSE;

    wl_surface_add_listener(window->wl.surface,
                            &surfaceListener,
                            window);

    wl_surface_set_user_data(window->wl.surface, window);

    window->wl.native = wl_egl_window_create(window->wl.surface,
                                             wndconfig->width,
                                             wndconfig->height);
    if (!window->wl.native)
        return LUM_FALSE;

    window->wl.width = wndconfig->width;
    window->wl.height = wndconfig->height;
    window->wl.scale = 1;
    window->wl.title = _lum_strdup(wndconfig->title);

    window->wl.transparent = fbconfig->transparent;
    if (!window->wl.transparent)
        setOpaqueRegion(window);

    if (window->monitor || wndconfig->visible)
    {
        if (!createXdgSurface(window))
            return LUM_FALSE;

        window->wl.visible = LUM_TRUE;
    }

    return LUM_TRUE;
}

static void setCursorImage(_LUMwindow* window,
                           _LUMcursorWayland* cursorWayland)
{
    struct itimerspec timer = {};
    struct wl_cursor* wlCursor = cursorWayland->cursor;
    struct wl_cursor_image* image;
    struct wl_buffer* buffer;
    struct wl_surface* surface = _lum.wl.cursorSurface;
    int scale = 1;

    if (!wlCursor)
        buffer = cursorWayland->buffer;
    else
    {
        if (window->wl.scale > 1 && cursorWayland->cursorHiDPI)
        {
            wlCursor = cursorWayland->cursorHiDPI;
            scale = 2;
        }

        image = wlCursor->images[cursorWayland->currentImage];
        buffer = wl_cursor_image_get_buffer(image);
        if (!buffer)
            return;

        timer.it_value.tv_sec = image->delay / 1000;
        timer.it_value.tv_nsec = (image->delay % 1000) * 1000000;
        timerfd_settime(_lum.wl.cursorTimerfd, 0, &timer, NULL);

        cursorWayland->width = image->width;
        cursorWayland->height = image->height;
        cursorWayland->xhot = image->hotspot_x;
        cursorWayland->yhot = image->hotspot_y;
    }

    wl_pointer_set_cursor(_lum.wl.pointer, _lum.wl.pointerEnterSerial,
                          surface,
                          cursorWayland->xhot / scale,
                          cursorWayland->yhot / scale);
    wl_surface_set_buffer_scale(surface, scale);
    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_damage(surface, 0, 0,
                      cursorWayland->width, cursorWayland->height);
    wl_surface_commit(surface);
}

static void incrementCursorImage(_LUMwindow* window)
{
    _LUMcursor* cursor;

    if (!window || window->wl.decorations.focus != mainWindow)
        return;

    cursor = window->wl.currentCursor;
    if (cursor && cursor->wl.cursor)
    {
        cursor->wl.currentImage += 1;
        cursor->wl.currentImage %= cursor->wl.cursor->image_count;
        setCursorImage(window, &cursor->wl);
    }
}

static void handleEvents(int timeout)
{
    struct pollfd fds[] =
    {
        { wl_display_get_fd(_lum.wl.display), POLLIN },
        { _lum.wl.timerfd, POLLIN },
        { _lum.wl.cursorTimerfd, POLLIN },
    };

    while (wl_display_prepare_read(_lum.wl.display) != 0)
        wl_display_dispatch_pending(_lum.wl.display);

    // If an error other than EAGAIN happens, we have likely been disconnected
    // from the Wayland session; try to handle that the best we can.
    if (wl_display_flush(_lum.wl.display) < 0 && errno != EAGAIN)
    {
        _LUMwindow* window = _lum.windowListHead;
        while (window)
        {
            _lumInputWindowCloseRequest(window);
            window = window->next;
        }

        wl_display_cancel_read(_lum.wl.display);
        return;
    }

    if (poll(fds, 3, timeout) > 0)
    {
        if (fds[0].revents & POLLIN)
        {
            wl_display_read_events(_lum.wl.display);
            wl_display_dispatch_pending(_lum.wl.display);
        }
        else
            wl_display_cancel_read(_lum.wl.display);

        if (fds[1].revents & POLLIN)
        {
            uint64_t repeats;

            if (read(_lum.wl.timerfd, &repeats, sizeof(repeats)) == 8)
            {
                for (uint64_t i = 0; i < repeats; i++)
                {
                    _lumInputKey(_lum.wl.keyboardFocus,
                                  _lum.wl.keyboardLastKey,
                                  _lum.wl.keyboardLastScancode,
                                  LUM_PRESS,
                                  _lum.wl.xkb.modifiers);
                    _lumInputTextWayland(_lum.wl.keyboardFocus,
                                          _lum.wl.keyboardLastScancode);
                }
            }
        }

        if (fds[2].revents & POLLIN)
        {
            uint64_t repeats;

            if (read(_lum.wl.cursorTimerfd, &repeats, sizeof(repeats)) == 8)
                incrementCursorImage(_lum.wl.pointerFocus);
        }
    }
    else
        wl_display_cancel_read(_lum.wl.display);
}

//////////////////////////////////////////////////////////////////////////
//////                       Lumina platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _lumCreateWindowWayland(_LUMwindow* window,
                             const _LUMwndconfig* wndconfig,
                             const _LUMctxconfig* ctxconfig,
                             const _LUMfbconfig* fbconfig)
{
    if (!createSurface(window, wndconfig, fbconfig))
        return LUM_FALSE;

    if (ctxconfig->client != LUM_NO_API)
    {
        if (ctxconfig->source == LUM_EGL_CONTEXT_API ||
            ctxconfig->source == LUM_NATIVE_CONTEXT_API)
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

    return LUM_TRUE;
}

void _lumDestroyWindowWayland(_LUMwindow* window)
{
    if (window == _lum.wl.pointerFocus)
    {
        _lum.wl.pointerFocus = NULL;
        _lumInputCursorEnter(window, LUM_FALSE);
    }
    if (window == _lum.wl.keyboardFocus)
    {
        _lum.wl.keyboardFocus = NULL;
        _lumInputWindowFocus(window, LUM_FALSE);
    }

    if (window->wl.idleInhibitor)
        zwp_idle_inhibitor_v1_destroy(window->wl.idleInhibitor);

    if (window->context.destroy)
        window->context.destroy(window);

    destroyDecorations(window);
    if (window->wl.xdg.decoration)
        zxdg_toplevel_decoration_v1_destroy(window->wl.xdg.decoration);

    if (window->wl.decorations.buffer)
        wl_buffer_destroy(window->wl.decorations.buffer);

    if (window->wl.native)
        wl_egl_window_destroy(window->wl.native);

    if (window->wl.xdg.toplevel)
        xdg_toplevel_destroy(window->wl.xdg.toplevel);

    if (window->wl.xdg.surface)
        xdg_surface_destroy(window->wl.xdg.surface);

    if (window->wl.surface)
        wl_surface_destroy(window->wl.surface);

    _lum_free(window->wl.title);
    _lum_free(window->wl.monitors);
}

void _lumSetWindowTitleWayland(_LUMwindow* window, const char* title)
{
    if (window->wl.title)
        _lum_free(window->wl.title);
    window->wl.title = _lum_strdup(title);
    if (window->wl.xdg.toplevel)
        xdg_toplevel_set_title(window->wl.xdg.toplevel, title);
}

void _lumSetWindowIconWayland(_LUMwindow* window,
                               int count, const LUMimage* images)
{
    _lumInputError(LUM_FEATURE_UNAVAILABLE,
                    "Wayland: The platform does not support setting the window icon");
}

void _lumGetWindowPosWayland(_LUMwindow* window, int* xpos, int* ypos)
{
    // A Wayland client is not aware of its position, so just warn and leave it
    // as (0, 0)

    _lumInputError(LUM_FEATURE_UNAVAILABLE,
                    "Wayland: The platform does not provide the window position");
}

void _lumSetWindowPosWayland(_LUMwindow* window, int xpos, int ypos)
{
    // A Wayland client can not set its position, so just warn

    _lumInputError(LUM_FEATURE_UNAVAILABLE,
                    "Wayland: The platform does not support setting the window position");
}

void _lumGetWindowSizeWayland(_LUMwindow* window, int* width, int* height)
{
    if (width)
        *width = window->wl.width;
    if (height)
        *height = window->wl.height;
}

void _lumSetWindowSizeWayland(_LUMwindow* window, int width, int height)
{
    window->wl.width = width;
    window->wl.height = height;
    resizeWindow(window);
}

void _lumSetWindowSizeLimitsWayland(_LUMwindow* window,
                                     int minwidth, int minheight,
                                     int maxwidth, int maxheight)
{
    if (window->wl.xdg.toplevel)
    {
        if (minwidth == LUM_DONT_CARE || minheight == LUM_DONT_CARE)
            minwidth = minheight = 0;
        if (maxwidth == LUM_DONT_CARE || maxheight == LUM_DONT_CARE)
            maxwidth = maxheight = 0;
        xdg_toplevel_set_min_size(window->wl.xdg.toplevel, minwidth, minheight);
        xdg_toplevel_set_max_size(window->wl.xdg.toplevel, maxwidth, maxheight);
        wl_surface_commit(window->wl.surface);
    }
}

void _lumSetWindowAspectRatioWayland(_LUMwindow* window, int numer, int denom)
{
    // TODO: find out how to trigger a resize.
    // The actual limits are checked in the xdg_toplevel::configure handler.
    _lumInputError(LUM_FEATURE_UNIMPLEMENTED,
                    "Wayland: Window aspect ratio not yet implemented");
}

void _lumGetFramebufferSizeWayland(_LUMwindow* window, int* width, int* height)
{
    _lumGetWindowSizeWayland(window, width, height);
    if (width)
        *width *= window->wl.scale;
    if (height)
        *height *= window->wl.scale;
}

void _lumGetWindowFrameSizeWayland(_LUMwindow* window,
                                    int* left, int* top,
                                    int* right, int* bottom)
{
    if (window->decorated && !window->monitor && !window->wl.decorations.serverSide)
    {
        if (top)
            *top = _LUM_DECORATION_TOP;
        if (left)
            *left = _LUM_DECORATION_WIDTH;
        if (right)
            *right = _LUM_DECORATION_WIDTH;
        if (bottom)
            *bottom = _LUM_DECORATION_WIDTH;
    }
}

void _lumGetWindowContentScaleWayland(_LUMwindow* window,
                                       float* xscale, float* yscale)
{
    if (xscale)
        *xscale = (float) window->wl.scale;
    if (yscale)
        *yscale = (float) window->wl.scale;
}

void _lumIconifyWindowWayland(_LUMwindow* window)
{
    if (window->wl.xdg.toplevel)
        xdg_toplevel_set_minimized(window->wl.xdg.toplevel);
}

void _lumRestoreWindowWayland(_LUMwindow* window)
{
    if (window->wl.xdg.toplevel)
    {
        if (window->monitor)
            xdg_toplevel_unset_fullscreen(window->wl.xdg.toplevel);
        if (window->wl.maximized)
            xdg_toplevel_unset_maximized(window->wl.xdg.toplevel);
        // There is no way to unset minimized, or even to know if we are
        // minimized, so there is nothing to do in this case.
    }
    _lumInputWindowMonitor(window, NULL);
    window->wl.maximized = LUM_FALSE;
}

void _lumMaximizeWindowWayland(_LUMwindow* window)
{
    if (window->wl.xdg.toplevel)
    {
        xdg_toplevel_set_maximized(window->wl.xdg.toplevel);
    }
    window->wl.maximized = LUM_TRUE;
}

void _lumShowWindowWayland(_LUMwindow* window)
{
    if (!window->wl.visible)
    {
        // NOTE: The XDG surface and role are created here so command-line applications
        //       with off-screen windows do not appear in for example the Unity dock
        if (!window->wl.xdg.toplevel)
            createXdgSurface(window);

        window->wl.visible = LUM_TRUE;
        _lumInputWindowDamage(window);
    }
}

void _lumHideWindowWayland(_LUMwindow* window)
{
    if (window->wl.visible)
    {
        window->wl.visible = LUM_FALSE;
        wl_surface_attach(window->wl.surface, NULL, 0, 0);
        wl_surface_commit(window->wl.surface);
    }
}

void _lumRequestWindowAttentionWayland(_LUMwindow* window)
{
    // TODO
    _lumInputError(LUM_FEATURE_UNIMPLEMENTED,
                    "Wayland: Window attention request not implemented yet");
}

void _lumFocusWindowWayland(_LUMwindow* window)
{
    _lumInputError(LUM_FEATURE_UNAVAILABLE,
                    "Wayland: The platform does not support setting the input focus");
}

void _lumSetWindowMonitorWayland(_LUMwindow* window,
                                  _LUMmonitor* monitor,
                                  int xpos, int ypos,
                                  int width, int height,
                                  int refreshRate)
{
    if (monitor)
    {
        setFullscreen(window, monitor, refreshRate);
    }
    else
    {
        if (window->wl.xdg.toplevel)
            xdg_toplevel_unset_fullscreen(window->wl.xdg.toplevel);
        setIdleInhibitor(window, LUM_FALSE);
        if (!_lum.wl.decorationManager)
            createDecorations(window);
    }
    _lumInputWindowMonitor(window, monitor);
}

int _lumWindowFocusedWayland(_LUMwindow* window)
{
    return _lum.wl.keyboardFocus == window;
}

int _lumWindowIconifiedWayland(_LUMwindow* window)
{
    // xdg-shell doesn’t give any way to request whether a surface is
    // iconified.
    return LUM_FALSE;
}

int _lumWindowVisibleWayland(_LUMwindow* window)
{
    return window->wl.visible;
}

int _lumWindowMaximizedWayland(_LUMwindow* window)
{
    return window->wl.maximized;
}

int _lumWindowHoveredWayland(_LUMwindow* window)
{
    return window->wl.hovered;
}

int _lumFramebufferTransparentWayland(_LUMwindow* window)
{
    return window->wl.transparent;
}

void _lumSetWindowResizableWayland(_LUMwindow* window, LUMbool enabled)
{
    // TODO
    _lumInputError(LUM_FEATURE_UNIMPLEMENTED,
                    "Wayland: Window attribute setting not implemented yet");
}

void _lumSetWindowDecoratedWayland(_LUMwindow* window, LUMbool enabled)
{
    if (!window->monitor)
    {
        if (enabled)
            createDecorations(window);
        else
            destroyDecorations(window);
    }
}


void _lumPlatformSetWindowTitlebar(_LUMwindow* window, LUMbool enabled)
{
    // TODO
    _lumInputError(LUM_PLATFORM_ERROR,
        "Wayland: Window attribute setting not implemented yet");
}

void _lumSetWindowFloatingWayland(_LUMwindow* window, LUMbool enabled)
{
    // TODO
    _lumInputError(LUM_FEATURE_UNIMPLEMENTED,
                    "Wayland: Window attribute setting not implemented yet");
}

void _lumSetWindowMousePassthroughWayland(_LUMwindow* window, LUMbool enabled)
{
    if (enabled)
    {
        struct wl_region* region = wl_compositor_create_region(_lum.wl.compositor);
        wl_surface_set_input_region(window->wl.surface, region);
        wl_region_destroy(region);
    }
    else
        wl_surface_set_input_region(window->wl.surface, 0);
    wl_surface_commit(window->wl.surface);
}

float _lumGetWindowOpacityWayland(_LUMwindow* window)
{
    return 1.f;
}

void _lumSetWindowOpacityWayland(_LUMwindow* window, float opacity)
{
    _lumInputError(LUM_FEATURE_UNAVAILABLE,
                    "Wayland: The platform does not support setting the window opacity");
}

void _lumSetRawMouseMotionWayland(_LUMwindow *window, LUMbool enabled)
{
    // This is handled in relativePointerHandleRelativeMotion
}

LUMbool _lumRawMouseMotionSupportedWayland(void)
{
    return LUM_TRUE;
}

void _lumPollEventsWayland(void)
{
    handleEvents(0);
}

void _lumWaitEventsWayland(void)
{
    handleEvents(-1);
}

void _lumWaitEventsTimeoutWayland(double timeout)
{
    handleEvents((int) (timeout * 1e3));
}

void _lumPostEmptyEventWayland(void)
{
    wl_display_sync(_lum.wl.display);
}

void _lumGetCursorPosWayland(_LUMwindow* window, double* xpos, double* ypos)
{
    if (xpos)
        *xpos = window->wl.cursorPosX;
    if (ypos)
        *ypos = window->wl.cursorPosY;
}

static LUMbool isPointerLocked(_LUMwindow* window);

void _lumSetCursorPosWayland(_LUMwindow* window, double x, double y)
{
    if (isPointerLocked(window))
    {
        zwp_locked_pointer_v1_set_cursor_position_hint(
            window->wl.pointerLock.lockedPointer,
            wl_fixed_from_double(x), wl_fixed_from_double(y));
        wl_surface_commit(window->wl.surface);
    }
}

void _lumSetCursorModeWayland(_LUMwindow* window, int mode)
{
    _lumSetCursorWayland(window, window->wl.currentCursor);
}

const char* _lumGetScancodeNameWayland(int scancode)
{
    if (scancode < 0 || scancode > 255 ||
        _lum.wl.keycodes[scancode] == LUM_KEY_UNKNOWN)
    {
        _lumInputError(LUM_INVALID_VALUE,
                        "Wayland: Invalid scancode %i",
                        scancode);
        return NULL;
    }

    const int key = _lum.wl.keycodes[scancode];
    const xkb_keycode_t keycode = scancode + 8;
    const xkb_layout_index_t layout =
        xkb_state_key_get_layout(_lum.wl.xkb.state, keycode);
    if (layout == XKB_LAYOUT_INVALID)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Failed to retrieve layout for key name");
        return NULL;
    }

    const xkb_keysym_t* keysyms = NULL;
    xkb_keymap_key_get_syms_by_level(_lum.wl.xkb.keymap,
                                     keycode,
                                     layout,
                                     0,
                                     &keysyms);
    if (keysyms == NULL)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Failed to retrieve keysym for key name");
        return NULL;
    }

    const uint32_t codepoint = _lumKeySym2Unicode(keysyms[0]);
    if (codepoint == LUM_INVALID_CODEPOINT)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Failed to retrieve codepoint for key name");
        return NULL;
    }

    const size_t count = _lumEncodeUTF8(_lum.wl.keynames[key],  codepoint);
    if (count == 0)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Failed to encode codepoint for key name");
        return NULL;
    }

    _lum.wl.keynames[key][count] = '\0';
    return _lum.wl.keynames[key];
}

int _lumGetKeyScancodeWayland(int key)
{
    return _lum.wl.scancodes[key];
}

int _lumCreateCursorWayland(_LUMcursor* cursor,
                             const LUMimage* image,
                             int xhot, int yhot)
{
    cursor->wl.buffer = createShmBuffer(image);
    if (!cursor->wl.buffer)
        return LUM_FALSE;

    cursor->wl.width = image->width;
    cursor->wl.height = image->height;
    cursor->wl.xhot = xhot;
    cursor->wl.yhot = yhot;
    return LUM_TRUE;
}

int _lumCreateStandardCursorWayland(_LUMcursor* cursor, int shape)
{
    const char* name = NULL;

    // Try the XDG names first
    switch (shape)
    {
        case LUM_ARROW_CURSOR:
            name = "default";
            break;
        case LUM_IBEAM_CURSOR:
            name = "text";
            break;
        case LUM_CROSSHAIR_CURSOR:
            name = "crosshair";
            break;
        case LUM_POINTING_HAND_CURSOR:
            name = "pointer";
            break;
        case LUM_RESIZE_EW_CURSOR:
            name = "ew-resize";
            break;
        case LUM_RESIZE_NS_CURSOR:
            name = "ns-resize";
            break;
        case LUM_RESIZE_NWSE_CURSOR:
            name = "nwse-resize";
            break;
        case LUM_RESIZE_NESW_CURSOR:
            name = "nesw-resize";
            break;
        case LUM_RESIZE_ALL_CURSOR:
            name = "all-scroll";
            break;
        case LUM_NOT_ALLOWED_CURSOR:
            name = "not-allowed";
            break;
    }

    cursor->wl.cursor = wl_cursor_theme_get_cursor(_lum.wl.cursorTheme, name);

    if (_lum.wl.cursorThemeHiDPI)
    {
        cursor->wl.cursorHiDPI =
            wl_cursor_theme_get_cursor(_lum.wl.cursorThemeHiDPI, name);
    }

    if (!cursor->wl.cursor)
    {
        // Fall back to the core X11 names
        switch (shape)
        {
            case LUM_ARROW_CURSOR:
                name = "left_ptr";
                break;
            case LUM_IBEAM_CURSOR:
                name = "xterm";
                break;
            case LUM_CROSSHAIR_CURSOR:
                name = "crosshair";
                break;
            case LUM_POINTING_HAND_CURSOR:
                name = "hand2";
                break;
            case LUM_RESIZE_EW_CURSOR:
                name = "sb_h_double_arrow";
                break;
            case LUM_RESIZE_NS_CURSOR:
                name = "sb_v_double_arrow";
                break;
            case LUM_RESIZE_ALL_CURSOR:
                name = "fleur";
                break;
            default:
                _lumInputError(LUM_CURSOR_UNAVAILABLE,
                                "Wayland: Standard cursor shape unavailable");
                return LUM_FALSE;
        }

        cursor->wl.cursor = wl_cursor_theme_get_cursor(_lum.wl.cursorTheme, name);
        if (!cursor->wl.cursor)
        {
            _lumInputError(LUM_PLATFORM_ERROR,
                            "Wayland: Failed to create standard cursor \"%s\"",
                            name);
            return LUM_FALSE;
        }

        if (_lum.wl.cursorThemeHiDPI)
        {
            if (!cursor->wl.cursorHiDPI)
            {
                cursor->wl.cursorHiDPI =
                    wl_cursor_theme_get_cursor(_lum.wl.cursorThemeHiDPI, name);
            }
        }
    }

    return LUM_TRUE;
}

void _lumDestroyCursorWayland(_LUMcursor* cursor)
{
    // If it's a standard cursor we don't need to do anything here
    if (cursor->wl.cursor)
        return;

    if (cursor->wl.buffer)
        wl_buffer_destroy(cursor->wl.buffer);
}

static void relativePointerHandleRelativeMotion(void* data,
                                                struct zwp_relative_pointer_v1* pointer,
                                                uint32_t timeHi,
                                                uint32_t timeLo,
                                                wl_fixed_t dx,
                                                wl_fixed_t dy,
                                                wl_fixed_t dxUnaccel,
                                                wl_fixed_t dyUnaccel)
{
    _LUMwindow* window = data;
    double xpos = window->virtualCursorPosX;
    double ypos = window->virtualCursorPosY;

    if (window->cursorMode != LUM_CURSOR_DISABLED)
        return;

    if (window->rawMouseMotion)
    {
        xpos += wl_fixed_to_double(dxUnaccel);
        ypos += wl_fixed_to_double(dyUnaccel);
    }
    else
    {
        xpos += wl_fixed_to_double(dx);
        ypos += wl_fixed_to_double(dy);
    }

    _lumInputCursorPos(window, xpos, ypos);
}

static const struct zwp_relative_pointer_v1_listener relativePointerListener = {
    relativePointerHandleRelativeMotion
};

static void lockedPointerHandleLocked(void* data,
                                      struct zwp_locked_pointer_v1* lockedPointer)
{
}

static void unlockPointer(_LUMwindow* window)
{
    struct zwp_relative_pointer_v1* relativePointer =
        window->wl.pointerLock.relativePointer;
    struct zwp_locked_pointer_v1* lockedPointer =
        window->wl.pointerLock.lockedPointer;

    zwp_relative_pointer_v1_destroy(relativePointer);
    zwp_locked_pointer_v1_destroy(lockedPointer);

    window->wl.pointerLock.relativePointer = NULL;
    window->wl.pointerLock.lockedPointer = NULL;
}

static void lockPointer(_LUMwindow* window);

static void lockedPointerHandleUnlocked(void* data,
                                        struct zwp_locked_pointer_v1* lockedPointer)
{
}

static const struct zwp_locked_pointer_v1_listener lockedPointerListener = {
    lockedPointerHandleLocked,
    lockedPointerHandleUnlocked
};

static void lockPointer(_LUMwindow* window)
{
    struct zwp_relative_pointer_v1* relativePointer;
    struct zwp_locked_pointer_v1* lockedPointer;

    if (!_lum.wl.relativePointerManager)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: no relative pointer manager");
        return;
    }

    relativePointer =
        zwp_relative_pointer_manager_v1_get_relative_pointer(
            _lum.wl.relativePointerManager,
            _lum.wl.pointer);
    zwp_relative_pointer_v1_add_listener(relativePointer,
                                         &relativePointerListener,
                                         window);

    lockedPointer =
        zwp_pointer_constraints_v1_lock_pointer(
            _lum.wl.pointerConstraints,
            window->wl.surface,
            _lum.wl.pointer,
            NULL,
            ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
    zwp_locked_pointer_v1_add_listener(lockedPointer,
                                       &lockedPointerListener,
                                       window);

    window->wl.pointerLock.relativePointer = relativePointer;
    window->wl.pointerLock.lockedPointer = lockedPointer;

    wl_pointer_set_cursor(_lum.wl.pointer, _lum.wl.pointerEnterSerial,
                          NULL, 0, 0);
}

static LUMbool isPointerLocked(_LUMwindow* window)
{
    return window->wl.pointerLock.lockedPointer != NULL;
}

void _lumSetCursorWayland(_LUMwindow* window, _LUMcursor* cursor)
{
    struct wl_cursor* defaultCursor;
    struct wl_cursor* defaultCursorHiDPI = NULL;

    if (!_lum.wl.pointer)
        return;

    window->wl.currentCursor = cursor;

    // If we're not in the correct window just save the cursor
    // the next time the pointer enters the window the cursor will change
    if (window != _lum.wl.pointerFocus || window->wl.decorations.focus != mainWindow)
        return;

    // Unlock possible pointer lock if no longer disabled.
    if (window->cursorMode != LUM_CURSOR_DISABLED && isPointerLocked(window))
        unlockPointer(window);

    if (window->cursorMode == LUM_CURSOR_NORMAL)
    {
        if (cursor)
            setCursorImage(window, &cursor->wl);
        else
        {
            defaultCursor = wl_cursor_theme_get_cursor(_lum.wl.cursorTheme,
                                                       "left_ptr");
            if (!defaultCursor)
            {
                _lumInputError(LUM_PLATFORM_ERROR,
                                "Wayland: Standard cursor not found");
                return;
            }
            if (_lum.wl.cursorThemeHiDPI)
                defaultCursorHiDPI =
                    wl_cursor_theme_get_cursor(_lum.wl.cursorThemeHiDPI,
                                               "left_ptr");
            _LUMcursorWayland cursorWayland = {
                defaultCursor,
                defaultCursorHiDPI,
                NULL,
                0, 0,
                0, 0,
                0
            };
            setCursorImage(window, &cursorWayland);
        }
    }
    else if (window->cursorMode == LUM_CURSOR_DISABLED)
    {
        if (!isPointerLocked(window))
            lockPointer(window);
    }
    else if (window->cursorMode == LUM_CURSOR_HIDDEN)
    {
        wl_pointer_set_cursor(_lum.wl.pointer, _lum.wl.pointerEnterSerial, NULL, 0, 0);
    }
}

static void dataSourceHandleTarget(void* data,
                                   struct wl_data_source* dataSource,
                                   const char* mimeType)
{
    if (_lum.wl.dataSource != dataSource)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Unknown clipboard data source");
        return;
    }
}

static void dataSourceHandleSend(void* data,
                                 struct wl_data_source* dataSource,
                                 const char* mimeType,
                                 int fd)
{
    const char* string = _lum.wl.clipboardSendString;
    size_t len = _lum.wl.clipboardSendSize;
    int ret;

    if (_lum.wl.dataSource != dataSource)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Unknown clipboard data source");
        return;
    }

    if (!string)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Copy requested from an invalid string");
        return;
    }

    if (strcmp(mimeType, "text/plain;charset=utf-8") != 0)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Wrong MIME type asked from clipboard");
        close(fd);
        return;
    }

    while (len > 0)
    {
        ret = write(fd, string, len);
        if (ret == -1 && errno == EINTR)
            continue;
        if (ret == -1)
        {
            // TODO: also report errno maybe.
            _lumInputError(LUM_PLATFORM_ERROR,
                            "Wayland: Error while writing the clipboard");
            close(fd);
            return;
        }
        len -= ret;
    }
    close(fd);
}

static void dataSourceHandleCancelled(void* data,
                                      struct wl_data_source* dataSource)
{
    wl_data_source_destroy(dataSource);

    if (_lum.wl.dataSource != dataSource)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Unknown clipboard data source");
        return;
    }

    _lum.wl.dataSource = NULL;
}

static const struct wl_data_source_listener dataSourceListener = {
    dataSourceHandleTarget,
    dataSourceHandleSend,
    dataSourceHandleCancelled,
};

void _lumSetClipboardStringWayland(const char* string)
{
    if (_lum.wl.dataSource)
    {
        wl_data_source_destroy(_lum.wl.dataSource);
        _lum.wl.dataSource = NULL;
    }

    if (_lum.wl.clipboardSendString)
    {
        _lum_free(_lum.wl.clipboardSendString);
        _lum.wl.clipboardSendString = NULL;
    }

    _lum.wl.clipboardSendString = _lum_strdup(string);
    if (!_lum.wl.clipboardSendString)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Impossible to allocate clipboard string");
        return;
    }
    _lum.wl.clipboardSendSize = strlen(string);
    _lum.wl.dataSource =
        wl_data_device_manager_create_data_source(_lum.wl.dataDeviceManager);
    if (!_lum.wl.dataSource)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Impossible to create clipboard source");
        _lum_free(_lum.wl.clipboardSendString);
        return;
    }
    wl_data_source_add_listener(_lum.wl.dataSource,
                                &dataSourceListener,
                                NULL);
    wl_data_source_offer(_lum.wl.dataSource, "text/plain;charset=utf-8");
    wl_data_device_set_selection(_lum.wl.dataDevice,
                                 _lum.wl.dataSource,
                                 _lum.wl.serial);
}

static LUMbool growClipboardString(void)
{
    char* clipboard = _lum.wl.clipboardString;

    clipboard = _lum_realloc(clipboard, _lum.wl.clipboardSize * 2);
    if (!clipboard)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Impossible to grow clipboard string");
        return LUM_FALSE;
    }
    _lum.wl.clipboardString = clipboard;
    _lum.wl.clipboardSize = _lum.wl.clipboardSize * 2;
    return LUM_TRUE;
}

const char* _lumGetClipboardStringWayland(void)
{
    int fds[2];
    int ret;
    size_t len = 0;

    if (!_lum.wl.dataOffer)
    {
        _lumInputError(LUM_FORMAT_UNAVAILABLE,
                        "No clipboard data has been sent yet");
        return NULL;
    }

    ret = pipe2(fds, O_CLOEXEC);
    if (ret < 0)
    {
        // TODO: also report errno maybe?
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Impossible to create clipboard pipe fds");
        return NULL;
    }

    wl_data_offer_receive(_lum.wl.dataOffer, "text/plain;charset=utf-8", fds[1]);
    close(fds[1]);

    // XXX: this is a huge hack, this function shouldn’t be synchronous!
    handleEvents(-1);

    while (1)
    {
        // Grow the clipboard if we need to paste something bigger, there is no
        // shrink operation yet.
        if (len + 4096 > _lum.wl.clipboardSize)
        {
            if (!growClipboardString())
            {
                close(fds[0]);
                return NULL;
            }
        }

        // Then read from the fd to the clipboard, handling all known errors.
        ret = read(fds[0], _lum.wl.clipboardString + len, 4096);
        if (ret == 0)
            break;
        if (ret == -1 && errno == EINTR)
            continue;
        if (ret == -1)
        {
            // TODO: also report errno maybe.
            _lumInputError(LUM_PLATFORM_ERROR,
                            "Wayland: Impossible to read from clipboard fd");
            close(fds[0]);
            return NULL;
        }
        len += ret;
    }
    close(fds[0]);
    if (len + 1 > _lum.wl.clipboardSize)
    {
        if (!growClipboardString())
            return NULL;
    }
    _lum.wl.clipboardString[len] = '\0';
    return _lum.wl.clipboardString;
}

EGLenum _lumGetEGLPlatformWayland(EGLint** attribs)
{
    if (_lum.egl.EXT_platform_base && _lum.egl.EXT_platform_wayland)
        return EGL_PLATFORM_WAYLAND_EXT;
    else
        return 0;
}

EGLNativeDisplayType _lumGetEGLNativeDisplayWayland(void)
{
    return _lum.wl.display;
}

EGLNativeWindowType _lumGetEGLNativeWindowWayland(_LUMwindow* window)
{
    return window->wl.native;
}

void _lumGetRequiredInstanceExtensionsWayland(char** extensions)
{
    if (!_lum.vk.KHR_surface || !_lum.vk.KHR_wayland_surface)
        return;

    extensions[0] = "VK_KHR_surface";
    extensions[1] = "VK_KHR_wayland_surface";
}

int _lumGetPhysicalDevicePresentationSupportWayland(VkInstance instance,
                                                     VkPhysicalDevice device,
                                                     uint32_t queuefamily)
{
    PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR
        vkGetPhysicalDeviceWaylandPresentationSupportKHR =
        (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)
        vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
    if (!vkGetPhysicalDeviceWaylandPresentationSupportKHR)
    {
        _lumInputError(LUM_API_UNAVAILABLE,
                        "Wayland: Vulkan instance missing VK_KHR_wayland_surface extension");
        return VK_NULL_HANDLE;
    }

    return vkGetPhysicalDeviceWaylandPresentationSupportKHR(device,
                                                            queuefamily,
                                                            _lum.wl.display);
}

VkResult _lumCreateWindowSurfaceWayland(VkInstance instance,
                                         _LUMwindow* window,
                                         const VkAllocationCallbacks* allocator,
                                         VkSurfaceKHR* surface)
{
    VkResult err;
    VkWaylandSurfaceCreateInfoKHR sci;
    PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR;

    vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)
        vkGetInstanceProcAddr(instance, "vkCreateWaylandSurfaceKHR");
    if (!vkCreateWaylandSurfaceKHR)
    {
        _lumInputError(LUM_API_UNAVAILABLE,
                        "Wayland: Vulkan instance missing VK_KHR_wayland_surface extension");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    memset(&sci, 0, sizeof(sci));
    sci.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    sci.display = _lum.wl.display;
    sci.surface = window->wl.surface;

    err = vkCreateWaylandSurfaceKHR(instance, &sci, allocator, surface);
    if (err)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Failed to create Vulkan surface: %s",
                        _lumGetVulkanResultString(err));
    }

    return err;
}


//////////////////////////////////////////////////////////////////////////
//////                        Lumina native API                       //////
//////////////////////////////////////////////////////////////////////////

LUMAPI struct wl_display* lumGetWaylandDisplay(void)
{
    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    if (_lum.platform.platformID != LUM_PLATFORM_WAYLAND)
    {
        _lumInputError(LUM_PLATFORM_UNAVAILABLE,
                        "Wayland: Platform not initialized");
        return NULL;
    }

    return _lum.wl.display;
}

LUMAPI struct wl_surface* lumGetWaylandWindow(LUMwindow* handle)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    if (_lum.platform.platformID != LUM_PLATFORM_WAYLAND)
    {
        _lumInputError(LUM_PLATFORM_UNAVAILABLE,
                        "Wayland: Platform not initialized");
        return NULL;
    }

    return window->wl.surface;
}

