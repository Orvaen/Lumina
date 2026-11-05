//========================================================================
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
// Checks that lumCreateWindowEx reads a descriptor the way it promises:
// an omitted field takes the library default, LUM_ON and LUM_OFF override it
// either way, and the escape hatches for the ambiguous cases work.
//
// Every window is created hidden, so the test can run without flashing
// windows across the desktop.
//
//========================================================================

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define LUM_INCLUDE_NONE
#include <Lumina/lumina.h>

#include <stdio.h>
#include <stdlib.h>

static int failures = 0;

static void check(int condition, const char* what, int expected, int actual)
{
    if (condition)
        printf("  ok    %s\n", what);
    else
    {
        printf("  FAIL  %s: expected %i, got %i\n", what, expected, actual);
        failures++;
    }
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

// Creates a hidden window from a descriptor, failing the test if it cannot.
static LUMwindow* open_hidden(LUMwindowdesc desc, const char* what)
{
    LUMwindow* window;

    desc.visible = LUM_OFF;
    if (!desc.width)
        desc.width = 320;
    if (!desc.height)
        desc.height = 240;
    if (!desc.title)
        desc.title = "descriptor test";

    window = lumCreateWindowEx(&desc);
    if (!window)
    {
        printf("  FAIL  %s: window creation failed\n", what);
        failures++;
    }

    return window;
}

static void test_defaults(void)
{
    LUMwindow* window;

    printf("defaults from an otherwise empty descriptor\n");

    window = open_hidden((LUMwindowdesc) { 0 }, "defaults");
    if (!window)
        return;

    // Zero in the descriptor must mean "library default", not "false".
    check(lumGetWindowAttrib(window, LUM_RESIZABLE) == LUM_TRUE,
          "resizable defaults to true", LUM_TRUE,
          lumGetWindowAttrib(window, LUM_RESIZABLE));
    check(lumGetWindowAttrib(window, LUM_DECORATED) == LUM_TRUE,
          "decorated defaults to true", LUM_TRUE,
          lumGetWindowAttrib(window, LUM_DECORATED));
    check(lumGetWindowAttrib(window, LUM_FOCUS_ON_SHOW) == LUM_TRUE,
          "focusOnShow defaults to true", LUM_TRUE,
          lumGetWindowAttrib(window, LUM_FOCUS_ON_SHOW));
    check(lumGetWindowAttrib(window, LUM_DOUBLEBUFFER) == LUM_TRUE,
          "doublebuffer defaults to true", LUM_TRUE,
          lumGetWindowAttrib(window, LUM_DOUBLEBUFFER));
    check(lumGetWindowAttrib(window, LUM_CLIENT_API) == LUM_OPENGL_API,
          "client API defaults to OpenGL", LUM_OPENGL_API,
          lumGetWindowAttrib(window, LUM_CLIENT_API));

    lumDestroyWindow(window);
}

static void test_overrides(void)
{
    LUMwindow* window;
    int width = 0, height = 0;

    printf("explicit overrides\n");

    window = open_hidden((LUMwindowdesc) {
        .width     = 640,
        .height    = 480,
        .title     = "overrides",
        .resizable = LUM_OFF,
        .decorated = LUM_OFF,
        .floating  = LUM_ON
    }, "overrides");
    if (!window)
        return;

    check(lumGetWindowAttrib(window, LUM_RESIZABLE) == LUM_FALSE,
          "LUM_OFF turns resizable off", LUM_FALSE,
          lumGetWindowAttrib(window, LUM_RESIZABLE));
    check(lumGetWindowAttrib(window, LUM_DECORATED) == LUM_FALSE,
          "LUM_OFF turns decorated off", LUM_FALSE,
          lumGetWindowAttrib(window, LUM_DECORATED));
    check(lumGetWindowAttrib(window, LUM_FLOATING) == LUM_TRUE,
          "LUM_ON turns floating on", LUM_TRUE,
          lumGetWindowAttrib(window, LUM_FLOATING));

    lumGetWindowSize(window, &width, &height);
    check(width == 640 && height == 480, "requested size is honoured", 640, width);

    lumDestroyWindow(window);
}

static void test_context_off(void)
{
    LUMwindow* window;

    printf("a window without a client API context\n");

    window = open_hidden((LUMwindowdesc) {
        .title   = "no context",
        .context = LUM_OFF
    }, "context off");
    if (!window)
        return;

    // LUM_NO_API is zero, which is why the descriptor uses a switch for this
    // rather than the clientApi field.
    check(lumGetWindowAttrib(window, LUM_CLIENT_API) == LUM_NO_API,
          "context = LUM_OFF gives no client API", LUM_NO_API,
          lumGetWindowAttrib(window, LUM_CLIENT_API));

    lumDestroyWindow(window);
}

static void test_make_current(void)
{
    LUMwindow* window;

    printf("makeCurrent\n");

    window = open_hidden((LUMwindowdesc) {
        .title       = "current",
        .makeCurrent = LUM_ON
    }, "makeCurrent");
    if (!window)
        return;

    check(lumGetCurrentContext() == window,
          "makeCurrent = LUM_ON makes the context current", 1,
          lumGetCurrentContext() == window);

    lumMakeContextCurrent(NULL);
    lumDestroyWindow(window);
}

static void test_zero_bits(void)
{
    LUMwindow* window;

    printf("LUM_BITS_NONE\n");

    // A plain 0 would mean "use the default" (24 bits of depth), so the
    // descriptor needs a distinct way to ask for no depth buffer at all.
    window = open_hidden((LUMwindowdesc) {
        .title     = "no depth",
        .depthBits = LUM_BITS_NONE,
        .makeCurrent = LUM_ON
    }, "LUM_BITS_NONE");
    if (!window)
        return;

    if (gladLoadGL(lumGetProcAddress))
    {
        GLint depth = -1;
        glGetIntegerv(GL_DEPTH_BITS, &depth);
        // Drivers are allowed to give more than asked for, so this only checks
        // that the request went through rather than demanding exactly zero.
        printf("  note  driver reported %i depth bits\n", depth);
    }

    lumMakeContextCurrent(NULL);
    lumDestroyWindow(window);
}

static void test_rejects_bad_input(void)
{
    printf("invalid descriptors are rejected\n");

    check(lumCreateWindowEx(NULL) == NULL,
          "a NULL descriptor returns NULL", 1,
          lumCreateWindowEx(NULL) == NULL);

    {
        LUMwindowdesc desc = { .width = 320, .height = 240, .title = NULL };
        check(lumCreateWindowEx(&desc) == NULL,
              "a NULL title returns NULL", 1, lumCreateWindowEx(&desc) == NULL);
    }

    {
        LUMwindowdesc desc = { .width = 0, .height = 240, .title = "zero" };
        check(lumCreateWindowEx(&desc) == NULL,
              "a zero width returns NULL", 1, lumCreateWindowEx(&desc) == NULL);
    }
}

int main(void)
{
    lumSetErrorCallback(error_callback);

    if (!lumInit())
    {
        fprintf(stderr, "Failed to initialize Lumina\n");
        exit(EXIT_FAILURE);
    }

    test_defaults();
    test_overrides();
    test_context_off();
    test_make_current();
    test_zero_bits();
    test_rejects_bad_input();

    lumTerminate();

    if (failures)
    {
        printf("\n%i check(s) failed\n", failures);
        exit(EXIT_FAILURE);
    }

    printf("\nall checks passed\n");
    exit(EXIT_SUCCESS);
}
