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
// Please use C89 style variable declarations in this file because VS 2010
//========================================================================

#include "internal.h"
#include "mappings.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Internal key state used for sticky keys
#define _LUM_STICK 3

// Internal constants for gamepad mapping source types
#define _LUM_JOYSTICK_AXIS     1
#define _LUM_JOYSTICK_BUTTON   2
#define _LUM_JOYSTICK_HATBIT   3

// Initializes the platform joystick API if it has not been already
//
static LUMbool initJoysticks(void)
{
    if (!_lum.joysticksInitialized)
    {
        if (!_lum.platform.initJoysticks())
        {
            _lum.platform.terminateJoysticks();
            return LUM_FALSE;
        }
    }

    return _lum.joysticksInitialized = LUM_TRUE;
}

// Finds a mapping based on joystick GUID
//
static _LUMmapping* findMapping(const char* guid)
{
    int i;

    for (i = 0;  i < _lum.mappingCount;  i++)
    {
        if (strcmp(_lum.mappings[i].guid, guid) == 0)
            return _lum.mappings + i;
    }

    return NULL;
}

// Checks whether a gamepad mapping element is present in the hardware
//
static LUMbool isValidElementForJoystick(const _LUMmapelement* e,
                                          const _LUMjoystick* js)
{
    if (e->type == _LUM_JOYSTICK_HATBIT && (e->index >> 4) >= js->hatCount)
        return LUM_FALSE;
    else if (e->type == _LUM_JOYSTICK_BUTTON && e->index >= js->buttonCount)
        return LUM_FALSE;
    else if (e->type == _LUM_JOYSTICK_AXIS && e->index >= js->axisCount)
        return LUM_FALSE;

    return LUM_TRUE;
}

// Finds a mapping based on joystick GUID and verifies element indices
//
static _LUMmapping* findValidMapping(const _LUMjoystick* js)
{
    _LUMmapping* mapping = findMapping(js->guid);
    if (mapping)
    {
        int i;

        for (i = 0;  i <= LUM_GAMEPAD_BUTTON_LAST;  i++)
        {
            if (!isValidElementForJoystick(mapping->buttons + i, js))
                return NULL;
        }

        for (i = 0;  i <= LUM_GAMEPAD_AXIS_LAST;  i++)
        {
            if (!isValidElementForJoystick(mapping->axes + i, js))
                return NULL;
        }
    }

    return mapping;
}

// Parses an SDL_GameControllerDB line and adds it to the mapping list
//
static LUMbool parseMapping(_LUMmapping* mapping, const char* string)
{
    const char* c = string;
    size_t i, length;
    struct
    {
        const char* name;
        _LUMmapelement* element;
    } fields[] =
    {
        { "platform",      NULL },
        { "a",             mapping->buttons + LUM_GAMEPAD_BUTTON_A },
        { "b",             mapping->buttons + LUM_GAMEPAD_BUTTON_B },
        { "x",             mapping->buttons + LUM_GAMEPAD_BUTTON_X },
        { "y",             mapping->buttons + LUM_GAMEPAD_BUTTON_Y },
        { "back",          mapping->buttons + LUM_GAMEPAD_BUTTON_BACK },
        { "start",         mapping->buttons + LUM_GAMEPAD_BUTTON_START },
        { "guide",         mapping->buttons + LUM_GAMEPAD_BUTTON_GUIDE },
        { "leftshoulder",  mapping->buttons + LUM_GAMEPAD_BUTTON_LEFT_BUMPER },
        { "rightshoulder", mapping->buttons + LUM_GAMEPAD_BUTTON_RIGHT_BUMPER },
        { "leftstick",     mapping->buttons + LUM_GAMEPAD_BUTTON_LEFT_THUMB },
        { "rightstick",    mapping->buttons + LUM_GAMEPAD_BUTTON_RIGHT_THUMB },
        { "dpup",          mapping->buttons + LUM_GAMEPAD_BUTTON_DPAD_UP },
        { "dpright",       mapping->buttons + LUM_GAMEPAD_BUTTON_DPAD_RIGHT },
        { "dpdown",        mapping->buttons + LUM_GAMEPAD_BUTTON_DPAD_DOWN },
        { "dpleft",        mapping->buttons + LUM_GAMEPAD_BUTTON_DPAD_LEFT },
        { "lefttrigger",   mapping->axes + LUM_GAMEPAD_AXIS_LEFT_TRIGGER },
        { "righttrigger",  mapping->axes + LUM_GAMEPAD_AXIS_RIGHT_TRIGGER },
        { "leftx",         mapping->axes + LUM_GAMEPAD_AXIS_LEFT_X },
        { "lefty",         mapping->axes + LUM_GAMEPAD_AXIS_LEFT_Y },
        { "rightx",        mapping->axes + LUM_GAMEPAD_AXIS_RIGHT_X },
        { "righty",        mapping->axes + LUM_GAMEPAD_AXIS_RIGHT_Y }
    };

    length = strcspn(c, ",");
    if (length != 32 || c[length] != ',')
    {
        _lumInputError(LUM_INVALID_VALUE, NULL);
        return LUM_FALSE;
    }

    memcpy(mapping->guid, c, length);
    c += length + 1;

    length = strcspn(c, ",");
    if (length >= sizeof(mapping->name) || c[length] != ',')
    {
        _lumInputError(LUM_INVALID_VALUE, NULL);
        return LUM_FALSE;
    }

    memcpy(mapping->name, c, length);
    c += length + 1;

    while (*c)
    {
        // TODO: Implement output modifiers
        if (*c == '+' || *c == '-')
            return LUM_FALSE;

        for (i = 0;  i < sizeof(fields) / sizeof(fields[0]);  i++)
        {
            length = strlen(fields[i].name);
            if (strncmp(c, fields[i].name, length) != 0 || c[length] != ':')
                continue;

            c += length + 1;

            if (fields[i].element)
            {
                _LUMmapelement* e = fields[i].element;
                int8_t minimum = -1;
                int8_t maximum = 1;

                if (*c == '+')
                {
                    minimum = 0;
                    c += 1;
                }
                else if (*c == '-')
                {
                    maximum = 0;
                    c += 1;
                }

                if (*c == 'a')
                    e->type = _LUM_JOYSTICK_AXIS;
                else if (*c == 'b')
                    e->type = _LUM_JOYSTICK_BUTTON;
                else if (*c == 'h')
                    e->type = _LUM_JOYSTICK_HATBIT;
                else
                    break;

                if (e->type == _LUM_JOYSTICK_HATBIT)
                {
                    const unsigned long hat = strtoul(c + 1, (char**) &c, 10);
                    const unsigned long bit = strtoul(c + 1, (char**) &c, 10);
                    e->index = (uint8_t) ((hat << 4) | bit);
                }
                else
                    e->index = (uint8_t) strtoul(c + 1, (char**) &c, 10);

                if (e->type == _LUM_JOYSTICK_AXIS)
                {
                    e->axisScale = 2 / (maximum - minimum);
                    e->axisOffset = -(maximum + minimum);

                    if (*c == '~')
                    {
                        e->axisScale = -e->axisScale;
                        e->axisOffset = -e->axisOffset;
                    }
                }
            }
            else
            {
                const char* name = _lum.platform.getMappingName();
                length = strlen(name);
                if (strncmp(c, name, length) != 0)
                    return LUM_FALSE;
            }

            break;
        }

        c += strcspn(c, ",");
        c += strspn(c, ",");
    }

    for (i = 0;  i < 32;  i++)
    {
        if (mapping->guid[i] >= 'A' && mapping->guid[i] <= 'F')
            mapping->guid[i] += 'a' - 'A';
    }

    _lum.platform.updateGamepadGUID(mapping->guid);
    return LUM_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                         Lumina event API                       //////
//////////////////////////////////////////////////////////////////////////

// Notifies shared code of a physical key event
//
void _lumInputKey(_LUMwindow* window, int key, int scancode, int action, int mods)
{
    if (key >= 0 && key <= LUM_KEY_LAST)
    {
        LUMbool repeated = LUM_FALSE;

        if (action == LUM_RELEASE && window->keys[key] == LUM_RELEASE)
            return;

        if (action == LUM_PRESS && window->keys[key] == LUM_PRESS)
            repeated = LUM_TRUE;

        if (action == LUM_RELEASE && window->stickyKeys)
            window->keys[key] = _LUM_STICK;
        else
            window->keys[key] = (char) action;

        if (repeated)
            action = LUM_REPEAT;
    }

    if (!window->lockKeyMods)
        mods &= ~(LUM_MOD_CAPS_LOCK | LUM_MOD_NUM_LOCK);

    if (window->callbacks.key)
        window->callbacks.key((LUMwindow*) window, key, scancode, action, mods);
}

// Notifies shared code of a Unicode codepoint input event
// The 'plain' parameter determines whether to emit a regular character event
//
void _lumInputChar(_LUMwindow* window, uint32_t codepoint, int mods, LUMbool plain)
{
    if (codepoint < 32 || (codepoint > 126 && codepoint < 160))
        return;

    if (!window->lockKeyMods)
        mods &= ~(LUM_MOD_CAPS_LOCK | LUM_MOD_NUM_LOCK);

    if (window->callbacks.charmods)
        window->callbacks.charmods((LUMwindow*) window, codepoint, mods);

    if (plain)
    {
        if (window->callbacks.character)
            window->callbacks.character((LUMwindow*) window, codepoint);
    }
}

// Notifies shared code of a scroll event
//
void _lumInputScroll(_LUMwindow* window, double xoffset, double yoffset)
{
    if (window->callbacks.scroll)
        window->callbacks.scroll((LUMwindow*) window, xoffset, yoffset);
}

// Notifies shared code of a mouse button click event
//
void _lumInputMouseClick(_LUMwindow* window, int button, int action, int mods)
{
    if (button < 0 || button > LUM_MOUSE_BUTTON_LAST)
        return;

    if (!window->lockKeyMods)
        mods &= ~(LUM_MOD_CAPS_LOCK | LUM_MOD_NUM_LOCK);

    if (action == LUM_RELEASE && window->stickyMouseButtons)
        window->mouseButtons[button] = _LUM_STICK;
    else
        window->mouseButtons[button] = (char) action;

    if (window->callbacks.mouseButton)
        window->callbacks.mouseButton((LUMwindow*) window, button, action, mods);
}

// Notifies shared code of a cursor motion event
// The position is specified in content area relative screen coordinates
//
void _lumInputCursorPos(_LUMwindow* window, double xpos, double ypos)
{
    if (window->virtualCursorPosX == xpos && window->virtualCursorPosY == ypos)
        return;

    window->virtualCursorPosX = xpos;
    window->virtualCursorPosY = ypos;

    if (window->callbacks.cursorPos)
        window->callbacks.cursorPos((LUMwindow*) window, xpos, ypos);
}

// Notifies shared code of a cursor enter/leave event
//
void _lumInputCursorEnter(_LUMwindow* window, LUMbool entered)
{
    if (window->callbacks.cursorEnter)
        window->callbacks.cursorEnter((LUMwindow*) window, entered);
}

// Notifies shared code of files or directories dropped on a window
//
void _lumInputDrop(_LUMwindow* window, int count, const char** paths)
{
    if (window->callbacks.drop)
        window->callbacks.drop((LUMwindow*) window, count, paths);
}

// Notifies shared code of a joystick connection or disconnection
//
void _lumInputJoystick(_LUMjoystick* js, int event)
{
    const int jid = (int) (js - _lum.joysticks);

    if (_lum.callbacks.joystick)
        _lum.callbacks.joystick(jid, event);
}

// Notifies shared code of the new value of a joystick axis
//
void _lumInputJoystickAxis(_LUMjoystick* js, int axis, float value)
{
    js->axes[axis] = value;
}

// Notifies shared code of the new value of a joystick button
//
void _lumInputJoystickButton(_LUMjoystick* js, int button, char value)
{
    js->buttons[button] = value;
}

// Notifies shared code of the new value of a joystick hat
//
void _lumInputJoystickHat(_LUMjoystick* js, int hat, char value)
{
    const int base = js->buttonCount + hat * 4;

    js->buttons[base + 0] = (value & 0x01) ? LUM_PRESS : LUM_RELEASE;
    js->buttons[base + 1] = (value & 0x02) ? LUM_PRESS : LUM_RELEASE;
    js->buttons[base + 2] = (value & 0x04) ? LUM_PRESS : LUM_RELEASE;
    js->buttons[base + 3] = (value & 0x08) ? LUM_PRESS : LUM_RELEASE;

    js->hats[hat] = value;
}


//////////////////////////////////////////////////////////////////////////
//////                       Lumina internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Adds the built-in set of gamepad mappings
//
void _lumInitGamepadMappings(void)
{
    size_t i;
    const size_t count = sizeof(_lumDefaultMappings) / sizeof(char*);
    _lum.mappings = _lum_calloc(count, sizeof(_LUMmapping));

    for (i = 0;  i < count;  i++)
    {
        if (parseMapping(&_lum.mappings[_lum.mappingCount], _lumDefaultMappings[i]))
            _lum.mappingCount++;
    }
}

// Returns an available joystick object with arrays and name allocated
//
_LUMjoystick* _lumAllocJoystick(const char* name,
                                  const char* guid,
                                  int axisCount,
                                  int buttonCount,
                                  int hatCount)
{
    int jid;
    _LUMjoystick* js;

    for (jid = 0;  jid <= LUM_JOYSTICK_LAST;  jid++)
    {
        if (!_lum.joysticks[jid].present)
            break;
    }

    if (jid > LUM_JOYSTICK_LAST)
        return NULL;

    js = _lum.joysticks + jid;
    js->present     = LUM_TRUE;
    js->axes        = _lum_calloc(axisCount, sizeof(float));
    js->buttons     = _lum_calloc(buttonCount + (size_t) hatCount * 4, 1);
    js->hats        = _lum_calloc(hatCount, 1);
    js->axisCount   = axisCount;
    js->buttonCount = buttonCount;
    js->hatCount    = hatCount;

    strncpy(js->name, name, sizeof(js->name) - 1);
    strncpy(js->guid, guid, sizeof(js->guid) - 1);
    js->mapping = findValidMapping(js);

    return js;
}

// Frees arrays and name and flags the joystick object as unused
//
void _lumFreeJoystick(_LUMjoystick* js)
{
    _lum_free(js->axes);
    _lum_free(js->buttons);
    _lum_free(js->hats);
    memset(js, 0, sizeof(_LUMjoystick));
}

// Center the cursor in the content area of the specified window
//
void _lumCenterCursorInContentArea(_LUMwindow* window)
{
    int width, height;

    _lum.platform.getWindowSize(window, &width, &height);
    _lum.platform.setCursorPos(window, width / 2.0, height / 2.0);
}


//////////////////////////////////////////////////////////////////////////
//////                        Lumina public API                       //////
//////////////////////////////////////////////////////////////////////////

LUMAPI int lumGetInputMode(LUMwindow* handle, int mode)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(0);

    switch (mode)
    {
        case LUM_CURSOR:
            return window->cursorMode;
        case LUM_STICKY_KEYS:
            return window->stickyKeys;
        case LUM_STICKY_MOUSE_BUTTONS:
            return window->stickyMouseButtons;
        case LUM_LOCK_KEY_MODS:
            return window->lockKeyMods;
        case LUM_RAW_MOUSE_MOTION:
            return window->rawMouseMotion;
    }

    _lumInputError(LUM_INVALID_ENUM, "Invalid input mode 0x%08X", mode);
    return 0;
}

LUMAPI void lumSetInputMode(LUMwindow* handle, int mode, int value)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT();

    switch (mode)
    {
        case LUM_CURSOR:
        {
            if (value != LUM_CURSOR_NORMAL &&
                value != LUM_CURSOR_HIDDEN &&
                value != LUM_CURSOR_DISABLED)
            {
                _lumInputError(LUM_INVALID_ENUM,
                                "Invalid cursor mode 0x%08X",
                                value);
                return;
            }

            if (window->cursorMode == value)
                return;

            window->cursorMode = value;

            _lum.platform.getCursorPos(window,
                                        &window->virtualCursorPosX,
                                        &window->virtualCursorPosY);
            _lum.platform.setCursorMode(window, value);
            return;
        }

        case LUM_STICKY_KEYS:
        {
            value = value ? LUM_TRUE : LUM_FALSE;
            if (window->stickyKeys == value)
                return;

            if (!value)
            {
                int i;

                // Release all sticky keys
                for (i = 0;  i <= LUM_KEY_LAST;  i++)
                {
                    if (window->keys[i] == _LUM_STICK)
                        window->keys[i] = LUM_RELEASE;
                }
            }

            window->stickyKeys = value;
            return;
        }

        case LUM_STICKY_MOUSE_BUTTONS:
        {
            value = value ? LUM_TRUE : LUM_FALSE;
            if (window->stickyMouseButtons == value)
                return;

            if (!value)
            {
                int i;

                // Release all sticky mouse buttons
                for (i = 0;  i <= LUM_MOUSE_BUTTON_LAST;  i++)
                {
                    if (window->mouseButtons[i] == _LUM_STICK)
                        window->mouseButtons[i] = LUM_RELEASE;
                }
            }

            window->stickyMouseButtons = value;
            return;
        }

        case LUM_LOCK_KEY_MODS:
        {
            window->lockKeyMods = value ? LUM_TRUE : LUM_FALSE;
            return;
        }

        case LUM_RAW_MOUSE_MOTION:
        {
            if (!_lum.platform.rawMouseMotionSupported())
            {
                _lumInputError(LUM_PLATFORM_ERROR,
                                "Raw mouse motion is not supported on this system");
                return;
            }

            value = value ? LUM_TRUE : LUM_FALSE;
            if (window->rawMouseMotion == value)
                return;

            window->rawMouseMotion = value;
            _lum.platform.setRawMouseMotion(window, value);
            return;
        }
    }

    _lumInputError(LUM_INVALID_ENUM, "Invalid input mode 0x%08X", mode);
}

LUMAPI int lumRawMouseMotionSupported(void)
{
    _LUM_REQUIRE_INIT_OR_RETURN(LUM_FALSE);
    return _lum.platform.rawMouseMotionSupported();
}

LUMAPI const char* lumGetKeyName(int key, int scancode)
{
    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    if (key != LUM_KEY_UNKNOWN)
    {
        if (key != LUM_KEY_KP_EQUAL &&
            (key < LUM_KEY_KP_0 || key > LUM_KEY_KP_ADD) &&
            (key < LUM_KEY_APOSTROPHE || key > LUM_KEY_WORLD_2))
        {
            return NULL;
        }

        scancode = _lum.platform.getKeyScancode(key);
    }

    return _lum.platform.getScancodeName(scancode);
}

LUMAPI int lumGetKeyScancode(int key)
{
    _LUM_REQUIRE_INIT_OR_RETURN(-1);

    if (key < LUM_KEY_SPACE || key > LUM_KEY_LAST)
    {
        _lumInputError(LUM_INVALID_ENUM, "Invalid key %i", key);
        return LUM_RELEASE;
    }

    return _lum.platform.getKeyScancode(key);
}

LUMAPI int lumGetKey(LUMwindow* handle, int key)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(LUM_RELEASE);

    if (key < LUM_KEY_SPACE || key > LUM_KEY_LAST)
    {
        _lumInputError(LUM_INVALID_ENUM, "Invalid key %i", key);
        return LUM_RELEASE;
    }

    if (window->keys[key] == _LUM_STICK)
    {
        // Sticky mode: release key now
        window->keys[key] = LUM_RELEASE;
        return LUM_PRESS;
    }

    return (int) window->keys[key];
}

LUMAPI int lumGetMouseButton(LUMwindow* handle, int button)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(LUM_RELEASE);

    if (button < LUM_MOUSE_BUTTON_1 || button > LUM_MOUSE_BUTTON_LAST)
    {
        _lumInputError(LUM_INVALID_ENUM, "Invalid mouse button %i", button);
        return LUM_RELEASE;
    }

    if (window->mouseButtons[button] == _LUM_STICK)
    {
        // Sticky mode: release mouse button now
        window->mouseButtons[button] = LUM_RELEASE;
        return LUM_PRESS;
    }

    return (int) window->mouseButtons[button];
}

LUMAPI void lumGetCursorPos(LUMwindow* handle, double* xpos, double* ypos)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 0;

    _LUM_REQUIRE_INIT();

    if (window->cursorMode == LUM_CURSOR_DISABLED)
    {
        if (xpos)
            *xpos = window->virtualCursorPosX;
        if (ypos)
            *ypos = window->virtualCursorPosY;
    }
    else
        _lum.platform.getCursorPos(window, xpos, ypos);
}

LUMAPI void lumSetCursorPos(LUMwindow* handle, double xpos, double ypos)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT();

    if (xpos != xpos || xpos < -DBL_MAX || xpos > DBL_MAX ||
        ypos != ypos || ypos < -DBL_MAX || ypos > DBL_MAX)
    {
        _lumInputError(LUM_INVALID_VALUE,
                        "Invalid cursor position %f %f",
                        xpos, ypos);
        return;
    }

    if (!_lum.platform.windowFocused(window))
        return;

    if (window->cursorMode == LUM_CURSOR_DISABLED)
    {
        // Only update the accumulated position if the cursor is disabled
        window->virtualCursorPosX = xpos;
        window->virtualCursorPosY = ypos;
    }
    else
    {
        // Update system cursor position
        _lum.platform.setCursorPos(window, xpos, ypos);
    }
}

LUMAPI LUMcursor* lumCreateCursor(const LUMimage* image, int xhot, int yhot)
{
    _LUMcursor* cursor;

    assert(image != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    cursor = _lum_calloc(1, sizeof(_LUMcursor));
    cursor->next = _lum.cursorListHead;
    _lum.cursorListHead = cursor;

    if (!_lum.platform.createCursor(cursor, image, xhot, yhot))
    {
        lumDestroyCursor((LUMcursor*) cursor);
        return NULL;
    }

    return (LUMcursor*) cursor;
}

LUMAPI LUMcursor* lumCreateStandardCursor(int shape)
{
    _LUMcursor* cursor;

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    if (shape != LUM_ARROW_CURSOR &&
        shape != LUM_IBEAM_CURSOR &&
        shape != LUM_CROSSHAIR_CURSOR &&
        shape != LUM_POINTING_HAND_CURSOR &&
        shape != LUM_RESIZE_EW_CURSOR &&
        shape != LUM_RESIZE_NS_CURSOR &&
        shape != LUM_RESIZE_NWSE_CURSOR &&
        shape != LUM_RESIZE_NESW_CURSOR &&
        shape != LUM_RESIZE_ALL_CURSOR &&
        shape != LUM_NOT_ALLOWED_CURSOR)
    {
        _lumInputError(LUM_INVALID_ENUM, "Invalid standard cursor 0x%08X", shape);
        return NULL;
    }

    cursor = _lum_calloc(1, sizeof(_LUMcursor));
    cursor->next = _lum.cursorListHead;
    _lum.cursorListHead = cursor;

    if (!_lum.platform.createStandardCursor(cursor, shape))
    {
        lumDestroyCursor((LUMcursor*) cursor);
        return NULL;
    }

    return (LUMcursor*) cursor;
}

LUMAPI void lumDestroyCursor(LUMcursor* handle)
{
    _LUMcursor* cursor = (_LUMcursor*) handle;

    _LUM_REQUIRE_INIT();

    if (cursor == NULL)
        return;

    // Make sure the cursor is not being used by any window
    {
        _LUMwindow* window;

        for (window = _lum.windowListHead;  window;  window = window->next)
        {
            if (window->cursor == cursor)
                lumSetCursor((LUMwindow*) window, NULL);
        }
    }

    _lum.platform.destroyCursor(cursor);

    // Unlink cursor from global linked list
    {
        _LUMcursor** prev = &_lum.cursorListHead;

        while (*prev != cursor)
            prev = &((*prev)->next);

        *prev = cursor->next;
    }

    _lum_free(cursor);
}

LUMAPI void lumSetCursor(LUMwindow* windowHandle, LUMcursor* cursorHandle)
{
    _LUMwindow* window = (_LUMwindow*) windowHandle;
    _LUMcursor* cursor = (_LUMcursor*) cursorHandle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT();

    window->cursor = cursor;

    _lum.platform.setCursor(window, cursor);
}

LUMAPI LUMkeyfun lumSetKeyCallback(LUMwindow* handle, LUMkeyfun cbfun)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    _LUM_SWAP(LUMkeyfun, window->callbacks.key, cbfun);
    return cbfun;
}

LUMAPI LUMcharfun lumSetCharCallback(LUMwindow* handle, LUMcharfun cbfun)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    _LUM_SWAP(LUMcharfun, window->callbacks.character, cbfun);
    return cbfun;
}

LUMAPI LUMcharmodsfun lumSetCharModsCallback(LUMwindow* handle, LUMcharmodsfun cbfun)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    _LUM_SWAP(LUMcharmodsfun, window->callbacks.charmods, cbfun);
    return cbfun;
}

LUMAPI LUMmousebuttonfun lumSetMouseButtonCallback(LUMwindow* handle,
                                                      LUMmousebuttonfun cbfun)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    _LUM_SWAP(LUMmousebuttonfun, window->callbacks.mouseButton, cbfun);
    return cbfun;
}

LUMAPI LUMcursorposfun lumSetCursorPosCallback(LUMwindow* handle,
                                                  LUMcursorposfun cbfun)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    _LUM_SWAP(LUMcursorposfun, window->callbacks.cursorPos, cbfun);
    return cbfun;
}

LUMAPI LUMcursorenterfun lumSetCursorEnterCallback(LUMwindow* handle,
                                                      LUMcursorenterfun cbfun)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    _LUM_SWAP(LUMcursorenterfun, window->callbacks.cursorEnter, cbfun);
    return cbfun;
}

LUMAPI LUMscrollfun lumSetScrollCallback(LUMwindow* handle,
                                            LUMscrollfun cbfun)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    _LUM_SWAP(LUMscrollfun, window->callbacks.scroll, cbfun);
    return cbfun;
}

LUMAPI LUMdropfun lumSetDropCallback(LUMwindow* handle, LUMdropfun cbfun)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    _LUM_SWAP(LUMdropfun, window->callbacks.drop, cbfun);
    return cbfun;
}

LUMAPI int lumJoystickPresent(int jid)
{
    _LUMjoystick* js;

    assert(jid >= LUM_JOYSTICK_1);
    assert(jid <= LUM_JOYSTICK_LAST);

    _LUM_REQUIRE_INIT_OR_RETURN(LUM_FALSE);

    if (jid < 0 || jid > LUM_JOYSTICK_LAST)
    {
        _lumInputError(LUM_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return LUM_FALSE;
    }

    if (!initJoysticks())
        return LUM_FALSE;

    js = _lum.joysticks + jid;
    if (!js->present)
        return LUM_FALSE;

    return _lum.platform.pollJoystick(js, _LUM_POLL_PRESENCE);
}

LUMAPI const float* lumGetJoystickAxes(int jid, int* count)
{
    _LUMjoystick* js;

    assert(jid >= LUM_JOYSTICK_1);
    assert(jid <= LUM_JOYSTICK_LAST);
    assert(count != NULL);

    *count = 0;

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > LUM_JOYSTICK_LAST)
    {
        _lumInputError(LUM_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    if (!initJoysticks())
        return NULL;

    js = _lum.joysticks + jid;
    if (!js->present)
        return NULL;

    if (!_lum.platform.pollJoystick(js, _LUM_POLL_AXES))
        return NULL;

    *count = js->axisCount;
    return js->axes;
}

LUMAPI const unsigned char* lumGetJoystickButtons(int jid, int* count)
{
    _LUMjoystick* js;

    assert(jid >= LUM_JOYSTICK_1);
    assert(jid <= LUM_JOYSTICK_LAST);
    assert(count != NULL);

    *count = 0;

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > LUM_JOYSTICK_LAST)
    {
        _lumInputError(LUM_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    if (!initJoysticks())
        return NULL;

    js = _lum.joysticks + jid;
    if (!js->present)
        return NULL;

    if (!_lum.platform.pollJoystick(js, _LUM_POLL_BUTTONS))
        return NULL;

    if (_lum.hints.init.hatButtons)
        *count = js->buttonCount + js->hatCount * 4;
    else
        *count = js->buttonCount;

    return js->buttons;
}

LUMAPI const unsigned char* lumGetJoystickHats(int jid, int* count)
{
    _LUMjoystick* js;

    assert(jid >= LUM_JOYSTICK_1);
    assert(jid <= LUM_JOYSTICK_LAST);
    assert(count != NULL);

    *count = 0;

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > LUM_JOYSTICK_LAST)
    {
        _lumInputError(LUM_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    if (!initJoysticks())
        return NULL;

    js = _lum.joysticks + jid;
    if (!js->present)
        return NULL;

    if (!_lum.platform.pollJoystick(js, _LUM_POLL_BUTTONS))
        return NULL;

    *count = js->hatCount;
    return js->hats;
}

LUMAPI const char* lumGetJoystickName(int jid)
{
    _LUMjoystick* js;

    assert(jid >= LUM_JOYSTICK_1);
    assert(jid <= LUM_JOYSTICK_LAST);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > LUM_JOYSTICK_LAST)
    {
        _lumInputError(LUM_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    if (!initJoysticks())
        return NULL;

    js = _lum.joysticks + jid;
    if (!js->present)
        return NULL;

    if (!_lum.platform.pollJoystick(js, _LUM_POLL_PRESENCE))
        return NULL;

    return js->name;
}

LUMAPI const char* lumGetJoystickGUID(int jid)
{
    _LUMjoystick* js;

    assert(jid >= LUM_JOYSTICK_1);
    assert(jid <= LUM_JOYSTICK_LAST);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > LUM_JOYSTICK_LAST)
    {
        _lumInputError(LUM_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    if (!initJoysticks())
        return NULL;

    js = _lum.joysticks + jid;
    if (!js->present)
        return NULL;

    if (!_lum.platform.pollJoystick(js, _LUM_POLL_PRESENCE))
        return NULL;

    return js->guid;
}

LUMAPI void lumSetJoystickUserPointer(int jid, void* pointer)
{
    _LUMjoystick* js;

    assert(jid >= LUM_JOYSTICK_1);
    assert(jid <= LUM_JOYSTICK_LAST);

    _LUM_REQUIRE_INIT();

    js = _lum.joysticks + jid;
    if (!js->present)
        return;

    js->userPointer = pointer;
}

LUMAPI void* lumGetJoystickUserPointer(int jid)
{
    _LUMjoystick* js;

    assert(jid >= LUM_JOYSTICK_1);
    assert(jid <= LUM_JOYSTICK_LAST);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    js = _lum.joysticks + jid;
    if (!js->present)
        return NULL;

    return js->userPointer;
}

LUMAPI LUMjoystickfun lumSetJoystickCallback(LUMjoystickfun cbfun)
{
    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    if (!initJoysticks())
        return NULL;

    _LUM_SWAP(LUMjoystickfun, _lum.callbacks.joystick, cbfun);
    return cbfun;
}

LUMAPI int lumUpdateGamepadMappings(const char* string)
{
    int jid;
    const char* c = string;

    assert(string != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(LUM_FALSE);

    while (*c)
    {
        if ((*c >= '0' && *c <= '9') ||
            (*c >= 'a' && *c <= 'f') ||
            (*c >= 'A' && *c <= 'F'))
        {
            char line[1024];

            const size_t length = strcspn(c, "\r\n");
            if (length < sizeof(line))
            {
                _LUMmapping mapping = {{0}};

                memcpy(line, c, length);
                line[length] = '\0';

                if (parseMapping(&mapping, line))
                {
                    _LUMmapping* previous = findMapping(mapping.guid);
                    if (previous)
                        *previous = mapping;
                    else
                    {
                        _lum.mappingCount++;
                        _lum.mappings =
                            _lum_realloc(_lum.mappings,
                                          sizeof(_LUMmapping) * _lum.mappingCount);
                        _lum.mappings[_lum.mappingCount - 1] = mapping;
                    }
                }
            }

            c += length;
        }
        else
        {
            c += strcspn(c, "\r\n");
            c += strspn(c, "\r\n");
        }
    }

    for (jid = 0;  jid <= LUM_JOYSTICK_LAST;  jid++)
    {
        _LUMjoystick* js = _lum.joysticks + jid;
        if (js->present)
            js->mapping = findValidMapping(js);
    }

    return LUM_TRUE;
}

LUMAPI int lumJoystickIsGamepad(int jid)
{
    _LUMjoystick* js;

    assert(jid >= LUM_JOYSTICK_1);
    assert(jid <= LUM_JOYSTICK_LAST);

    _LUM_REQUIRE_INIT_OR_RETURN(LUM_FALSE);

    if (jid < 0 || jid > LUM_JOYSTICK_LAST)
    {
        _lumInputError(LUM_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return LUM_FALSE;
    }

    if (!initJoysticks())
        return LUM_FALSE;

    js = _lum.joysticks + jid;
    if (!js->present)
        return LUM_FALSE;

    if (!_lum.platform.pollJoystick(js, _LUM_POLL_PRESENCE))
        return LUM_FALSE;

    return js->mapping != NULL;
}

LUMAPI const char* lumGetGamepadName(int jid)
{
    _LUMjoystick* js;

    assert(jid >= LUM_JOYSTICK_1);
    assert(jid <= LUM_JOYSTICK_LAST);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    if (jid < 0 || jid > LUM_JOYSTICK_LAST)
    {
        _lumInputError(LUM_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return NULL;
    }

    if (!initJoysticks())
        return NULL;

    js = _lum.joysticks + jid;
    if (!js->present)
        return NULL;

    if (!_lum.platform.pollJoystick(js, _LUM_POLL_PRESENCE))
        return NULL;

    if (!js->mapping)
        return NULL;

    return js->mapping->name;
}

LUMAPI int lumGetGamepadState(int jid, LUMgamepadstate* state)
{
    int i;
    _LUMjoystick* js;

    assert(jid >= LUM_JOYSTICK_1);
    assert(jid <= LUM_JOYSTICK_LAST);
    assert(state != NULL);

    memset(state, 0, sizeof(LUMgamepadstate));

    _LUM_REQUIRE_INIT_OR_RETURN(LUM_FALSE);

    if (jid < 0 || jid > LUM_JOYSTICK_LAST)
    {
        _lumInputError(LUM_INVALID_ENUM, "Invalid joystick ID %i", jid);
        return LUM_FALSE;
    }

    if (!initJoysticks())
        return LUM_FALSE;

    js = _lum.joysticks + jid;
    if (!js->present)
        return LUM_FALSE;

    if (!_lum.platform.pollJoystick(js, _LUM_POLL_ALL))
        return LUM_FALSE;

    if (!js->mapping)
        return LUM_FALSE;

    for (i = 0;  i <= LUM_GAMEPAD_BUTTON_LAST;  i++)
    {
        const _LUMmapelement* e = js->mapping->buttons + i;
        if (e->type == _LUM_JOYSTICK_AXIS)
        {
            const float value = js->axes[e->index] * e->axisScale + e->axisOffset;
            // HACK: This should be baked into the value transform
            // TODO: Bake into transform when implementing output modifiers
            if (e->axisOffset < 0 || (e->axisOffset == 0 && e->axisScale > 0))
            {
                if (value >= 0.f)
                    state->buttons[i] = LUM_PRESS;
            }
            else
            {
                if (value <= 0.f)
                    state->buttons[i] = LUM_PRESS;
            }
        }
        else if (e->type == _LUM_JOYSTICK_HATBIT)
        {
            const unsigned int hat = e->index >> 4;
            const unsigned int bit = e->index & 0xf;
            if (js->hats[hat] & bit)
                state->buttons[i] = LUM_PRESS;
        }
        else if (e->type == _LUM_JOYSTICK_BUTTON)
            state->buttons[i] = js->buttons[e->index];
    }

    for (i = 0;  i <= LUM_GAMEPAD_AXIS_LAST;  i++)
    {
        const _LUMmapelement* e = js->mapping->axes + i;
        if (e->type == _LUM_JOYSTICK_AXIS)
        {
            const float value = js->axes[e->index] * e->axisScale + e->axisOffset;
            state->axes[i] = _lum_fminf(_lum_fmaxf(value, -1.f), 1.f);
        }
        else if (e->type == _LUM_JOYSTICK_HATBIT)
        {
            const unsigned int hat = e->index >> 4;
            const unsigned int bit = e->index & 0xf;
            if (js->hats[hat] & bit)
                state->axes[i] = 1.f;
            else
                state->axes[i] = -1.f;
        }
        else if (e->type == _LUM_JOYSTICK_BUTTON)
            state->axes[i] = js->buttons[e->index] * 2.f - 1.f;
    }

    return LUM_TRUE;
}

LUMAPI void lumSetClipboardString(LUMwindow* handle, const char* string)
{
    assert(string != NULL);

    _LUM_REQUIRE_INIT();
    _lum.platform.setClipboardString(string);
}

LUMAPI const char* lumGetClipboardString(LUMwindow* handle)
{
    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    return _lum.platform.getClipboardString();
}

LUMAPI double lumGetTime(void)
{
    _LUM_REQUIRE_INIT_OR_RETURN(0.0);
    return (double) (_lumPlatformGetTimerValue() - _lum.timer.offset) /
        _lumPlatformGetTimerFrequency();
}

LUMAPI void lumSetTime(double time)
{
    _LUM_REQUIRE_INIT();

    if (time != time || time < 0.0 || time > 18446744073.0)
    {
        _lumInputError(LUM_INVALID_VALUE, "Invalid time %f", time);
        return;
    }

    _lum.timer.offset = _lumPlatformGetTimerValue() -
        (uint64_t) (time * _lumPlatformGetTimerFrequency());
}

LUMAPI uint64_t lumGetTimerValue(void)
{
    _LUM_REQUIRE_INIT_OR_RETURN(0);
    return _lumPlatformGetTimerValue();
}

LUMAPI uint64_t lumGetTimerFrequency(void)
{
    _LUM_REQUIRE_INIT_OR_RETURN(0);
    return _lumPlatformGetTimerFrequency();
}
