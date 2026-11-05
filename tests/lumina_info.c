//========================================================================
// Context creation and information tool
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
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

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLAD_VULKAN_IMPLEMENTATION
#include <glad/vulkan.h>
#define LUM_INCLUDE_NONE
#include <Lumina/lumina.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "getopt.h"

#ifdef _MSC_VER
#define strcasecmp(x, y) _stricmp(x, y)
#endif

#define API_NAME_OPENGL     "gl"
#define API_NAME_OPENGL_ES  "es"

#define API_NAME_NATIVE     "native"
#define API_NAME_EGL        "egl"
#define API_NAME_OSMESA     "osmesa"

#define PROFILE_NAME_CORE   "core"
#define PROFILE_NAME_COMPAT "compat"

#define STRATEGY_NAME_NONE  "none"
#define STRATEGY_NAME_LOSE  "lose"

#define BEHAVIOR_NAME_NONE  "none"
#define BEHAVIOR_NAME_FLUSH "flush"

#define ANGLE_TYPE_OPENGL   "gl"
#define ANGLE_TYPE_OPENGLES "es"
#define ANGLE_TYPE_D3D9     "d3d9"
#define ANGLE_TYPE_D3D11    "d3d11"
#define ANGLE_TYPE_VULKAN   "vk"
#define ANGLE_TYPE_METAL    "mtl"

#define PLATFORM_NAME_ANY   "any"
#define PLATFORM_NAME_WIN32 "win32"
#define PLATFORM_NAME_COCOA "cooca"
#define PLATFORM_NAME_WL    "wayland"
#define PLATFORM_NAME_X11   "x11"
#define PLATFORM_NAME_NULL  "null"

static void usage(void)
{
    printf("Usage: lumina_info [OPTION]...\n");
    printf("Options:\n");
    printf("      --platform=PLATFORM   the platform to use ("
                                        PLATFORM_NAME_ANY " or "
                                        PLATFORM_NAME_WIN32 " or "
                                        PLATFORM_NAME_COCOA " or "
                                        PLATFORM_NAME_X11 " or "
                                        PLATFORM_NAME_WL " or "
                                        PLATFORM_NAME_NULL ")\n");
    printf("  -a, --client-api=API      the client API to use ("
                                        API_NAME_OPENGL " or "
                                        API_NAME_OPENGL_ES ")\n");
    printf("  -b, --behavior=BEHAVIOR   the release behavior to use ("
                                        BEHAVIOR_NAME_NONE " or "
                                        BEHAVIOR_NAME_FLUSH ")\n");
    printf("  -c, --context-api=API     the context creation API to use ("
                                        API_NAME_NATIVE " or "
                                        API_NAME_EGL " or "
                                        API_NAME_OSMESA ")\n");
    printf("  -d, --debug               request a debug context\n");
    printf("  -f, --forward             require a forward-compatible context\n");
    printf("  -h, --help                show this help\n");
    printf("  -l, --list-extensions     list all Vulkan and client API extensions\n");
    printf("      --list-layers         list all Vulkan layers\n");
    printf("  -m, --major=MAJOR         the major number of the required "
                                        "client API version\n");
    printf("  -n, --minor=MINOR         the minor number of the required "
                                        "client API version\n");
    printf("  -p, --profile=PROFILE     the OpenGL profile to use ("
                                        PROFILE_NAME_CORE " or "
                                        PROFILE_NAME_COMPAT ")\n");
    printf("  -s, --robustness=STRATEGY the robustness strategy to use ("
                                        STRATEGY_NAME_NONE " or "
                                        STRATEGY_NAME_LOSE ")\n");
    printf("  -v, --version             print version information\n");
    printf("      --red-bits=N          the number of red bits to request\n");
    printf("      --green-bits=N        the number of green bits to request\n");
    printf("      --blue-bits=N         the number of blue bits to request\n");
    printf("      --alpha-bits=N        the number of alpha bits to request\n");
    printf("      --depth-bits=N        the number of depth bits to request\n");
    printf("      --stencil-bits=N      the number of stencil bits to request\n");
    printf("      --accum-red-bits=N    the number of red bits to request\n");
    printf("      --accum-green-bits=N  the number of green bits to request\n");
    printf("      --accum-blue-bits=N   the number of blue bits to request\n");
    printf("      --accum-alpha-bits=N  the number of alpha bits to request\n");
    printf("      --aux-buffers=N       the number of aux buffers to request\n");
    printf("      --samples=N           the number of MSAA samples to request\n");
    printf("      --stereo              request stereo rendering\n");
    printf("      --srgb                request an sRGB capable framebuffer\n");
    printf("      --singlebuffer        request single-buffering\n");
    printf("      --no-error            request a context that does not emit errors\n");
    printf("      --angle-type=TYPE     the ANGLE platform type to use ("
                                        ANGLE_TYPE_OPENGL ", "
                                        ANGLE_TYPE_OPENGLES ", "
                                        ANGLE_TYPE_D3D9 ", "
                                        ANGLE_TYPE_D3D11 ", "
                                        ANGLE_TYPE_VULKAN " or "
                                        ANGLE_TYPE_METAL ")\n");
    printf("      --graphics-switching  request macOS graphics switching\n");
    printf("      --disable-xcb-surface disable VK_KHR_xcb_surface extension\n");
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static const char* get_platform_name(int platform)
{
    if (platform == LUM_PLATFORM_WIN32)
        return "Win32";
    else if (platform == LUM_PLATFORM_COCOA)
        return "Cocoa";
    else if (platform == LUM_PLATFORM_WAYLAND)
        return "Wayland";
    else if (platform == LUM_PLATFORM_X11)
        return "X11";
    else if (platform == LUM_PLATFORM_NULL)
        return "Null";

    return "unknown";
}

static const char* get_device_type_name(VkPhysicalDeviceType type)
{
    if (type == VK_PHYSICAL_DEVICE_TYPE_OTHER)
        return "other";
    else if (type == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        return "integrated GPU";
    else if (type == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        return "discrete GPU";
    else if (type == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
        return "virtual GPU";
    else if (type == VK_PHYSICAL_DEVICE_TYPE_CPU)
        return "CPU";

    return "unknown";
}

static const char* get_api_name(int api)
{
    if (api == LUM_OPENGL_API)
        return "OpenGL";
    else if (api == LUM_OPENGL_ES_API)
        return "OpenGL ES";

    return "Unknown API";
}

static const char* get_profile_name_gl(GLint mask)
{
    if (mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
        return PROFILE_NAME_COMPAT;
    if (mask & GL_CONTEXT_CORE_PROFILE_BIT)
        return PROFILE_NAME_CORE;

    return "unknown";
}

static const char* get_profile_name_lum(int profile)
{
    if (profile == LUM_OPENGL_COMPAT_PROFILE)
        return PROFILE_NAME_COMPAT;
    if (profile == LUM_OPENGL_CORE_PROFILE)
        return PROFILE_NAME_CORE;

    return "unknown";
}

static const char* get_strategy_name_gl(GLint strategy)
{
    if (strategy == GL_LOSE_CONTEXT_ON_RESET_ARB)
        return STRATEGY_NAME_LOSE;
    if (strategy == GL_NO_RESET_NOTIFICATION_ARB)
        return STRATEGY_NAME_NONE;

    return "unknown";
}

static const char* get_strategy_name_lum(int strategy)
{
    if (strategy == LUM_LOSE_CONTEXT_ON_RESET)
        return STRATEGY_NAME_LOSE;
    if (strategy == LUM_NO_RESET_NOTIFICATION)
        return STRATEGY_NAME_NONE;

    return "unknown";
}

static void list_context_extensions(int client, int major, int minor)
{
    printf("%s context extensions:\n", get_api_name(client));

    if (client == LUM_OPENGL_API && major > 2)
    {
        GLint count;
        glGetIntegerv(GL_NUM_EXTENSIONS, &count);

        for (int i = 0;  i < count;  i++)
            printf(" %s\n", (const char*) glGetStringi(GL_EXTENSIONS, i));
    }
    else
    {
        const GLubyte* extensions = glGetString(GL_EXTENSIONS);
        while (*extensions != '\0')
        {
            putchar(' ');

            while (*extensions != '\0' && *extensions != ' ')
            {
                putchar(*extensions);
                extensions++;
            }

            while (*extensions == ' ')
                extensions++;

            putchar('\n');
        }
    }
}

static void list_vulkan_instance_extensions(void)
{
    printf("Vulkan instance extensions:\n");

    uint32_t ep_count;
    vkEnumerateInstanceExtensionProperties(NULL, &ep_count, NULL);
    VkExtensionProperties* ep = calloc(ep_count, sizeof(VkExtensionProperties));
    vkEnumerateInstanceExtensionProperties(NULL, &ep_count, ep);

    for (uint32_t i = 0;  i < ep_count;  i++)
        printf(" %s (spec version %u)\n", ep[i].extensionName, ep[i].specVersion);

    free(ep);
}

static void list_vulkan_instance_layers(void)
{
    printf("Vulkan instance layers:\n");

    uint32_t lp_count;
    vkEnumerateInstanceLayerProperties(&lp_count, NULL);
    VkLayerProperties* lp = calloc(lp_count, sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&lp_count, lp);

    for (uint32_t i = 0;  i < lp_count;  i++)
    {
        printf(" %s (spec version %u) \"%s\"\n",
               lp[i].layerName,
               lp[i].specVersion >> 22,
               lp[i].description);
    }

    free(lp);
}

static void list_vulkan_device_extensions(VkInstance instance, VkPhysicalDevice device)
{
    printf("Vulkan device extensions:\n");

    uint32_t ep_count;
    vkEnumerateDeviceExtensionProperties(device, NULL, &ep_count, NULL);
    VkExtensionProperties* ep = calloc(ep_count, sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(device, NULL, &ep_count, ep);

    for (uint32_t i = 0;  i < ep_count;  i++)
        printf(" %s (spec version %u)\n", ep[i].extensionName, ep[i].specVersion);

    free(ep);
}

static void list_vulkan_device_layers(VkInstance instance, VkPhysicalDevice device)
{
    printf("Vulkan device layers:\n");

    uint32_t lp_count;
    vkEnumerateDeviceLayerProperties(device, &lp_count, NULL);
    VkLayerProperties* lp = calloc(lp_count, sizeof(VkLayerProperties));
    vkEnumerateDeviceLayerProperties(device, &lp_count, lp);

    for (uint32_t i = 0;  i < lp_count;  i++)
    {
        printf(" %s (spec version %u) \"%s\"\n",
               lp[i].layerName,
               lp[i].specVersion >> 22,
               lp[i].description);
    }

    free(lp);
}

static bool valid_version(void)
{
    int major, minor, revision;
    lumGetVersion(&major, &minor, &revision);

    if (major != LUM_VERSION_MAJOR)
    {
        printf("*** ERROR: Lumina major version mismatch! ***\n");
        return false;
    }

    if (minor != LUM_VERSION_MINOR || revision != LUM_VERSION_REVISION)
        printf("*** WARNING: Lumina version mismatch! ***\n");

    return true;
}

static void print_version(void)
{
    int major, minor, revision;
    lumGetVersion(&major, &minor, &revision);

    printf("Lumina header version: %u.%u.%u\n",
           LUM_VERSION_MAJOR,
           LUM_VERSION_MINOR,
           LUM_VERSION_REVISION);
    printf("Lumina library version: %u.%u.%u\n", major, minor, revision);
    printf("Lumina library version string: \"%s\"\n", lumGetVersionString());
}

static void print_platform(void)
{
    const int platforms[] =
    {
        LUM_PLATFORM_WIN32,
        LUM_PLATFORM_COCOA,
        LUM_PLATFORM_WAYLAND,
        LUM_PLATFORM_X11,
        LUM_PLATFORM_NULL
    };

    printf("Lumina platform: %s\n", get_platform_name(lumGetPlatform()));
    printf("Lumina supported platforms:\n");

    for (size_t i = 0;  i < sizeof(platforms) / sizeof(platforms[0]);  i++)
    {
        if (lumPlatformSupported(platforms[i]))
            printf(" %s\n", get_platform_name(platforms[i]));
    }
}

int main(int argc, char** argv)
{
    int ch;
    bool list_extensions = false, list_layers = false;

    // These duplicate the defaults for each hint
    int platform = LUM_ANY_PLATFORM;
    int client_api = LUM_OPENGL_API;
    int context_major = 1;
    int context_minor = 0;
    int context_release = LUM_ANY_RELEASE_BEHAVIOR;
    int context_creation_api = LUM_NATIVE_CONTEXT_API;
    int context_robustness = LUM_NO_ROBUSTNESS;
    bool context_debug = false;
    bool context_no_error = false;
    bool opengl_forward = false;
    int opengl_profile = LUM_OPENGL_ANY_PROFILE;
    int fb_red_bits = 8;
    int fb_green_bits = 8;
    int fb_blue_bits = 8;
    int fb_alpha_bits = 8;
    int fb_depth_bits = 24;
    int fb_stencil_bits = 8;
    int fb_accum_red_bits = 0;
    int fb_accum_green_bits = 0;
    int fb_accum_blue_bits = 0;
    int fb_accum_alpha_bits = 0;
    int fb_aux_buffers = 0;
    int fb_samples = 0;
    bool fb_stereo = false;
    bool fb_srgb = false;
    bool fb_doublebuffer = true;
    int angle_type = LUM_ANGLE_PLATFORM_TYPE_NONE;
    bool cocoa_graphics_switching = false;
    bool disable_xcb_surface = false;

    enum { PLATFORM, CLIENT, CONTEXT, BEHAVIOR, DEBUG_CONTEXT, FORWARD, HELP,
           EXTENSIONS, LAYERS,
           MAJOR, MINOR, PROFILE, ROBUSTNESS, VERSION,
           REDBITS, GREENBITS, BLUEBITS, ALPHABITS, DEPTHBITS, STENCILBITS,
           ACCUMREDBITS, ACCUMGREENBITS, ACCUMBLUEBITS, ACCUMALPHABITS,
           AUXBUFFERS, SAMPLES, STEREO, SRGB, SINGLEBUFFER, NOERROR_SRSLY,
           ANGLE_TYPE, GRAPHICS_SWITCHING, XCB_SURFACE };
    const struct option options[] =
    {
        { "platform",           1, NULL, PLATFORM },
        { "behavior",           1, NULL, BEHAVIOR },
        { "client-api",         1, NULL, CLIENT },
        { "context-api",        1, NULL, CONTEXT },
        { "debug",              0, NULL, DEBUG_CONTEXT },
        { "forward",            0, NULL, FORWARD },
        { "help",               0, NULL, HELP },
        { "list-extensions",    0, NULL, EXTENSIONS },
        { "list-layers",        0, NULL, LAYERS },
        { "major",              1, NULL, MAJOR },
        { "minor",              1, NULL, MINOR },
        { "profile",            1, NULL, PROFILE },
        { "robustness",         1, NULL, ROBUSTNESS },
        { "version",            0, NULL, VERSION },
        { "red-bits",           1, NULL, REDBITS },
        { "green-bits",         1, NULL, GREENBITS },
        { "blue-bits",          1, NULL, BLUEBITS },
        { "alpha-bits",         1, NULL, ALPHABITS },
        { "depth-bits",         1, NULL, DEPTHBITS },
        { "stencil-bits",       1, NULL, STENCILBITS },
        { "accum-red-bits",     1, NULL, ACCUMREDBITS },
        { "accum-green-bits",   1, NULL, ACCUMGREENBITS },
        { "accum-blue-bits",    1, NULL, ACCUMBLUEBITS },
        { "accum-alpha-bits",   1, NULL, ACCUMALPHABITS },
        { "aux-buffers",        1, NULL, AUXBUFFERS },
        { "samples",            1, NULL, SAMPLES },
        { "stereo",             0, NULL, STEREO },
        { "srgb",               0, NULL, SRGB },
        { "singlebuffer",       0, NULL, SINGLEBUFFER },
        { "no-error",           0, NULL, NOERROR_SRSLY },
        { "angle-type",         1, NULL, ANGLE_TYPE },
        { "graphics-switching", 0, NULL, GRAPHICS_SWITCHING },
        { "vk-xcb-surface",     0, NULL, XCB_SURFACE },
        { NULL, 0, NULL, 0 }
    };

    while ((ch = getopt_long(argc, argv, "a:b:c:dfhlm:n:p:s:v", options, NULL)) != -1)
    {
        switch (ch)
        {
            case PLATFORM:
                if (strcasecmp(optarg, PLATFORM_NAME_ANY) == 0)
                    platform = LUM_ANY_PLATFORM;
                else if (strcasecmp(optarg, PLATFORM_NAME_WIN32) == 0)
                    platform = LUM_PLATFORM_WIN32;
                else if (strcasecmp(optarg, PLATFORM_NAME_COCOA) == 0)
                    platform = LUM_PLATFORM_COCOA;
                else if (strcasecmp(optarg, PLATFORM_NAME_WL) == 0)
                    platform = LUM_PLATFORM_WAYLAND;
                else if (strcasecmp(optarg, PLATFORM_NAME_X11) == 0)
                    platform = LUM_PLATFORM_X11;
                else if (strcasecmp(optarg, PLATFORM_NAME_NULL) == 0)
                    platform = LUM_PLATFORM_NULL;
                else
                {
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;
            case 'a':
            case CLIENT:
                if (strcasecmp(optarg, API_NAME_OPENGL) == 0)
                    client_api = LUM_OPENGL_API;
                else if (strcasecmp(optarg, API_NAME_OPENGL_ES) == 0)
                    client_api = LUM_OPENGL_ES_API;
                else
                {
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;
            case 'b':
            case BEHAVIOR:
                if (strcasecmp(optarg, BEHAVIOR_NAME_NONE) == 0)
                    context_release = LUM_RELEASE_BEHAVIOR_NONE;
                else if (strcasecmp(optarg, BEHAVIOR_NAME_FLUSH) == 0)
                    context_release = LUM_RELEASE_BEHAVIOR_FLUSH;
                else
                {
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;
            case 'c':
            case CONTEXT:
                if (strcasecmp(optarg, API_NAME_NATIVE) == 0)
                    context_creation_api = LUM_NATIVE_CONTEXT_API;
                else if (strcasecmp(optarg, API_NAME_EGL) == 0)
                    context_creation_api = LUM_EGL_CONTEXT_API;
                else if (strcasecmp(optarg, API_NAME_OSMESA) == 0)
                    context_creation_api = LUM_OSMESA_CONTEXT_API;
                else
                {
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;
            case 'd':
            case DEBUG_CONTEXT:
                context_debug = true;
                break;
            case 'f':
            case FORWARD:
                opengl_forward = true;
                break;
            case 'h':
            case HELP:
                usage();
                exit(EXIT_SUCCESS);
            case 'l':
            case EXTENSIONS:
                list_extensions = true;
                break;
            case LAYERS:
                list_layers = true;
                break;
            case 'm':
            case MAJOR:
                context_major = atoi(optarg);
                break;
            case 'n':
            case MINOR:
                context_minor = atoi(optarg);
                break;
            case 'p':
            case PROFILE:
                if (strcasecmp(optarg, PROFILE_NAME_CORE) == 0)
                    opengl_profile = LUM_OPENGL_CORE_PROFILE;
                else if (strcasecmp(optarg, PROFILE_NAME_COMPAT) == 0)
                    opengl_profile = LUM_OPENGL_COMPAT_PROFILE;
                else
                {
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;
            case 's':
            case ROBUSTNESS:
                if (strcasecmp(optarg, STRATEGY_NAME_NONE) == 0)
                    context_robustness = LUM_NO_RESET_NOTIFICATION;
                else if (strcasecmp(optarg, STRATEGY_NAME_LOSE) == 0)
                    context_robustness = LUM_LOSE_CONTEXT_ON_RESET;
                else
                {
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;
            case 'v':
            case VERSION:
                print_version();
                exit(EXIT_SUCCESS);
            case REDBITS:
                if (strcmp(optarg, "-") == 0)
                    fb_red_bits = LUM_DONT_CARE;
                else
                    fb_red_bits = atoi(optarg);
                break;
            case GREENBITS:
                if (strcmp(optarg, "-") == 0)
                    fb_green_bits = LUM_DONT_CARE;
                else
                    fb_green_bits = atoi(optarg);
                break;
            case BLUEBITS:
                if (strcmp(optarg, "-") == 0)
                    fb_blue_bits = LUM_DONT_CARE;
                else
                    fb_blue_bits = atoi(optarg);
                break;
            case ALPHABITS:
                if (strcmp(optarg, "-") == 0)
                    fb_alpha_bits = LUM_DONT_CARE;
                else
                    fb_alpha_bits = atoi(optarg);
                break;
            case DEPTHBITS:
                if (strcmp(optarg, "-") == 0)
                    fb_depth_bits = LUM_DONT_CARE;
                else
                    fb_depth_bits = atoi(optarg);
                break;
            case STENCILBITS:
                if (strcmp(optarg, "-") == 0)
                    fb_stencil_bits = LUM_DONT_CARE;
                else
                    fb_stencil_bits = atoi(optarg);
                break;
            case ACCUMREDBITS:
                if (strcmp(optarg, "-") == 0)
                    fb_accum_red_bits = LUM_DONT_CARE;
                else
                    fb_accum_red_bits = atoi(optarg);
                break;
            case ACCUMGREENBITS:
                if (strcmp(optarg, "-") == 0)
                    fb_accum_green_bits = LUM_DONT_CARE;
                else
                    fb_accum_green_bits = atoi(optarg);
                break;
            case ACCUMBLUEBITS:
                if (strcmp(optarg, "-") == 0)
                    fb_accum_blue_bits = LUM_DONT_CARE;
                else
                    fb_accum_blue_bits = atoi(optarg);
                break;
            case ACCUMALPHABITS:
                if (strcmp(optarg, "-") == 0)
                    fb_accum_alpha_bits = LUM_DONT_CARE;
                else
                    fb_accum_alpha_bits = atoi(optarg);
                break;
            case AUXBUFFERS:
                if (strcmp(optarg, "-") == 0)
                    fb_aux_buffers = LUM_DONT_CARE;
                else
                    fb_aux_buffers = atoi(optarg);
                break;
            case SAMPLES:
                if (strcmp(optarg, "-") == 0)
                    fb_samples = LUM_DONT_CARE;
                else
                    fb_samples = atoi(optarg);
                break;
            case STEREO:
                fb_stereo = true;
                break;
            case SRGB:
                fb_srgb = true;
                break;
            case SINGLEBUFFER:
                fb_doublebuffer = false;
                break;
            case NOERROR_SRSLY:
                context_no_error = true;
                break;
            case ANGLE_TYPE:
                if (strcmp(optarg, ANGLE_TYPE_OPENGL) == 0)
                    angle_type = LUM_ANGLE_PLATFORM_TYPE_OPENGL;
                else if (strcmp(optarg, ANGLE_TYPE_OPENGLES) == 0)
                    angle_type = LUM_ANGLE_PLATFORM_TYPE_OPENGLES;
                else if (strcmp(optarg, ANGLE_TYPE_D3D9) == 0)
                    angle_type = LUM_ANGLE_PLATFORM_TYPE_D3D9;
                else if (strcmp(optarg, ANGLE_TYPE_D3D11) == 0)
                    angle_type = LUM_ANGLE_PLATFORM_TYPE_D3D11;
                else if (strcmp(optarg, ANGLE_TYPE_VULKAN) == 0)
                    angle_type = LUM_ANGLE_PLATFORM_TYPE_VULKAN;
                else if (strcmp(optarg, ANGLE_TYPE_METAL) == 0)
                    angle_type = LUM_ANGLE_PLATFORM_TYPE_METAL;
                else
                {
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;
            case GRAPHICS_SWITCHING:
                cocoa_graphics_switching = true;
                break;
            case XCB_SURFACE:
                disable_xcb_surface = true;
                break;
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    // Initialize Lumina and create window

    if (!valid_version())
        exit(EXIT_FAILURE);

    lumSetErrorCallback(error_callback);

    lumInitHint(LUM_PLATFORM, platform);

    lumInitHint(LUM_COCOA_MENUBAR, false);

    lumInitHint(LUM_ANGLE_PLATFORM_TYPE, angle_type);
    lumInitHint(LUM_X11_XCB_VULKAN_SURFACE, !disable_xcb_surface);

    if (!lumInit())
        exit(EXIT_FAILURE);

    print_version();
    print_platform();

    lumWindowHint(LUM_VISIBLE, false);

    lumWindowHint(LUM_CLIENT_API, client_api);
    lumWindowHint(LUM_CONTEXT_VERSION_MAJOR, context_major);
    lumWindowHint(LUM_CONTEXT_VERSION_MINOR, context_minor);
    lumWindowHint(LUM_CONTEXT_RELEASE_BEHAVIOR, context_release);
    lumWindowHint(LUM_CONTEXT_CREATION_API, context_creation_api);
    lumWindowHint(LUM_CONTEXT_ROBUSTNESS, context_robustness);
    lumWindowHint(LUM_CONTEXT_DEBUG, context_debug);
    lumWindowHint(LUM_CONTEXT_NO_ERROR, context_no_error);
    lumWindowHint(LUM_OPENGL_FORWARD_COMPAT, opengl_forward);
    lumWindowHint(LUM_OPENGL_PROFILE, opengl_profile);

    lumWindowHint(LUM_RED_BITS, fb_red_bits);
    lumWindowHint(LUM_BLUE_BITS, fb_blue_bits);
    lumWindowHint(LUM_GREEN_BITS, fb_green_bits);
    lumWindowHint(LUM_ALPHA_BITS, fb_alpha_bits);
    lumWindowHint(LUM_DEPTH_BITS, fb_depth_bits);
    lumWindowHint(LUM_STENCIL_BITS, fb_stencil_bits);
    lumWindowHint(LUM_ACCUM_RED_BITS, fb_accum_red_bits);
    lumWindowHint(LUM_ACCUM_GREEN_BITS, fb_accum_green_bits);
    lumWindowHint(LUM_ACCUM_BLUE_BITS, fb_accum_blue_bits);
    lumWindowHint(LUM_ACCUM_ALPHA_BITS, fb_accum_alpha_bits);
    lumWindowHint(LUM_AUX_BUFFERS, fb_aux_buffers);
    lumWindowHint(LUM_SAMPLES, fb_samples);
    lumWindowHint(LUM_STEREO, fb_stereo);
    lumWindowHint(LUM_SRGB_CAPABLE, fb_srgb);
    lumWindowHint(LUM_DOUBLEBUFFER, fb_doublebuffer);

    lumWindowHint(LUM_COCOA_GRAPHICS_SWITCHING, cocoa_graphics_switching);

    LUMwindow* window = lumCreateWindow(200, 200, "Version", NULL, NULL);
    if (!window)
    {
        lumTerminate();
        exit(EXIT_FAILURE);
    }

    lumMakeContextCurrent(window);
    gladLoadGL(lumGetProcAddress);

    const GLenum error = glGetError();
    if (error != GL_NO_ERROR)
        printf("*** OpenGL error after make current: 0x%08x ***\n", error);

    // Report client API version

    const int client = lumGetWindowAttrib(window, LUM_CLIENT_API);
    const int major = lumGetWindowAttrib(window, LUM_CONTEXT_VERSION_MAJOR);
    const int minor = lumGetWindowAttrib(window, LUM_CONTEXT_VERSION_MINOR);
    const int revision = lumGetWindowAttrib(window, LUM_CONTEXT_REVISION);
    const int profile = lumGetWindowAttrib(window, LUM_OPENGL_PROFILE);

    printf("%s context version string: \"%s\"\n",
           get_api_name(client),
           glGetString(GL_VERSION));

    printf("%s context version parsed by Lumina: %u.%u.%u\n",
           get_api_name(client),
           major, minor, revision);

    // Report client API context properties

    if (client == LUM_OPENGL_API)
    {
        if (major >= 3)
        {
            GLint flags;

            glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
            printf("%s context flags (0x%08x):", get_api_name(client), flags);

            if (flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
                printf(" forward-compatible");
            if (flags & 2/*GL_CONTEXT_FLAG_DEBUG_BIT*/)
                printf(" debug");
            if (flags & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT_ARB)
                printf(" robustness");
            if (flags & 8/*GL_CONTEXT_FLAG_NO_ERROR_BIT_KHR*/)
                printf(" no-error");
            putchar('\n');

            printf("%s context flags parsed by Lumina:", get_api_name(client));

            if (lumGetWindowAttrib(window, LUM_OPENGL_FORWARD_COMPAT))
                printf(" forward-compatible");
            if (lumGetWindowAttrib(window, LUM_CONTEXT_DEBUG))
                printf(" debug");
            if (lumGetWindowAttrib(window, LUM_CONTEXT_ROBUSTNESS) == LUM_LOSE_CONTEXT_ON_RESET)
                printf(" robustness");
            if (lumGetWindowAttrib(window, LUM_CONTEXT_NO_ERROR))
                printf(" no-error");
            putchar('\n');
        }

        if (major >= 4 || (major == 3 && minor >= 2))
        {
            GLint mask;
            glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &mask);

            printf("%s profile mask (0x%08x): %s\n",
                   get_api_name(client),
                   mask,
                   get_profile_name_gl(mask));

            printf("%s profile mask parsed by Lumina: %s\n",
                   get_api_name(client),
                   get_profile_name_lum(profile));
        }

        if (GLAD_GL_ARB_robustness)
        {
            const int robustness = lumGetWindowAttrib(window, LUM_CONTEXT_ROBUSTNESS);
            GLint strategy;
            glGetIntegerv(GL_RESET_NOTIFICATION_STRATEGY_ARB, &strategy);

            printf("%s robustness strategy (0x%08x): %s\n",
                   get_api_name(client),
                   strategy,
                   get_strategy_name_gl(strategy));

            printf("%s robustness strategy parsed by Lumina: %s\n",
                   get_api_name(client),
                   get_strategy_name_lum(robustness));
        }
    }

    printf("%s context renderer string: \"%s\"\n",
           get_api_name(client),
           glGetString(GL_RENDERER));
    printf("%s context vendor string: \"%s\"\n",
           get_api_name(client),
           glGetString(GL_VENDOR));

    if (major >= 2)
    {
        printf("%s context shading language version: \"%s\"\n",
               get_api_name(client),
               glGetString(GL_SHADING_LANGUAGE_VERSION));
    }

    printf("%s framebuffer:\n", get_api_name(client));

    GLint redbits, greenbits, bluebits, alphabits, depthbits, stencilbits;

    if (client == LUM_OPENGL_API && profile == LUM_OPENGL_CORE_PROFILE)
    {
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                              GL_BACK_LEFT,
                                              GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE,
                                              &redbits);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                              GL_BACK_LEFT,
                                              GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE,
                                              &greenbits);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                              GL_BACK_LEFT,
                                              GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE,
                                              &bluebits);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                              GL_BACK_LEFT,
                                              GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE,
                                              &alphabits);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                              GL_DEPTH,
                                              GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE,
                                              &depthbits);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,
                                              GL_STENCIL,
                                              GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE,
                                              &stencilbits);
    }
    else
    {
        glGetIntegerv(GL_RED_BITS, &redbits);
        glGetIntegerv(GL_GREEN_BITS, &greenbits);
        glGetIntegerv(GL_BLUE_BITS, &bluebits);
        glGetIntegerv(GL_ALPHA_BITS, &alphabits);
        glGetIntegerv(GL_DEPTH_BITS, &depthbits);
        glGetIntegerv(GL_STENCIL_BITS, &stencilbits);
    }

    printf(" red: %u green: %u blue: %u alpha: %u depth: %u stencil: %u\n",
           redbits, greenbits, bluebits, alphabits, depthbits, stencilbits);

    if (client == LUM_OPENGL_ES_API ||
        GLAD_GL_ARB_multisample ||
        major > 1 || minor >= 3)
    {
        GLint samples, samplebuffers;
        glGetIntegerv(GL_SAMPLES, &samples);
        glGetIntegerv(GL_SAMPLE_BUFFERS, &samplebuffers);

        printf(" samples: %u sample buffers: %u\n", samples, samplebuffers);
    }

    if (client == LUM_OPENGL_API && profile != LUM_OPENGL_CORE_PROFILE)
    {
        GLint accumredbits, accumgreenbits, accumbluebits, accumalphabits;
        GLint auxbuffers;

        glGetIntegerv(GL_ACCUM_RED_BITS, &accumredbits);
        glGetIntegerv(GL_ACCUM_GREEN_BITS, &accumgreenbits);
        glGetIntegerv(GL_ACCUM_BLUE_BITS, &accumbluebits);
        glGetIntegerv(GL_ACCUM_ALPHA_BITS, &accumalphabits);
        glGetIntegerv(GL_AUX_BUFFERS, &auxbuffers);

        printf(" accum red: %u accum green: %u accum blue: %u accum alpha: %u aux buffers: %u\n",
               accumredbits, accumgreenbits, accumbluebits, accumalphabits, auxbuffers);
    }

    if (list_extensions)
        list_context_extensions(client, major, minor);

    lumDestroyWindow(window);

    lumWindowHint(LUM_CLIENT_API, LUM_NO_API);

    window = lumCreateWindow(200, 200, "Version", NULL, NULL);
    if (!window)
    {
        lumTerminate();
        exit(EXIT_FAILURE);
    }

    printf("Vulkan loader: %s\n",
           lumVulkanSupported() ? "available" : "missing");

    if (lumVulkanSupported())
    {
        gladLoadVulkanUserPtr(NULL, (GLADuserptrloadfunc) lumGetInstanceProcAddress, NULL);

        uint32_t loader_version = VK_API_VERSION_1_0;

        if (vkEnumerateInstanceVersion)
        {
            uint32_t version;
            if (vkEnumerateInstanceVersion(&version) == VK_SUCCESS)
                loader_version = version;
        }

        printf("Vulkan loader API version: %i.%i\n",
               VK_VERSION_MAJOR(loader_version),
               VK_VERSION_MINOR(loader_version));

        uint32_t re_count;
        const char** re = lumGetRequiredInstanceExtensions(&re_count);

        if (re)
        {
            printf("Vulkan window surface required instance extensions:\n");
            for (uint32_t i = 0;  i < re_count;  i++)
                printf(" %s\n", re[i]);
        }
        else
            printf("Vulkan window surface extensions missing\n");

        if (list_extensions)
            list_vulkan_instance_extensions();

        if (list_layers)
            list_vulkan_instance_layers();

        VkApplicationInfo ai = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
        ai.pApplicationName = "lumina_info";
        ai.applicationVersion = VK_MAKE_VERSION(LUM_VERSION_MAJOR,
                                                LUM_VERSION_MINOR,
                                                LUM_VERSION_REVISION);

        if (loader_version >= VK_API_VERSION_1_1)
            ai.apiVersion = VK_API_VERSION_1_1;
        else
            ai.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo ici = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
        ici.pApplicationInfo = &ai;
        ici.enabledExtensionCount = re_count;
        ici.ppEnabledExtensionNames = re;

        VkInstance instance = VK_NULL_HANDLE;

        if (vkCreateInstance(&ici, NULL, &instance) != VK_SUCCESS)
        {
            lumTerminate();
            exit(EXIT_FAILURE);
        }

        gladLoadVulkanUserPtr(NULL, (GLADuserptrloadfunc) lumGetInstanceProcAddress, instance);

        if (re)
        {
            VkSurfaceKHR surface = VK_NULL_HANDLE;

            if (lumCreateWindowSurface(instance, window, NULL, &surface) == VK_SUCCESS)
            {
                printf("Vulkan window surface created successfully\n");
                vkDestroySurfaceKHR(instance, surface, NULL);
            }
            else
                printf("Failed to create Vulkan window surface\n");
        }

        uint32_t pd_count;
        vkEnumeratePhysicalDevices(instance, &pd_count, NULL);
        VkPhysicalDevice* pd = calloc(pd_count, sizeof(VkPhysicalDevice));
        vkEnumeratePhysicalDevices(instance, &pd_count, pd);

        for (uint32_t i = 0;  i < pd_count;  i++)
        {
            VkPhysicalDeviceProperties pdp;
            vkGetPhysicalDeviceProperties(pd[i], &pdp);

            printf("Vulkan %s device: \"%s\" (API version %i.%i)\n",
                   get_device_type_name(pdp.deviceType),
                   pdp.deviceName,
                   VK_VERSION_MAJOR(pdp.apiVersion),
                   VK_VERSION_MINOR(pdp.apiVersion));

            uint32_t qfp_count;
            vkGetPhysicalDeviceQueueFamilyProperties(pd[i], &qfp_count, NULL);

            if (re)
            {
                printf("Vulkan device queue family presentation support:\n");
                for (uint32_t j = 0;  j < qfp_count;  j++)
                {
                    printf(" %u: ", j);
                    if (lumGetPhysicalDevicePresentationSupport(instance, pd[i], j))
                        printf("supported\n");
                    else
                        printf("no\n");
                }
            }

            if (list_extensions)
                list_vulkan_device_extensions(instance, pd[i]);

            if (list_layers)
                list_vulkan_device_layers(instance, pd[i]);
        }

        free(pd);
        vkDestroyInstance(instance, NULL);
    }

    lumDestroyWindow(window);

    lumTerminate();
    exit(EXIT_SUCCESS);
}

