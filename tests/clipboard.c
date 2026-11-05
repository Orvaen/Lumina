//========================================================================
// Clipboard test program
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
// This program is used to test the clipboard functionality.
//
//========================================================================

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define LUM_INCLUDE_NONE
#include <Lumina/lumina.h>

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

#if defined(__APPLE__)
 #define MODIFIER LUM_MOD_SUPER
#else
 #define MODIFIER LUM_MOD_CONTROL
#endif

static void usage(void)
{
    printf("Usage: clipboard [-h]\n");
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(LUMwindow* window, int key, int scancode, int action, int mods)
{
    if (action != LUM_PRESS)
        return;

    switch (key)
    {
        case LUM_KEY_ESCAPE:
            lumSetWindowShouldClose(window, LUM_TRUE);
            break;

        case LUM_KEY_V:
            if (mods == MODIFIER)
            {
                const char* string;

                string = lumGetClipboardString(NULL);
                if (string)
                    printf("Clipboard contains \"%s\"\n", string);
                else
                    printf("Clipboard does not contain a string\n");
            }
            break;

        case LUM_KEY_C:
            if (mods == MODIFIER)
            {
                const char* string = "Hello Lumina World!";
                lumSetClipboardString(NULL, string);
                printf("Setting clipboard to \"%s\"\n", string);
            }
            break;
    }
}

int main(int argc, char** argv)
{
    int ch;
    LUMwindow* window;

    while ((ch = getopt(argc, argv, "h")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);

            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    lumSetErrorCallback(error_callback);

    if (!lumInit())
    {
        fprintf(stderr, "Failed to initialize Lumina\n");
        exit(EXIT_FAILURE);
    }

    window = lumCreateWindow(200, 200, "Clipboard Test", NULL, NULL);
    if (!window)
    {
        lumTerminate();

        fprintf(stderr, "Failed to open Lumina window\n");
        exit(EXIT_FAILURE);
    }

    lumMakeContextCurrent(window);
    gladLoadGL(lumGetProcAddress);
    lumSwapInterval(1);

    lumSetKeyCallback(window, key_callback);

    glClearColor(0.5f, 0.5f, 0.5f, 0);

    while (!lumWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        lumSwapBuffers(window);
        lumWaitEvents();
    }

    lumTerminate();
    exit(EXIT_SUCCESS);
}

