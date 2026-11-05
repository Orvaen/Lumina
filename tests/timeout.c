//========================================================================
// Event wait timeout test
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
// This test is intended to verify that waiting for events with timeout works
//
//========================================================================

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define LUM_INCLUDE_NONE
#include <Lumina/lumina.h>

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(LUMwindow* window, int key, int scancode, int action, int mods)
{
    if (key == LUM_KEY_ESCAPE && action == LUM_PRESS)
        lumSetWindowShouldClose(window, LUM_TRUE);
}

static float nrand(void)
{
    return (float) rand() / (float) RAND_MAX;
}

int main(void)
{
    LUMwindow* window;

    srand((unsigned int) time(NULL));

    lumSetErrorCallback(error_callback);

    if (!lumInit())
        exit(EXIT_FAILURE);

    window = lumCreateWindow(640, 480, "Event Wait Timeout Test", NULL, NULL);
    if (!window)
    {
        lumTerminate();
        exit(EXIT_FAILURE);
    }

    lumMakeContextCurrent(window);
    gladLoadGL(lumGetProcAddress);
    lumSetKeyCallback(window, key_callback);

    while (!lumWindowShouldClose(window))
    {
        int width, height;
        float r = nrand(), g = nrand(), b = nrand();
        float l = (float) sqrt(r * r + g * g + b * b);

        lumGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);
        glClearColor(r / l, g / l, b / l, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        lumSwapBuffers(window);

        lumWaitEventsTimeout(1.0);
    }

    lumDestroyWindow(window);

    lumTerminate();
    exit(EXIT_SUCCESS);
}

