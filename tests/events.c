//========================================================================
// Event linter (event spewer)
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
// This test hooks every available callback and outputs their arguments
//
// Log messages go to stdout, error messages to stderr
//
// Every event also gets a (sequential) number to aid discussion of logs
//
//========================================================================

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define LUM_INCLUDE_NONE
#include <Lumina/lumina.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <locale.h>

#include "getopt.h"

// Event index
static unsigned int counter = 0;

typedef struct
{
    LUMwindow* window;
    int number;
    int closeable;
} Slot;

static void usage(void)
{
    printf("Usage: events [-f] [-h] [-n WINDOWS]\n");
    printf("Options:\n");
    printf("  -f use full screen\n");
    printf("  -h show this help\n");
    printf("  -n the number of windows to create\n");
}

static const char* get_key_name(int key)
{
    switch (key)
    {
        // Printable keys
        case LUM_KEY_A:            return "A";
        case LUM_KEY_B:            return "B";
        case LUM_KEY_C:            return "C";
        case LUM_KEY_D:            return "D";
        case LUM_KEY_E:            return "E";
        case LUM_KEY_F:            return "F";
        case LUM_KEY_G:            return "G";
        case LUM_KEY_H:            return "H";
        case LUM_KEY_I:            return "I";
        case LUM_KEY_J:            return "J";
        case LUM_KEY_K:            return "K";
        case LUM_KEY_L:            return "L";
        case LUM_KEY_M:            return "M";
        case LUM_KEY_N:            return "N";
        case LUM_KEY_O:            return "O";
        case LUM_KEY_P:            return "P";
        case LUM_KEY_Q:            return "Q";
        case LUM_KEY_R:            return "R";
        case LUM_KEY_S:            return "S";
        case LUM_KEY_T:            return "T";
        case LUM_KEY_U:            return "U";
        case LUM_KEY_V:            return "V";
        case LUM_KEY_W:            return "W";
        case LUM_KEY_X:            return "X";
        case LUM_KEY_Y:            return "Y";
        case LUM_KEY_Z:            return "Z";
        case LUM_KEY_1:            return "1";
        case LUM_KEY_2:            return "2";
        case LUM_KEY_3:            return "3";
        case LUM_KEY_4:            return "4";
        case LUM_KEY_5:            return "5";
        case LUM_KEY_6:            return "6";
        case LUM_KEY_7:            return "7";
        case LUM_KEY_8:            return "8";
        case LUM_KEY_9:            return "9";
        case LUM_KEY_0:            return "0";
        case LUM_KEY_SPACE:        return "SPACE";
        case LUM_KEY_MINUS:        return "MINUS";
        case LUM_KEY_EQUAL:        return "EQUAL";
        case LUM_KEY_LEFT_BRACKET: return "LEFT BRACKET";
        case LUM_KEY_RIGHT_BRACKET: return "RIGHT BRACKET";
        case LUM_KEY_BACKSLASH:    return "BACKSLASH";
        case LUM_KEY_SEMICOLON:    return "SEMICOLON";
        case LUM_KEY_APOSTROPHE:   return "APOSTROPHE";
        case LUM_KEY_GRAVE_ACCENT: return "GRAVE ACCENT";
        case LUM_KEY_COMMA:        return "COMMA";
        case LUM_KEY_PERIOD:       return "PERIOD";
        case LUM_KEY_SLASH:        return "SLASH";
        case LUM_KEY_WORLD_1:      return "WORLD 1";
        case LUM_KEY_WORLD_2:      return "WORLD 2";

        // Function keys
        case LUM_KEY_ESCAPE:       return "ESCAPE";
        case LUM_KEY_F1:           return "F1";
        case LUM_KEY_F2:           return "F2";
        case LUM_KEY_F3:           return "F3";
        case LUM_KEY_F4:           return "F4";
        case LUM_KEY_F5:           return "F5";
        case LUM_KEY_F6:           return "F6";
        case LUM_KEY_F7:           return "F7";
        case LUM_KEY_F8:           return "F8";
        case LUM_KEY_F9:           return "F9";
        case LUM_KEY_F10:          return "F10";
        case LUM_KEY_F11:          return "F11";
        case LUM_KEY_F12:          return "F12";
        case LUM_KEY_F13:          return "F13";
        case LUM_KEY_F14:          return "F14";
        case LUM_KEY_F15:          return "F15";
        case LUM_KEY_F16:          return "F16";
        case LUM_KEY_F17:          return "F17";
        case LUM_KEY_F18:          return "F18";
        case LUM_KEY_F19:          return "F19";
        case LUM_KEY_F20:          return "F20";
        case LUM_KEY_F21:          return "F21";
        case LUM_KEY_F22:          return "F22";
        case LUM_KEY_F23:          return "F23";
        case LUM_KEY_F24:          return "F24";
        case LUM_KEY_F25:          return "F25";
        case LUM_KEY_UP:           return "UP";
        case LUM_KEY_DOWN:         return "DOWN";
        case LUM_KEY_LEFT:         return "LEFT";
        case LUM_KEY_RIGHT:        return "RIGHT";
        case LUM_KEY_LEFT_SHIFT:   return "LEFT SHIFT";
        case LUM_KEY_RIGHT_SHIFT:  return "RIGHT SHIFT";
        case LUM_KEY_LEFT_CONTROL: return "LEFT CONTROL";
        case LUM_KEY_RIGHT_CONTROL: return "RIGHT CONTROL";
        case LUM_KEY_LEFT_ALT:     return "LEFT ALT";
        case LUM_KEY_RIGHT_ALT:    return "RIGHT ALT";
        case LUM_KEY_TAB:          return "TAB";
        case LUM_KEY_ENTER:        return "ENTER";
        case LUM_KEY_BACKSPACE:    return "BACKSPACE";
        case LUM_KEY_INSERT:       return "INSERT";
        case LUM_KEY_DELETE:       return "DELETE";
        case LUM_KEY_PAGE_UP:      return "PAGE UP";
        case LUM_KEY_PAGE_DOWN:    return "PAGE DOWN";
        case LUM_KEY_HOME:         return "HOME";
        case LUM_KEY_END:          return "END";
        case LUM_KEY_KP_0:         return "KEYPAD 0";
        case LUM_KEY_KP_1:         return "KEYPAD 1";
        case LUM_KEY_KP_2:         return "KEYPAD 2";
        case LUM_KEY_KP_3:         return "KEYPAD 3";
        case LUM_KEY_KP_4:         return "KEYPAD 4";
        case LUM_KEY_KP_5:         return "KEYPAD 5";
        case LUM_KEY_KP_6:         return "KEYPAD 6";
        case LUM_KEY_KP_7:         return "KEYPAD 7";
        case LUM_KEY_KP_8:         return "KEYPAD 8";
        case LUM_KEY_KP_9:         return "KEYPAD 9";
        case LUM_KEY_KP_DIVIDE:    return "KEYPAD DIVIDE";
        case LUM_KEY_KP_MULTIPLY:  return "KEYPAD MULTIPLY";
        case LUM_KEY_KP_SUBTRACT:  return "KEYPAD SUBTRACT";
        case LUM_KEY_KP_ADD:       return "KEYPAD ADD";
        case LUM_KEY_KP_DECIMAL:   return "KEYPAD DECIMAL";
        case LUM_KEY_KP_EQUAL:     return "KEYPAD EQUAL";
        case LUM_KEY_KP_ENTER:     return "KEYPAD ENTER";
        case LUM_KEY_PRINT_SCREEN: return "PRINT SCREEN";
        case LUM_KEY_NUM_LOCK:     return "NUM LOCK";
        case LUM_KEY_CAPS_LOCK:    return "CAPS LOCK";
        case LUM_KEY_SCROLL_LOCK:  return "SCROLL LOCK";
        case LUM_KEY_PAUSE:        return "PAUSE";
        case LUM_KEY_LEFT_SUPER:   return "LEFT SUPER";
        case LUM_KEY_RIGHT_SUPER:  return "RIGHT SUPER";
        case LUM_KEY_MENU:         return "MENU";

        default:                    return "UNKNOWN";
    }
}

static const char* get_action_name(int action)
{
    switch (action)
    {
        case LUM_PRESS:
            return "pressed";
        case LUM_RELEASE:
            return "released";
        case LUM_REPEAT:
            return "repeated";
    }

    return "caused unknown action";
}

static const char* get_button_name(int button)
{
    switch (button)
    {
        case LUM_MOUSE_BUTTON_LEFT:
            return "left";
        case LUM_MOUSE_BUTTON_RIGHT:
            return "right";
        case LUM_MOUSE_BUTTON_MIDDLE:
            return "middle";
        default:
        {
            static char name[16];
            snprintf(name, sizeof(name), "%i", button);
            return name;
        }
    }
}

static const char* get_mods_name(int mods)
{
    static char name[512];

    if (mods == 0)
        return " no mods";

    name[0] = '\0';

    if (mods & LUM_MOD_SHIFT)
        strcat(name, " shift");
    if (mods & LUM_MOD_CONTROL)
        strcat(name, " control");
    if (mods & LUM_MOD_ALT)
        strcat(name, " alt");
    if (mods & LUM_MOD_SUPER)
        strcat(name, " super");
    if (mods & LUM_MOD_CAPS_LOCK)
        strcat(name, " capslock-on");
    if (mods & LUM_MOD_NUM_LOCK)
        strcat(name, " numlock-on");

    return name;
}

static size_t encode_utf8(char* s, unsigned int ch)
{
    size_t count = 0;

    if (ch < 0x80)
        s[count++] = (char) ch;
    else if (ch < 0x800)
    {
        s[count++] = (ch >> 6) | 0xc0;
        s[count++] = (ch & 0x3f) | 0x80;
    }
    else if (ch < 0x10000)
    {
        s[count++] = (ch >> 12) | 0xe0;
        s[count++] = ((ch >> 6) & 0x3f) | 0x80;
        s[count++] = (ch & 0x3f) | 0x80;
    }
    else if (ch < 0x110000)
    {
        s[count++] = (ch >> 18) | 0xf0;
        s[count++] = ((ch >> 12) & 0x3f) | 0x80;
        s[count++] = ((ch >> 6) & 0x3f) | 0x80;
        s[count++] = (ch & 0x3f) | 0x80;
    }

    return count;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void window_pos_callback(LUMwindow* window, int x, int y)
{
    Slot* slot = lumGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window position: %i %i\n",
           counter++, slot->number, lumGetTime(), x, y);
}

static void window_size_callback(LUMwindow* window, int width, int height)
{
    Slot* slot = lumGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window size: %i %i\n",
           counter++, slot->number, lumGetTime(), width, height);
}

static void framebuffer_size_callback(LUMwindow* window, int width, int height)
{
    Slot* slot = lumGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Framebuffer size: %i %i\n",
           counter++, slot->number, lumGetTime(), width, height);
}

static void window_content_scale_callback(LUMwindow* window, float xscale, float yscale)
{
    Slot* slot = lumGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window content scale: %0.3f %0.3f\n",
           counter++, slot->number, lumGetTime(), xscale, yscale);
}

static void window_close_callback(LUMwindow* window)
{
    Slot* slot = lumGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window close\n",
           counter++, slot->number, lumGetTime());

    if (!slot->closeable)
    {
        printf("(( closing is disabled, press %s to re-enable )\n",
               lumGetKeyName(LUM_KEY_C, 0));
    }

    lumSetWindowShouldClose(window, slot->closeable);
}

static void window_refresh_callback(LUMwindow* window)
{
    Slot* slot = lumGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window refresh\n",
           counter++, slot->number, lumGetTime());

    lumMakeContextCurrent(window);
    glClear(GL_COLOR_BUFFER_BIT);
    lumSwapBuffers(window);
}

static void window_focus_callback(LUMwindow* window, int focused)
{
    Slot* slot = lumGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window %s\n",
           counter++, slot->number, lumGetTime(),
           focused ? "focused" : "defocused");
}

static void window_iconify_callback(LUMwindow* window, int iconified)
{
    Slot* slot = lumGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window was %s\n",
           counter++, slot->number, lumGetTime(),
           iconified ? "iconified" : "uniconified");
}

static void window_maximize_callback(LUMwindow* window, int maximized)
{
    Slot* slot = lumGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Window was %s\n",
           counter++, slot->number, lumGetTime(),
           maximized ? "maximized" : "unmaximized");
}

static void mouse_button_callback(LUMwindow* window, int button, int action, int mods)
{
    Slot* slot = lumGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Mouse button %i (%s) (with%s) was %s\n",
           counter++, slot->number, lumGetTime(), button,
           get_button_name(button),
           get_mods_name(mods),
           get_action_name(action));
}

static void cursor_position_callback(LUMwindow* window, double x, double y)
{
    Slot* slot = lumGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Cursor position: %f %f\n",
           counter++, slot->number, lumGetTime(), x, y);
}

static void cursor_enter_callback(LUMwindow* window, int entered)
{
    Slot* slot = lumGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Cursor %s window\n",
           counter++, slot->number, lumGetTime(),
           entered ? "entered" : "left");
}

static void scroll_callback(LUMwindow* window, double x, double y)
{
    Slot* slot = lumGetWindowUserPointer(window);
    printf("%08x to %i at %0.3f: Scroll: %0.3f %0.3f\n",
           counter++, slot->number, lumGetTime(), x, y);
}

static void key_callback(LUMwindow* window, int key, int scancode, int action, int mods)
{
    Slot* slot = lumGetWindowUserPointer(window);
    const char* name = lumGetKeyName(key, scancode);

    if (name)
    {
        printf("%08x to %i at %0.3f: Key 0x%04x Scancode 0x%04x (%s) (%s) (with%s) was %s\n",
               counter++, slot->number, lumGetTime(), key, scancode,
               get_key_name(key),
               name,
               get_mods_name(mods),
               get_action_name(action));
    }
    else
    {
        printf("%08x to %i at %0.3f: Key 0x%04x Scancode 0x%04x (%s) (with%s) was %s\n",
               counter++, slot->number, lumGetTime(), key, scancode,
               get_key_name(key),
               get_mods_name(mods),
               get_action_name(action));
    }

    if (action != LUM_PRESS)
        return;

    switch (key)
    {
        case LUM_KEY_C:
        {
            slot->closeable = !slot->closeable;

            printf("(( closing %s ))\n", slot->closeable ? "enabled" : "disabled");
            break;
        }

        case LUM_KEY_L:
        {
            const int state = lumGetInputMode(window, LUM_LOCK_KEY_MODS);
            lumSetInputMode(window, LUM_LOCK_KEY_MODS, !state);

            printf("(( lock key mods %s ))\n", !state ? "enabled" : "disabled");
            break;
        }
    }
}

static void char_callback(LUMwindow* window, unsigned int codepoint)
{
    Slot* slot = lumGetWindowUserPointer(window);
    char string[5] = "";

    encode_utf8(string, codepoint);
    printf("%08x to %i at %0.3f: Character 0x%08x (%s) input\n",
           counter++, slot->number, lumGetTime(), codepoint, string);
}

static void drop_callback(LUMwindow* window, int count, const char* paths[])
{
    int i;
    Slot* slot = lumGetWindowUserPointer(window);

    printf("%08x to %i at %0.3f: Drop input\n",
           counter++, slot->number, lumGetTime());

    for (i = 0;  i < count;  i++)
        printf("  %i: \"%s\"\n", i, paths[i]);
}

static void monitor_callback(LUMmonitor* monitor, int event)
{
    if (event == LUM_CONNECTED)
    {
        int x, y, widthMM, heightMM;
        const LUMvidmode* mode = lumGetVideoMode(monitor);

        lumGetMonitorPos(monitor, &x, &y);
        lumGetMonitorPhysicalSize(monitor, &widthMM, &heightMM);

        printf("%08x at %0.3f: Monitor %s (%ix%i at %ix%i, %ix%i mm) was connected\n",
               counter++,
               lumGetTime(),
               lumGetMonitorName(monitor),
               mode->width, mode->height,
               x, y,
               widthMM, heightMM);
    }
    else if (event == LUM_DISCONNECTED)
    {
        printf("%08x at %0.3f: Monitor %s was disconnected\n",
               counter++,
               lumGetTime(),
               lumGetMonitorName(monitor));
    }
}

static void joystick_callback(int jid, int event)
{
    if (event == LUM_CONNECTED)
    {
        int axisCount, buttonCount, hatCount;

        lumGetJoystickAxes(jid, &axisCount);
        lumGetJoystickButtons(jid, &buttonCount);
        lumGetJoystickHats(jid, &hatCount);

        printf("%08x at %0.3f: Joystick %i (%s) was connected with %i axes, %i buttons, and %i hats\n",
               counter++, lumGetTime(),
               jid,
               lumGetJoystickName(jid),
               axisCount,
               buttonCount,
               hatCount);

        if (lumJoystickIsGamepad(jid))
        {
            printf("  Joystick %i (%s) has a gamepad mapping (%s)\n",
                   jid,
                   lumGetJoystickGUID(jid),
                   lumGetGamepadName(jid));
        }
        else
        {
            printf("  Joystick %i (%s) has no gamepad mapping\n",
                   jid,
                   lumGetJoystickGUID(jid));
        }
    }
    else
    {
        printf("%08x at %0.3f: Joystick %i was disconnected\n",
               counter++, lumGetTime(), jid);
    }
}

int main(int argc, char** argv)
{
    Slot* slots;
    LUMmonitor* monitor = NULL;
    int ch, i, width, height, count = 1;

    lumSetErrorCallback(error_callback);

    if (!lumInit())
        exit(EXIT_FAILURE);

    printf("Library initialized\n");

    lumSetMonitorCallback(monitor_callback);
    lumSetJoystickCallback(joystick_callback);

    while ((ch = getopt(argc, argv, "hfn:")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);

            case 'f':
                monitor = lumGetPrimaryMonitor();
                break;

            case 'n':
                count = (int) strtoul(optarg, NULL, 10);
                break;

            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

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
        width  = 640;
        height = 480;
    }

    slots = calloc(count, sizeof(Slot));

    for (i = 0;  i < count;  i++)
    {
        char title[128];

        slots[i].closeable = LUM_TRUE;
        slots[i].number = i + 1;

        snprintf(title, sizeof(title), "Event Linter (Window %i)", slots[i].number);

        if (monitor)
        {
            printf("Creating full screen window %i (%ix%i on %s)\n",
                   slots[i].number,
                   width, height,
                   lumGetMonitorName(monitor));
        }
        else
        {
            printf("Creating windowed mode window %i (%ix%i)\n",
                   slots[i].number,
                   width, height);
        }

        slots[i].window = lumCreateWindow(width, height, title, monitor, NULL);
        if (!slots[i].window)
        {
            free(slots);
            lumTerminate();
            exit(EXIT_FAILURE);
        }

        lumSetWindowUserPointer(slots[i].window, slots + i);

        lumSetWindowPosCallback(slots[i].window, window_pos_callback);
        lumSetWindowSizeCallback(slots[i].window, window_size_callback);
        lumSetFramebufferSizeCallback(slots[i].window, framebuffer_size_callback);
        lumSetWindowContentScaleCallback(slots[i].window, window_content_scale_callback);
        lumSetWindowCloseCallback(slots[i].window, window_close_callback);
        lumSetWindowRefreshCallback(slots[i].window, window_refresh_callback);
        lumSetWindowFocusCallback(slots[i].window, window_focus_callback);
        lumSetWindowIconifyCallback(slots[i].window, window_iconify_callback);
        lumSetWindowMaximizeCallback(slots[i].window, window_maximize_callback);
        lumSetMouseButtonCallback(slots[i].window, mouse_button_callback);
        lumSetCursorPosCallback(slots[i].window, cursor_position_callback);
        lumSetCursorEnterCallback(slots[i].window, cursor_enter_callback);
        lumSetScrollCallback(slots[i].window, scroll_callback);
        lumSetKeyCallback(slots[i].window, key_callback);
        lumSetCharCallback(slots[i].window, char_callback);
        lumSetDropCallback(slots[i].window, drop_callback);

        lumMakeContextCurrent(slots[i].window);
        gladLoadGL(lumGetProcAddress);
        lumSwapInterval(1);
    }

    printf("Main loop starting\n");

    for (;;)
    {
        for (i = 0;  i < count;  i++)
        {
            if (lumWindowShouldClose(slots[i].window))
                break;
        }

        if (i < count)
            break;

        lumWaitEvents();

        // Workaround for an issue with msvcrt and mintty
        fflush(stdout);
    }

    free(slots);
    lumTerminate();
    exit(EXIT_SUCCESS);
}

