//========================================================================
// The smallest useful Lumina program
// Lumina - fork of GLFW 3.4 (www.glfw.org)
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
//
// This is the program the "Open a window" guide walks through.  The guide
// pulls the code straight out of this file, so what is documented is what is
// compiled.
//
//========================================================================

//! [code]
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define LUM_INCLUDE_NONE
#include <Lumina/lumina.h>

#include <stdio.h>
#include <stdlib.h>

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Lumina error: %s\n", description);
}

static void key_callback(LUMwindow* window,
                         int key, int scancode, int action, int mods)
{
    if (key == LUM_KEY_ESCAPE && action == LUM_PRESS)
        lumSetWindowShouldClose(window, LUM_TRUE);
}

int main(void)
{
    LUMwindow* window;

    lumSetErrorCallback(error_callback);

    if (!lumInit())
        return EXIT_FAILURE;

    window = lumCreateWindowEx(&(LUMwindowdesc)
    {
        .width       = 640,
        .height      = 480,
        .title       = "My first Lumina window",
        .makeCurrent = LUM_ON
    });

    if (!window)
    {
        lumTerminate();
        return EXIT_FAILURE;
    }

    lumSetKeyCallback(window, key_callback);

    gladLoadGL(lumGetProcAddress);
    lumSwapInterval(1);

    while (!lumWindowShouldClose(window))
    {
        int width, height;
        lumGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClearColor(0.18f, 0.12f, 0.30f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        lumSwapBuffers(window);
        lumPollEvents();
    }

    lumDestroyWindow(window);
    lumTerminate();
    return EXIT_SUCCESS;
}
//! [code]
