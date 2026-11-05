// Return values, and the brief summaries of constants.

export default [
  // ---- returns -------------------------------------------------------------
  ['The ASCII encoded Lumina version string.', 'La chaîne de version de Lumina, encodée en ASCII.'],
  [
    'The last error code for the calling thread, or @ref LUM_NO_ERROR (zero).',
    "Le dernier code d'erreur du thread appelant, ou @ref LUM_NO_ERROR (zéro).",
  ],
  [
    'The previously set callback, or `NULL` if no callback was set.',
    "La fonction de rappel précédemment installée, ou `NULL` si aucune ne l'était.",
  ],
  [
    'The currently selected platform, or zero if an error occurred.',
    'La plateforme actuellement retenue, ou zéro si une erreur est survenue.',
  ],
  [
    '`LUM_TRUE` if the platform is supported, or `LUM_FALSE` otherwise.',
    '`LUM_TRUE` si la plateforme est prise en charge, `LUM_FALSE` sinon.',
  ],
  [
    'An array of monitor handles, or `NULL` if no monitors were found or if an [error](@ref error_handling) occurred.',
    "Un tableau de handles d'écran, ou `NULL` si aucun écran n'a été trouvé ou si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'The primary monitor, or `NULL` if no monitors were found or if an [error](@ref error_handling) occurred.',
    "L'écran principal, ou `NULL` si aucun écran n'a été trouvé ou si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'The UTF-8 encoded name of the monitor, or `NULL` if an [error](@ref error_handling) occurred.',
    "Le nom de l'écran, encodé en UTF-8, ou `NULL` si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'An array of video modes, or `NULL` if an [error](@ref error_handling) occurred.',
    'Un tableau de modes vidéo, ou `NULL` si une [erreur](@ref error_handling) est survenue.',
  ],
  [
    'The current mode of the monitor, or `NULL` if an [error](@ref error_handling) occurred.',
    "Le mode courant de l'écran, ou `NULL` si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'The current gamma ramp, or `NULL` if an [error](@ref error_handling) occurred.',
    'La rampe gamma courante, ou `NULL` si une [erreur](@ref error_handling) est survenue.',
  ],
  [
    'The handle of the created window, or `NULL` if an [error](@ref error_handling) occurred.',
    'Le handle de la fenêtre créée, ou `NULL` si une [erreur](@ref error_handling) est survenue.',
  ],
  ['The value of the close flag.', "La valeur de l'indicateur de fermeture."],
  ['The opacity value of the specified window.', "La valeur d'opacité de la fenêtre indiquée."],
  [
    'The monitor, or `NULL` if the window is in windowed mode or an [error](@ref error_handling) occurred.',
    "L'écran, ou `NULL` si la fenêtre est en mode fenêtré ou si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'The value of the attribute, or zero if an [error](@ref error_handling) occurred.',
    "La valeur de l'attribut, ou zéro si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    '`LUM_TRUE` if raw mouse motion is supported on the current machine, or `LUM_FALSE` otherwise.',
    '`LUM_TRUE` si le mouvement de souris brut est pris en charge sur la machine courante, `LUM_FALSE` sinon.',
  ],
  [
    'The UTF-8 encoded, layout-specific name of the key, or `NULL`.',
    'Le nom de la touche, encodé en UTF-8 et propre à la disposition clavier, ou `NULL`.',
  ],
  [
    'The platform-specific scancode for the key, or `-1` if an [error](@ref error_handling) occurred.',
    'Le scancode de la touche, propre à la plateforme, ou `-1` si une [erreur](@ref error_handling) est survenue.',
  ],
  [
    'The handle of the created cursor, or `NULL` if an [error](@ref error_handling) occurred.',
    'Le handle du curseur créé, ou `NULL` si une [erreur](@ref error_handling) est survenue.',
  ],
  [
    'A new cursor ready to use or `NULL` if an [error](@ref error_handling) occurred.',
    "Un nouveau curseur prêt à l'emploi, ou `NULL` si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'The previously set callback, or `NULL` if no callback was set or an [error](@ref error_handling) occurred.',
    "La fonction de rappel précédemment installée, ou `NULL` si aucune ne l'était ou si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    '`LUM_TRUE` if the joystick is present, or `LUM_FALSE` otherwise.',
    '`LUM_TRUE` si la manette est présente, `LUM_FALSE` sinon.',
  ],
  [
    'An array of axis values, or `NULL` if the joystick is not present or an [error](@ref error_handling) occurred.',
    "Un tableau de valeurs d'axes, ou `NULL` si la manette est absente ou si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'An array of button states, or `NULL` if the joystick is not present or an [error](@ref error_handling) occurred.',
    "Un tableau d'états de boutons, ou `NULL` si la manette est absente ou si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'An array of hat states, or `NULL` if the joystick is not present or an [error](@ref error_handling) occurred.',
    "Un tableau d'états de croix directionnelles, ou `NULL` si la manette est absente ou si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'The UTF-8 encoded name of the joystick, or `NULL` if the joystick is not present or an [error](@ref error_handling) occurred.',
    'Le nom de la manette, encodé en UTF-8, ou `NULL` si la manette est absente ou si une [erreur](@ref error_handling) est survenue.',
  ],
  [
    'The UTF-8 encoded GUID of the joystick, or `NULL` if the joystick is not present or an [error](@ref error_handling) occurred.',
    'Le GUID de la manette, encodé en UTF-8, ou `NULL` si la manette est absente ou si une [erreur](@ref error_handling) est survenue.',
  ],
  [
    '`LUM_TRUE` if a joystick is both present and has a gamepad mapping, or `LUM_FALSE` otherwise.',
    "`LUM_TRUE` si la manette est présente et dispose d'une correspondance de manette de jeu, `LUM_FALSE` sinon.",
  ],
  [
    'The UTF-8 encoded name of the gamepad, or `NULL` if the joystick is not present, does not have a mapping or an [error](@ref error_handling) occurred.',
    "Le nom de la manette de jeu, encodé en UTF-8, ou `NULL` si la manette est absente, n'a pas de correspondance, ou si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    '`LUM_TRUE` if successful, or `LUM_FALSE` if no joystick is connected, it has no gamepad mapping or an [error](@ref error_handling) occurred.',
    "`LUM_TRUE` en cas de succès, ou `LUM_FALSE` si aucune manette n'est connectée, si elle n'a pas de correspondance, ou si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'The contents of the clipboard as a UTF-8 encoded string, or `NULL` if an [error](@ref error_handling) occurred.',
    'Le contenu du presse-papiers sous forme de chaîne encodée en UTF-8, ou `NULL` si une [erreur](@ref error_handling) est survenue.',
  ],
  [
    'The current time, in seconds, or zero if an [error](@ref error_handling) occurred.',
    'Le temps courant, en secondes, ou zéro si une [erreur](@ref error_handling) est survenue.',
  ],
  [
    'The value of the timer, or zero if an [error](@ref error_handling) occurred.',
    'La valeur du minuteur, ou zéro si une [erreur](@ref error_handling) est survenue.',
  ],
  [
    'The frequency of the timer, in Hz, or zero if an [error](@ref error_handling) occurred.',
    'La fréquence du minuteur, en Hz, ou zéro si une [erreur](@ref error_handling) est survenue.',
  ],
  [
    "The window whose context is current, or `NULL` if no window's context is current.",
    "La fenêtre dont le contexte est courant, ou `NULL` si aucun contexte de fenêtre ne l'est.",
  ],
  [
    '`LUM_TRUE` if the extension is available, or `LUM_FALSE` otherwise.',
    "`LUM_TRUE` si l'extension est disponible, `LUM_FALSE` sinon.",
  ],
  [
    '`LUM_TRUE` if Vulkan is minimally available, or `LUM_FALSE` otherwise.',
    '`LUM_TRUE` si Vulkan est minimalement disponible, `LUM_FALSE` sinon.',
  ],
  [
    'An array of ASCII encoded extension names, or `NULL` if an [error](@ref error_handling) occurred.',
    "Un tableau de noms d'extension encodés en ASCII, ou `NULL` si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    '`LUM_TRUE` if the queue family supports presentation, or `LUM_FALSE` otherwise.',
    '`LUM_TRUE` si la famille de files prend en charge la présentation, `LUM_FALSE` sinon.',
  ],
  [
    '`VK_SUCCESS` if successful, or a Vulkan error code if an [error](@ref error_handling) occurred.',
    "`VK_SUCCESS` en cas de succès, ou un code d'erreur Vulkan si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'The UTF-8 encoded adapter device name (for example `\\\\.\\DISPLAY1`) of the specified monitor, or `NULL` if an [error](@ref error_handling) occurred.',
    "Le nom de périphérique de l'adaptateur de l'écran indiqué, encodé en UTF-8 (par exemple `\\\\.\\DISPLAY1`), ou `NULL` si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'The UTF-8 encoded display device name (for example `\\\\.\\DISPLAY1\\Monitor0`) of the specified monitor, or `NULL` if an [error](@ref error_handling) occurred.',
    "Le nom de périphérique d'affichage de l'écran indiqué, encodé en UTF-8 (par exemple `\\\\.\\DISPLAY1\\Monitor0`), ou `NULL` si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'The `HWND` of the specified window, or `NULL` if an [error](@ref error_handling) occurred.',
    'Le `HWND` de la fenêtre indiquée, ou `NULL` si une [erreur](@ref error_handling) est survenue.',
  ],
  [
    'The `HGLRC` of the specified window, or `NULL` if an [error](@ref error_handling) occurred.',
    'Le `HGLRC` de la fenêtre indiquée, ou `NULL` si une [erreur](@ref error_handling) est survenue.',
  ],
  [
    'The `CGDirectDisplayID` of the specified monitor, or `kCGNullDirectDisplay` if an [error](@ref error_handling) occurred.',
    "Le `CGDirectDisplayID` de l'écran indiqué, ou `kCGNullDirectDisplay` si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'The `NSWindow` of the specified window, or `nil` if an [error](@ref error_handling) occurred.',
    'La `NSWindow` de la fenêtre indiquée, ou `nil` si une [erreur](@ref error_handling) est survenue.',
  ],
  [
    'The `NSOpenGLContext` of the specified window, or `nil` if an [error](@ref error_handling) occurred.',
    'Le `NSOpenGLContext` de la fenêtre indiquée, ou `nil` si une [erreur](@ref error_handling) est survenue.',
  ],
  [
    'The `Display` used by Lumina, or `NULL` if an [error](@ref error_handling) occurred.',
    "Le `Display` qu'utilise Lumina, ou `NULL` si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'The `RRCrtc` of the specified monitor, or `None` if an [error](@ref error_handling) occurred.',
    "Le `RRCrtc` de l'écran indiqué, ou `None` si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'The `RROutput` of the specified monitor, or `None` if an [error](@ref error_handling) occurred.',
    "Le `RROutput` de l'écran indiqué, ou `None` si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'The `Window` of the specified window, or `None` if an [error](@ref error_handling) occurred.',
    'La `Window` de la fenêtre indiquée, ou `None` si une [erreur](@ref error_handling) est survenue.',
  ],
  [
    'The contents of the selection as a UTF-8 encoded string, or `NULL` if an [error](@ref error_handling) occurred.',
    'Le contenu de la sélection sous forme de chaîne encodée en UTF-8, ou `NULL` si une [erreur](@ref error_handling) est survenue.',
  ],
  [
    'The `GLXContext` of the specified window, or `NULL` if an [error](@ref error_handling) occurred.',
    'Le `GLXContext` de la fenêtre indiquée, ou `NULL` si une [erreur](@ref error_handling) est survenue.',
  ],
  [
    'The `GLXWindow` of the specified window, or `None` if an [error](@ref error_handling) occurred.',
    'La `GLXWindow` de la fenêtre indiquée, ou `None` si une [erreur](@ref error_handling) est survenue.',
  ],
  [
    'The `struct wl_display*` used by Lumina, or `NULL` if an [error](@ref error_handling) occurred.',
    "Le `struct wl_display*` qu'utilise Lumina, ou `NULL` si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'The `struct wl_output*` of the specified monitor, or `NULL` if an [error](@ref error_handling) occurred.',
    "Le `struct wl_output*` de l'écran indiqué, ou `NULL` si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'The main `struct wl_surface*` of the specified window, or `NULL` if an [error](@ref error_handling) occurred.',
    'Le `struct wl_surface*` principal de la fenêtre indiquée, ou `NULL` si une [erreur](@ref error_handling) est survenue.',
  ],
  [
    'The `EGLDisplay` used by Lumina, or `EGL_NO_DISPLAY` if an [error](@ref error_handling) occurred.',
    "L'`EGLDisplay` qu'utilise Lumina, ou `EGL_NO_DISPLAY` si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'The `EGLContext` of the specified window, or `EGL_NO_CONTEXT` if an [error](@ref error_handling) occurred.',
    "L'`EGLContext` de la fenêtre indiquée, ou `EGL_NO_CONTEXT` si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'The `EGLSurface` of the specified window, or `EGL_NO_SURFACE` if an [error](@ref error_handling) occurred.',
    "L'`EGLSurface` de la fenêtre indiquée, ou `EGL_NO_SURFACE` si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'The `OSMesaContext` of the specified window, or `NULL` if an [error](@ref error_handling) occurred.',
    "L'`OSMesaContext` de la fenêtre indiquée, ou `NULL` si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'The address of the newly allocated memory block, or `NULL` if an error occurred.',
    "L'adresse du bloc mémoire nouvellement alloué, ou `NULL` si une erreur est survenue.",
  ],
  [
    'The address of the newly allocated or resized memory block, or `NULL` if an error occurred.',
    "L'adresse du bloc mémoire nouvellement alloué ou redimensionné, ou `NULL` si une erreur est survenue.",
  ],

  // ---- constants: brief summaries -----------------------------------------
  ['The major version number of the Lumina header.', "Le numéro de version majeure de l'en-tête de Lumina."],
  ['The minor version number of the Lumina header.', "Le numéro de version mineure de l'en-tête de Lumina."],
  ['The revision number of the Lumina header.', "Le numéro de révision de l'en-tête de Lumina."],
  ['One.', 'Un.'],
  ['Zero.', 'Zéro.'],
  [
    'If this bit is set the Caps Lock key is enabled.',
    'Si ce bit est positionné, la touche Verr. Maj est active.',
  ],
  ['If this bit is set the Num Lock key is enabled.', 'Si ce bit est positionné, la touche Verr. Num est active.'],
  ['Lumina has not been initialized.', "Lumina n'a pas été initialisée."],
  ['No context is current for this thread.', "Aucun contexte n'est courant pour ce thread."],
  [
    'One of the arguments to the function was an invalid enum value.',
    "L'un des arguments de la fonction était une valeur d'énumération invalide.",
  ],
  [
    'One of the arguments to the function was an invalid value.',
    "L'un des arguments de la fonction était une valeur invalide.",
  ],
  [
    'The requested OpenGL or OpenGL ES version is not available.',
    "La version d'OpenGL ou d'OpenGL ES demandée n'est pas disponible.",
  ],
  [
    'The requested format is not supported or available.',
    "Le format demandé n'est pas pris en charge ou pas disponible.",
  ],
  [
    'The specified window does not have an OpenGL or OpenGL ES context.',
    "La fenêtre indiquée n'a pas de contexte OpenGL ni OpenGL ES.",
  ],
  ['The specified cursor shape is not available.', "La forme de curseur indiquée n'est pas disponible."],
  [
    'The requested feature is not provided by the platform.',
    "La fonctionnalité demandée n'est pas fournie par la plateforme.",
  ],
  [
    'The requested feature is not implemented for the platform.',
    "La fonctionnalité demandée n'est pas implémentée pour cette plateforme.",
  ],
  [
    'Platform unavailable or no matching platform was found.',
    "Plateforme indisponible, ou aucune plateforme correspondante n'a été trouvée.",
  ],
  ['Input focus window hint and attribute', 'Indice et attribut de fenêtre pour le focus'],
  ['Window iconification window attribute', 'Attribut de fenêtre pour la réduction en icône'],
  ['Window resize-ability window hint and attribute', 'Indice et attribut de fenêtre pour la redimensionnabilité'],
  ['Window visibility window hint and attribute', 'Indice et attribut de fenêtre pour la visibilité'],
  [
    'Window auto-iconification window hint and attribute',
    'Indice et attribut de fenêtre pour la réduction automatique en icône',
  ],
  ['Window maximization window hint and attribute', 'Indice et attribut de fenêtre pour la maximisation'],
  ['Cursor centering window hint', 'Indice de fenêtre pour le centrage du curseur'],
  [
    'Window framebuffer transparency hint and attribute',
    "Indice et attribut de fenêtre pour la transparence du tampon d'image",
  ],
  ['Mouse cursor hover window attribute.', 'Attribut de fenêtre pour le survol du curseur de souris.'],
  [
    'Input focus on calling show window hint and attribute',
    "Indice et attribut de fenêtre pour le focus à l'affichage",
  ],
  ['Window has titlebar window hint and attribute', 'Indice et attribut de fenêtre pour la présence de barre de titre'],
  [
    'Mouse input transparency window hint and attribute',
    'Indice et attribut de fenêtre pour la transparence aux entrées souris',
  ],
  ['Framebuffer auxiliary buffer hint.', "Indice de tampons auxiliaires du tampon d'image."],
  ['OpenGL stereoscopic rendering hint.', 'Indice de rendu stéréoscopique OpenGL.'],
  ['Framebuffer MSAA samples hint.', "Indice d'échantillons MSAA du tampon d'image."],
  ['Framebuffer sRGB hint.', "Indice sRGB du tampon d'image."],
  ['Monitor refresh rate hint.', 'Indice de taux de rafraîchissement de l\'écran.'],
  [
    'Framebuffer double buffering hint and attribute.',
    "Indice et attribut de double tampon du tampon d'image.",
  ],
  ['Context client API hint and attribute.', "Indice et attribut d'API cliente du contexte."],
  [
    'Context client API major version hint and attribute.',
    "Indice et attribut de version majeure de l'API cliente du contexte.",
  ],
  [
    'Context client API minor version hint and attribute.',
    "Indice et attribut de version mineure de l'API cliente du contexte.",
  ],
  [
    'Context client API revision number attribute.',
    "Attribut de numéro de révision de l'API cliente du contexte.",
  ],
  ['Context robustness hint and attribute.', 'Indice et attribut de robustesse du contexte.'],
  [
    'OpenGL forward-compatibility hint and attribute.',
    'Indice et attribut de compatibilité en avant OpenGL.',
  ],
  ['Debug mode context hint and attribute.', 'Indice et attribut de contexte en mode débogage.'],
  ['OpenGL profile hint and attribute.', 'Indice et attribut de profil OpenGL.'],
  [
    'Context flush-on-release hint and attribute.',
    'Indice et attribut de vidage du contexte à sa libération.',
  ],
  [
    'Context error suppression hint and attribute.',
    "Indice et attribut de suppression des erreurs du contexte.",
  ],
  ['Context creation API hint and attribute.', "Indice et attribut d'API de création de contexte."],
  [
    'Window content area scaling window [window hint](@ref LUM_SCALE_TO_MONITOR).',
    "[Indice de fenêtre](@ref LUM_SCALE_TO_MONITOR) pour la mise à l'échelle de la zone de contenu.",
  ],
  [
    'macOS specific [window hint](@ref LUM_COCOA_RETINA_FRAMEBUFFER_hint).',
    '[Indice de fenêtre](@ref LUM_COCOA_RETINA_FRAMEBUFFER_hint) propre à macOS.',
  ],
  [
    'macOS specific [window hint](@ref LUM_COCOA_FRAME_NAME_hint).',
    '[Indice de fenêtre](@ref LUM_COCOA_FRAME_NAME_hint) propre à macOS.',
  ],
  [
    'macOS specific [window hint](@ref LUM_COCOA_GRAPHICS_SWITCHING_hint).',
    '[Indice de fenêtre](@ref LUM_COCOA_GRAPHICS_SWITCHING_hint) propre à macOS.',
  ],
  ['The horizontal resize/move arrow shape.', 'La forme de flèche de redimensionnement ou déplacement horizontal.'],
  ['The vertical resize/move arrow shape.', 'La forme de flèche de redimensionnement ou déplacement vertical.'],
  [
    'The top-left to bottom-right diagonal resize/move arrow shape.',
    'La forme de flèche de redimensionnement ou déplacement en diagonale haut-gauche vers bas-droite.',
  ],
  [
    'The top-right to bottom-left diagonal resize/move arrow shape.',
    'La forme de flèche de redimensionnement ou déplacement en diagonale haut-droite vers bas-gauche.',
  ],
  [
    'The omni-directional resize/move cursor shape.',
    'La forme de curseur de redimensionnement ou déplacement dans toutes les directions.',
  ],
  ['The operation-not-allowed shape.', "La forme signalant une opération non permise."],
  ['Joystick hat buttons init hint.', "Indice d'initialisation des croix de manette en boutons."],
  ['ANGLE rendering backend init hint.', "Indice d'initialisation du backend de rendu ANGLE."],
  ['Platform selection init hint.', "Indice d'initialisation de choix de la plateforme."],
  ['X11 specific init hint.', "Indice d'initialisation propre à X11."],
  [
    'Hint value that enables automatic platform selection.',
    "Valeur d'indice activant le choix automatique de la plateforme.",
  ],
];
