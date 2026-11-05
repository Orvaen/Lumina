//========================================================================
// Iconify/restore test program
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
//
// This program is used to test the iconify/restore functionality for
// both full screen and windowed mode windows
//
//========================================================================

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define LUM_INCLUDE_NONE
#include <Lumina/lumina.h>

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

static int windowed_xpos, windowed_ypos, windowed_width = 640, windowed_height = 480;

static void usage(void)
{
    printf("Usage: iconify [-h] [-f [-a] [-n]]\n");
    printf("Options:\n");
    printf("  -a create windows for all monitors\n");
    printf("  -f create full screen window(s)\n");
    printf("  -h show this help\n");
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(LUMwindow* window, int key, int scancode, int action, int mods)
{
    printf("%0.2f Key %s\n",
           lumGetTime(),
           action == LUM_PRESS ? "pressed" : "released");

    if (action != LUM_PRESS)
        return;

    switch (key)
    {
        case LUM_KEY_I:
            lumIconifyWindow(window);
            break;
        case LUM_KEY_M:
            lumMaximizeWindow(window);
            break;
        case LUM_KEY_R:
            lumRestoreWindow(window);
            break;
        case LUM_KEY_ESCAPE:
            lumSetWindowShouldClose(window, LUM_TRUE);
            break;
        case LUM_KEY_A:
            lumSetWindowAttrib(window, LUM_AUTO_ICONIFY, !lumGetWindowAttrib(window, LUM_AUTO_ICONIFY));
            break;
        case LUM_KEY_B:
            lumSetWindowAttrib(window, LUM_RESIZABLE, !lumGetWindowAttrib(window, LUM_RESIZABLE));
            break;
        case LUM_KEY_D:
            lumSetWindowAttrib(window, LUM_DECORATED, !lumGetWindowAttrib(window, LUM_DECORATED));
            break;
        case LUM_KEY_F:
            lumSetWindowAttrib(window, LUM_FLOATING, !lumGetWindowAttrib(window, LUM_FLOATING));
            break;
        case LUM_KEY_F11:
        case LUM_KEY_ENTER:
        {
            if (mods != LUM_MOD_ALT)
                return;

            if (lumGetWindowMonitor(window))
            {
                lumSetWindowMonitor(window, NULL,
                                     windowed_xpos, windowed_ypos,
                                     windowed_width, windowed_height,
                                     0);
            }
            else
            {
                LUMmonitor* monitor = lumGetPrimaryMonitor();
                if (monitor)
                {
                    const LUMvidmode* mode = lumGetVideoMode(monitor);
                    lumGetWindowPos(window, &windowed_xpos, &windowed_ypos);
                    lumGetWindowSize(window, &windowed_width, &windowed_height);
                    lumSetWindowMonitor(window, monitor,
                                         0, 0, mode->width, mode->height,
                                         mode->refreshRate);
                }
            }

            break;
        }
    }
}

static void window_size_callback(LUMwindow* window, int width, int height)
{
    printf("%0.2f Window resized to %ix%i\n", lumGetTime(), width, height);
}

static void framebuffer_size_callback(LUMwindow* window, int width, int height)
{
    printf("%0.2f Framebuffer resized to %ix%i\n", lumGetTime(), width, height);
}

static void window_focus_callback(LUMwindow* window, int focused)
{
    printf("%0.2f Window %s\n",
           lumGetTime(),
           focused ? "focused" : "defocused");
}

static void window_iconify_callback(LUMwindow* window, int iconified)
{
    printf("%0.2f Window %s\n",
           lumGetTime(),
           iconified ? "iconified" : "uniconified");
}

static void window_maximize_callback(LUMwindow* window, int maximized)
{
    printf("%0.2f Window %s\n",
           lumGetTime(),
           maximized ? "maximized" : "unmaximized");
}

static void window_refresh_callback(LUMwindow* window)
{
    printf("%0.2f Window refresh\n", lumGetTime());

    lumMakeContextCurrent(window);

    glClear(GL_COLOR_BUFFER_BIT);
    lumSwapBuffers(window);
}

static LUMwindow* create_window(LUMmonitor* monitor)
{
    int width, height;
    LUMwindow* window;

    if (monitor)
    {
        const LUMvidmode* mode = lumGetVideoMode(monitor);

        lumWindowHint(LUM_REFRESH_RATE, mode->refreshRate);
        lumWindowHint(LUM_RED_BITS, mode->redBits);
        lumWindowHint(LUM_GREEN_BITS, mode->greenBits);
        lumWindowHint(LUM_BLUE_BITS, mode->blueBits);

        width = mode->width;
        height = mode->height;
    }
    else
    {
        width = windowed_width;
        height = windowed_height;
    }

    window = lumCreateWindow(width, height, "Iconify", monitor, NULL);
    if (!window)
    {
        lumTerminate();
        exit(EXIT_FAILURE);
    }

    lumMakeContextCurrent(window);
    gladLoadGL(lumGetProcAddress);

    return window;
}

int main(int argc, char** argv)
{
    int ch, i, window_count;
    int fullscreen = LUM_FALSE, all_monitors = LUM_FALSE;
    LUMwindow** windows;

    while ((ch = getopt(argc, argv, "afhn")) != -1)
    {
        switch (ch)
        {
            case 'a':
                all_monitors = LUM_TRUE;
                break;

            case 'h':
                usage();
                exit(EXIT_SUCCESS);

            case 'f':
                fullscreen = LUM_TRUE;
                break;

            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    lumSetErrorCallback(error_callback);

    if (!lumInit())
        exit(EXIT_FAILURE);

    if (fullscreen && all_monitors)
    {
        int monitor_count;
        LUMmonitor** monitors = lumGetMonitors(&monitor_count);

        window_count = monitor_count;
        windows = calloc(window_count, sizeof(LUMwindow*));

        for (i = 0;  i < monitor_count;  i++)
        {
            windows[i] = create_window(monitors[i]);
            if (!windows[i])
                break;
        }
    }
    else
    {
        LUMmonitor* monitor = NULL;

        if (fullscreen)
            monitor = lumGetPrimaryMonitor();

        window_count = 1;
        windows = calloc(window_count, sizeof(LUMwindow*));
        windows[0] = create_window(monitor);
    }

    for (i = 0;  i < window_count;  i++)
    {
        lumSetKeyCallback(windows[i], key_callback);
        lumSetFramebufferSizeCallback(windows[i], framebuffer_size_callback);
        lumSetWindowSizeCallback(windows[i], window_size_callback);
        lumSetWindowFocusCallback(windows[i], window_focus_callback);
        lumSetWindowIconifyCallback(windows[i], window_iconify_callback);
        lumSetWindowMaximizeCallback(windows[i], window_maximize_callback);
        lumSetWindowRefreshCallback(windows[i], window_refresh_callback);

        window_refresh_callback(windows[i]);

        printf("Window is %s and %s\n",
            lumGetWindowAttrib(windows[i], LUM_ICONIFIED) ? "iconified" : "restored",
            lumGetWindowAttrib(windows[i], LUM_FOCUSED) ? "focused" : "defocused");
    }

    for (;;)
    {
        lumWaitEvents();

        for (i = 0;  i < window_count;  i++)
        {
            if (lumWindowShouldClose(windows[i]))
                break;
        }

        if (i < window_count)
            break;

        // Workaround for an issue with msvcrt and mintty
        fflush(stdout);
    }

    lumTerminate();
    exit(EXIT_SUCCESS);
}

