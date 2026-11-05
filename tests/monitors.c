//========================================================================
// Monitor information tool
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
// This test prints monitor and video mode information or verifies video
// modes
//
//========================================================================

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define LUM_INCLUDE_NONE
#include <Lumina/lumina.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "getopt.h"

enum Mode
{
    LIST_MODE,
    TEST_MODE
};

static void usage(void)
{
    printf("Usage: monitors [-t]\n");
    printf("       monitors -h\n");
}

static int euclid(int a, int b)
{
    return b ? euclid(b, a % b) : a;
}

static const char* format_mode(const LUMvidmode* mode)
{
    static char buffer[512];
    const int gcd = euclid(mode->width, mode->height);

    snprintf(buffer,
             sizeof(buffer),
             "%i x %i x %i (%i:%i) (%i %i %i) %i Hz",
             mode->width, mode->height,
             mode->redBits + mode->greenBits + mode->blueBits,
             mode->width / gcd, mode->height / gcd,
             mode->redBits, mode->greenBits, mode->blueBits,
             mode->refreshRate);

    buffer[sizeof(buffer) - 1] = '\0';
    return buffer;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void framebuffer_size_callback(LUMwindow* window, int width, int height)
{
    printf("Framebuffer resized to %ix%i\n", width, height);

    glViewport(0, 0, width, height);
}

static void key_callback(LUMwindow* window, int key, int scancode, int action, int mods)
{
    if (key == LUM_KEY_ESCAPE)
        lumSetWindowShouldClose(window, LUM_TRUE);
}

static void list_modes(LUMmonitor* monitor)
{
    int count, x, y, width_mm, height_mm, i;
    int workarea_x, workarea_y, workarea_width, workarea_height;
    float xscale, yscale;

    const LUMvidmode* mode = lumGetVideoMode(monitor);
    const LUMvidmode* modes = lumGetVideoModes(monitor, &count);

    lumGetMonitorPos(monitor, &x, &y);
    lumGetMonitorPhysicalSize(monitor, &width_mm, &height_mm);
    lumGetMonitorContentScale(monitor, &xscale, &yscale);
    lumGetMonitorWorkarea(monitor, &workarea_x, &workarea_y, &workarea_width, &workarea_height);

    printf("Name: %s (%s)\n",
           lumGetMonitorName(monitor),
           lumGetPrimaryMonitor() == monitor ? "primary" : "secondary");
    printf("Current mode: %s\n", format_mode(mode));
    printf("Virtual position: %i, %i\n", x, y);
    printf("Content scale: %f x %f\n", xscale, yscale);

    printf("Physical size: %i x %i mm (%0.2f dpi at %i x %i)\n",
           width_mm, height_mm, mode->width * 25.4f / width_mm, mode->width, mode->height);
    printf("Monitor work area: %i x %i starting at %i, %i\n",
            workarea_width, workarea_height, workarea_x, workarea_y);

    printf("Modes:\n");

    for (i = 0;  i < count;  i++)
    {
        printf("%3u: %s", (unsigned int) i, format_mode(modes + i));

        if (memcmp(mode, modes + i, sizeof(LUMvidmode)) == 0)
            printf(" (current mode)");

        putchar('\n');
    }
}

static void test_modes(LUMmonitor* monitor)
{
    int i, count;
    LUMwindow* window;
    const LUMvidmode* modes = lumGetVideoModes(monitor, &count);

    for (i = 0;  i < count;  i++)
    {
        const LUMvidmode* mode = modes + i;
        LUMvidmode current;

        lumWindowHint(LUM_RED_BITS, mode->redBits);
        lumWindowHint(LUM_GREEN_BITS, mode->greenBits);
        lumWindowHint(LUM_BLUE_BITS, mode->blueBits);
        lumWindowHint(LUM_REFRESH_RATE, mode->refreshRate);

        printf("Testing mode %u on monitor %s: %s\n",
               (unsigned int) i,
               lumGetMonitorName(monitor),
               format_mode(mode));

        window = lumCreateWindow(mode->width, mode->height,
                                  "Video Mode Test",
                                  lumGetPrimaryMonitor(),
                                  NULL);
        if (!window)
        {
            printf("Failed to enter mode %u: %s\n",
                   (unsigned int) i,
                   format_mode(mode));
            continue;
        }

        lumSetFramebufferSizeCallback(window, framebuffer_size_callback);
        lumSetKeyCallback(window, key_callback);

        lumMakeContextCurrent(window);
        gladLoadGL(lumGetProcAddress);
        lumSwapInterval(1);

        lumSetTime(0.0);

        while (lumGetTime() < 5.0)
        {
            glClear(GL_COLOR_BUFFER_BIT);
            lumSwapBuffers(window);
            lumPollEvents();

            if (lumWindowShouldClose(window))
            {
                printf("User terminated program\n");

                lumTerminate();
                exit(EXIT_SUCCESS);
            }
        }

        glGetIntegerv(GL_RED_BITS, &current.redBits);
        glGetIntegerv(GL_GREEN_BITS, &current.greenBits);
        glGetIntegerv(GL_BLUE_BITS, &current.blueBits);

        lumGetWindowSize(window, &current.width, &current.height);

        if (current.redBits != mode->redBits ||
            current.greenBits != mode->greenBits ||
            current.blueBits != mode->blueBits)
        {
            printf("*** Color bit mismatch: (%i %i %i) instead of (%i %i %i)\n",
                   current.redBits, current.greenBits, current.blueBits,
                   mode->redBits, mode->greenBits, mode->blueBits);
        }

        if (current.width != mode->width || current.height != mode->height)
        {
            printf("*** Size mismatch: %ix%i instead of %ix%i\n",
                   current.width, current.height,
                   mode->width, mode->height);
        }

        printf("Closing window\n");

        lumDestroyWindow(window);
        window = NULL;

        lumPollEvents();
    }
}

int main(int argc, char** argv)
{
    int ch, i, count, mode = LIST_MODE;
    LUMmonitor** monitors;

    while ((ch = getopt(argc, argv, "th")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            case 't':
                mode = TEST_MODE;
                break;
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    lumSetErrorCallback(error_callback);

    lumInitHint(LUM_COCOA_MENUBAR, LUM_FALSE);

    if (!lumInit())
        exit(EXIT_FAILURE);

    monitors = lumGetMonitors(&count);

    for (i = 0;  i < count;  i++)
    {
        if (mode == LIST_MODE)
            list_modes(monitors[i]);
        else if (mode == TEST_MODE)
            test_modes(monitors[i]);
    }

    lumTerminate();
    exit(EXIT_SUCCESS);
}

