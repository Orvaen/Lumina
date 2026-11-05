// Detailed descriptions of the constants.

export default [
  [
    'The major version number of the Lumina header. This is incremented when the API is changed in non-compatible ways.',
    "Le numéro de version majeure de l'en-tête de Lumina.  Il est incrémenté lorsque l'API change de façon non compatible.",
  ],
  [
    'The minor version number of the Lumina header. This is incremented when features are added to the API but it remains backward-compatible.',
    "Le numéro de version mineure de l'en-tête de Lumina.  Il est incrémenté lorsque des fonctionnalités sont ajoutées à l'API tout en restant rétrocompatible.",
  ],
  [
    'The revision number of the Lumina header. This is incremented when a bug fix release is made that does not contain any API changes.',
    "Le numéro de révision de l'en-tête de Lumina.  Il est incrémenté lors d'une version corrective sans changement d'API.",
  ],
  [
    'This is only semantic sugar for the number 1. You can instead use `1` or `true` or `_True` or `GL_TRUE` or `VK_TRUE` or anything else that is equal to one.',
    "Ce n'est que du sucre sémantique pour le nombre 1.  Vous pouvez tout aussi bien employer `1`, `true`, `_True`, `GL_TRUE`, `VK_TRUE` ou n'importe quoi d'autre valant un.",
  ],
  [
    'This is only semantic sugar for the number 0. You can instead use `0` or `false` or `_False` or `GL_FALSE` or `VK_FALSE` or anything else that is equal to zero.',
    "Ce n'est que du sucre sémantique pour le nombre 0.  Vous pouvez tout aussi bien employer `0`, `false`, `_False`, `GL_FALSE`, `VK_FALSE` ou n'importe quoi d'autre valant zéro.",
  ],
  [
    'If this bit is set the Caps Lock key is enabled and the @ref LUM_LOCK_KEY_MODS input mode is set.',
    "Si ce bit est positionné, la touche Verr. Maj est active et le mode d'entrée @ref LUM_LOCK_KEY_MODS est activé.",
  ],
  [
    'If this bit is set the Num Lock key is enabled and the @ref LUM_LOCK_KEY_MODS input mode is set.',
    "Si ce bit est positionné, la touche Verr. Num est active et le mode d'entrée @ref LUM_LOCK_KEY_MODS est activé.",
  ],
  [
    'This occurs if a Lumina function was called that must not be called unless the library is [initialized](@ref intro_init).',
    "Survient lorsqu'une fonction de Lumina a été appelée alors qu'elle exige que la bibliothèque soit [initialisée](@ref intro_init).",
  ],
  [
    'This occurs if a Lumina function was called that needs and operates on the current OpenGL or OpenGL ES context but no context is current on the calling thread. One such function is @ref lumSwapInterval.',
    "Survient lorsqu'une fonction de Lumina agissant sur le contexte OpenGL ou OpenGL ES courant a été appelée alors qu'aucun contexte n'est courant sur le thread appelant.  @ref lumSwapInterval en est un exemple.",
  ],
  [
    'One of the arguments to the function was an invalid enum value, for example requesting @ref LUM_RED_BITS with @ref lumGetWindowAttrib.',
    "L'un des arguments de la fonction était une valeur d'énumération invalide : par exemple, demander @ref LUM_RED_BITS à @ref lumGetWindowAttrib.",
  ],
  [
    'One of the arguments to the function was an invalid value, for example requesting a non-existent OpenGL or OpenGL ES version like 2.7. Requesting a valid but unavailable OpenGL or OpenGL ES version will instead result in a @ref LUM_VERSION_UNAVAILABLE error.',
    "L'un des arguments de la fonction était une valeur invalide : par exemple, demander une version d'OpenGL ou d'OpenGL ES inexistante comme 2.7.  Demander une version valide mais indisponible produit plutôt une erreur @ref LUM_VERSION_UNAVAILABLE.",
  ],
  [
    'The requested OpenGL or OpenGL ES version (including any requested context or framebuffer hints) is not available on this machine.',
    "La version d'OpenGL ou d'OpenGL ES demandée, y compris les indices de contexte ou de tampon d'image demandés, n'est pas disponible sur cette machine.",
  ],
  [
    'If emitted during window creation, the requested pixel format is not supported. If emitted when querying the clipboard, the contents of the clipboard could not be converted to the requested format.',
    "Émise à la création d'une fenêtre, elle signale que le format de pixel demandé n'est pas pris en charge.  Émise lors de l'interrogation du presse-papiers, elle signale que son contenu n'a pas pu être converti au format demandé.",
  ],
  [
    'A window that does not have an OpenGL or OpenGL ES context was passed to a function that requires it to have one.',
    "Une fenêtre dépourvue de contexte OpenGL ou OpenGL ES a été passée à une fonction qui en exige un.",
  ],
  [
    'The specified standard cursor shape is not available, either because the current platform cursor theme does not provide it or because it is not available on the platform.',
    "La forme de curseur standard indiquée n'est pas disponible, soit parce que le thème de curseurs courant ne la fournit pas, soit parce qu'elle n'existe pas sur la plateforme.",
  ],
  [
    'The requested feature is not provided by the platform, so Lumina is unable to implement it. The documentation for each function notes if it could emit this error.',
    "La fonctionnalité demandée n'est pas fournie par la plateforme : Lumina ne peut donc pas l'implémenter.  La documentation de chaque fonction précise si elle peut émettre cette erreur.",
  ],
  [
    'The requested feature has not yet been implemented in Lumina for this platform.',
    "La fonctionnalité demandée n'est pas encore implémentée dans Lumina pour cette plateforme.",
  ],
  [
    'If emitted during initialization, no matching platform was found. If @ref LUM_PLATFORM is set to `LUM_ANY_PLATFORM`, Lumina could not detect any of the platforms supported by this library binary, except for the Null platform. If set to a specific platform, it is either not supported by this library binary or Lumina was not able to detect it. If emitted by a native access function, Lumina was initialized for a different platform than the function is for.',
    "Émise à l'initialisation, elle signale qu'aucune plateforme correspondante n'a été trouvée.  Si @ref LUM_PLATFORM vaut `LUM_ANY_PLATFORM`, Lumina n'a détecté aucune des plateformes prises en charge par ce binaire, hormis la plateforme Null.  Si une plateforme précise est demandée, soit ce binaire ne la prend pas en charge, soit Lumina n'a pas su la détecter.  Émise par une fonction d'accès natif, elle signale que Lumina a été initialisée pour une autre plateforme que celle de la fonction.",
  ],

  // ---- hints and attributes ------------------------------------------------
  [
    'Input focus [window hint](@ref LUM_FOCUSED_hint) or [window attribute](@ref LUM_FOCUSED_attrib).',
    '[Indice](@ref LUM_FOCUSED_hint) ou [attribut](@ref LUM_FOCUSED_attrib) de fenêtre pour le focus.',
  ],
  [
    'Window iconification [window attribute](@ref LUM_ICONIFIED_attrib).',
    "[Attribut de fenêtre](@ref LUM_ICONIFIED_attrib) pour la réduction en icône.",
  ],
  [
    'Window resize-ability [window hint](@ref LUM_RESIZABLE_hint) and [window attribute](@ref LUM_RESIZABLE_attrib).',
    '[Indice](@ref LUM_RESIZABLE_hint) et [attribut](@ref LUM_RESIZABLE_attrib) de fenêtre pour la redimensionnabilité.',
  ],
  [
    'Window visibility [window hint](@ref LUM_VISIBLE_hint) and [window attribute](@ref LUM_VISIBLE_attrib).',
    '[Indice](@ref LUM_VISIBLE_hint) et [attribut](@ref LUM_VISIBLE_attrib) de fenêtre pour la visibilité.',
  ],
  [
    'Window decoration [window hint](@ref LUM_DECORATED_hint) and [window attribute](@ref LUM_DECORATED_attrib).',
    '[Indice](@ref LUM_DECORATED_hint) et [attribut](@ref LUM_DECORATED_attrib) de fenêtre pour la décoration.',
  ],
  [
    'Window auto-iconification [window hint](@ref LUM_AUTO_ICONIFY_hint) and [window attribute](@ref LUM_AUTO_ICONIFY_attrib).',
    '[Indice](@ref LUM_AUTO_ICONIFY_hint) et [attribut](@ref LUM_AUTO_ICONIFY_attrib) de fenêtre pour la réduction automatique en icône.',
  ],
  [
    'Window decoration [window hint](@ref LUM_FLOATING_hint) and [window attribute](@ref LUM_FLOATING_attrib).',
    '[Indice](@ref LUM_FLOATING_hint) et [attribut](@ref LUM_FLOATING_attrib) de fenêtre pour le caractère flottant.',
  ],
  [
    'Window maximization [window hint](@ref LUM_MAXIMIZED_hint) and [window attribute](@ref LUM_MAXIMIZED_attrib).',
    '[Indice](@ref LUM_MAXIMIZED_hint) et [attribut](@ref LUM_MAXIMIZED_attrib) de fenêtre pour la maximisation.',
  ],
  [
    'Cursor centering [window hint](@ref LUM_CENTER_CURSOR_hint).',
    '[Indice de fenêtre](@ref LUM_CENTER_CURSOR_hint) pour le centrage du curseur.',
  ],
  [
    'Window framebuffer transparency [window hint](@ref LUM_TRANSPARENT_FRAMEBUFFER_hint) and [window attribute](@ref LUM_TRANSPARENT_FRAMEBUFFER_attrib).',
    "[Indice](@ref LUM_TRANSPARENT_FRAMEBUFFER_hint) et [attribut](@ref LUM_TRANSPARENT_FRAMEBUFFER_attrib) de fenêtre pour la transparence du tampon d'image.",
  ],
  [
    'Mouse cursor hover [window attribute](@ref LUM_HOVERED_attrib).',
    '[Attribut de fenêtre](@ref LUM_HOVERED_attrib) pour le survol du curseur de souris.',
  ],
  [
    'Input focus [window hint](@ref LUM_FOCUS_ON_SHOW_hint) or [window attribute](@ref LUM_FOCUS_ON_SHOW_attrib).',
    "[Indice](@ref LUM_FOCUS_ON_SHOW_hint) ou [attribut](@ref LUM_FOCUS_ON_SHOW_attrib) de fenêtre pour le focus à l'affichage.",
  ],
  [
    'Window has titlebar [window hint](@ref LUM_TITLEBAR_hint) and [window attribute](@ref LUM_TITLEBAR_attrib). NOTE: Added by Hazel',
    '[Indice](@ref LUM_TITLEBAR_hint) et [attribut](@ref LUM_TITLEBAR_attrib) de fenêtre pour la présence de barre de titre.  NOTE : ajouté par Hazel',
  ],
  [
    'Mouse input transparency [window hint](@ref LUM_MOUSE_PASSTHROUGH_hint) or [window attribute](@ref LUM_MOUSE_PASSTHROUGH_attrib).',
    '[Indice](@ref LUM_MOUSE_PASSTHROUGH_hint) ou [attribut](@ref LUM_MOUSE_PASSTHROUGH_attrib) de fenêtre pour la transparence aux entrées souris.',
  ],
  [
    'Framebuffer bit depth [hint](@ref LUM_RED_BITS).',
    "[Indice](@ref LUM_RED_BITS) de profondeur de bits du tampon d'image.",
  ],
  [
    'Framebuffer bit depth [hint](@ref LUM_GREEN_BITS).',
    "[Indice](@ref LUM_GREEN_BITS) de profondeur de bits du tampon d'image.",
  ],
  [
    'Framebuffer bit depth [hint](@ref LUM_BLUE_BITS).',
    "[Indice](@ref LUM_BLUE_BITS) de profondeur de bits du tampon d'image.",
  ],
  [
    'Framebuffer bit depth [hint](@ref LUM_ALPHA_BITS).',
    "[Indice](@ref LUM_ALPHA_BITS) de profondeur de bits du tampon d'image.",
  ],
  [
    'Framebuffer bit depth [hint](@ref LUM_DEPTH_BITS).',
    "[Indice](@ref LUM_DEPTH_BITS) de profondeur de bits du tampon d'image.",
  ],
  [
    'Framebuffer bit depth [hint](@ref LUM_STENCIL_BITS).',
    "[Indice](@ref LUM_STENCIL_BITS) de profondeur de bits du tampon d'image.",
  ],
  [
    'Framebuffer bit depth [hint](@ref LUM_ACCUM_RED_BITS).',
    "[Indice](@ref LUM_ACCUM_RED_BITS) de profondeur de bits du tampon d'image.",
  ],
  [
    'Framebuffer bit depth [hint](@ref LUM_ACCUM_GREEN_BITS).',
    "[Indice](@ref LUM_ACCUM_GREEN_BITS) de profondeur de bits du tampon d'image.",
  ],
  [
    'Framebuffer bit depth [hint](@ref LUM_ACCUM_BLUE_BITS).',
    "[Indice](@ref LUM_ACCUM_BLUE_BITS) de profondeur de bits du tampon d'image.",
  ],
  [
    'Framebuffer bit depth [hint](@ref LUM_ACCUM_ALPHA_BITS).',
    "[Indice](@ref LUM_ACCUM_ALPHA_BITS) de profondeur de bits du tampon d'image.",
  ],
  [
    'Framebuffer auxiliary buffer [hint](@ref LUM_AUX_BUFFERS).',
    "[Indice](@ref LUM_AUX_BUFFERS) de tampons auxiliaires du tampon d'image.",
  ],
  [
    'OpenGL stereoscopic rendering [hint](@ref LUM_STEREO).',
    '[Indice](@ref LUM_STEREO) de rendu stéréoscopique OpenGL.',
  ],
  [
    'Framebuffer MSAA samples [hint](@ref LUM_SAMPLES).',
    "[Indice](@ref LUM_SAMPLES) d'échantillons MSAA du tampon d'image.",
  ],
  ['Framebuffer sRGB [hint](@ref LUM_SRGB_CAPABLE).', "[Indice](@ref LUM_SRGB_CAPABLE) sRGB du tampon d'image."],
  [
    'Monitor refresh rate [hint](@ref LUM_REFRESH_RATE).',
    "[Indice](@ref LUM_REFRESH_RATE) de taux de rafraîchissement de l'écran.",
  ],
  [
    'Framebuffer double buffering [hint](@ref LUM_DOUBLEBUFFER_hint) and [attribute](@ref LUM_DOUBLEBUFFER_attrib).',
    "[Indice](@ref LUM_DOUBLEBUFFER_hint) et [attribut](@ref LUM_DOUBLEBUFFER_attrib) de double tampon du tampon d'image.",
  ],
  [
    'Context client API [hint](@ref LUM_CLIENT_API_hint) and [attribute](@ref LUM_CLIENT_API_attrib).',
    "[Indice](@ref LUM_CLIENT_API_hint) et [attribut](@ref LUM_CLIENT_API_attrib) d'API cliente du contexte.",
  ],
  [
    'Context client API major version [hint](@ref LUM_CONTEXT_VERSION_MAJOR_hint) and [attribute](@ref LUM_CONTEXT_VERSION_MAJOR_attrib).',
    "[Indice](@ref LUM_CONTEXT_VERSION_MAJOR_hint) et [attribut](@ref LUM_CONTEXT_VERSION_MAJOR_attrib) de version majeure de l'API cliente du contexte.",
  ],
  [
    'Context client API minor version [hint](@ref LUM_CONTEXT_VERSION_MINOR_hint) and [attribute](@ref LUM_CONTEXT_VERSION_MINOR_attrib).',
    "[Indice](@ref LUM_CONTEXT_VERSION_MINOR_hint) et [attribut](@ref LUM_CONTEXT_VERSION_MINOR_attrib) de version mineure de l'API cliente du contexte.",
  ],
  [
    'Context client API revision number [attribute](@ref LUM_CONTEXT_REVISION_attrib).',
    "[Attribut](@ref LUM_CONTEXT_REVISION_attrib) de numéro de révision de l'API cliente du contexte.",
  ],
  [
    'Context client API revision number [hint](@ref LUM_CONTEXT_ROBUSTNESS_hint) and [attribute](@ref LUM_CONTEXT_ROBUSTNESS_attrib).',
    '[Indice](@ref LUM_CONTEXT_ROBUSTNESS_hint) et [attribut](@ref LUM_CONTEXT_ROBUSTNESS_attrib) de robustesse du contexte.',
  ],
  [
    'OpenGL forward-compatibility [hint](@ref LUM_OPENGL_FORWARD_COMPAT_hint) and [attribute](@ref LUM_OPENGL_FORWARD_COMPAT_attrib).',
    '[Indice](@ref LUM_OPENGL_FORWARD_COMPAT_hint) et [attribut](@ref LUM_OPENGL_FORWARD_COMPAT_attrib) de compatibilité en avant OpenGL.',
  ],
  [
    'Debug mode context [hint](@ref LUM_CONTEXT_DEBUG_hint) and [attribute](@ref LUM_CONTEXT_DEBUG_attrib).',
    '[Indice](@ref LUM_CONTEXT_DEBUG_hint) et [attribut](@ref LUM_CONTEXT_DEBUG_attrib) de contexte en mode débogage.',
  ],
  [
    'OpenGL profile [hint](@ref LUM_OPENGL_PROFILE_hint) and [attribute](@ref LUM_OPENGL_PROFILE_attrib).',
    '[Indice](@ref LUM_OPENGL_PROFILE_hint) et [attribut](@ref LUM_OPENGL_PROFILE_attrib) de profil OpenGL.',
  ],
  [
    'Context flush-on-release [hint](@ref LUM_CONTEXT_RELEASE_BEHAVIOR_hint) and [attribute](@ref LUM_CONTEXT_RELEASE_BEHAVIOR_attrib).',
    '[Indice](@ref LUM_CONTEXT_RELEASE_BEHAVIOR_hint) et [attribut](@ref LUM_CONTEXT_RELEASE_BEHAVIOR_attrib) de vidage du contexte à sa libération.',
  ],
  [
    'Context error suppression [hint](@ref LUM_CONTEXT_NO_ERROR_hint) and [attribute](@ref LUM_CONTEXT_NO_ERROR_attrib).',
    '[Indice](@ref LUM_CONTEXT_NO_ERROR_hint) et [attribut](@ref LUM_CONTEXT_NO_ERROR_attrib) de suppression des erreurs du contexte.',
  ],
  [
    'Context creation API [hint](@ref LUM_CONTEXT_CREATION_API_hint) and [attribute](@ref LUM_CONTEXT_CREATION_API_attrib).',
    "[Indice](@ref LUM_CONTEXT_CREATION_API_hint) et [attribut](@ref LUM_CONTEXT_CREATION_API_attrib) d'API de création de contexte.",
  ],

  // ---- cursor shapes -------------------------------------------------------
  [
    'The horizontal resize/move arrow shape. This is usually a horizontal double-headed arrow.',
    "La forme de flèche de redimensionnement ou déplacement horizontal.  C'est en général une double flèche horizontale.",
  ],
  [
    'The vertical resize/move shape. This is usually a vertical double-headed arrow.',
    "La forme de redimensionnement ou déplacement vertical.  C'est en général une double flèche verticale.",
  ],
  [
    'The top-left to bottom-right diagonal resize/move shape. This is usually a diagonal double-headed arrow.',
    "La forme de redimensionnement ou déplacement en diagonale haut-gauche vers bas-droite.  C'est en général une double flèche diagonale.",
  ],
  [
    'The top-right to bottom-left diagonal resize/move shape. This is usually a diagonal double-headed arrow.',
    "La forme de redimensionnement ou déplacement en diagonale haut-droite vers bas-gauche.  C'est en général une double flèche diagonale.",
  ],
  [
    'The omni-directional resize cursor/move shape. This is usually either a combined horizontal and vertical double-headed arrow or a grabbing hand.',
    "La forme de redimensionnement ou déplacement dans toutes les directions.  C'est en général une double flèche horizontale et verticale combinée, ou une main qui agrippe.",
  ],
  [
    'The operation-not-allowed shape. This is usually a circle with a diagonal line through it.',
    "La forme signalant une opération non permise.  C'est en général un cercle barré d'un trait diagonal.",
  ],

  // ---- init hints ----------------------------------------------------------
  [
    'Joystick hat buttons [init hint](@ref LUM_JOYSTICK_HAT_BUTTONS).',
    "[Indice d'initialisation](@ref LUM_JOYSTICK_HAT_BUTTONS) des croix de manette en boutons.",
  ],
  [
    'ANGLE rendering backend [init hint](@ref LUM_ANGLE_PLATFORM_TYPE_hint).',
    "[Indice d'initialisation](@ref LUM_ANGLE_PLATFORM_TYPE_hint) du backend de rendu ANGLE.",
  ],
  [
    'Platform selection [init hint](@ref LUM_PLATFORM).',
    "[Indice d'initialisation](@ref LUM_PLATFORM) de choix de la plateforme.",
  ],
  [
    'macOS specific [init hint](@ref LUM_COCOA_CHDIR_RESOURCES_hint).',
    "[Indice d'initialisation](@ref LUM_COCOA_CHDIR_RESOURCES_hint) propre à macOS.",
  ],
  [
    'macOS specific [init hint](@ref LUM_COCOA_MENUBAR_hint).',
    "[Indice d'initialisation](@ref LUM_COCOA_MENUBAR_hint) propre à macOS.",
  ],
  [
    'X11 specific [init hint](@ref LUM_X11_XCB_VULKAN_SURFACE_hint).',
    "[Indice d'initialisation](@ref LUM_X11_XCB_VULKAN_SURFACE_hint) propre à X11.",
  ],
  [
    'Hint value for @ref LUM_PLATFORM that enables automatic platform selection.',
    "Valeur d'indice pour @ref LUM_PLATFORM activant le choix automatique de la plateforme.",
  ],
];
