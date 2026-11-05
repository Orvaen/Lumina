// Platform notes and other remarks.

export default [
  [
    "@macos This function will change the current directory of the application to the `Contents/Resources` subdirectory of the application's bundle, if present. This can be disabled with the @ref LUM_COCOA_CHDIR_RESOURCES init hint.",
    "@macos Cette fonction place le répertoire courant de l'application dans le sous-répertoire `Contents/Resources` de son bundle, lorsqu'il existe.  Cela se désactive avec l'indice d'initialisation @ref LUM_COCOA_CHDIR_RESOURCES.",
  ],
  [
    "@macos This function will create the main menu and dock icon for the application. If Lumina finds a `MainMenu.nib` it is loaded and assumed to contain a menu bar. Otherwise a minimal menu bar is created manually with common commands like Hide, Quit and About. The About entry opens a minimal about dialog with information from the application's bundle. The menu bar and dock icon can be disabled entirely with the @ref LUM_COCOA_MENUBAR init hint.",
    "@macos Cette fonction crée le menu principal et l'icône du dock de l'application.  Si Lumina trouve un `MainMenu.nib`, il est chargé et supposé contenir une barre de menus.  Sinon, une barre minimale est créée à la main, avec les commandes courantes Masquer, Quitter et À propos.  L'entrée À propos ouvre une fenêtre minimale reprenant les informations du bundle de l'application.  La barre de menus et l'icône du dock se désactivent entièrement avec l'indice d'initialisation @ref LUM_COCOA_MENUBAR.",
  ],
  [
    '@x11 This function will set the `LC_CTYPE` category of the application locale according to the current environment if that category is still "C". This is because the "C" locale breaks Unicode text input.',
    "@x11 Cette fonction règle la catégorie `LC_CTYPE` de la locale de l'application d'après l'environnement courant si cette catégorie vaut encore « C ».  La locale « C » casse en effet la saisie de texte Unicode.",
  ],
  [
    'The primary monitor is always first in the array returned by @ref lumGetMonitors.',
    "L'écran principal est toujours le premier du tableau retourné par @ref lumGetMonitors.",
  ],
  [
    '@win32 On Windows 8 and earlier the physical size is calculated from the current resolution and system DPI instead of querying the monitor EDID data.',
    '@win32 Sous Windows 8 et antérieurs, la taille physique est calculée à partir de la résolution courante et du DPI système, au lieu de lire les données EDID de l\'écran.',
  ],
  [
    '@wayland Gamma handling is a privileged protocol, this function will thus never be implemented and emits @ref LUM_PLATFORM_ERROR while returning `NULL`.',
    "@wayland La gestion du gamma relève d'un protocole privilégié : cette fonction ne sera donc jamais implémentée ; elle émet @ref LUM_PLATFORM_ERROR et retourne `NULL`.",
  ],
  [
    'The size of the specified gamma ramp should match the size of the current ramp for that monitor.',
    'La taille de la rampe gamma indiquée devrait correspondre à celle de la rampe courante de cet écran.',
  ],
  ['@win32 The gamma ramp size must be 256.', '@win32 La taille de la rampe gamma doit être 256.'],
  [
    '@win32 Window creation will fail if the Microsoft GDI software OpenGL implementation is the only one available.',
    "@win32 La création de la fenêtre échouera si l'implémentation OpenGL logicielle GDI de Microsoft est la seule disponible.",
  ],
  [
    '@win32 If the executable has an icon resource named `LUM_ICON,` it will be set as the initial icon for the window. If no such icon is present, the `IDI_APPLICATION` icon will be used instead. To set a different icon, see @ref lumSetWindowIcon.',
    "@win32 Si l'exécutable possède une ressource d'icône nommée `LUM_ICON,`, elle devient l'icône initiale de la fenêtre.  À défaut, l'icône `IDI_APPLICATION` est utilisée.  Pour en poser une autre, voyez @ref lumSetWindowIcon.",
  ],
  [
    '@win32 The context to share resources with must not be current on any other thread.',
    '@win32 Le contexte avec lequel partager les ressources ne doit être courant sur aucun autre thread.',
  ],
  [
    '@macos The OS only supports core profile contexts for OpenGL versions 3.2 and later. Before creating an OpenGL context of version 3.2 or later you must set the [LUM_OPENGL_PROFILE](@ref LUM_OPENGL_PROFILE_hint) hint accordingly. OpenGL 3.0 and 3.1 contexts are not supported at all on macOS.',
    "@macos Le système ne prend en charge que les contextes à profil core pour OpenGL 3.2 et au-delà.  Avant de créer un contexte OpenGL 3.2 ou ultérieur, vous devez positionner l'indice [LUM_OPENGL_PROFILE](@ref LUM_OPENGL_PROFILE_hint) en conséquence.  Les contextes OpenGL 3.0 et 3.1 ne sont pas du tout pris en charge sous macOS.",
  ],
  [
    "@macos The Lumina window has no icon, as it is not a document window, but the dock icon will be the same as the application bundle's icon. For more information on bundles, see the [Bundle Programming Guide](https://developer.apple.com/library/mac/documentation/CoreFoundation/Conceptual/CFBundles/) in the Mac Developer Library.",
    "@macos La fenêtre Lumina n'a pas d'icône, n'étant pas une fenêtre de document, mais l'icône du dock sera celle du bundle de l'application.  Pour en savoir plus sur les bundles, voyez le [Bundle Programming Guide](https://developer.apple.com/library/mac/documentation/CoreFoundation/Conceptual/CFBundles/) de la Mac Developer Library.",
  ],
  [
    "@macos On OS X 10.10 and later the window frame will not be rendered at full resolution on Retina displays unless the [LUM_COCOA_RETINA_FRAMEBUFFER](@ref LUM_COCOA_RETINA_FRAMEBUFFER_hint) hint is `LUM_TRUE` and the `NSHighResolutionCapable` key is enabled in the application bundle's `Info.plist`. For more information, see [High Resolution Guidelines for OS X](https://developer.apple.com/library/mac/documentation/GraphicsAnimation/Conceptual/HighResolutionOSX/Explained/Explained.html) in the Mac Developer Library. The Lumina test and example programs use a custom `Info.plist` template for this, which can be found as `CMake/Info.plist.in` in the source tree.",
    "@macos Sous OS X 10.10 et au-delà, le cadre de la fenêtre n'est pas rendu en pleine résolution sur les écrans Retina, sauf si l'indice [LUM_COCOA_RETINA_FRAMEBUFFER](@ref LUM_COCOA_RETINA_FRAMEBUFFER_hint) vaut `LUM_TRUE` et que la clé `NSHighResolutionCapable` est activée dans l'`Info.plist` du bundle.  Pour en savoir plus, voyez les [High Resolution Guidelines for OS X](https://developer.apple.com/library/mac/documentation/GraphicsAnimation/Conceptual/HighResolutionOSX/Explained/Explained.html) de la Mac Developer Library.  Les programmes de test et d'exemple de Lumina emploient pour cela un modèle d'`Info.plist` dédié, dans `CMake/Info.plist.in` de l'arborescence des sources.",
  ],
  [
    '@macos When activating frame autosaving with [LUM_COCOA_FRAME_NAME](@ref LUM_COCOA_FRAME_NAME_hint), the specified window size and position may be overridden by previously saved values.',
    "@macos Lorsque l'enregistrement automatique du cadre est activé par [LUM_COCOA_FRAME_NAME](@ref LUM_COCOA_FRAME_NAME_hint), la taille et la position indiquées peuvent être écrasées par des valeurs enregistrées auparavant.",
  ],
  [
    '@x11 Some window managers will not respect the placement of initially hidden windows.',
    '@x11 Certains gestionnaires de fenêtres ne respectent pas le placement des fenêtres initialement masquées.',
  ],
  [
    '@x11 Due to the asynchronous nature of X11, it may take a moment for a window to reach its requested state. This means you may not be able to query the final size, position or other attributes directly after window creation.',
    "@x11 X11 étant asynchrone, une fenêtre peut mettre un instant à atteindre l'état demandé.  Vous ne pourrez donc pas forcément interroger sa taille, sa position ou d'autres attributs définitifs juste après sa création.",
  ],
  [
    '@x11 The class part of the `WM_CLASS` window property will by default be set to the window title passed to this function. The instance part will use the contents of the `RESOURCE_NAME` environment variable, if present and not empty, or fall back to the window title. Set the [LUM_X11_CLASS_NAME](@ref LUM_X11_CLASS_NAME_hint) and [LUM_X11_INSTANCE_NAME](@ref LUM_X11_INSTANCE_NAME_hint) window hints to override this.',
    "@x11 La partie classe de la propriété `WM_CLASS` prend par défaut le titre de fenêtre passé à cette fonction.  La partie instance emploie le contenu de la variable d'environnement `RESOURCE_NAME`, si elle existe et n'est pas vide, et retombe sinon sur le titre de la fenêtre.  Positionnez les indices [LUM_X11_CLASS_NAME](@ref LUM_X11_CLASS_NAME_hint) et [LUM_X11_INSTANCE_NAME](@ref LUM_X11_INSTANCE_NAME_hint) pour les surcharger.",
  ],
  [
    "@wayland Compositors should implement the xdg-decoration protocol for Lumina to decorate the window properly. If this protocol isn't supported, or if the compositor prefers client-side decorations, a very simple fallback frame will be drawn using the wp_viewporter protocol. A compositor can still emit close, maximize or fullscreen events, using for instance a keybind mechanism. If neither of these protocols is supported, the window won't be decorated.",
    "@wayland Les compositeurs devraient implémenter le protocole xdg-decoration pour que Lumina décore correctement la fenêtre.  Si ce protocole n'est pas pris en charge, ou si le compositeur préfère des décorations côté client, un cadre de repli très simple est dessiné avec le protocole wp_viewporter.  Un compositeur peut tout de même émettre des événements de fermeture, de maximisation ou de plein écran, par exemple via un raccourci clavier.  Si aucun de ces protocoles n'est pris en charge, la fenêtre ne sera pas décorée.",
  ],
  [
    '@wayland A full screen window will not attempt to change the mode, no matter what the requested size or refresh rate.',
    '@wayland Une fenêtre plein écran ne tentera pas de changer de mode, quelles que soient la taille et la fréquence demandées.',
  ],
  [
    "@wayland Screensaver inhibition requires the idle-inhibit protocol to be implemented in the user's compositor.",
    "@wayland L'inhibition de l'économiseur d'écran exige que le compositeur de l'utilisateur implémente le protocole idle-inhibit.",
  ],
  [
    '@macos The window title will not be updated until the next time you process events.',
    '@macos Le titre de la fenêtre ne sera mis à jour qu\'au prochain traitement des événements.',
  ],
  [
    "@macos Regular windows do not have icons on macOS. This function will emit @ref LUM_FEATURE_UNAVAILABLE. The dock icon will be the same as the application bundle's icon. For more information on bundles, see the [Bundle Programming Guide](https://developer.apple.com/library/mac/documentation/CoreFoundation/Conceptual/CFBundles/) in the Mac Developer Library.",
    "@macos Les fenêtres ordinaires n'ont pas d'icône sous macOS.  Cette fonction émet @ref LUM_FEATURE_UNAVAILABLE.  L'icône du dock sera celle du bundle de l'application.  Pour en savoir plus sur les bundles, voyez le [Bundle Programming Guide](https://developer.apple.com/library/mac/documentation/CoreFoundation/Conceptual/CFBundles/) de la Mac Developer Library.",
  ],
  [
    "@wayland There is no existing protocol to change an icon, the window will thus inherit the one defined in the application's desktop file. This function will emit @ref LUM_FEATURE_UNAVAILABLE.",
    "@wayland Aucun protocole ne permet de changer d'icône : la fenêtre héritera donc de celle définie dans le fichier desktop de l'application.  Cette fonction émet @ref LUM_FEATURE_UNAVAILABLE.",
  ],
  [
    '@wayland There is no way for an application to retrieve the global position of its windows. This function will emit @ref LUM_FEATURE_UNAVAILABLE.',
    "@wayland Une application ne peut pas connaître la position globale de ses fenêtres.  Cette fonction émet @ref LUM_FEATURE_UNAVAILABLE.",
  ],
  [
    '@wayland There is no way for an application to set the global position of its windows. This function will emit @ref LUM_FEATURE_UNAVAILABLE.',
    "@wayland Une application ne peut pas fixer la position globale de ses fenêtres.  Cette fonction émet @ref LUM_FEATURE_UNAVAILABLE.",
  ],
  [
    '@wayland The size limits will not be applied until the window is actually resized, either by the user or by the compositor.',
    "@wayland Les limites de taille ne s'appliqueront qu'au prochain redimensionnement effectif de la fenêtre, par l'utilisateur ou par le compositeur.",
  ],
  [
    '@wayland The aspect ratio will not be applied until the window is actually resized, either by the user or by the compositor.',
    "@wayland La proportion ne s'appliquera qu'au prochain redimensionnement effectif de la fenêtre, par l'utilisateur ou par le compositeur.",
  ],
  [
    '@wayland A full screen window will not attempt to change the mode, no matter what the requested size.',
    '@wayland Une fenêtre plein écran ne tentera pas de changer de mode, quelle que soit la taille demandée.',
  ],
  [
    '@wayland There is no way to set an opacity factor for a window. This function will emit @ref LUM_FEATURE_UNAVAILABLE.',
    "@wayland Il n'existe aucun moyen de fixer un facteur d'opacité pour une fenêtre.  Cette fonction émet @ref LUM_FEATURE_UNAVAILABLE.",
  ],
  [
    '@wayland Once a window is iconified, @ref lumRestoreWindow won’t be able to restore it. This is a design decision of the xdg-shell protocol.',
    "@wayland Une fois une fenêtre réduite en icône, @ref lumRestoreWindow ne pourra pas la restaurer.  C'est un choix de conception du protocole xdg-shell.",
  ],
  [
    '@wayland Because Wayland wants every frame of the desktop to be complete, this function does not immediately make the window visible. Instead it will become visible the next time the window framebuffer is updated after this call.',
    "@wayland Wayland exigeant que chaque image du bureau soit complète, cette fonction ne rend pas la fenêtre visible immédiatement : elle le deviendra à la prochaine mise à jour de son tampon d'image après cet appel.",
  ],
  [
    '@wayland It is not possible for an application to set the input focus. This function will emit @ref LUM_FEATURE_UNAVAILABLE.',
    "@wayland Une application ne peut pas attribuer le focus.  Cette fonction émet @ref LUM_FEATURE_UNAVAILABLE.",
  ],
  [
    '@macos Attention is requested to the application as a whole, not the specific window.',
    "@macos L'attention est demandée sur l'application entière, pas sur la fenêtre en particulier.",
  ],
  [
    'The OpenGL or OpenGL ES context will not be destroyed or otherwise affected by any resizing or mode switching, although you may need to update your viewport if the framebuffer size has changed.',
    "Le contexte OpenGL ou OpenGL ES n'est ni détruit ni affecté par un redimensionnement ou un changement de mode ; vous devrez en revanche peut-être mettre à jour votre viewport si la taille du tampon d'image a changé.",
  ],
  [
    '@wayland The desired window position is ignored, as there is no way for an application to set this property.',
    "@wayland La position de fenêtre voulue est ignorée : une application ne peut pas fixer cette propriété.",
  ],
  [
    '@wayland Setting the window to full screen will not attempt to change the mode, no matter what the requested size or refresh rate.',
    '@wayland Passer la fenêtre en plein écran ne tentera pas de changer de mode, quelles que soient la taille et la fréquence demandées.',
  ],
  [
    'Framebuffer related hints are not window attributes. See @ref window_attribs_fb for more information.',
    "Les indices liés au tampon d'image ne sont pas des attributs de fenêtre.  Voyez @ref window_attribs_fb pour en savoir plus.",
  ],
  [
    'Zero is a valid value for many window and context related attributes so you cannot use a return value of zero as an indication of errors. However, this function should not fail as long as it is passed valid arguments and the library has been [initialized](@ref intro_init).',
    "Zéro est une valeur valide pour beaucoup d'attributs de fenêtre et de contexte : vous ne pouvez donc pas prendre un retour nul pour un signe d'erreur.  Cela dit, cette fonction ne devrait pas échouer tant qu'elle reçoit des arguments valides et que la bibliothèque a été [initialisée](@ref intro_init).",
  ],
  [
    'Calling @ref lumGetWindowAttrib will always return the latest value, even if that value is ignored by the current mode of the window.',
    "Appeler @ref lumGetWindowAttrib retourne toujours la dernière valeur, même si le mode courant de la fenêtre l'ignore.",
  ],
  [
    '@wayland This callback will never be called, as there is no way for an application to know its global position.',
    "@wayland Cette fonction de rappel ne sera jamais appelée : une application ne peut pas connaître sa position globale.",
  ],
  [
    '@macos Selecting Quit from the application menu will trigger the close callback for all windows.',
    '@macos Choisir Quitter dans le menu de l\'application déclenche la fonction de rappel de fermeture de toutes les fenêtres.',
  ],
  [
    'The contents of the returned string may change when a keyboard layout change event is received.',
    "Le contenu de la chaîne retournée peut changer à la réception d'un événement de changement de disposition clavier.",
  ],
  [
    '@wayland This function will only work when the cursor mode is `LUM_CURSOR_DISABLED`, otherwise it will do nothing.',
    '@wayland Cette fonction ne marche que si le mode de curseur vaut `LUM_CURSOR_DISABLED` ; sinon elle ne fait rien.',
  ],
  ['@wayland File drop is currently unimplemented.', '@wayland Le dépôt de fichiers n\'est pas encore implémenté.'],
  [
    'The upper limit of Lumina time is calculated as floor((2<sup>64</sup> - 1) / 10<sup>9</sup>) and is due to implementations storing nanoseconds in 64 bits. The limit may be increased in the future.',
    "La limite supérieure du temps Lumina vaut floor((2<sup>64</sup> - 1) / 10<sup>9</sup>) ; elle tient au stockage des nanosecondes sur 64 bits.  Elle pourra être relevée à l'avenir.",
  ],
  [
    '__EGL:__ The context of the specified window must be current on the calling thread.',
    '__EGL :__ le contexte de la fenêtre indiquée doit être courant sur le thread appelant.',
  ],
  [
    'This function is not called during context creation, leaving the swap interval set to whatever is the default for that API. This is done because some swap interval extensions used by Lumina do not allow the swap interval to be reset to zero once it has been set to a non-zero value.',
    "Cette fonction n'est pas appelée à la création du contexte : l'intervalle d'échange garde la valeur par défaut de l'API.  C'est voulu, car certaines extensions d'intervalle d'échange qu'emploie Lumina ne permettent pas de le remettre à zéro une fois une valeur non nulle posée.",
  ],
  [
    "Some GPU drivers do not honor the requested swap interval, either because of a user setting that overrides the application's request or due to bugs in the driver.",
    "Certains pilotes graphiques ne respectent pas l'intervalle d'échange demandé, soit à cause d'un réglage utilisateur qui écrase la demande de l'application, soit à cause de bogues du pilote.",
  ],
  [
    'The address of a given function is not guaranteed to be the same between contexts.',
    "Rien ne garantit que l'adresse d'une fonction donnée soit la même d'un contexte à l'autre.",
  ],
  [
    'This function may return a non-`NULL` address despite the associated version or extension not being available. Always check the context version or extension string first.',
    "Cette fonction peut retourner une adresse non `NULL` alors même que la version ou l'extension associée est indisponible.  Vérifiez toujours d'abord la version du contexte ou la chaîne d'extensions.",
  ],
  [
    'Additional extensions may be required by future versions of Lumina. You should check if any extensions you wish to enable are already in the returned array, as it is an error to specify an extension more than once in the `VkInstanceCreateInfo` struct.',
    "De futures versions de Lumina pourront exiger d'autres extensions.  Vérifiez que les extensions que vous souhaitez activer ne figurent pas déjà dans le tableau retourné : indiquer deux fois une même extension dans la structure `VkInstanceCreateInfo` est une erreur.",
  ],
  [
    '@macos This function currently always returns `LUM_TRUE`, as the `VK_MVK_macos_surface` and `VK_EXT_metal_surface` extensions do not provide a `vkGetPhysicalDevice*PresentationSupport` type function.',
    "@macos Cette fonction retourne actuellement toujours `LUM_TRUE`, les extensions `VK_MVK_macos_surface` et `VK_EXT_metal_surface` ne fournissant pas de fonction du type `vkGetPhysicalDevice*PresentationSupport`.",
  ],
  [
    'If an error occurs before the creation call is made, Lumina returns the Vulkan error code most appropriate for the error. Appropriate use of @ref lumVulkanSupported and @ref lumGetRequiredInstanceExtensions should eliminate almost all occurrences of these errors.',
    "Si une erreur survient avant l'appel de création, Lumina retourne le code d'erreur Vulkan le plus approprié.  Un usage correct de @ref lumVulkanSupported et @ref lumGetRequiredInstanceExtensions devrait éliminer la quasi-totalité de ces erreurs.",
  ],
  [
    '@macos Lumina prefers the `VK_EXT_metal_surface` extension, with the `VK_MVK_macos_surface` extension as a fallback. The name of the selected extension, if any, is included in the array returned by @ref lumGetRequiredInstanceExtensions.',
    "@macos Lumina préfère l'extension `VK_EXT_metal_surface`, avec `VK_MVK_macos_surface` en repli.  Le nom de l'extension retenue, s'il y en a une, figure dans le tableau retourné par @ref lumGetRequiredInstanceExtensions.",
  ],
  [
    '@macos This function creates and sets a `CAMetalLayer` instance for the window content view, which is required for MoltenVK to function.',
    "@macos Cette fonction crée et installe une instance de `CAMetalLayer` pour la vue de contenu de la fenêtre, indispensable au fonctionnement de MoltenVK.",
  ],
  [
    '@x11 By default Lumina prefers the `VK_KHR_xcb_surface` extension, with the `VK_KHR_xlib_surface` extension as a fallback. You can make `VK_KHR_xlib_surface` the preferred extension by setting the [LUM_X11_XCB_VULKAN_SURFACE](@ref LUM_X11_XCB_VULKAN_SURFACE_hint) init hint. The name of the selected extension, if any, is included in the array returned by @ref lumGetRequiredInstanceExtensions.',
    "@x11 Par défaut, Lumina préfère l'extension `VK_KHR_xcb_surface`, avec `VK_KHR_xlib_surface` en repli.  Vous pouvez faire de `VK_KHR_xlib_surface` l'extension préférée en positionnant l'indice d'initialisation [LUM_X11_XCB_VULKAN_SURFACE](@ref LUM_X11_XCB_VULKAN_SURFACE_hint).  Le nom de l'extension retenue, s'il y en a une, figure dans le tableau retourné par @ref lumGetRequiredInstanceExtensions.",
  ],
  [
    'The `HDC` associated with the window can be queried with the [GetDC](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdc) function. @code HDC dc = GetDC(lumGetWin32Window(window)); @endcode This DC is private and does not need to be released.',
    "Le `HDC` associé à la fenêtre s'obtient avec la fonction [GetDC](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdc).\n@code\nHDC dc = GetDC(lumGetWin32Window(window));\n@endcode\nCe DC est privé et n'a pas besoin d'être libéré.",
  ],
];
