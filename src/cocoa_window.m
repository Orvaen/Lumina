//========================================================================
// Lumina macOS - fork of GLFW 3.4 (www.glfw.org)
//------------------------------------------------------------------------
// Copyright (c) 2009-2019 Camilla Löwy <elmindreda@glfw.org>
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
// It is fine to use C99 in this file because it will not be built with VS
//========================================================================

#include "internal.h"

#include <float.h>
#include <string.h>

// Returns the style mask corresponding to the window settings
//
static NSUInteger getStyleMask(_LUMwindow* window)
{
    NSUInteger styleMask = NSWindowStyleMaskMiniaturizable;

    if (window->monitor || !window->decorated)
        styleMask |= NSWindowStyleMaskBorderless;
    else
    {
        styleMask |= NSWindowStyleMaskTitled |
                     NSWindowStyleMaskClosable;

        if (window->resizable)
            styleMask |= NSWindowStyleMaskResizable;
    }

    return styleMask;
}

// Returns whether the cursor is in the content area of the specified window
//
static LUMbool cursorInContentArea(_LUMwindow* window)
{
    const NSPoint pos = [window->ns.object mouseLocationOutsideOfEventStream];
    return [window->ns.view mouse:pos inRect:[window->ns.view frame]];
}

// Hides the cursor if not already hidden
//
static void hideCursor(_LUMwindow* window)
{
    if (!_lum.ns.cursorHidden)
    {
        [NSCursor hide];
        _lum.ns.cursorHidden = LUM_TRUE;
    }
}

// Shows the cursor if not already shown
//
static void showCursor(_LUMwindow* window)
{
    if (_lum.ns.cursorHidden)
    {
        [NSCursor unhide];
        _lum.ns.cursorHidden = LUM_FALSE;
    }
}

// Updates the cursor image according to its cursor mode
//
static void updateCursorImage(_LUMwindow* window)
{
    if (window->cursorMode == LUM_CURSOR_NORMAL)
    {
        showCursor(window);

        if (window->cursor)
            [(NSCursor*) window->cursor->ns.object set];
        else
            [[NSCursor arrowCursor] set];
    }
    else
        hideCursor(window);
}

// Apply chosen cursor mode to a focused window
//
static void updateCursorMode(_LUMwindow* window)
{
    if (window->cursorMode == LUM_CURSOR_DISABLED)
    {
        _lum.ns.disabledCursorWindow = window;
        _lumGetCursorPosCocoa(window,
                               &_lum.ns.restoreCursorPosX,
                               &_lum.ns.restoreCursorPosY);
        _lumCenterCursorInContentArea(window);
        CGAssociateMouseAndMouseCursorPosition(false);
    }
    else if (_lum.ns.disabledCursorWindow == window)
    {
        _lum.ns.disabledCursorWindow = NULL;
        _lumSetCursorPosCocoa(window,
                               _lum.ns.restoreCursorPosX,
                               _lum.ns.restoreCursorPosY);
        // NOTE: The matching CGAssociateMouseAndMouseCursorPosition call is
        //       made in _lumSetCursorPosCocoa as part of a workaround
    }

    if (cursorInContentArea(window))
        updateCursorImage(window);
}

// Make the specified window and its video mode active on its monitor
//
static void acquireMonitor(_LUMwindow* window)
{
    _lumSetVideoModeCocoa(window->monitor, &window->videoMode);
    const CGRect bounds = CGDisplayBounds(window->monitor->ns.displayID);
    const NSRect frame = NSMakeRect(bounds.origin.x,
                                    _lumTransformYCocoa(bounds.origin.y + bounds.size.height - 1),
                                    bounds.size.width,
                                    bounds.size.height);

    [window->ns.object setFrame:frame display:YES];

    _lumInputMonitorWindow(window->monitor, window);
}

// Remove the window and restore the original video mode
//
static void releaseMonitor(_LUMwindow* window)
{
    if (window->monitor->window != window)
        return;

    _lumInputMonitorWindow(window->monitor, NULL);
    _lumRestoreVideoModeCocoa(window->monitor);
}

// Translates macOS key modifiers into Lumina ones
//
static int translateFlags(NSUInteger flags)
{
    int mods = 0;

    if (flags & NSEventModifierFlagShift)
        mods |= LUM_MOD_SHIFT;
    if (flags & NSEventModifierFlagControl)
        mods |= LUM_MOD_CONTROL;
    if (flags & NSEventModifierFlagOption)
        mods |= LUM_MOD_ALT;
    if (flags & NSEventModifierFlagCommand)
        mods |= LUM_MOD_SUPER;
    if (flags & NSEventModifierFlagCapsLock)
        mods |= LUM_MOD_CAPS_LOCK;

    return mods;
}

// Translates a macOS keycode to a Lumina keycode
//
static int translateKey(unsigned int key)
{
    if (key >= sizeof(_lum.ns.keycodes) / sizeof(_lum.ns.keycodes[0]))
        return LUM_KEY_UNKNOWN;

    return _lum.ns.keycodes[key];
}

// Translate a Lumina keycode to a Cocoa modifier flag
//
static NSUInteger translateKeyToModifierFlag(int key)
{
    switch (key)
    {
        case LUM_KEY_LEFT_SHIFT:
        case LUM_KEY_RIGHT_SHIFT:
            return NSEventModifierFlagShift;
        case LUM_KEY_LEFT_CONTROL:
        case LUM_KEY_RIGHT_CONTROL:
            return NSEventModifierFlagControl;
        case LUM_KEY_LEFT_ALT:
        case LUM_KEY_RIGHT_ALT:
            return NSEventModifierFlagOption;
        case LUM_KEY_LEFT_SUPER:
        case LUM_KEY_RIGHT_SUPER:
            return NSEventModifierFlagCommand;
        case LUM_KEY_CAPS_LOCK:
            return NSEventModifierFlagCapsLock;
    }

    return 0;
}

// Defines a constant for empty ranges in NSTextInputClient
//
static const NSRange kEmptyRange = { NSNotFound, 0 };


//------------------------------------------------------------------------
// Delegate for window related notifications
//------------------------------------------------------------------------

@interface LUMWindowDelegate : NSObject
{
    _LUMwindow* window;
}

- (instancetype)initWithLumWindow:(_LUMwindow *)initWindow;

@end

@implementation LUMWindowDelegate

- (instancetype)initWithLumWindow:(_LUMwindow *)initWindow
{
    self = [super init];
    if (self != nil)
        window = initWindow;

    return self;
}

- (BOOL)windowShouldClose:(id)sender
{
    _lumInputWindowCloseRequest(window);
    return NO;
}

- (void)windowDidResize:(NSNotification *)notification
{
    if (window->context.source == LUM_NATIVE_CONTEXT_API)
        [window->context.nsgl.object update];

    if (_lum.ns.disabledCursorWindow == window)
        _lumCenterCursorInContentArea(window);

    const int maximized = [window->ns.object isZoomed];
    if (window->ns.maximized != maximized)
    {
        window->ns.maximized = maximized;
        _lumInputWindowMaximize(window, maximized);
    }

    const NSRect contentRect = [window->ns.view frame];
    const NSRect fbRect = [window->ns.view convertRectToBacking:contentRect];

    if (fbRect.size.width != window->ns.fbWidth ||
        fbRect.size.height != window->ns.fbHeight)
    {
        window->ns.fbWidth  = fbRect.size.width;
        window->ns.fbHeight = fbRect.size.height;
        _lumInputFramebufferSize(window, fbRect.size.width, fbRect.size.height);
    }

    if (contentRect.size.width != window->ns.width ||
        contentRect.size.height != window->ns.height)
    {
        window->ns.width  = contentRect.size.width;
        window->ns.height = contentRect.size.height;
        _lumInputWindowSize(window, contentRect.size.width, contentRect.size.height);
    }
}

- (void)windowDidMove:(NSNotification *)notification
{
    if (window->context.source == LUM_NATIVE_CONTEXT_API)
        [window->context.nsgl.object update];

    if (_lum.ns.disabledCursorWindow == window)
        _lumCenterCursorInContentArea(window);

    int x, y;
    _lumGetWindowPosCocoa(window, &x, &y);
    _lumInputWindowPos(window, x, y);
}

- (void)windowDidMiniaturize:(NSNotification *)notification
{
    if (window->monitor)
        releaseMonitor(window);

    _lumInputWindowIconify(window, LUM_TRUE);
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
    if (window->monitor)
        acquireMonitor(window);

    _lumInputWindowIconify(window, LUM_FALSE);
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    if (_lum.ns.disabledCursorWindow == window)
        _lumCenterCursorInContentArea(window);

    _lumInputWindowFocus(window, LUM_TRUE);
    updateCursorMode(window);
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    if (window->monitor && window->autoIconify)
        _lumIconifyWindowCocoa(window);

    _lumInputWindowFocus(window, LUM_FALSE);
}

- (void)windowDidChangeOcclusionState:(NSNotification* )notification
{
    if ([window->ns.object occlusionState] & NSWindowOcclusionStateVisible)
        window->ns.occluded = LUM_FALSE;
    else
        window->ns.occluded = LUM_TRUE;
}

@end


//------------------------------------------------------------------------
// Content view class for the Lumina window
//------------------------------------------------------------------------

@interface LUMContentView : NSView <NSTextInputClient>
{
    _LUMwindow* window;
    NSTrackingArea* trackingArea;
    NSMutableAttributedString* markedText;
}

- (instancetype)initWithLumWindow:(_LUMwindow *)initWindow;

@end

@implementation LUMContentView

- (instancetype)initWithLumWindow:(_LUMwindow *)initWindow
{
    self = [super init];
    if (self != nil)
    {
        window = initWindow;
        trackingArea = nil;
        markedText = [[NSMutableAttributedString alloc] init];

        [self updateTrackingAreas];
        [self registerForDraggedTypes:@[NSPasteboardTypeURL]];
    }

    return self;
}

- (void)dealloc
{
    [trackingArea release];
    [markedText release];
    [super dealloc];
}

- (BOOL)isOpaque
{
    return [window->ns.object isOpaque];
}

- (BOOL)canBecomeKeyView
{
    return YES;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)wantsUpdateLayer
{
    return YES;
}

- (void)updateLayer
{
    if (window->context.source == LUM_NATIVE_CONTEXT_API)
        [window->context.nsgl.object update];

    _lumInputWindowDamage(window);
}

- (void)cursorUpdate:(NSEvent *)event
{
    updateCursorImage(window);
}

- (BOOL)acceptsFirstMouse:(NSEvent *)event
{
    return YES;
}

- (void)mouseDown:(NSEvent *)event
{
    _lumInputMouseClick(window,
                         LUM_MOUSE_BUTTON_LEFT,
                         LUM_PRESS,
                         translateFlags([event modifierFlags]));
}

- (void)mouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}

- (void)mouseUp:(NSEvent *)event
{
    _lumInputMouseClick(window,
                         LUM_MOUSE_BUTTON_LEFT,
                         LUM_RELEASE,
                         translateFlags([event modifierFlags]));
}

- (void)mouseMoved:(NSEvent *)event
{
    if (window->cursorMode == LUM_CURSOR_DISABLED)
    {
        const double dx = [event deltaX] - window->ns.cursorWarpDeltaX;
        const double dy = [event deltaY] - window->ns.cursorWarpDeltaY;

        _lumInputCursorPos(window,
                            window->virtualCursorPosX + dx,
                            window->virtualCursorPosY + dy);
    }
    else
    {
        const NSRect contentRect = [window->ns.view frame];
        // NOTE: The returned location uses base 0,1 not 0,0
        const NSPoint pos = [event locationInWindow];

        _lumInputCursorPos(window, pos.x, contentRect.size.height - pos.y);
    }

    window->ns.cursorWarpDeltaX = 0;
    window->ns.cursorWarpDeltaY = 0;
}

- (void)rightMouseDown:(NSEvent *)event
{
    _lumInputMouseClick(window,
                         LUM_MOUSE_BUTTON_RIGHT,
                         LUM_PRESS,
                         translateFlags([event modifierFlags]));
}

- (void)rightMouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}

- (void)rightMouseUp:(NSEvent *)event
{
    _lumInputMouseClick(window,
                         LUM_MOUSE_BUTTON_RIGHT,
                         LUM_RELEASE,
                         translateFlags([event modifierFlags]));
}

- (void)otherMouseDown:(NSEvent *)event
{
    _lumInputMouseClick(window,
                         (int) [event buttonNumber],
                         LUM_PRESS,
                         translateFlags([event modifierFlags]));
}

- (void)otherMouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}

- (void)otherMouseUp:(NSEvent *)event
{
    _lumInputMouseClick(window,
                         (int) [event buttonNumber],
                         LUM_RELEASE,
                         translateFlags([event modifierFlags]));
}

- (void)mouseExited:(NSEvent *)event
{
    if (window->cursorMode == LUM_CURSOR_HIDDEN)
        showCursor(window);

    _lumInputCursorEnter(window, LUM_FALSE);
}

- (void)mouseEntered:(NSEvent *)event
{
    if (window->cursorMode == LUM_CURSOR_HIDDEN)
        hideCursor(window);

    _lumInputCursorEnter(window, LUM_TRUE);
}

- (void)viewDidChangeBackingProperties
{
    const NSRect contentRect = [window->ns.view frame];
    const NSRect fbRect = [window->ns.view convertRectToBacking:contentRect];
    const float xscale = fbRect.size.width / contentRect.size.width;
    const float yscale = fbRect.size.height / contentRect.size.height;

    if (xscale != window->ns.xscale || yscale != window->ns.yscale)
    {
        if (window->ns.retina && window->ns.layer)
            [window->ns.layer setContentsScale:[window->ns.object backingScaleFactor]];

        window->ns.xscale = xscale;
        window->ns.yscale = yscale;
        _lumInputWindowContentScale(window, xscale, yscale);
    }

    if (fbRect.size.width != window->ns.fbWidth ||
        fbRect.size.height != window->ns.fbHeight)
    {
        window->ns.fbWidth  = fbRect.size.width;
        window->ns.fbHeight = fbRect.size.height;
        _lumInputFramebufferSize(window, fbRect.size.width, fbRect.size.height);
    }
}

- (void)drawRect:(NSRect)rect
{
    _lumInputWindowDamage(window);
}

- (void)updateTrackingAreas
{
    if (trackingArea != nil)
    {
        [self removeTrackingArea:trackingArea];
        [trackingArea release];
    }

    const NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited |
                                          NSTrackingActiveInKeyWindow |
                                          NSTrackingEnabledDuringMouseDrag |
                                          NSTrackingCursorUpdate |
                                          NSTrackingInVisibleRect |
                                          NSTrackingAssumeInside;

    trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds]
                                                options:options
                                                  owner:self
                                               userInfo:nil];

    [self addTrackingArea:trackingArea];
    [super updateTrackingAreas];
}

- (void)keyDown:(NSEvent *)event
{
    const int key = translateKey([event keyCode]);
    const int mods = translateFlags([event modifierFlags]);

    _lumInputKey(window, key, [event keyCode], LUM_PRESS, mods);

    [self interpretKeyEvents:@[event]];
}

- (void)flagsChanged:(NSEvent *)event
{
    int action;
    const unsigned int modifierFlags =
        [event modifierFlags] & NSEventModifierFlagDeviceIndependentFlagsMask;
    const int key = translateKey([event keyCode]);
    const int mods = translateFlags(modifierFlags);
    const NSUInteger keyFlag = translateKeyToModifierFlag(key);

    if (keyFlag & modifierFlags)
    {
        if (window->keys[key] == LUM_PRESS)
            action = LUM_RELEASE;
        else
            action = LUM_PRESS;
    }
    else
        action = LUM_RELEASE;

    _lumInputKey(window, key, [event keyCode], action, mods);
}

- (void)keyUp:(NSEvent *)event
{
    const int key = translateKey([event keyCode]);
    const int mods = translateFlags([event modifierFlags]);
    _lumInputKey(window, key, [event keyCode], LUM_RELEASE, mods);
}

- (void)scrollWheel:(NSEvent *)event
{
    double deltaX = [event scrollingDeltaX];
    double deltaY = [event scrollingDeltaY];

    if ([event hasPreciseScrollingDeltas])
    {
        deltaX *= 0.1;
        deltaY *= 0.1;
    }

    if (fabs(deltaX) > 0.0 || fabs(deltaY) > 0.0)
        _lumInputScroll(window, deltaX, deltaY);
}

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
    // HACK: We don't know what to say here because we don't know what the
    //       application wants to do with the paths
    return NSDragOperationGeneric;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
    const NSRect contentRect = [window->ns.view frame];
    // NOTE: The returned location uses base 0,1 not 0,0
    const NSPoint pos = [sender draggingLocation];
    _lumInputCursorPos(window, pos.x, contentRect.size.height - pos.y);

    NSPasteboard* pasteboard = [sender draggingPasteboard];
    NSDictionary* options = @{NSPasteboardURLReadingFileURLsOnlyKey:@YES};
    NSArray* urls = [pasteboard readObjectsForClasses:@[[NSURL class]]
                                              options:options];
    const NSUInteger count = [urls count];
    if (count)
    {
        char** paths = _lum_calloc(count, sizeof(char*));

        for (NSUInteger i = 0;  i < count;  i++)
            paths[i] = _lum_strdup([urls[i] fileSystemRepresentation]);

        _lumInputDrop(window, (int) count, (const char**) paths);

        for (NSUInteger i = 0;  i < count;  i++)
            _lum_free(paths[i]);
        _lum_free(paths);
    }

    return YES;
}

- (BOOL)hasMarkedText
{
    return [markedText length] > 0;
}

- (NSRange)markedRange
{
    if ([markedText length] > 0)
        return NSMakeRange(0, [markedText length] - 1);
    else
        return kEmptyRange;
}

- (NSRange)selectedRange
{
    return kEmptyRange;
}

- (void)setMarkedText:(id)string
        selectedRange:(NSRange)selectedRange
     replacementRange:(NSRange)replacementRange
{
    [markedText release];
    if ([string isKindOfClass:[NSAttributedString class]])
        markedText = [[NSMutableAttributedString alloc] initWithAttributedString:string];
    else
        markedText = [[NSMutableAttributedString alloc] initWithString:string];
}

- (void)unmarkText
{
    [[markedText mutableString] setString:@""];
}

- (NSArray*)validAttributesForMarkedText
{
    return [NSArray array];
}

- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range
                                               actualRange:(NSRangePointer)actualRange
{
    return nil;
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point
{
    return 0;
}

- (NSRect)firstRectForCharacterRange:(NSRange)range
                         actualRange:(NSRangePointer)actualRange
{
    const NSRect frame = [window->ns.view frame];
    return NSMakeRect(frame.origin.x, frame.origin.y, 0.0, 0.0);
}

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange
{
    NSString* characters;
    NSEvent* event = [NSApp currentEvent];
    const int mods = translateFlags([event modifierFlags]);
    const int plain = !(mods & LUM_MOD_SUPER);

    if ([string isKindOfClass:[NSAttributedString class]])
        characters = [string string];
    else
        characters = (NSString*) string;

    NSRange range = NSMakeRange(0, [characters length]);
    while (range.length)
    {
        uint32_t codepoint = 0;

        if ([characters getBytes:&codepoint
                       maxLength:sizeof(codepoint)
                      usedLength:NULL
                        encoding:NSUTF32StringEncoding
                         options:0
                           range:range
                  remainingRange:&range])
        {
            if (codepoint >= 0xf700 && codepoint <= 0xf7ff)
                continue;

            _lumInputChar(window, codepoint, mods, plain);
        }
    }
}

- (void)doCommandBySelector:(SEL)selector
{
}

@end


//------------------------------------------------------------------------
// Lumina window class
//------------------------------------------------------------------------

@interface LUMWindow : NSWindow {}
@end

@implementation LUMWindow

- (BOOL)canBecomeKeyWindow
{
    // Required for NSWindowStyleMaskBorderless windows
    return YES;
}

- (BOOL)canBecomeMainWindow
{
    return YES;
}

@end


// Create the Cocoa window
//
static LUMbool createNativeWindow(_LUMwindow* window,
                                   const _LUMwndconfig* wndconfig,
                                   const _LUMfbconfig* fbconfig)
{
    window->ns.delegate = [[LUMWindowDelegate alloc] initWithLumWindow:window];
    if (window->ns.delegate == nil)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Cocoa: Failed to create window delegate");
        return LUM_FALSE;
    }

    NSRect contentRect;

    if (window->monitor)
    {
        LUMvidmode mode;
        int xpos, ypos;

        _lumGetVideoModeCocoa(window->monitor, &mode);
        _lumGetMonitorPosCocoa(window->monitor, &xpos, &ypos);

        contentRect = NSMakeRect(xpos, ypos, mode.width, mode.height);
    }
    else
        contentRect = NSMakeRect(0, 0, wndconfig->width, wndconfig->height);

    window->ns.object = [[LUMWindow alloc]
        initWithContentRect:contentRect
                  styleMask:getStyleMask(window)
                    backing:NSBackingStoreBuffered
                      defer:NO];

    if (window->ns.object == nil)
    {
        _lumInputError(LUM_PLATFORM_ERROR, "Cocoa: Failed to create window");
        return LUM_FALSE;
    }

    if (window->monitor)
        [window->ns.object setLevel:NSMainMenuWindowLevel + 1];
    else
    {
        [(NSWindow*) window->ns.object center];
        _lum.ns.cascadePoint =
            NSPointToCGPoint([window->ns.object cascadeTopLeftFromPoint:
                              NSPointFromCGPoint(_lum.ns.cascadePoint)]);

        if (wndconfig->resizable)
        {
            const NSWindowCollectionBehavior behavior =
                NSWindowCollectionBehaviorFullScreenPrimary |
                NSWindowCollectionBehaviorManaged;
            [window->ns.object setCollectionBehavior:behavior];
        }

        if (wndconfig->floating)
            [window->ns.object setLevel:NSFloatingWindowLevel];

        if (wndconfig->maximized)
            [window->ns.object zoom:nil];
    }

    if (strlen(wndconfig->ns.frameName))
        [window->ns.object setFrameAutosaveName:@(wndconfig->ns.frameName)];

    window->ns.view = [[LUMContentView alloc] initWithLumWindow:window];
    window->ns.retina = wndconfig->ns.retina;

    if (fbconfig->transparent)
    {
        [window->ns.object setOpaque:NO];
        [window->ns.object setHasShadow:NO];
        [window->ns.object setBackgroundColor:[NSColor clearColor]];
    }

    [window->ns.object setContentView:window->ns.view];
    [window->ns.object makeFirstResponder:window->ns.view];
    [window->ns.object setTitle:@(wndconfig->title)];
    [window->ns.object setDelegate:window->ns.delegate];
    [window->ns.object setAcceptsMouseMovedEvents:YES];
    [window->ns.object setRestorable:NO];

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 101200
    if ([window->ns.object respondsToSelector:@selector(setTabbingMode:)])
        [window->ns.object setTabbingMode:NSWindowTabbingModeDisallowed];
#endif

    _lumGetWindowSizeCocoa(window, &window->ns.width, &window->ns.height);
    _lumGetFramebufferSizeCocoa(window, &window->ns.fbWidth, &window->ns.fbHeight);

    return LUM_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                       Lumina internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Transforms a y-coordinate between the CG display and NS screen spaces
//
float _lumTransformYCocoa(float y)
{
    return CGDisplayBounds(CGMainDisplayID()).size.height - y - 1;
}


//////////////////////////////////////////////////////////////////////////
//////                       Lumina platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _lumCreateWindowCocoa(_LUMwindow* window,
                           const _LUMwndconfig* wndconfig,
                           const _LUMctxconfig* ctxconfig,
                           const _LUMfbconfig* fbconfig)
{
    @autoreleasepool {

    if (!createNativeWindow(window, wndconfig, fbconfig))
        return LUM_FALSE;

    if (ctxconfig->client != LUM_NO_API)
    {
        if (ctxconfig->source == LUM_NATIVE_CONTEXT_API)
        {
            if (!_lumInitNSGL())
                return LUM_FALSE;
            if (!_lumCreateContextNSGL(window, ctxconfig, fbconfig))
                return LUM_FALSE;
        }
        else if (ctxconfig->source == LUM_EGL_CONTEXT_API)
        {
            // EGL implementation on macOS use CALayer* EGLNativeWindowType so we
            // need to get the layer for EGL window surface creation.
            [window->ns.view setWantsLayer:YES];
            window->ns.layer = [window->ns.view layer];

            if (!_lumInitEGL())
                return LUM_FALSE;
            if (!_lumCreateContextEGL(window, ctxconfig, fbconfig))
                return LUM_FALSE;
        }
        else if (ctxconfig->source == LUM_OSMESA_CONTEXT_API)
        {
            if (!_lumInitOSMesa())
                return LUM_FALSE;
            if (!_lumCreateContextOSMesa(window, ctxconfig, fbconfig))
                return LUM_FALSE;
        }
    }

    if (window->monitor)
    {
        _lumShowWindowCocoa(window);
        _lumFocusWindowCocoa(window);
        acquireMonitor(window);
    }

    return LUM_TRUE;

    } // autoreleasepool
}

void _lumDestroyWindowCocoa(_LUMwindow* window)
{
    @autoreleasepool {

    if (_lum.ns.disabledCursorWindow == window)
        _lum.ns.disabledCursorWindow = NULL;

    [window->ns.object orderOut:nil];

    if (window->monitor)
        releaseMonitor(window);

    if (window->context.destroy)
        window->context.destroy(window);

    [window->ns.object setDelegate:nil];
    [window->ns.delegate release];
    window->ns.delegate = nil;

    [window->ns.view release];
    window->ns.view = nil;

    [window->ns.object close];
    window->ns.object = nil;

    // HACK: Allow Cocoa to catch up before returning
    _lumPollEventsCocoa();

    } // autoreleasepool
}

void _lumSetWindowTitleCocoa(_LUMwindow* window, const char* title)
{
    @autoreleasepool {
    NSString* string = @(title);
    [window->ns.object setTitle:string];
    // HACK: Set the miniwindow title explicitly as setTitle: doesn't update it
    //       if the window lacks NSWindowStyleMaskTitled
    [window->ns.object setMiniwindowTitle:string];
    } // autoreleasepool
}

void _lumSetWindowIconCocoa(_LUMwindow* window,
                             int count, const LUMimage* images)
{
    _lumInputError(LUM_FEATURE_UNAVAILABLE,
                    "Cocoa: Regular windows do not have icons on macOS");
}

void _lumGetWindowPosCocoa(_LUMwindow* window, int* xpos, int* ypos)
{
    @autoreleasepool {

    const NSRect contentRect =
        [window->ns.object contentRectForFrameRect:[window->ns.object frame]];

    if (xpos)
        *xpos = contentRect.origin.x;
    if (ypos)
        *ypos = _lumTransformYCocoa(contentRect.origin.y + contentRect.size.height - 1);

    } // autoreleasepool
}

void _lumSetWindowPosCocoa(_LUMwindow* window, int x, int y)
{
    @autoreleasepool {

    const NSRect contentRect = [window->ns.view frame];
    const NSRect dummyRect = NSMakeRect(x, _lumTransformYCocoa(y + contentRect.size.height - 1), 0, 0);
    const NSRect frameRect = [window->ns.object frameRectForContentRect:dummyRect];
    [window->ns.object setFrameOrigin:frameRect.origin];

    } // autoreleasepool
}

void _lumGetWindowSizeCocoa(_LUMwindow* window, int* width, int* height)
{
    @autoreleasepool {

    const NSRect contentRect = [window->ns.view frame];

    if (width)
        *width = contentRect.size.width;
    if (height)
        *height = contentRect.size.height;

    } // autoreleasepool
}

void _lumSetWindowSizeCocoa(_LUMwindow* window, int width, int height)
{
    @autoreleasepool {

    if (window->monitor)
    {
        if (window->monitor->window == window)
            acquireMonitor(window);
    }
    else
    {
        NSRect contentRect =
            [window->ns.object contentRectForFrameRect:[window->ns.object frame]];
        contentRect.origin.y += contentRect.size.height - height;
        contentRect.size = NSMakeSize(width, height);
        [window->ns.object setFrame:[window->ns.object frameRectForContentRect:contentRect]
                            display:YES];
    }

    } // autoreleasepool
}

void _lumSetWindowSizeLimitsCocoa(_LUMwindow* window,
                                   int minwidth, int minheight,
                                   int maxwidth, int maxheight)
{
    @autoreleasepool {

    if (minwidth == LUM_DONT_CARE || minheight == LUM_DONT_CARE)
        [window->ns.object setContentMinSize:NSMakeSize(0, 0)];
    else
        [window->ns.object setContentMinSize:NSMakeSize(minwidth, minheight)];

    if (maxwidth == LUM_DONT_CARE || maxheight == LUM_DONT_CARE)
        [window->ns.object setContentMaxSize:NSMakeSize(DBL_MAX, DBL_MAX)];
    else
        [window->ns.object setContentMaxSize:NSMakeSize(maxwidth, maxheight)];

    } // autoreleasepool
}

void _lumSetWindowAspectRatioCocoa(_LUMwindow* window, int numer, int denom)
{
    @autoreleasepool {
    if (numer == LUM_DONT_CARE || denom == LUM_DONT_CARE)
        [window->ns.object setResizeIncrements:NSMakeSize(1.0, 1.0)];
    else
        [window->ns.object setContentAspectRatio:NSMakeSize(numer, denom)];
    } // autoreleasepool
}

void _lumGetFramebufferSizeCocoa(_LUMwindow* window, int* width, int* height)
{
    @autoreleasepool {

    const NSRect contentRect = [window->ns.view frame];
    const NSRect fbRect = [window->ns.view convertRectToBacking:contentRect];

    if (width)
        *width = (int) fbRect.size.width;
    if (height)
        *height = (int) fbRect.size.height;

    } // autoreleasepool
}

void _lumGetWindowFrameSizeCocoa(_LUMwindow* window,
                                  int* left, int* top,
                                  int* right, int* bottom)
{
    @autoreleasepool {

    const NSRect contentRect = [window->ns.view frame];
    const NSRect frameRect = [window->ns.object frameRectForContentRect:contentRect];

    if (left)
        *left = contentRect.origin.x - frameRect.origin.x;
    if (top)
        *top = frameRect.origin.y + frameRect.size.height -
               contentRect.origin.y - contentRect.size.height;
    if (right)
        *right = frameRect.origin.x + frameRect.size.width -
                 contentRect.origin.x - contentRect.size.width;
    if (bottom)
        *bottom = contentRect.origin.y - frameRect.origin.y;

    } // autoreleasepool
}

void _lumGetWindowContentScaleCocoa(_LUMwindow* window,
                                     float* xscale, float* yscale)
{
    @autoreleasepool {

    const NSRect points = [window->ns.view frame];
    const NSRect pixels = [window->ns.view convertRectToBacking:points];

    if (xscale)
        *xscale = (float) (pixels.size.width / points.size.width);
    if (yscale)
        *yscale = (float) (pixels.size.height / points.size.height);

    } // autoreleasepool
}

void _lumIconifyWindowCocoa(_LUMwindow* window)
{
    @autoreleasepool {
    [window->ns.object miniaturize:nil];
    } // autoreleasepool
}

void _lumRestoreWindowCocoa(_LUMwindow* window)
{
    @autoreleasepool {
    if ([window->ns.object isMiniaturized])
        [window->ns.object deminiaturize:nil];
    else if ([window->ns.object isZoomed])
        [window->ns.object zoom:nil];
    } // autoreleasepool
}

void _lumMaximizeWindowCocoa(_LUMwindow* window)
{
    @autoreleasepool {
    if (![window->ns.object isZoomed])
        [window->ns.object zoom:nil];
    } // autoreleasepool
}

void _lumShowWindowCocoa(_LUMwindow* window)
{
    @autoreleasepool {
    [window->ns.object orderFront:nil];
    } // autoreleasepool
}

void _lumHideWindowCocoa(_LUMwindow* window)
{
    @autoreleasepool {
    [window->ns.object orderOut:nil];
    } // autoreleasepool
}

void _lumRequestWindowAttentionCocoa(_LUMwindow* window)
{
    @autoreleasepool {
    [NSApp requestUserAttention:NSInformationalRequest];
    } // autoreleasepool
}

void _lumFocusWindowCocoa(_LUMwindow* window)
{
    @autoreleasepool {
    // Make us the active application
    // HACK: This is here to prevent applications using only hidden windows from
    //       being activated, but should probably not be done every time any
    //       window is shown
    [NSApp activateIgnoringOtherApps:YES];
    [window->ns.object makeKeyAndOrderFront:nil];
    } // autoreleasepool
}

void _lumSetWindowMonitorCocoa(_LUMwindow* window,
                                _LUMmonitor* monitor,
                                int xpos, int ypos,
                                int width, int height,
                                int refreshRate)
{
    @autoreleasepool {

    if (window->monitor == monitor)
    {
        if (monitor)
        {
            if (monitor->window == window)
                acquireMonitor(window);
        }
        else
        {
            const NSRect contentRect =
                NSMakeRect(xpos, _lumTransformYCocoa(ypos + height - 1), width, height);
            const NSRect frameRect =
                [window->ns.object frameRectForContentRect:contentRect
                                                 styleMask:getStyleMask(window)];

            [window->ns.object setFrame:frameRect display:YES];
        }

        return;
    }

    if (window->monitor)
        releaseMonitor(window);

    _lumInputWindowMonitor(window, monitor);

    // HACK: Allow the state cached in Cocoa to catch up to reality
    // TODO: Solve this in a less terrible way
    _lumPollEventsCocoa();

    const NSUInteger styleMask = getStyleMask(window);
    [window->ns.object setStyleMask:styleMask];
    // HACK: Changing the style mask can cause the first responder to be cleared
    [window->ns.object makeFirstResponder:window->ns.view];

    if (window->monitor)
    {
        [window->ns.object setLevel:NSMainMenuWindowLevel + 1];
        [window->ns.object setHasShadow:NO];

        acquireMonitor(window);
    }
    else
    {
        NSRect contentRect = NSMakeRect(xpos, _lumTransformYCocoa(ypos + height - 1),
                                        width, height);
        NSRect frameRect = [window->ns.object frameRectForContentRect:contentRect
                                                            styleMask:styleMask];
        [window->ns.object setFrame:frameRect display:YES];

        if (window->numer != LUM_DONT_CARE &&
            window->denom != LUM_DONT_CARE)
        {
            [window->ns.object setContentAspectRatio:NSMakeSize(window->numer,
                                                                window->denom)];
        }

        if (window->minwidth != LUM_DONT_CARE &&
            window->minheight != LUM_DONT_CARE)
        {
            [window->ns.object setContentMinSize:NSMakeSize(window->minwidth,
                                                            window->minheight)];
        }

        if (window->maxwidth != LUM_DONT_CARE &&
            window->maxheight != LUM_DONT_CARE)
        {
            [window->ns.object setContentMaxSize:NSMakeSize(window->maxwidth,
                                                            window->maxheight)];
        }

        if (window->floating)
            [window->ns.object setLevel:NSFloatingWindowLevel];
        else
            [window->ns.object setLevel:NSNormalWindowLevel];

        [window->ns.object setHasShadow:YES];
        // HACK: Clearing NSWindowStyleMaskTitled resets and disables the window
        //       title property but the miniwindow title property is unaffected
        [window->ns.object setTitle:[window->ns.object miniwindowTitle]];
    }

    } // autoreleasepool
}

int _lumWindowFocusedCocoa(_LUMwindow* window)
{
    @autoreleasepool {
    return [window->ns.object isKeyWindow];
    } // autoreleasepool
}

int _lumWindowIconifiedCocoa(_LUMwindow* window)
{
    @autoreleasepool {
    return [window->ns.object isMiniaturized];
    } // autoreleasepool
}

int _lumWindowVisibleCocoa(_LUMwindow* window)
{
    @autoreleasepool {
    return [window->ns.object isVisible];
    } // autoreleasepool
}

int _lumWindowMaximizedCocoa(_LUMwindow* window)
{
    @autoreleasepool {
    return [window->ns.object isZoomed];
    } // autoreleasepool
}

int _lumWindowHoveredCocoa(_LUMwindow* window)
{
    @autoreleasepool {

    const NSPoint point = [NSEvent mouseLocation];

    if ([NSWindow windowNumberAtPoint:point belowWindowWithWindowNumber:0] !=
        [window->ns.object windowNumber])
    {
        return LUM_FALSE;
    }

    return NSMouseInRect(point,
        [window->ns.object convertRectToScreen:[window->ns.view frame]], NO);

    } // autoreleasepool
}

int _lumFramebufferTransparentCocoa(_LUMwindow* window)
{
    @autoreleasepool {
    return ![window->ns.object isOpaque] && ![window->ns.view isOpaque];
    } // autoreleasepool
}

void _lumSetWindowResizableCocoa(_LUMwindow* window, LUMbool enabled)
{
    @autoreleasepool {
    [window->ns.object setStyleMask:getStyleMask(window)];
    } // autoreleasepool
}

void _lumSetWindowDecoratedCocoa(_LUMwindow* window, LUMbool enabled)
{
    @autoreleasepool {
    [window->ns.object setStyleMask:getStyleMask(window)];
    [window->ns.object makeFirstResponder:window->ns.view];
    } // autoreleasepool
}

void _lumPlatformSetWindowTitlebar(_LUMwindow* window, LUMbool enabled)
{
    // TODO
    _lumInputError(LUM_PLATFORM_ERROR,
        "Cocoa: Window attribute setting not implemented yet");
}

void _lumPlatformSetWindowFloating(_LUMwindow* window, LUMbool enabled)
{
    @autoreleasepool {
    if (enabled)
        [window->ns.object setLevel:NSFloatingWindowLevel];
    else
        [window->ns.object setLevel:NSNormalWindowLevel];
    } // autoreleasepool
}

void _lumSetWindowMousePassthroughCocoa(_LUMwindow* window, LUMbool enabled)
{
    @autoreleasepool {
    [window->ns.object setIgnoresMouseEvents:enabled];
    }
}

float _lumGetWindowOpacityCocoa(_LUMwindow* window)
{
    @autoreleasepool {
    return (float) [window->ns.object alphaValue];
    } // autoreleasepool
}

void _lumSetWindowOpacityCocoa(_LUMwindow* window, float opacity)
{
    @autoreleasepool {
    [window->ns.object setAlphaValue:opacity];
    } // autoreleasepool
}

void _lumSetRawMouseMotionCocoa(_LUMwindow *window, LUMbool enabled)
{
    _lumInputError(LUM_FEATURE_UNIMPLEMENTED,
                    "Cocoa: Raw mouse motion not yet implemented");
}

LUMbool _lumRawMouseMotionSupportedCocoa(void)
{
    return LUM_FALSE;
}

void _lumPollEventsCocoa(void)
{
    @autoreleasepool {

    for (;;)
    {
        NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                            untilDate:[NSDate distantPast]
                                               inMode:NSDefaultRunLoopMode
                                              dequeue:YES];
        if (event == nil)
            break;

        [NSApp sendEvent:event];
    }

    } // autoreleasepool
}

void _lumWaitEventsCocoa(void)
{
    @autoreleasepool {

    // I wanted to pass NO to dequeue:, and rely on PollEvents to
    // dequeue and send.  For reasons not at all clear to me, passing
    // NO to dequeue: causes this method never to return.
    NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:[NSDate distantFuture]
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];
    [NSApp sendEvent:event];

    _lumPollEventsCocoa();

    } // autoreleasepool
}

void _lumWaitEventsTimeoutCocoa(double timeout)
{
    @autoreleasepool {

    NSDate* date = [NSDate dateWithTimeIntervalSinceNow:timeout];
    NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:date
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];
    if (event)
        [NSApp sendEvent:event];

    _lumPollEventsCocoa();

    } // autoreleasepool
}

void _lumPostEmptyEventCocoa(void)
{
    @autoreleasepool {

    NSEvent* event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                        location:NSMakePoint(0, 0)
                                   modifierFlags:0
                                       timestamp:0
                                    windowNumber:0
                                         context:nil
                                         subtype:0
                                           data1:0
                                           data2:0];
    [NSApp postEvent:event atStart:YES];

    } // autoreleasepool
}

void _lumGetCursorPosCocoa(_LUMwindow* window, double* xpos, double* ypos)
{
    @autoreleasepool {

    const NSRect contentRect = [window->ns.view frame];
    // NOTE: The returned location uses base 0,1 not 0,0
    const NSPoint pos = [window->ns.object mouseLocationOutsideOfEventStream];

    if (xpos)
        *xpos = pos.x;
    if (ypos)
        *ypos = contentRect.size.height - pos.y;

    } // autoreleasepool
}

void _lumSetCursorPosCocoa(_LUMwindow* window, double x, double y)
{
    @autoreleasepool {

    updateCursorImage(window);

    const NSRect contentRect = [window->ns.view frame];
    // NOTE: The returned location uses base 0,1 not 0,0
    const NSPoint pos = [window->ns.object mouseLocationOutsideOfEventStream];

    window->ns.cursorWarpDeltaX += x - pos.x;
    window->ns.cursorWarpDeltaY += y - contentRect.size.height + pos.y;

    if (window->monitor)
    {
        CGDisplayMoveCursorToPoint(window->monitor->ns.displayID,
                                   CGPointMake(x, y));
    }
    else
    {
        const NSRect localRect = NSMakeRect(x, contentRect.size.height - y - 1, 0, 0);
        const NSRect globalRect = [window->ns.object convertRectToScreen:localRect];
        const NSPoint globalPoint = globalRect.origin;

        CGWarpMouseCursorPosition(CGPointMake(globalPoint.x,
                                              _lumTransformYCocoa(globalPoint.y)));
    }

    // HACK: Calling this right after setting the cursor position prevents macOS
    //       from freezing the cursor for a fraction of a second afterwards
    if (window->cursorMode != LUM_CURSOR_DISABLED)
        CGAssociateMouseAndMouseCursorPosition(true);

    } // autoreleasepool
}

void _lumSetCursorModeCocoa(_LUMwindow* window, int mode)
{
    @autoreleasepool {
    if (_lumWindowFocusedCocoa(window))
        updateCursorMode(window);
    } // autoreleasepool
}

const char* _lumGetScancodeNameCocoa(int scancode)
{
    @autoreleasepool {

    if (scancode < 0 || scancode > 0xff ||
        _lum.ns.keycodes[scancode] == LUM_KEY_UNKNOWN)
    {
        _lumInputError(LUM_INVALID_VALUE, "Invalid scancode %i", scancode);
        return NULL;
    }

    const int key = _lum.ns.keycodes[scancode];

    UInt32 deadKeyState = 0;
    UniChar characters[4];
    UniCharCount characterCount = 0;

    if (UCKeyTranslate([(NSData*) _lum.ns.unicodeData bytes],
                       scancode,
                       kUCKeyActionDisplay,
                       0,
                       LMGetKbdType(),
                       kUCKeyTranslateNoDeadKeysBit,
                       &deadKeyState,
                       sizeof(characters) / sizeof(characters[0]),
                       &characterCount,
                       characters) != noErr)
    {
        return NULL;
    }

    if (!characterCount)
        return NULL;

    CFStringRef string = CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault,
                                                            characters,
                                                            characterCount,
                                                            kCFAllocatorNull);
    CFStringGetCString(string,
                       _lum.ns.keynames[key],
                       sizeof(_lum.ns.keynames[key]),
                       kCFStringEncodingUTF8);
    CFRelease(string);

    return _lum.ns.keynames[key];

    } // autoreleasepool
}

int _lumGetKeyScancodeCocoa(int key)
{
    return _lum.ns.scancodes[key];
}

int _lumCreateCursorCocoa(_LUMcursor* cursor,
                           const LUMimage* image,
                           int xhot, int yhot)
{
    @autoreleasepool {

    NSImage* native;
    NSBitmapImageRep* rep;

    rep = [[NSBitmapImageRep alloc]
        initWithBitmapDataPlanes:NULL
                      pixelsWide:image->width
                      pixelsHigh:image->height
                   bitsPerSample:8
                 samplesPerPixel:4
                        hasAlpha:YES
                        isPlanar:NO
                  colorSpaceName:NSCalibratedRGBColorSpace
                    bitmapFormat:NSBitmapFormatAlphaNonpremultiplied
                     bytesPerRow:image->width * 4
                    bitsPerPixel:32];

    if (rep == nil)
        return LUM_FALSE;

    memcpy([rep bitmapData], image->pixels, image->width * image->height * 4);

    native = [[NSImage alloc] initWithSize:NSMakeSize(image->width, image->height)];
    [native addRepresentation:rep];

    cursor->ns.object = [[NSCursor alloc] initWithImage:native
                                                hotSpot:NSMakePoint(xhot, yhot)];

    [native release];
    [rep release];

    if (cursor->ns.object == nil)
        return LUM_FALSE;

    return LUM_TRUE;

    } // autoreleasepool
}

int _lumCreateStandardCursorCocoa(_LUMcursor* cursor, int shape)
{
    @autoreleasepool {

    SEL cursorSelector = NULL;

    // HACK: Try to use a private message
    switch (shape)
    {
        case LUM_RESIZE_EW_CURSOR:
            cursorSelector = NSSelectorFromString(@"_windowResizeEastWestCursor");
            break;
        case LUM_RESIZE_NS_CURSOR:
            cursorSelector = NSSelectorFromString(@"_windowResizeNorthSouthCursor");
            break;
        case LUM_RESIZE_NWSE_CURSOR:
            cursorSelector = NSSelectorFromString(@"_windowResizeNorthWestSouthEastCursor");
            break;
        case LUM_RESIZE_NESW_CURSOR:
            cursorSelector = NSSelectorFromString(@"_windowResizeNorthEastSouthWestCursor");
            break;
    }

    if (cursorSelector && [NSCursor respondsToSelector:cursorSelector])
    {
        id object = [NSCursor performSelector:cursorSelector];
        if ([object isKindOfClass:[NSCursor class]])
            cursor->ns.object = object;
    }

    if (!cursor->ns.object)
    {
        switch (shape)
        {
            case LUM_ARROW_CURSOR:
                cursor->ns.object = [NSCursor arrowCursor];
                break;
            case LUM_IBEAM_CURSOR:
                cursor->ns.object = [NSCursor IBeamCursor];
                break;
            case LUM_CROSSHAIR_CURSOR:
                cursor->ns.object = [NSCursor crosshairCursor];
                break;
            case LUM_POINTING_HAND_CURSOR:
                cursor->ns.object = [NSCursor pointingHandCursor];
                break;
            case LUM_RESIZE_EW_CURSOR:
                cursor->ns.object = [NSCursor resizeLeftRightCursor];
                break;
            case LUM_RESIZE_NS_CURSOR:
                cursor->ns.object = [NSCursor resizeUpDownCursor];
                break;
            case LUM_RESIZE_ALL_CURSOR:
                cursor->ns.object = [NSCursor closedHandCursor];
                break;
            case LUM_NOT_ALLOWED_CURSOR:
                cursor->ns.object = [NSCursor operationNotAllowedCursor];
                break;
        }
    }

    if (!cursor->ns.object)
    {
        _lumInputError(LUM_CURSOR_UNAVAILABLE,
                        "Cocoa: Standard cursor shape unavailable");
        return LUM_FALSE;
    }

    [cursor->ns.object retain];
    return LUM_TRUE;

    } // autoreleasepool
}

void _lumDestroyCursorCocoa(_LUMcursor* cursor)
{
    @autoreleasepool {
    if (cursor->ns.object)
        [(NSCursor*) cursor->ns.object release];
    } // autoreleasepool
}

void _lumSetCursorCocoa(_LUMwindow* window, _LUMcursor* cursor)
{
    @autoreleasepool {
    if (cursorInContentArea(window))
        updateCursorImage(window);
    } // autoreleasepool
}

void _lumSetClipboardStringCocoa(const char* string)
{
    @autoreleasepool {
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
    [pasteboard declareTypes:@[NSPasteboardTypeString] owner:nil];
    [pasteboard setString:@(string) forType:NSPasteboardTypeString];
    } // autoreleasepool
}

const char* _lumGetClipboardStringCocoa(void)
{
    @autoreleasepool {

    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];

    if (![[pasteboard types] containsObject:NSPasteboardTypeString])
    {
        _lumInputError(LUM_FORMAT_UNAVAILABLE,
                        "Cocoa: Failed to retrieve string from pasteboard");
        return NULL;
    }

    NSString* object = [pasteboard stringForType:NSPasteboardTypeString];
    if (!object)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Cocoa: Failed to retrieve object from pasteboard");
        return NULL;
    }

    _lum_free(_lum.ns.clipboardString);
    _lum.ns.clipboardString = _lum_strdup([object UTF8String]);

    return _lum.ns.clipboardString;

    } // autoreleasepool
}

EGLenum _lumGetEGLPlatformCocoa(EGLint** attribs)
{
    if (_lum.egl.ANGLE_platform_angle)
    {
        int type = 0;

        if (_lum.egl.ANGLE_platform_angle_opengl)
        {
            if (_lum.hints.init.angleType == LUM_ANGLE_PLATFORM_TYPE_OPENGL)
                type = EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE;
        }

        if (_lum.egl.ANGLE_platform_angle_metal)
        {
            if (_lum.hints.init.angleType == LUM_ANGLE_PLATFORM_TYPE_METAL)
                type = EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE;
        }

        if (type)
        {
            *attribs = _lum_calloc(3, sizeof(EGLint));
            (*attribs)[0] = EGL_PLATFORM_ANGLE_TYPE_ANGLE;
            (*attribs)[1] = type;
            (*attribs)[2] = EGL_NONE;
            return EGL_PLATFORM_ANGLE_ANGLE;
        }
    }

    return 0;
}

EGLNativeDisplayType _lumGetEGLNativeDisplayCocoa(void)
{
    return EGL_DEFAULT_DISPLAY;
}

EGLNativeWindowType _lumGetEGLNativeWindowCocoa(_LUMwindow* window)
{
    return window->ns.layer;
}

void _lumGetRequiredInstanceExtensionsCocoa(char** extensions)
{
    if (_lum.vk.KHR_surface && _lum.vk.EXT_metal_surface)
    {
        extensions[0] = "VK_KHR_surface";
        extensions[1] = "VK_EXT_metal_surface";
    }
    else if (_lum.vk.KHR_surface && _lum.vk.MVK_macos_surface)
    {
        extensions[0] = "VK_KHR_surface";
        extensions[1] = "VK_MVK_macos_surface";
    }
}

int _lumGetPhysicalDevicePresentationSupportCocoa(VkInstance instance,
                                                   VkPhysicalDevice device,
                                                   uint32_t queuefamily)
{
    return LUM_TRUE;
}

VkResult _lumCreateWindowSurfaceCocoa(VkInstance instance,
                                       _LUMwindow* window,
                                       const VkAllocationCallbacks* allocator,
                                       VkSurfaceKHR* surface)
{
    @autoreleasepool {

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 101100
    // HACK: Dynamically load Core Animation to avoid adding an extra
    //       dependency for the majority who don't use MoltenVK
    NSBundle* bundle = [NSBundle bundleWithPath:@"/System/Library/Frameworks/QuartzCore.framework"];
    if (!bundle)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Cocoa: Failed to find QuartzCore.framework");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    // NOTE: Create the layer here as makeBackingLayer should not return nil
    window->ns.layer = [[bundle classNamed:@"CAMetalLayer"] layer];
    if (!window->ns.layer)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Cocoa: Failed to create layer for view");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    if (window->ns.retina)
        [window->ns.layer setContentsScale:[window->ns.object backingScaleFactor]];

    [window->ns.view setLayer:window->ns.layer];
    [window->ns.view setWantsLayer:YES];

    VkResult err;

    if (_lum.vk.EXT_metal_surface)
    {
        VkMetalSurfaceCreateInfoEXT sci;

        PFN_vkCreateMetalSurfaceEXT vkCreateMetalSurfaceEXT;
        vkCreateMetalSurfaceEXT = (PFN_vkCreateMetalSurfaceEXT)
            vkGetInstanceProcAddr(instance, "vkCreateMetalSurfaceEXT");
        if (!vkCreateMetalSurfaceEXT)
        {
            _lumInputError(LUM_API_UNAVAILABLE,
                            "Cocoa: Vulkan instance missing VK_EXT_metal_surface extension");
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        memset(&sci, 0, sizeof(sci));
        sci.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
        sci.pLayer = window->ns.layer;

        err = vkCreateMetalSurfaceEXT(instance, &sci, allocator, surface);
    }
    else
    {
        VkMacOSSurfaceCreateInfoMVK sci;

        PFN_vkCreateMacOSSurfaceMVK vkCreateMacOSSurfaceMVK;
        vkCreateMacOSSurfaceMVK = (PFN_vkCreateMacOSSurfaceMVK)
            vkGetInstanceProcAddr(instance, "vkCreateMacOSSurfaceMVK");
        if (!vkCreateMacOSSurfaceMVK)
        {
            _lumInputError(LUM_API_UNAVAILABLE,
                            "Cocoa: Vulkan instance missing VK_MVK_macos_surface extension");
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        memset(&sci, 0, sizeof(sci));
        sci.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
        sci.pView = window->ns.view;

        err = vkCreateMacOSSurfaceMVK(instance, &sci, allocator, surface);
    }

    if (err)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Cocoa: Failed to create Vulkan surface: %s",
                        _lumGetVulkanResultString(err));
    }

    return err;
#else
    return VK_ERROR_EXTENSION_NOT_PRESENT;
#endif

    } // autoreleasepool
}


//////////////////////////////////////////////////////////////////////////
//////                        Lumina native API                       //////
//////////////////////////////////////////////////////////////////////////

LUMAPI id lumGetCocoaWindow(LUMwindow* handle)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    _LUM_REQUIRE_INIT_OR_RETURN(nil);

    if (_lum.platform.platformID != LUM_PLATFORM_COCOA)
    {
        _lumInputError(LUM_PLATFORM_UNAVAILABLE,
                        "Cocoa: Platform not initialized");
        return NULL;
    }

    return window->ns.object;
}

