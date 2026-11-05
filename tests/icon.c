//========================================================================
// Window icon test program
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
// This program is used to test the icon feature.
//
//========================================================================

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define LUM_INCLUDE_NONE
#include <Lumina/lumina.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// a simple lumina logo
const char* const logo[] =
{
    "................",
    "................",
    "...0000..0......",
    "...0.....0......",
    "...0.00..0......",
    "...0..0..0......",
    "...0000..0000...",
    "................",
    "................",
    "...000..0...0...",
    "...0....0...0...",
    "...000..0.0.0...",
    "...0....0.0.0...",
    "...0....00000...",
    "................",
    "................"
};

const unsigned char icon_colors[5][4] =
{
    {   0,   0,   0, 255 }, // black
    { 255,   0,   0, 255 }, // red
    {   0, 255,   0, 255 }, // green
    {   0,   0, 255, 255 }, // blue
    { 255, 255, 255, 255 }  // white
};

static int cur_icon_color = 0;

static void set_icon(LUMwindow* window, int icon_color)
{
    int x, y;
    unsigned char pixels[16 * 16 * 4];
    unsigned char* target = pixels;
    LUMimage img = { 16, 16, pixels };

    for (y = 0;  y < img.width;  y++)
    {
        for (x = 0;  x < img.height;  x++)
        {
            if (logo[y][x] == '0')
                memcpy(target, icon_colors[icon_color], 4);
            else
                memset(target, 0, 4);

            target += 4;
        }
    }

    lumSetWindowIcon(window, 1, &img);
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
        case LUM_KEY_SPACE:
            cur_icon_color = (cur_icon_color + 1) % 5;
            set_icon(window, cur_icon_color);
            break;
        case LUM_KEY_X:
            lumSetWindowIcon(window, 0, NULL);
            break;
    }
}

int main(int argc, char** argv)
{
    LUMwindow* window;

    if (!lumInit())
    {
        fprintf(stderr, "Failed to initialize Lumina\n");
        exit(EXIT_FAILURE);
    }

    window = lumCreateWindow(200, 200, "Window Icon", NULL, NULL);
    if (!window)
    {
        lumTerminate();

        fprintf(stderr, "Failed to open Lumina window\n");
        exit(EXIT_FAILURE);
    }

    lumMakeContextCurrent(window);
    gladLoadGL(lumGetProcAddress);

    lumSetKeyCallback(window, key_callback);
    set_icon(window, cur_icon_color);

    while (!lumWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        lumSwapBuffers(window);
        lumWaitEvents();
    }

    lumDestroyWindow(window);
    lumTerminate();
    exit(EXIT_SUCCESS);
}

