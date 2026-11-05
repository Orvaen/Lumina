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

#include <wayland-client-core.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>

typedef VkFlags VkWaylandSurfaceCreateFlagsKHR;

typedef struct VkWaylandSurfaceCreateInfoKHR
{
    VkStructureType                 sType;
    const void*                     pNext;
    VkWaylandSurfaceCreateFlagsKHR  flags;
    struct wl_display*              display;
    struct wl_surface*              surface;
} VkWaylandSurfaceCreateInfoKHR;

typedef VkResult (APIENTRY *PFN_vkCreateWaylandSurfaceKHR)(VkInstance,const VkWaylandSurfaceCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);
typedef VkBool32 (APIENTRY *PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)(VkPhysicalDevice,uint32_t,struct wl_display*);

#include "xkb_unicode.h"

typedef int (* PFN_wl_display_flush)(struct wl_display *display);
typedef void (* PFN_wl_display_cancel_read)(struct wl_display *display);
typedef int (* PFN_wl_display_dispatch_pending)(struct wl_display *display);
typedef int (* PFN_wl_display_read_events)(struct wl_display *display);
typedef struct wl_display* (* PFN_wl_display_connect)(const char*);
typedef void (* PFN_wl_display_disconnect)(struct wl_display*);
typedef int (* PFN_wl_display_roundtrip)(struct wl_display*);
typedef int (* PFN_wl_display_get_fd)(struct wl_display*);
typedef int (* PFN_wl_display_prepare_read)(struct wl_display*);
typedef void (* PFN_wl_proxy_marshal)(struct wl_proxy*,uint32_t,...);
typedef int (* PFN_wl_proxy_add_listener)(struct wl_proxy*,void(**)(void),void*);
typedef void (* PFN_wl_proxy_destroy)(struct wl_proxy*);
typedef struct wl_proxy* (* PFN_wl_proxy_marshal_constructor)(struct wl_proxy*,uint32_t,const struct wl_interface*,...);
typedef struct wl_proxy* (* PFN_wl_proxy_marshal_constructor_versioned)(struct wl_proxy*,uint32_t,const struct wl_interface*,uint32_t,...);
typedef void* (* PFN_wl_proxy_get_user_data)(struct wl_proxy*);
typedef void (* PFN_wl_proxy_set_user_data)(struct wl_proxy*,void*);
typedef uint32_t (* PFN_wl_proxy_get_version)(struct wl_proxy*);
typedef struct wl_proxy* (* PFN_wl_proxy_marshal_flags)(struct wl_proxy*,uint32_t,const struct wl_interface*,uint32_t,uint32_t,...);
#define wl_display_flush _lum.wl.client.display_flush
#define wl_display_cancel_read _lum.wl.client.display_cancel_read
#define wl_display_dispatch_pending _lum.wl.client.display_dispatch_pending
#define wl_display_read_events _lum.wl.client.display_read_events
#define wl_display_disconnect _lum.wl.client.display_disconnect
#define wl_display_roundtrip _lum.wl.client.display_roundtrip
#define wl_display_get_fd _lum.wl.client.display_get_fd
#define wl_display_prepare_read _lum.wl.client.display_prepare_read
#define wl_proxy_marshal _lum.wl.client.proxy_marshal
#define wl_proxy_add_listener _lum.wl.client.proxy_add_listener
#define wl_proxy_destroy _lum.wl.client.proxy_destroy
#define wl_proxy_marshal_constructor _lum.wl.client.proxy_marshal_constructor
#define wl_proxy_marshal_constructor_versioned _lum.wl.client.proxy_marshal_constructor_versioned
#define wl_proxy_get_user_data _lum.wl.client.proxy_get_user_data
#define wl_proxy_set_user_data _lum.wl.client.proxy_set_user_data
#define wl_proxy_get_version _lum.wl.client.proxy_get_version
#define wl_proxy_marshal_flags _lum.wl.client.proxy_marshal_flags

struct wl_shm;

#define wl_display_interface _lum_wl_display_interface
#define wl_subcompositor_interface _lum_wl_subcompositor_interface
#define wl_compositor_interface _lum_wl_compositor_interface
#define wl_shm_interface _lum_wl_shm_interface
#define wl_data_device_manager_interface _lum_wl_data_device_manager_interface
#define wl_shell_interface _lum_wl_shell_interface
#define wl_buffer_interface _lum_wl_buffer_interface
#define wl_callback_interface _lum_wl_callback_interface
#define wl_data_device_interface _lum_wl_data_device_interface
#define wl_data_offer_interface _lum_wl_data_offer_interface
#define wl_data_source_interface _lum_wl_data_source_interface
#define wl_keyboard_interface _lum_wl_keyboard_interface
#define wl_output_interface _lum_wl_output_interface
#define wl_pointer_interface _lum_wl_pointer_interface
#define wl_region_interface _lum_wl_region_interface
#define wl_registry_interface _lum_wl_registry_interface
#define wl_seat_interface _lum_wl_seat_interface
#define wl_shell_surface_interface _lum_wl_shell_surface_interface
#define wl_shm_pool_interface _lum_wl_shm_pool_interface
#define wl_subsurface_interface _lum_wl_subsurface_interface
#define wl_surface_interface _lum_wl_surface_interface
#define wl_touch_interface _lum_wl_touch_interface
#define zwp_idle_inhibitor_v1_interface _lum_zwp_idle_inhibitor_v1_interface
#define zwp_idle_inhibit_manager_v1_interface _lum_zwp_idle_inhibit_manager_v1_interface
#define zwp_confined_pointer_v1_interface _lum_zwp_confined_pointer_v1_interface
#define zwp_locked_pointer_v1_interface _lum_zwp_locked_pointer_v1_interface
#define zwp_pointer_constraints_v1_interface _lum_zwp_pointer_constraints_v1_interface
#define zwp_relative_pointer_v1_interface _lum_zwp_relative_pointer_v1_interface
#define zwp_relative_pointer_manager_v1_interface _lum_zwp_relative_pointer_manager_v1_interface
#define wp_viewport_interface _lum_wp_viewport_interface
#define wp_viewporter_interface _lum_wp_viewporter_interface
#define xdg_toplevel_interface _lum_xdg_toplevel_interface
#define zxdg_toplevel_decoration_v1_interface _lum_zxdg_toplevel_decoration_v1_interface
#define zxdg_decoration_manager_v1_interface _lum_zxdg_decoration_manager_v1_interface
#define xdg_popup_interface _lum_xdg_popup_interface
#define xdg_positioner_interface _lum_xdg_positioner_interface
#define xdg_surface_interface _lum_xdg_surface_interface
#define xdg_toplevel_interface _lum_xdg_toplevel_interface
#define xdg_wm_base_interface _lum_xdg_wm_base_interface

#define LUM_WAYLAND_WINDOW_STATE         _LUMwindowWayland  wl;
#define LUM_WAYLAND_LIBRARY_WINDOW_STATE _LUMlibraryWayland wl;
#define LUM_WAYLAND_MONITOR_STATE        _LUMmonitorWayland wl;
#define LUM_WAYLAND_CURSOR_STATE         _LUMcursorWayland  wl;

struct wl_cursor_image {
    uint32_t width;
    uint32_t height;
    uint32_t hotspot_x;
    uint32_t hotspot_y;
    uint32_t delay;
};
struct wl_cursor {
    unsigned int image_count;
    struct wl_cursor_image** images;
    char* name;
};
typedef struct wl_cursor_theme* (* PFN_wl_cursor_theme_load)(const char*, int, struct wl_shm*);
typedef void (* PFN_wl_cursor_theme_destroy)(struct wl_cursor_theme*);
typedef struct wl_cursor* (* PFN_wl_cursor_theme_get_cursor)(struct wl_cursor_theme*, const char*);
typedef struct wl_buffer* (* PFN_wl_cursor_image_get_buffer)(struct wl_cursor_image*);
#define wl_cursor_theme_load _lum.wl.cursor.theme_load
#define wl_cursor_theme_destroy _lum.wl.cursor.theme_destroy
#define wl_cursor_theme_get_cursor _lum.wl.cursor.theme_get_cursor
#define wl_cursor_image_get_buffer _lum.wl.cursor.image_get_buffer

typedef struct wl_egl_window* (* PFN_wl_egl_window_create)(struct wl_surface*, int, int);
typedef void (* PFN_wl_egl_window_destroy)(struct wl_egl_window*);
typedef void (* PFN_wl_egl_window_resize)(struct wl_egl_window*, int, int, int, int);
#define wl_egl_window_create _lum.wl.egl.window_create
#define wl_egl_window_destroy _lum.wl.egl.window_destroy
#define wl_egl_window_resize _lum.wl.egl.window_resize

typedef struct xkb_context* (* PFN_xkb_context_new)(enum xkb_context_flags);
typedef void (* PFN_xkb_context_unref)(struct xkb_context*);
typedef struct xkb_keymap* (* PFN_xkb_keymap_new_from_string)(struct xkb_context*, const char*, enum xkb_keymap_format, enum xkb_keymap_compile_flags);
typedef void (* PFN_xkb_keymap_unref)(struct xkb_keymap*);
typedef xkb_mod_index_t (* PFN_xkb_keymap_mod_get_index)(struct xkb_keymap*, const char*);
typedef int (* PFN_xkb_keymap_key_repeats)(struct xkb_keymap*, xkb_keycode_t);
typedef int (* PFN_xkb_keymap_key_get_syms_by_level)(struct xkb_keymap*,xkb_keycode_t,xkb_layout_index_t,xkb_level_index_t,const xkb_keysym_t**);
typedef struct xkb_state* (* PFN_xkb_state_new)(struct xkb_keymap*);
typedef void (* PFN_xkb_state_unref)(struct xkb_state*);
typedef int (* PFN_xkb_state_key_get_syms)(struct xkb_state*, xkb_keycode_t, const xkb_keysym_t**);
typedef enum xkb_state_component (* PFN_xkb_state_update_mask)(struct xkb_state*, xkb_mod_mask_t, xkb_mod_mask_t, xkb_mod_mask_t, xkb_layout_index_t, xkb_layout_index_t, xkb_layout_index_t);
typedef xkb_mod_mask_t (* PFN_xkb_state_serialize_mods)(struct xkb_state*, enum xkb_state_component);
typedef xkb_layout_index_t (* PFN_xkb_state_key_get_layout)(struct xkb_state*,xkb_keycode_t);
#define xkb_context_new _lum.wl.xkb.context_new
#define xkb_context_unref _lum.wl.xkb.context_unref
#define xkb_keymap_new_from_string _lum.wl.xkb.keymap_new_from_string
#define xkb_keymap_unref _lum.wl.xkb.keymap_unref
#define xkb_keymap_mod_get_index _lum.wl.xkb.keymap_mod_get_index
#define xkb_keymap_key_repeats _lum.wl.xkb.keymap_key_repeats
#define xkb_keymap_key_get_syms_by_level _lum.wl.xkb.keymap_key_get_syms_by_level
#define xkb_state_new _lum.wl.xkb.state_new
#define xkb_state_unref _lum.wl.xkb.state_unref
#define xkb_state_key_get_syms _lum.wl.xkb.state_key_get_syms
#define xkb_state_update_mask _lum.wl.xkb.state_update_mask
#define xkb_state_serialize_mods _lum.wl.xkb.state_serialize_mods
#define xkb_state_key_get_layout _lum.wl.xkb.state_key_get_layout

typedef struct xkb_compose_table* (* PFN_xkb_compose_table_new_from_locale)(struct xkb_context*, const char*, enum xkb_compose_compile_flags);
typedef void (* PFN_xkb_compose_table_unref)(struct xkb_compose_table*);
typedef struct xkb_compose_state* (* PFN_xkb_compose_state_new)(struct xkb_compose_table*, enum xkb_compose_state_flags);
typedef void (* PFN_xkb_compose_state_unref)(struct xkb_compose_state*);
typedef enum xkb_compose_feed_result (* PFN_xkb_compose_state_feed)(struct xkb_compose_state*, xkb_keysym_t);
typedef enum xkb_compose_status (* PFN_xkb_compose_state_get_status)(struct xkb_compose_state*);
typedef xkb_keysym_t (* PFN_xkb_compose_state_get_one_sym)(struct xkb_compose_state*);
#define xkb_compose_table_new_from_locale _lum.wl.xkb.compose_table_new_from_locale
#define xkb_compose_table_unref _lum.wl.xkb.compose_table_unref
#define xkb_compose_state_new _lum.wl.xkb.compose_state_new
#define xkb_compose_state_unref _lum.wl.xkb.compose_state_unref
#define xkb_compose_state_feed _lum.wl.xkb.compose_state_feed
#define xkb_compose_state_get_status _lum.wl.xkb.compose_state_get_status
#define xkb_compose_state_get_one_sym _lum.wl.xkb.compose_state_get_one_sym

#define _LUM_DECORATION_WIDTH 4
#define _LUM_DECORATION_TOP 24
#define _LUM_DECORATION_VERTICAL (_LUM_DECORATION_TOP + _LUM_DECORATION_WIDTH)
#define _LUM_DECORATION_HORIZONTAL (2 * _LUM_DECORATION_WIDTH)

typedef enum _LUMdecorationSideWayland
{
    mainWindow,
    topDecoration,
    leftDecoration,
    rightDecoration,
    bottomDecoration,
} _LUMdecorationSideWayland;

typedef struct _LUMdecorationWayland
{
    struct wl_surface*          surface;
    struct wl_subsurface*       subsurface;
    struct wp_viewport*         viewport;
} _LUMdecorationWayland;

// Wayland-specific per-window data
//
typedef struct _LUMwindowWayland
{
    int                         width, height;
    LUMbool                    visible;
    LUMbool                    maximized;
    LUMbool                    hovered;
    LUMbool                    transparent;
    struct wl_surface*          surface;
    struct wl_egl_window*       native;
    struct wl_callback*         callback;

    struct {
        struct xdg_surface*     surface;
        struct xdg_toplevel*    toplevel;
        struct zxdg_toplevel_decoration_v1* decoration;
    } xdg;

    _LUMcursor*                currentCursor;
    double                      cursorPosX, cursorPosY;

    char*                       title;

    // We need to track the monitors the window spans on to calculate the
    // optimal scaling factor.
    int                         scale;
    _LUMmonitor**              monitors;
    int                         monitorsCount;
    int                         monitorsSize;

    struct {
        struct zwp_relative_pointer_v1*    relativePointer;
        struct zwp_locked_pointer_v1*      lockedPointer;
    } pointerLock;

    struct zwp_idle_inhibitor_v1*          idleInhibitor;

    LUMbool                    wasFullscreen;

    struct {
        LUMbool                           serverSide;
        struct wl_buffer*                  buffer;
        _LUMdecorationWayland             top, left, right, bottom;
        int                                focus;
    } decorations;
} _LUMwindowWayland;

// Wayland-specific global data
//
typedef struct _LUMlibraryWayland
{
    struct wl_display*          display;
    struct wl_registry*         registry;
    struct wl_compositor*       compositor;
    struct wl_subcompositor*    subcompositor;
    struct wl_shm*              shm;
    struct wl_seat*             seat;
    struct wl_pointer*          pointer;
    struct wl_keyboard*         keyboard;
    struct wl_data_device_manager*          dataDeviceManager;
    struct wl_data_device*      dataDevice;
    struct wl_data_offer*       dataOffer;
    struct wl_data_source*      dataSource;
    struct xdg_wm_base*         wmBase;
    struct zxdg_decoration_manager_v1*      decorationManager;
    struct wp_viewporter*       viewporter;
    struct zwp_relative_pointer_manager_v1* relativePointerManager;
    struct zwp_pointer_constraints_v1*      pointerConstraints;
    struct zwp_idle_inhibit_manager_v1*     idleInhibitManager;

    int                         compositorVersion;
    int                         seatVersion;

    struct wl_cursor_theme*     cursorTheme;
    struct wl_cursor_theme*     cursorThemeHiDPI;
    struct wl_surface*          cursorSurface;
    const char*                 cursorPreviousName;
    int                         cursorTimerfd;
    uint32_t                    serial;
    uint32_t                    pointerEnterSerial;

    int32_t                     keyboardRepeatRate;
    int32_t                     keyboardRepeatDelay;
    int                         keyboardLastKey;
    int                         keyboardLastScancode;
    char*                       clipboardString;
    size_t                      clipboardSize;
    char*                       clipboardSendString;
    size_t                      clipboardSendSize;
    int                         timerfd;
    short int                   keycodes[256];
    short int                   scancodes[LUM_KEY_LAST + 1];
    char                        keynames[LUM_KEY_LAST + 1][5];

    struct {
        void*                   handle;
        struct xkb_context*     context;
        struct xkb_keymap*      keymap;
        struct xkb_state*       state;

        struct xkb_compose_state* composeState;

        xkb_mod_mask_t          controlMask;
        xkb_mod_mask_t          altMask;
        xkb_mod_mask_t          shiftMask;
        xkb_mod_mask_t          superMask;
        xkb_mod_mask_t          capsLockMask;
        xkb_mod_mask_t          numLockMask;
        unsigned int            modifiers;

        PFN_xkb_context_new context_new;
        PFN_xkb_context_unref context_unref;
        PFN_xkb_keymap_new_from_string keymap_new_from_string;
        PFN_xkb_keymap_unref keymap_unref;
        PFN_xkb_keymap_mod_get_index keymap_mod_get_index;
        PFN_xkb_keymap_key_repeats keymap_key_repeats;
        PFN_xkb_keymap_key_get_syms_by_level keymap_key_get_syms_by_level;
        PFN_xkb_state_new state_new;
        PFN_xkb_state_unref state_unref;
        PFN_xkb_state_key_get_syms state_key_get_syms;
        PFN_xkb_state_update_mask state_update_mask;
        PFN_xkb_state_serialize_mods state_serialize_mods;
        PFN_xkb_state_key_get_layout state_key_get_layout;

        PFN_xkb_compose_table_new_from_locale compose_table_new_from_locale;
        PFN_xkb_compose_table_unref compose_table_unref;
        PFN_xkb_compose_state_new compose_state_new;
        PFN_xkb_compose_state_unref compose_state_unref;
        PFN_xkb_compose_state_feed compose_state_feed;
        PFN_xkb_compose_state_get_status compose_state_get_status;
        PFN_xkb_compose_state_get_one_sym compose_state_get_one_sym;
    } xkb;

    _LUMwindow*                pointerFocus;
    _LUMwindow*                keyboardFocus;

    struct {
        void*                                       handle;
        PFN_wl_display_flush                        display_flush;
        PFN_wl_display_cancel_read                  display_cancel_read;
        PFN_wl_display_dispatch_pending             display_dispatch_pending;
        PFN_wl_display_read_events                  display_read_events;
        PFN_wl_display_disconnect                   display_disconnect;
        PFN_wl_display_roundtrip                    display_roundtrip;
        PFN_wl_display_get_fd                       display_get_fd;
        PFN_wl_display_prepare_read                 display_prepare_read;
        PFN_wl_proxy_marshal                        proxy_marshal;
        PFN_wl_proxy_add_listener                   proxy_add_listener;
        PFN_wl_proxy_destroy                        proxy_destroy;
        PFN_wl_proxy_marshal_constructor            proxy_marshal_constructor;
        PFN_wl_proxy_marshal_constructor_versioned  proxy_marshal_constructor_versioned;
        PFN_wl_proxy_get_user_data                  proxy_get_user_data;
        PFN_wl_proxy_set_user_data                  proxy_set_user_data;
        PFN_wl_proxy_get_version                    proxy_get_version;
        PFN_wl_proxy_marshal_flags                  proxy_marshal_flags;
    } client;

    struct {
        void*                   handle;

        PFN_wl_cursor_theme_load theme_load;
        PFN_wl_cursor_theme_destroy theme_destroy;
        PFN_wl_cursor_theme_get_cursor theme_get_cursor;
        PFN_wl_cursor_image_get_buffer image_get_buffer;
    } cursor;

    struct {
        void*                   handle;

        PFN_wl_egl_window_create window_create;
        PFN_wl_egl_window_destroy window_destroy;
        PFN_wl_egl_window_resize window_resize;
    } egl;
} _LUMlibraryWayland;

// Wayland-specific per-monitor data
//
typedef struct _LUMmonitorWayland
{
    struct wl_output*           output;
    uint32_t                    name;
    int                         currentMode;

    int                         x;
    int                         y;
    int                         scale;
} _LUMmonitorWayland;

// Wayland-specific per-cursor data
//
typedef struct _LUMcursorWayland
{
    struct wl_cursor*           cursor;
    struct wl_cursor*           cursorHiDPI;
    struct wl_buffer*           buffer;
    int                         width, height;
    int                         xhot, yhot;
    int                         currentImage;
} _LUMcursorWayland;

LUMbool _lumConnectWayland(int platformID, _LUMplatform* platform);
int _lumInitWayland(void);
void _lumTerminateWayland(void);

int _lumCreateWindowWayland(_LUMwindow* window, const _LUMwndconfig* wndconfig, const _LUMctxconfig* ctxconfig, const _LUMfbconfig* fbconfig);
void _lumDestroyWindowWayland(_LUMwindow* window);
void _lumSetWindowTitleWayland(_LUMwindow* window, const char* title);
void _lumSetWindowIconWayland(_LUMwindow* window, int count, const LUMimage* images);
void _lumGetWindowPosWayland(_LUMwindow* window, int* xpos, int* ypos);
void _lumSetWindowPosWayland(_LUMwindow* window, int xpos, int ypos);
void _lumGetWindowSizeWayland(_LUMwindow* window, int* width, int* height);
void _lumSetWindowSizeWayland(_LUMwindow* window, int width, int height);
void _lumSetWindowSizeLimitsWayland(_LUMwindow* window, int minwidth, int minheight, int maxwidth, int maxheight);
void _lumSetWindowAspectRatioWayland(_LUMwindow* window, int numer, int denom);
void _lumGetFramebufferSizeWayland(_LUMwindow* window, int* width, int* height);
void _lumGetWindowFrameSizeWayland(_LUMwindow* window, int* left, int* top, int* right, int* bottom);
void _lumGetWindowContentScaleWayland(_LUMwindow* window, float* xscale, float* yscale);
void _lumIconifyWindowWayland(_LUMwindow* window);
void _lumRestoreWindowWayland(_LUMwindow* window);
void _lumMaximizeWindowWayland(_LUMwindow* window);
void _lumShowWindowWayland(_LUMwindow* window);
void _lumHideWindowWayland(_LUMwindow* window);
void _lumRequestWindowAttentionWayland(_LUMwindow* window);
void _lumFocusWindowWayland(_LUMwindow* window);
void _lumSetWindowMonitorWayland(_LUMwindow* window, _LUMmonitor* monitor, int xpos, int ypos, int width, int height, int refreshRate);
int _lumWindowFocusedWayland(_LUMwindow* window);
int _lumWindowIconifiedWayland(_LUMwindow* window);
int _lumWindowVisibleWayland(_LUMwindow* window);
int _lumWindowMaximizedWayland(_LUMwindow* window);
int _lumWindowHoveredWayland(_LUMwindow* window);
int _lumFramebufferTransparentWayland(_LUMwindow* window);
void _lumSetWindowResizableWayland(_LUMwindow* window, LUMbool enabled);
void _lumSetWindowDecoratedWayland(_LUMwindow* window, LUMbool enabled);
void _lumSetWindowFloatingWayland(_LUMwindow* window, LUMbool enabled);
float _lumGetWindowOpacityWayland(_LUMwindow* window);
void _lumSetWindowOpacityWayland(_LUMwindow* window, float opacity);
void _lumSetWindowMousePassthroughWayland(_LUMwindow* window, LUMbool enabled);

void _lumSetRawMouseMotionWayland(_LUMwindow *window, LUMbool enabled);
LUMbool _lumRawMouseMotionSupportedWayland(void);

void _lumPollEventsWayland(void);
void _lumWaitEventsWayland(void);
void _lumWaitEventsTimeoutWayland(double timeout);
void _lumPostEmptyEventWayland(void);

void _lumGetCursorPosWayland(_LUMwindow* window, double* xpos, double* ypos);
void _lumSetCursorPosWayland(_LUMwindow* window, double xpos, double ypos);
void _lumSetCursorModeWayland(_LUMwindow* window, int mode);
const char* _lumGetScancodeNameWayland(int scancode);
int _lumGetKeyScancodeWayland(int key);
int _lumCreateCursorWayland(_LUMcursor* cursor, const LUMimage* image, int xhot, int yhot);
int _lumCreateStandardCursorWayland(_LUMcursor* cursor, int shape);
void _lumDestroyCursorWayland(_LUMcursor* cursor);
void _lumSetCursorWayland(_LUMwindow* window, _LUMcursor* cursor);
void _lumSetClipboardStringWayland(const char* string);
const char* _lumGetClipboardStringWayland(void);

EGLenum _lumGetEGLPlatformWayland(EGLint** attribs);
EGLNativeDisplayType _lumGetEGLNativeDisplayWayland(void);
EGLNativeWindowType _lumGetEGLNativeWindowWayland(_LUMwindow* window);

void _lumGetRequiredInstanceExtensionsWayland(char** extensions);
int _lumGetPhysicalDevicePresentationSupportWayland(VkInstance instance, VkPhysicalDevice device, uint32_t queuefamily);
VkResult _lumCreateWindowSurfaceWayland(VkInstance instance, _LUMwindow* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface);

void _lumFreeMonitorWayland(_LUMmonitor* monitor);
void _lumGetMonitorPosWayland(_LUMmonitor* monitor, int* xpos, int* ypos);
void _lumGetMonitorContentScaleWayland(_LUMmonitor* monitor, float* xscale, float* yscale);
void _lumGetMonitorWorkareaWayland(_LUMmonitor* monitor, int* xpos, int* ypos, int* width, int* height);
LUMvidmode* _lumGetVideoModesWayland(_LUMmonitor* monitor, int* count);
void _lumGetVideoModeWayland(_LUMmonitor* monitor, LUMvidmode* mode);
LUMbool _lumGetGammaRampWayland(_LUMmonitor* monitor, LUMgammaramp* ramp);
void _lumSetGammaRampWayland(_LUMmonitor* monitor, const LUMgammaramp* ramp);

void _lumAddOutputWayland(uint32_t name, uint32_t version);
LUMbool _lumInputTextWayland(_LUMwindow* window, uint32_t scancode);

