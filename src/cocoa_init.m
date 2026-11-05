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
#include <sys/param.h> // For MAXPATHLEN

// Needed for _NSGetProgname
#include <crt_externs.h>

// Change to our application bundle's resources directory, if present
//
static void changeToResourcesDirectory(void)
{
    char resourcesPath[MAXPATHLEN];

    CFBundleRef bundle = CFBundleGetMainBundle();
    if (!bundle)
        return;

    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(bundle);

    CFStringRef last = CFURLCopyLastPathComponent(resourcesURL);
    if (CFStringCompare(CFSTR("Resources"), last, 0) != kCFCompareEqualTo)
    {
        CFRelease(last);
        CFRelease(resourcesURL);
        return;
    }

    CFRelease(last);

    if (!CFURLGetFileSystemRepresentation(resourcesURL,
                                          true,
                                          (UInt8*) resourcesPath,
                                          MAXPATHLEN))
    {
        CFRelease(resourcesURL);
        return;
    }

    CFRelease(resourcesURL);

    chdir(resourcesPath);
}

// Set up the menu bar (manually)
// This is nasty, nasty stuff -- calls to undocumented semi-private APIs that
// could go away at any moment, lots of stuff that really should be
// localize(d|able), etc.  Add a nib to save us this horror.
//
static void createMenuBar(void)
{
    NSString* appName = nil;
    NSDictionary* bundleInfo = [[NSBundle mainBundle] infoDictionary];
    NSString* nameKeys[] =
    {
        @"CFBundleDisplayName",
        @"CFBundleName",
        @"CFBundleExecutable",
    };

    // Try to figure out what the calling application is called

    for (size_t i = 0;  i < sizeof(nameKeys) / sizeof(nameKeys[0]);  i++)
    {
        id name = bundleInfo[nameKeys[i]];
        if (name &&
            [name isKindOfClass:[NSString class]] &&
            ![name isEqualToString:@""])
        {
            appName = name;
            break;
        }
    }

    if (!appName)
    {
        char** progname = _NSGetProgname();
        if (progname && *progname)
            appName = @(*progname);
        else
            appName = @"Lumina Application";
    }

    NSMenu* bar = [[NSMenu alloc] init];
    [NSApp setMainMenu:bar];

    NSMenuItem* appMenuItem =
        [bar addItemWithTitle:@"" action:NULL keyEquivalent:@""];
    NSMenu* appMenu = [[NSMenu alloc] init];
    [appMenuItem setSubmenu:appMenu];

    [appMenu addItemWithTitle:[NSString stringWithFormat:@"About %@", appName]
                       action:@selector(orderFrontStandardAboutPanel:)
                keyEquivalent:@""];
    [appMenu addItem:[NSMenuItem separatorItem]];
    NSMenu* servicesMenu = [[NSMenu alloc] init];
    [NSApp setServicesMenu:servicesMenu];
    [[appMenu addItemWithTitle:@"Services"
                       action:NULL
                keyEquivalent:@""] setSubmenu:servicesMenu];
    [servicesMenu release];
    [appMenu addItem:[NSMenuItem separatorItem]];
    [appMenu addItemWithTitle:[NSString stringWithFormat:@"Hide %@", appName]
                       action:@selector(hide:)
                keyEquivalent:@"h"];
    [[appMenu addItemWithTitle:@"Hide Others"
                       action:@selector(hideOtherApplications:)
                keyEquivalent:@"h"]
        setKeyEquivalentModifierMask:NSEventModifierFlagOption | NSEventModifierFlagCommand];
    [appMenu addItemWithTitle:@"Show All"
                       action:@selector(unhideAllApplications:)
                keyEquivalent:@""];
    [appMenu addItem:[NSMenuItem separatorItem]];
    [appMenu addItemWithTitle:[NSString stringWithFormat:@"Quit %@", appName]
                       action:@selector(terminate:)
                keyEquivalent:@"q"];

    NSMenuItem* windowMenuItem =
        [bar addItemWithTitle:@"" action:NULL keyEquivalent:@""];
    [bar release];
    NSMenu* windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
    [NSApp setWindowsMenu:windowMenu];
    [windowMenuItem setSubmenu:windowMenu];

    [windowMenu addItemWithTitle:@"Minimize"
                          action:@selector(performMiniaturize:)
                   keyEquivalent:@"m"];
    [windowMenu addItemWithTitle:@"Zoom"
                          action:@selector(performZoom:)
                   keyEquivalent:@""];
    [windowMenu addItem:[NSMenuItem separatorItem]];
    [windowMenu addItemWithTitle:@"Bring All to Front"
                          action:@selector(arrangeInFront:)
                   keyEquivalent:@""];

    // TODO: Make this appear at the bottom of the menu (for consistency)
    [windowMenu addItem:[NSMenuItem separatorItem]];
    [[windowMenu addItemWithTitle:@"Enter Full Screen"
                           action:@selector(toggleFullScreen:)
                    keyEquivalent:@"f"]
     setKeyEquivalentModifierMask:NSEventModifierFlagControl | NSEventModifierFlagCommand];

    // Prior to Snow Leopard, we need to use this oddly-named semi-private API
    // to get the application menu working properly.
    SEL setAppleMenuSelector = NSSelectorFromString(@"setAppleMenu:");
    [NSApp performSelector:setAppleMenuSelector withObject:appMenu];
}

// Create key code translation tables
//
static void createKeyTables(void)
{
    memset(_lum.ns.keycodes, -1, sizeof(_lum.ns.keycodes));
    memset(_lum.ns.scancodes, -1, sizeof(_lum.ns.scancodes));

    _lum.ns.keycodes[0x1D] = LUM_KEY_0;
    _lum.ns.keycodes[0x12] = LUM_KEY_1;
    _lum.ns.keycodes[0x13] = LUM_KEY_2;
    _lum.ns.keycodes[0x14] = LUM_KEY_3;
    _lum.ns.keycodes[0x15] = LUM_KEY_4;
    _lum.ns.keycodes[0x17] = LUM_KEY_5;
    _lum.ns.keycodes[0x16] = LUM_KEY_6;
    _lum.ns.keycodes[0x1A] = LUM_KEY_7;
    _lum.ns.keycodes[0x1C] = LUM_KEY_8;
    _lum.ns.keycodes[0x19] = LUM_KEY_9;
    _lum.ns.keycodes[0x00] = LUM_KEY_A;
    _lum.ns.keycodes[0x0B] = LUM_KEY_B;
    _lum.ns.keycodes[0x08] = LUM_KEY_C;
    _lum.ns.keycodes[0x02] = LUM_KEY_D;
    _lum.ns.keycodes[0x0E] = LUM_KEY_E;
    _lum.ns.keycodes[0x03] = LUM_KEY_F;
    _lum.ns.keycodes[0x05] = LUM_KEY_G;
    _lum.ns.keycodes[0x04] = LUM_KEY_H;
    _lum.ns.keycodes[0x22] = LUM_KEY_I;
    _lum.ns.keycodes[0x26] = LUM_KEY_J;
    _lum.ns.keycodes[0x28] = LUM_KEY_K;
    _lum.ns.keycodes[0x25] = LUM_KEY_L;
    _lum.ns.keycodes[0x2E] = LUM_KEY_M;
    _lum.ns.keycodes[0x2D] = LUM_KEY_N;
    _lum.ns.keycodes[0x1F] = LUM_KEY_O;
    _lum.ns.keycodes[0x23] = LUM_KEY_P;
    _lum.ns.keycodes[0x0C] = LUM_KEY_Q;
    _lum.ns.keycodes[0x0F] = LUM_KEY_R;
    _lum.ns.keycodes[0x01] = LUM_KEY_S;
    _lum.ns.keycodes[0x11] = LUM_KEY_T;
    _lum.ns.keycodes[0x20] = LUM_KEY_U;
    _lum.ns.keycodes[0x09] = LUM_KEY_V;
    _lum.ns.keycodes[0x0D] = LUM_KEY_W;
    _lum.ns.keycodes[0x07] = LUM_KEY_X;
    _lum.ns.keycodes[0x10] = LUM_KEY_Y;
    _lum.ns.keycodes[0x06] = LUM_KEY_Z;

    _lum.ns.keycodes[0x27] = LUM_KEY_APOSTROPHE;
    _lum.ns.keycodes[0x2A] = LUM_KEY_BACKSLASH;
    _lum.ns.keycodes[0x2B] = LUM_KEY_COMMA;
    _lum.ns.keycodes[0x18] = LUM_KEY_EQUAL;
    _lum.ns.keycodes[0x32] = LUM_KEY_GRAVE_ACCENT;
    _lum.ns.keycodes[0x21] = LUM_KEY_LEFT_BRACKET;
    _lum.ns.keycodes[0x1B] = LUM_KEY_MINUS;
    _lum.ns.keycodes[0x2F] = LUM_KEY_PERIOD;
    _lum.ns.keycodes[0x1E] = LUM_KEY_RIGHT_BRACKET;
    _lum.ns.keycodes[0x29] = LUM_KEY_SEMICOLON;
    _lum.ns.keycodes[0x2C] = LUM_KEY_SLASH;
    _lum.ns.keycodes[0x0A] = LUM_KEY_WORLD_1;

    _lum.ns.keycodes[0x33] = LUM_KEY_BACKSPACE;
    _lum.ns.keycodes[0x39] = LUM_KEY_CAPS_LOCK;
    _lum.ns.keycodes[0x75] = LUM_KEY_DELETE;
    _lum.ns.keycodes[0x7D] = LUM_KEY_DOWN;
    _lum.ns.keycodes[0x77] = LUM_KEY_END;
    _lum.ns.keycodes[0x24] = LUM_KEY_ENTER;
    _lum.ns.keycodes[0x35] = LUM_KEY_ESCAPE;
    _lum.ns.keycodes[0x7A] = LUM_KEY_F1;
    _lum.ns.keycodes[0x78] = LUM_KEY_F2;
    _lum.ns.keycodes[0x63] = LUM_KEY_F3;
    _lum.ns.keycodes[0x76] = LUM_KEY_F4;
    _lum.ns.keycodes[0x60] = LUM_KEY_F5;
    _lum.ns.keycodes[0x61] = LUM_KEY_F6;
    _lum.ns.keycodes[0x62] = LUM_KEY_F7;
    _lum.ns.keycodes[0x64] = LUM_KEY_F8;
    _lum.ns.keycodes[0x65] = LUM_KEY_F9;
    _lum.ns.keycodes[0x6D] = LUM_KEY_F10;
    _lum.ns.keycodes[0x67] = LUM_KEY_F11;
    _lum.ns.keycodes[0x6F] = LUM_KEY_F12;
    _lum.ns.keycodes[0x69] = LUM_KEY_PRINT_SCREEN;
    _lum.ns.keycodes[0x6B] = LUM_KEY_F14;
    _lum.ns.keycodes[0x71] = LUM_KEY_F15;
    _lum.ns.keycodes[0x6A] = LUM_KEY_F16;
    _lum.ns.keycodes[0x40] = LUM_KEY_F17;
    _lum.ns.keycodes[0x4F] = LUM_KEY_F18;
    _lum.ns.keycodes[0x50] = LUM_KEY_F19;
    _lum.ns.keycodes[0x5A] = LUM_KEY_F20;
    _lum.ns.keycodes[0x73] = LUM_KEY_HOME;
    _lum.ns.keycodes[0x72] = LUM_KEY_INSERT;
    _lum.ns.keycodes[0x7B] = LUM_KEY_LEFT;
    _lum.ns.keycodes[0x3A] = LUM_KEY_LEFT_ALT;
    _lum.ns.keycodes[0x3B] = LUM_KEY_LEFT_CONTROL;
    _lum.ns.keycodes[0x38] = LUM_KEY_LEFT_SHIFT;
    _lum.ns.keycodes[0x37] = LUM_KEY_LEFT_SUPER;
    _lum.ns.keycodes[0x6E] = LUM_KEY_MENU;
    _lum.ns.keycodes[0x47] = LUM_KEY_NUM_LOCK;
    _lum.ns.keycodes[0x79] = LUM_KEY_PAGE_DOWN;
    _lum.ns.keycodes[0x74] = LUM_KEY_PAGE_UP;
    _lum.ns.keycodes[0x7C] = LUM_KEY_RIGHT;
    _lum.ns.keycodes[0x3D] = LUM_KEY_RIGHT_ALT;
    _lum.ns.keycodes[0x3E] = LUM_KEY_RIGHT_CONTROL;
    _lum.ns.keycodes[0x3C] = LUM_KEY_RIGHT_SHIFT;
    _lum.ns.keycodes[0x36] = LUM_KEY_RIGHT_SUPER;
    _lum.ns.keycodes[0x31] = LUM_KEY_SPACE;
    _lum.ns.keycodes[0x30] = LUM_KEY_TAB;
    _lum.ns.keycodes[0x7E] = LUM_KEY_UP;

    _lum.ns.keycodes[0x52] = LUM_KEY_KP_0;
    _lum.ns.keycodes[0x53] = LUM_KEY_KP_1;
    _lum.ns.keycodes[0x54] = LUM_KEY_KP_2;
    _lum.ns.keycodes[0x55] = LUM_KEY_KP_3;
    _lum.ns.keycodes[0x56] = LUM_KEY_KP_4;
    _lum.ns.keycodes[0x57] = LUM_KEY_KP_5;
    _lum.ns.keycodes[0x58] = LUM_KEY_KP_6;
    _lum.ns.keycodes[0x59] = LUM_KEY_KP_7;
    _lum.ns.keycodes[0x5B] = LUM_KEY_KP_8;
    _lum.ns.keycodes[0x5C] = LUM_KEY_KP_9;
    _lum.ns.keycodes[0x45] = LUM_KEY_KP_ADD;
    _lum.ns.keycodes[0x41] = LUM_KEY_KP_DECIMAL;
    _lum.ns.keycodes[0x4B] = LUM_KEY_KP_DIVIDE;
    _lum.ns.keycodes[0x4C] = LUM_KEY_KP_ENTER;
    _lum.ns.keycodes[0x51] = LUM_KEY_KP_EQUAL;
    _lum.ns.keycodes[0x43] = LUM_KEY_KP_MULTIPLY;
    _lum.ns.keycodes[0x4E] = LUM_KEY_KP_SUBTRACT;

    for (int scancode = 0;  scancode < 256;  scancode++)
    {
        // Store the reverse translation for faster key name lookup
        if (_lum.ns.keycodes[scancode] >= 0)
            _lum.ns.scancodes[_lum.ns.keycodes[scancode]] = scancode;
    }
}

// Retrieve Unicode data for the current keyboard layout
//
static LUMbool updateUnicodeData(void)
{
    if (_lum.ns.inputSource)
    {
        CFRelease(_lum.ns.inputSource);
        _lum.ns.inputSource = NULL;
        _lum.ns.unicodeData = nil;
    }

    _lum.ns.inputSource = TISCopyCurrentKeyboardLayoutInputSource();
    if (!_lum.ns.inputSource)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Cocoa: Failed to retrieve keyboard layout input source");
        return LUM_FALSE;
    }

    _lum.ns.unicodeData =
        TISGetInputSourceProperty(_lum.ns.inputSource,
                                  kTISPropertyUnicodeKeyLayoutData);
    if (!_lum.ns.unicodeData)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Cocoa: Failed to retrieve keyboard layout Unicode data");
        return LUM_FALSE;
    }

    return LUM_TRUE;
}

// Load HIToolbox.framework and the TIS symbols we need from it
//
static LUMbool initializeTIS(void)
{
    // This works only because Cocoa has already loaded it properly
    _lum.ns.tis.bundle =
        CFBundleGetBundleWithIdentifier(CFSTR("com.apple.HIToolbox"));
    if (!_lum.ns.tis.bundle)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Cocoa: Failed to load HIToolbox.framework");
        return LUM_FALSE;
    }

    CFStringRef* kPropertyUnicodeKeyLayoutData =
        CFBundleGetDataPointerForName(_lum.ns.tis.bundle,
                                      CFSTR("kTISPropertyUnicodeKeyLayoutData"));
    _lum.ns.tis.CopyCurrentKeyboardLayoutInputSource =
        CFBundleGetFunctionPointerForName(_lum.ns.tis.bundle,
                                          CFSTR("TISCopyCurrentKeyboardLayoutInputSource"));
    _lum.ns.tis.GetInputSourceProperty =
        CFBundleGetFunctionPointerForName(_lum.ns.tis.bundle,
                                          CFSTR("TISGetInputSourceProperty"));
    _lum.ns.tis.GetKbdType =
        CFBundleGetFunctionPointerForName(_lum.ns.tis.bundle,
                                          CFSTR("LMGetKbdType"));

    if (!kPropertyUnicodeKeyLayoutData ||
        !TISCopyCurrentKeyboardLayoutInputSource ||
        !TISGetInputSourceProperty ||
        !LMGetKbdType)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Cocoa: Failed to load TIS API symbols");
        return LUM_FALSE;
    }

    _lum.ns.tis.kPropertyUnicodeKeyLayoutData =
        *kPropertyUnicodeKeyLayoutData;

    return updateUnicodeData();
}

@interface LUMHelper : NSObject
@end

@implementation LUMHelper

- (void)selectedKeyboardInputSourceChanged:(NSObject* )object
{
    updateUnicodeData();
}

- (void)doNothing:(id)object
{
}

@end // LUMHelper

@interface LUMApplicationDelegate : NSObject <NSApplicationDelegate>
@end

@implementation LUMApplicationDelegate

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    for (_LUMwindow* window = _lum.windowListHead;  window;  window = window->next)
        _lumInputWindowCloseRequest(window);

    return NSTerminateCancel;
}

- (void)applicationDidChangeScreenParameters:(NSNotification *) notification
{
    for (_LUMwindow* window = _lum.windowListHead;  window;  window = window->next)
    {
        if (window->context.client != LUM_NO_API)
            [window->context.nsgl.object update];
    }

    _lumPollMonitorsCocoa();
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification
{
    if (_lum.hints.init.ns.menubar)
    {
        // Menu bar setup must go between sharedApplication and finishLaunching
        // in order to properly emulate the behavior of NSApplicationMain

        if ([[NSBundle mainBundle] pathForResource:@"MainMenu" ofType:@"nib"])
        {
            [[NSBundle mainBundle] loadNibNamed:@"MainMenu"
                                          owner:NSApp
                                topLevelObjects:&_lum.ns.nibObjects];
        }
        else
            createMenuBar();
    }
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    _lumPostEmptyEventCocoa();
    [NSApp stop:nil];
}

- (void)applicationDidHide:(NSNotification *)notification
{
    for (int i = 0;  i < _lum.monitorCount;  i++)
        _lumRestoreVideoModeCocoa(_lum.monitors[i]);
}

@end // LUMApplicationDelegate


//////////////////////////////////////////////////////////////////////////
//////                       Lumina internal API                      //////
//////////////////////////////////////////////////////////////////////////

void* _lumLoadLocalVulkanLoaderCocoa(void)
{
    CFBundleRef bundle = CFBundleGetMainBundle();
    if (!bundle)
        return NULL;

    CFURLRef url =
        CFBundleCopyAuxiliaryExecutableURL(bundle, CFSTR("libvulkan.1.dylib"));
    if (!url)
        return NULL;

    char path[PATH_MAX];
    void* handle = NULL;

    if (CFURLGetFileSystemRepresentation(url, true, (UInt8*) path, sizeof(path) - 1))
        handle = _lumPlatformLoadModule(path);

    CFRelease(url);
    return handle;
}


//////////////////////////////////////////////////////////////////////////
//////                       Lumina platform API                      //////
//////////////////////////////////////////////////////////////////////////

LUMbool _lumConnectCocoa(int platformID, _LUMplatform* platform)
{
    const _LUMplatform cocoa =
    {
        LUM_PLATFORM_COCOA,
        _lumInitCocoa,
        _lumTerminateCocoa,
        _lumGetCursorPosCocoa,
        _lumSetCursorPosCocoa,
        _lumSetCursorModeCocoa,
        _lumSetRawMouseMotionCocoa,
        _lumRawMouseMotionSupportedCocoa,
        _lumCreateCursorCocoa,
        _lumCreateStandardCursorCocoa,
        _lumDestroyCursorCocoa,
        _lumSetCursorCocoa,
        _lumGetScancodeNameCocoa,
        _lumGetKeyScancodeCocoa,
        _lumSetClipboardStringCocoa,
        _lumGetClipboardStringCocoa,
        _lumInitJoysticksCocoa,
        _lumTerminateJoysticksCocoa,
        _lumPollJoystickCocoa,
        _lumGetMappingNameCocoa,
        _lumUpdateGamepadGUIDCocoa,
        _lumFreeMonitorCocoa,
        _lumGetMonitorPosCocoa,
        _lumGetMonitorContentScaleCocoa,
        _lumGetMonitorWorkareaCocoa,
        _lumGetVideoModesCocoa,
        _lumGetVideoModeCocoa,
        _lumGetGammaRampCocoa,
        _lumSetGammaRampCocoa,
        _lumCreateWindowCocoa,
        _lumDestroyWindowCocoa,
        _lumSetWindowTitleCocoa,
        _lumSetWindowIconCocoa,
        _lumGetWindowPosCocoa,
        _lumSetWindowPosCocoa,
        _lumGetWindowSizeCocoa,
        _lumSetWindowSizeCocoa,
        _lumSetWindowSizeLimitsCocoa,
        _lumSetWindowAspectRatioCocoa,
        _lumGetFramebufferSizeCocoa,
        _lumGetWindowFrameSizeCocoa,
        _lumGetWindowContentScaleCocoa,
        _lumIconifyWindowCocoa,
        _lumRestoreWindowCocoa,
        _lumMaximizeWindowCocoa,
        _lumShowWindowCocoa,
        _lumHideWindowCocoa,
        _lumRequestWindowAttentionCocoa,
        _lumFocusWindowCocoa,
        _lumSetWindowMonitorCocoa,
        _lumWindowFocusedCocoa,
        _lumWindowIconifiedCocoa,
        _lumWindowVisibleCocoa,
        _lumWindowMaximizedCocoa,
        _lumWindowHoveredCocoa,
        _lumFramebufferTransparentCocoa,
        _lumGetWindowOpacityCocoa,
        _lumSetWindowResizableCocoa,
        _lumSetWindowDecoratedCocoa,
        _lumSetWindowFloatingNull,
        _lumSetWindowOpacityCocoa,
        _lumSetWindowMousePassthroughCocoa,
        _lumPollEventsCocoa,
        _lumWaitEventsCocoa,
        _lumWaitEventsTimeoutCocoa,
        _lumPostEmptyEventCocoa,

        // Hazel
        _lumSetWindowTitlebarNull,

        _lumGetEGLPlatformCocoa,
        _lumGetEGLNativeDisplayCocoa,
        _lumGetEGLNativeWindowCocoa,
        _lumGetRequiredInstanceExtensionsCocoa,
        _lumGetPhysicalDevicePresentationSupportCocoa,
        _lumCreateWindowSurfaceCocoa,
    };

    *platform = cocoa;
    return LUM_TRUE;
}

int _lumInitCocoa(void)
{
    @autoreleasepool {

    _lum.ns.helper = [[LUMHelper alloc] init];

    [NSThread detachNewThreadSelector:@selector(doNothing:)
                             toTarget:_lum.ns.helper
                           withObject:nil];

    [NSApplication sharedApplication];

    _lum.ns.delegate = [[LUMApplicationDelegate alloc] init];
    if (_lum.ns.delegate == nil)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Cocoa: Failed to create application delegate");
        return LUM_FALSE;
    }

    [NSApp setDelegate:_lum.ns.delegate];

    NSEvent* (^block)(NSEvent*) = ^ NSEvent* (NSEvent* event)
    {
        if ([event modifierFlags] & NSEventModifierFlagCommand)
            [[NSApp keyWindow] sendEvent:event];

        return event;
    };

    _lum.ns.keyUpMonitor =
        [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskKeyUp
                                              handler:block];

    if (_lum.hints.init.ns.chdir)
        changeToResourcesDirectory();

    // Press and Hold prevents some keys from emitting repeated characters
    NSDictionary* defaults = @{@"ApplePressAndHoldEnabled":@NO};
    [[NSUserDefaults standardUserDefaults] registerDefaults:defaults];

    [[NSNotificationCenter defaultCenter]
        addObserver:_lum.ns.helper
           selector:@selector(selectedKeyboardInputSourceChanged:)
               name:NSTextInputContextKeyboardSelectionDidChangeNotification
             object:nil];

    createKeyTables();

    _lum.ns.eventSource = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
    if (!_lum.ns.eventSource)
        return LUM_FALSE;

    CGEventSourceSetLocalEventsSuppressionInterval(_lum.ns.eventSource, 0.0);

    if (!initializeTIS())
        return LUM_FALSE;

    _lumPollMonitorsCocoa();

    if (![[NSRunningApplication currentApplication] isFinishedLaunching])
        [NSApp run];

    // In case we are unbundled, make us a proper UI application
    if (_lum.hints.init.ns.menubar)
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    return LUM_TRUE;

    } // autoreleasepool
}

void _lumTerminateCocoa(void)
{
    @autoreleasepool {

    if (_lum.ns.inputSource)
    {
        CFRelease(_lum.ns.inputSource);
        _lum.ns.inputSource = NULL;
        _lum.ns.unicodeData = nil;
    }

    if (_lum.ns.eventSource)
    {
        CFRelease(_lum.ns.eventSource);
        _lum.ns.eventSource = NULL;
    }

    if (_lum.ns.delegate)
    {
        [NSApp setDelegate:nil];
        [_lum.ns.delegate release];
        _lum.ns.delegate = nil;
    }

    if (_lum.ns.helper)
    {
        [[NSNotificationCenter defaultCenter]
            removeObserver:_lum.ns.helper
                      name:NSTextInputContextKeyboardSelectionDidChangeNotification
                    object:nil];
        [[NSNotificationCenter defaultCenter]
            removeObserver:_lum.ns.helper];
        [_lum.ns.helper release];
        _lum.ns.helper = nil;
    }

    if (_lum.ns.keyUpMonitor)
        [NSEvent removeMonitor:_lum.ns.keyUpMonitor];

    _lum_free(_lum.ns.clipboardString);

    _lumTerminateNSGL();

    } // autoreleasepool
}

