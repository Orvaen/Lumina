// Translation memory for the API reference.
//
// Each entry is [english, french]. The key is matched after collapsing
// whitespace, so it does not matter how the header happens to wrap its
// comment. A string with no entry here is rendered in English and listed in
// reference.todo.txt by the generator, sorted by how often it occurs.
//
// Identifiers, macro names, `@ref` targets and code stay as they are.
//
// Split across files by kind of string, because there are a thousand of them.

import briefs from './reference.briefs.mjs';
import returns from './reference.returns.mjs';
import params from './reference.params.mjs';
import constants from './reference.constants.mjs';
import remarks from './reference.remarks.mjs';
import windowdesc from './reference.windowdesc.mjs';

export default [
  ...briefs,
  ...returns,
  ...params,
  ...constants,
  ...remarks,
  ...windowdesc,

  // ---- thread safety, reentrancy -----------------------------------------
  [
    'This function must only be called from the main thread.',
    'Cette fonction ne doit être appelée que depuis le thread principal.',
  ],
  [
    'This function may be called from any thread.  Access is not synchronized.',
    "Cette fonction peut être appelée depuis n'importe quel thread.  L'accès n'est pas synchronisé.",
  ],
  [
    'This function may be called from any thread.',
    "Cette fonction peut être appelée depuis n'importe quel thread.",
  ],
  [
    'This function may be called from any thread that calls Lumina functions.',
    "Cette fonction peut être appelée depuis n'importe quel thread qui appelle des fonctions Lumina.",
  ],
  [
    'This function must not be called from a callback.',
    "Cette fonction ne doit pas être appelée depuis une fonction de rappel.",
  ],
  [
    'This function may be called before @ref lumInit.',
    'Cette fonction peut être appelée avant @ref lumInit.',
  ],
  [
    'This function should not call any Lumina function.',
    "Cette fonction ne devrait appeler aucune fonction de Lumina.",
  ],

  // ---- errors --------------------------------------------------------------
  [
    'Possible errors include @ref LUM_NOT_INITIALIZED.',
    'Les erreurs possibles incluent @ref LUM_NOT_INITIALIZED.',
  ],
  [
    'Possible errors include @ref LUM_NOT_INITIALIZED and @ref LUM_PLATFORM_ERROR.',
    'Les erreurs possibles incluent @ref LUM_NOT_INITIALIZED et @ref LUM_PLATFORM_ERROR.',
  ],
  [
    'Possible errors include @ref LUM_NO_WINDOW_CONTEXT and @ref LUM_NOT_INITIALIZED.',
    'Les erreurs possibles incluent @ref LUM_NO_WINDOW_CONTEXT et @ref LUM_NOT_INITIALIZED.',
  ],
  [
    'Possible errors include @ref LUM_NOT_INITIALIZED and @ref LUM_INVALID_ENUM.',
    'Les erreurs possibles incluent @ref LUM_NOT_INITIALIZED et @ref LUM_INVALID_ENUM.',
  ],
  [
    'Possible errors include @ref LUM_NOT_INITIALIZED, @ref LUM_INVALID_ENUM and @ref LUM_PLATFORM_ERROR.',
    'Les erreurs possibles incluent @ref LUM_NOT_INITIALIZED, @ref LUM_INVALID_ENUM et @ref LUM_PLATFORM_ERROR.',
  ],
  [
    'Possible errors include @ref LUM_NOT_INITIALIZED, @ref LUM_PLATFORM_ERROR and @ref LUM_FEATURE_UNAVAILABLE (see remarks).',
    'Les erreurs possibles incluent @ref LUM_NOT_INITIALIZED, @ref LUM_PLATFORM_ERROR et @ref LUM_FEATURE_UNAVAILABLE (voir les remarques).',
  ],
  [
    'Possible errors include @ref LUM_NOT_INITIALIZED, @ref LUM_INVALID_VALUE and @ref LUM_PLATFORM_ERROR.',
    'Les erreurs possibles incluent @ref LUM_NOT_INITIALIZED, @ref LUM_INVALID_VALUE et @ref LUM_PLATFORM_ERROR.',
  ],
  [
    'Possible errors include @ref LUM_NOT_INITIALIZED and @ref LUM_INVALID_VALUE.',
    'Les erreurs possibles incluent @ref LUM_NOT_INITIALIZED et @ref LUM_INVALID_VALUE.',
  ],
  [
    'Possible errors include @ref LUM_NOT_INITIALIZED, @ref LUM_NO_WINDOW_CONTEXT and @ref LUM_PLATFORM_ERROR.',
    'Les erreurs possibles incluent @ref LUM_NOT_INITIALIZED, @ref LUM_NO_WINDOW_CONTEXT et @ref LUM_PLATFORM_ERROR.',
  ],
  [
    'Possible errors include @ref LUM_NOT_INITIALIZED, @ref LUM_NO_CURRENT_CONTEXT and @ref LUM_PLATFORM_ERROR.',
    'Les erreurs possibles incluent @ref LUM_NOT_INITIALIZED, @ref LUM_NO_CURRENT_CONTEXT et @ref LUM_PLATFORM_ERROR.',
  ],
  [
    'Possible errors include @ref LUM_NOT_INITIALIZED and @ref LUM_API_UNAVAILABLE.',
    'Les erreurs possibles incluent @ref LUM_NOT_INITIALIZED et @ref LUM_API_UNAVAILABLE.',
  ],

  // ---- since ---------------------------------------------------------------
  ['Added in version 1.0.', 'Ajoutée en version 1.0.'],
  ['Added in version 2.1.', 'Ajoutée en version 2.1.'],
  ['Added in version 2.5.', 'Ajoutée en version 2.5.'],
  ['Added in version 3.0.', 'Ajoutée en version 3.0.'],
  ['Added in version 3.1.', 'Ajoutée en version 3.1.'],
  ['Added in version 3.2.', 'Ajoutée en version 3.2.'],
  ['Added in version 3.3.', 'Ajoutée en version 3.3.'],
  ['Added in version 3.4.', 'Ajoutée en version 3.4.'],
  [
    'Added in version 1.0.  @lumina Added window handle parameter.',
    'Ajoutée en version 1.0.  @lumina Ajout du paramètre de handle de fenêtre.',
  ],
  [
    'Added in version 1.0.  @lumina Added window handle parameter and return value.',
    'Ajoutée en version 1.0.  @lumina Ajout du paramètre de handle de fenêtre et de la valeur de retour.',
  ],
  [
    'Added in version 2.1.  @lumina Added window handle parameter.',
    'Ajoutée en version 2.1.  @lumina Ajout du paramètre de handle de fenêtre.',
  ],
  [
    'Added in version 2.5.  @lumina Added window handle parameter.',
    'Ajoutée en version 2.5.  @lumina Ajout du paramètre de handle de fenêtre.',
  ],
  [
    'Added in version 2.5.  @lumina Added window handle parameter and return value.',
    'Ajoutée en version 2.5.  @lumina Ajout du paramètre de handle de fenêtre et de la valeur de retour.',
  ],
  ['Scheduled for removal in version 4.0.', 'Suppression prévue en version 4.0.'],

  // ---- callbacks: common parameters and returns ---------------------------
  [
    'The previously set callback, or `NULL` if no callback was set or the library had not been [initialized](@ref intro_init).',
    "La fonction de rappel précédemment installée, ou `NULL` si aucune ne l'était ou si la bibliothèque n'avait pas été [initialisée](@ref intro_init).",
  ],
  [
    'The new callback, or `NULL` to remove the currently set callback.',
    "La nouvelle fonction de rappel, ou `NULL` pour retirer celle actuellement installée.",
  ],
  ['The window whose callback to set.', 'La fenêtre dont on installe la fonction de rappel.'],
  ['The window to query.', 'La fenêtre à interroger.'],
  ['The window that received the event.', "La fenêtre qui a reçu l'événement."],
  ['The window that was moved.', 'La fenêtre qui a été déplacée.'],
  ['The desired window.', 'La fenêtre voulue.'],
  ['The monitor to query.', "L'écran à interroger."],
  ['The [joystick](@ref joysticks) to query.', 'La [manette](@ref joysticks) à interroger.'],
  ['The new value.', 'La nouvelle valeur.'],
  ['A UTF-8 encoded string.', 'Une chaîne encodée en UTF-8.'],
  ['Deprecated.  Any valid window or `NULL`.', 'Déprécié.  Une fenêtre valide quelconque, ou `NULL`.'],
  ['The ASCII encoded name of the function.', 'Le nom de la fonction, encodé en ASCII.'],
  ['The Unicode code point of the character.', 'Le point de code Unicode du caractère.'],
  [
    'The user-defined pointer from the allocator.',
    "Le pointeur défini par l'utilisateur, issu de l'allocateur.",
  ],
  [
    'Bit field describing which [modifier keys](@ref mods) were held down.',
    'Champ de bits décrivant quelles [touches de modification](@ref mods) étaient enfoncées.',
  ],
  [
    'One of `LUM_CURSOR`, `LUM_STICKY_KEYS`, `LUM_STICKY_MOUSE_BUTTONS`, `LUM_LOCK_KEY_MODS` or `LUM_RAW_MOUSE_MOTION`.',
    "L'un de `LUM_CURSOR`, `LUM_STICKY_KEYS`, `LUM_STICKY_MOUSE_BUTTONS`, `LUM_LOCK_KEY_MODS` ou `LUM_RAW_MOUSE_MOTION`.",
  ],
  ['One of `LUM_PRESS` or `LUM_RELEASE`.', "L'un de `LUM_PRESS` ou `LUM_RELEASE`."],
  [
    'One of `LUM_CONNECTED` or `LUM_DISCONNECTED`.  Future releases may add more events.',
    "L'un de `LUM_CONNECTED` ou `LUM_DISCONNECTED`.  De futures versions pourront ajouter d'autres événements.",
  ],
  [
    '`LUM_TRUE` if successful, or `LUM_FALSE` if an [error](@ref error_handling) occurred.',
    "`LUM_TRUE` en cas de succès, ou `LUM_FALSE` si une [erreur](@ref error_handling) est survenue.",
  ],
  [
    'The address of the function, or `NULL` if an [error](@ref error_handling) occurred.',
    "L'adresse de la fonction, ou `NULL` si une [erreur](@ref error_handling) est survenue.",
  ],
  ['Where to store the monitor x-coordinate, or `NULL`.', "Où stocker l'abscisse de l'écran, ou `NULL`."],
  ['Where to store the monitor y-coordinate, or `NULL`.', "Où stocker l'ordonnée de l'écran, ou `NULL`."],
  [
    'Where to store the x-axis content scale, or `NULL`.',
    "Où stocker le facteur d'échelle du contenu sur l'axe X, ou `NULL`.",
  ],
  [
    'Where to store the y-axis content scale, or `NULL`.',
    "Où stocker le facteur d'échelle du contenu sur l'axe Y, ou `NULL`.",
  ],
  ['The monitor whose gamma ramp to set.', "L'écran dont on règle la rampe gamma."],
  ['The [window hint](@ref window_hints) to set.', "L'[indice de fenêtre](@ref window_hints) à régler."],
  ['The new value of the window hint.', "La nouvelle valeur de l'indice de fenêtre."],
  ['The window to set limits for.', 'La fenêtre dont on fixe les limites.'],
  ['None.', 'Aucune.'],

  // ---- pointer lifetime ----------------------------------------------------
  [
    'The specified string is copied before this function returns.',
    'La chaîne indiquée est copiée avant que cette fonction ne rende la main.',
  ],
  [
    'The specified image data is copied before this function returns.',
    "Les données d'image indiquées sont copiées avant que cette fonction ne rende la main.",
  ],
  [
    'The returned array is allocated and freed by Lumina.  You should not free it yourself.  It is valid until the specified joystick is disconnected or the library is terminated.',
    "Le tableau retourné est alloué et libéré par Lumina.  Vous ne devez pas le libérer vous-même.  Il reste valide jusqu'à la déconnexion de la manette indiquée ou la terminaison de la bibliothèque.",
  ],
  [
    'The returned string is allocated and freed by Lumina.  You should not free it yourself.  It is valid until the specified joystick is disconnected or the library is terminated.',
    "La chaîne retournée est allouée et libérée par Lumina.  Vous ne devez pas la libérer vous-même.  Elle reste valide jusqu'à la déconnexion de la manette indiquée ou la terminaison de la bibliothèque.",
  ],
  [
    'The returned memory block must be valid at least until it is deallocated.',
    "Le bloc mémoire retourné doit rester valide au moins jusqu'à sa libération.",
  ],

  // ---- remarks and notes ---------------------------------------------------
  [
    'Application programmer error.  Fix the offending call.',
    "Erreur de programmation de l'application.  Corrigez l'appel fautif.",
  ],
  [
    '@x11 This shape is provided by a newer standard not supported by all cursor themes.',
    '@x11 Cette forme provient d\'un standard récent que tous les thèmes de curseur ne prennent pas en charge.',
  ],
  [
    '@wayland This shape is provided by a newer standard not supported by all cursor themes.',
    '@wayland Cette forme provient d\'un standard récent que tous les thèmes de curseur ne prennent pas en charge.',
  ],
  [
    '@macos This shape is provided by a private system API and may fail with @ref LUM_CURSOR_UNAVAILABLE in the future.',
    '@macos Cette forme provient d\'une API système privée et pourra échouer avec @ref LUM_CURSOR_UNAVAILABLE à l\'avenir.',
  ],
  [
    '@wayland Gamma handling is a privileged protocol, this function will thus never be implemented and emits @ref LUM_PLATFORM_ERROR.',
    '@wayland La gestion du gamma relève d\'un protocole privilégié : cette fonction ne sera donc jamais implémentée et émet @ref LUM_PLATFORM_ERROR.',
  ],
  [
    'If you set size limits and an aspect ratio that conflict, the results are undefined.',
    'Si les limites de taille et la proportion que vous fixez entrent en conflit, le résultat est indéfini.',
  ],
  [
    'This function may be called from any thread.  For synchronization details of Vulkan objects, see the Vulkan specification.',
    "Cette fonction peut être appelée depuis n'importe quel thread.  Pour les détails de synchronisation des objets Vulkan, voyez la spécification Vulkan.",
  ],
  ['See @ref gamepad for how these are used.', 'Voyez @ref gamepad pour leur usage.'],

  // ---- group descriptions --------------------------------------------------
  [
    'Functions and types related to OpenGL and OpenGL ES contexts.',
    'Fonctions et types liés aux contextes OpenGL et OpenGL ES.',
  ],
  [
    'This is the reference documentation for OpenGL and OpenGL ES context related functions.  For more task-oriented information, see the @ref context_guide.',
    'Documentation de référence des fonctions liées aux contextes OpenGL et OpenGL ES.  Pour une approche par tâches, voyez @ref context_guide.',
  ],
  ['Functions and types related to Vulkan.', 'Fonctions et types liés à Vulkan.'],
  [
    'This is the reference documentation for Vulkan related functions and types.  For more task-oriented information, see the @ref vulkan_guide.',
    'Documentation de référence des fonctions et types liés à Vulkan.  Pour une approche par tâches, voyez @ref vulkan_guide.',
  ],
  [
    'Functions and types related to initialization and error handling.',
    "Fonctions et types liés à l'initialisation et à la gestion des erreurs.",
  ],
  [
    'This is the reference documentation for initialization and termination of the library, version management and error handling.  For more task-oriented information, see the @ref intro_guide.',
    "Documentation de référence de l'initialisation et de la terminaison de la bibliothèque, de la gestion des versions et des erreurs.  Pour une approche par tâches, voyez @ref intro_guide.",
  ],
  [
    'Functions and types related to input handling.',
    'Fonctions et types liés à la gestion des entrées.',
  ],
  [
    'This is the reference documentation for input related functions and types.  For more task-oriented information, see the @ref input_guide.',
    'Documentation de référence des fonctions et types liés aux entrées.  Pour une approche par tâches, voyez @ref input_guide.',
  ],
  ['Functions and types related to monitors.', 'Fonctions et types liés aux écrans.'],
  [
    'This is the reference documentation for monitor related functions and types.  For more task-oriented information, see the @ref monitor_guide.',
    'Documentation de référence des fonctions et types liés aux écrans.  Pour une approche par tâches, voyez @ref monitor_guide.',
  ],
  ['Functions and types related to windows.', 'Fonctions et types liés aux fenêtres.'],

  // ---- types ---------------------------------------------------------------
  ['Opaque monitor object.', 'Objet écran opaque.'],
  ['Opaque window object.', 'Objet fenêtre opaque.'],
  ['Opaque cursor object.', 'Objet curseur opaque.'],

  // ---- constants -----------------------------------------------------------
  ['Framebuffer bit depth hint.', "Indice de profondeur de bits du tampon d'image."],
  ['Legacy name for compatibility.', 'Ancien nom conservé par compatibilité.'],
  [
    'This is an alias for compatibility with earlier versions.',
    'Alias conservé par compatibilité avec les versions antérieures.',
  ],
  ['The key or mouse button was released.', 'La touche ou le bouton de souris a été relâché.'],
  ['The key or mouse button was pressed.', 'La touche ou le bouton de souris a été enfoncé.'],
  [
    'The key was held down until it repeated.',
    "La touche est restée enfoncée jusqu'à se répéter.",
  ],
  [
    'If this bit is set one or more Shift keys were held down.',
    'Si ce bit est positionné, une ou plusieurs touches Maj étaient enfoncées.',
  ],
  [
    'If this bit is set one or more Control keys were held down.',
    'Si ce bit est positionné, une ou plusieurs touches Ctrl étaient enfoncées.',
  ],
  [
    'If this bit is set one or more Alt keys were held down.',
    'Si ce bit est positionné, une ou plusieurs touches Alt étaient enfoncées.',
  ],
  [
    'If this bit is set one or more Super keys were held down.',
    'Si ce bit est positionné, une ou plusieurs touches Super étaient enfoncées.',
  ],
  ['No error has occurred.', "Aucune erreur n'est survenue."],
  ['A memory allocation failed.', 'Une allocation mémoire a échoué.'],
  [
    'Lumina could not find support for the requested API on the system.',
    "Lumina n'a pas trouvé de prise en charge de l'API demandée sur le système.",
  ],
  [
    'A platform-specific error occurred that does not match any of the more specific categories.',
    "Une erreur propre à la plateforme est survenue, ne correspondant à aucune catégorie plus précise.",
  ],
  [
    'Window decoration window hint and attribute',
    'Indice et attribut de fenêtre pour la décoration',
  ],
  [
    'X11 specific [window hint](@ref LUM_X11_CLASS_NAME_hint).',
    '[Indice de fenêtre](@ref LUM_X11_CLASS_NAME_hint) propre à X11.',
  ],
  ['macOS specific init hint.', "Indice d'initialisation propre à macOS."],
  ['The regular arrow cursor shape.', 'La forme de curseur flèche habituelle.'],
  ['The text input I-beam cursor shape.', 'La forme de curseur en I pour la saisie de texte.'],
  ['The crosshair cursor shape.', 'La forme de curseur réticule.'],
  ['The pointing hand cursor shape.', 'La forme de curseur main pointant.'],

  // ---- misc ----------------------------------------------------------------
  ['Sets the specified window hint to the desired value.', "Règle l'indice de fenêtre indiqué à la valeur voulue."],

  // ---- since, remaining ----------------------------------------------------
  [
    'Added in version 1.0. @lumina Changed to return an array of modes for a specific monitor.',
    "Ajoutée en version 1.0. @lumina Retourne désormais un tableau de modes pour un écran donné.",
  ],
  ['Added in version 3.0. Replaces `lumGetDesktopMode`.', 'Ajoutée en version 3.0. Remplace `lumGetDesktopMode`.'],
  ['Added in version 3.0. Replaces `lumOpenWindowHint`.', 'Ajoutée en version 3.0. Remplace `lumOpenWindowHint`.'],
  ['Added in version 3.0. Replaces `lumOpenWindow`.', 'Ajoutée en version 3.0. Remplace `lumOpenWindow`.'],
  ['Added in version 3.0. Replaces `lumCloseWindow`.', 'Ajoutée en version 3.0. Remplace `lumCloseWindow`.'],
  ['Added in Lumina 3.2.', 'Ajoutée dans Lumina 3.2.'],
  [
    'Added in version 3.0. Replaces `lumGetWindowParam` and `lumGetGLVersion`.',
    'Ajoutée en version 3.0. Remplace `lumGetWindowParam` et `lumGetGLVersion`.',
  ],
  [
    'Added in version 3.0. Replaces `lumEnable` and `lumDisable`.',
    'Ajoutée en version 3.0. Remplace `lumEnable` et `lumDisable`.',
  ],
  ['Added in version 3.0. Replaces `lumGetMousePos`.', 'Ajoutée en version 3.0. Remplace `lumGetMousePos`.'],
  ['Added in version 3.0. Replaces `lumSetMousePos`.', 'Ajoutée en version 3.0. Remplace `lumSetMousePos`.'],
  [
    'Added in version 2.4. @lumina Added window handle parameter and return value.',
    'Ajoutée en version 2.4. @lumina Ajout du paramètre de handle de fenêtre et de la valeur de retour.',
  ],
  [
    'Added in version 3.0. Replaces `lumSetMousePosCallback`.',
    'Ajoutée en version 3.0. Remplace `lumSetMousePosCallback`.',
  ],
  [
    'Added in version 3.0. Replaces `lumSetMouseWheelCallback`.',
    'Ajoutée en version 3.0. Remplace `lumSetMouseWheelCallback`.',
  ],
  ['Added in version 3.0. Replaces `lumGetJoystickParam`.', 'Ajoutée en version 3.0. Remplace `lumGetJoystickParam`.'],
  ['Added in version 3.0. Replaces `lumGetJoystickPos`.', 'Ajoutée en version 3.0. Remplace `lumGetJoystickPos`.'],
  [
    'Added in version 2.2. @lumina Changed to return a dynamic array.',
    'Ajoutée en version 2.2. @lumina Retourne désormais un tableau dynamique.',
  ],
  ['Added in version 2.2.', 'Ajoutée en version 2.2.'],
  [
    'Added in version 1.0. @lumina Added window handle and modifier mask parameters.',
    'Ajoutée en version 1.0. @lumina Ajout des paramètres de handle de fenêtre et de masque de modificateurs.',
  ],
  ['Added in version 3.0. Replaces `LUMmouseposfun`.', 'Ajoutée en version 3.0. Remplace `LUMmouseposfun`.'],
  ['Added in version 3.0. Replaces `LUMmousewheelfun`.', 'Ajoutée en version 3.0. Remplace `LUMmousewheelfun`.'],
  [
    'Added in version 1.0. @lumina Added window handle, scancode and modifier mask parameters.',
    'Ajoutée en version 1.0. @lumina Ajout des paramètres de handle de fenêtre, de scancode et de masque de modificateurs.',
  ],
  [
    'Added in version 2.4. @lumina Added window handle parameter.',
    'Ajoutée en version 2.4. @lumina Ajout du paramètre de handle de fenêtre.',
  ],
  [
    'Added in version 1.0. @lumina Added refresh rate member.',
    'Ajoutée en version 1.0. @lumina Ajout du membre taux de rafraîchissement.',
  ],
  [
    'Added in version 2.1. @lumina Removed format and bytes-per-pixel members.',
    'Ajoutée en version 2.1. @lumina Suppression des membres format et octets par pixel.',
  ],

  // ---- thread safety, remaining -------------------------------------------
  [
    'This function may be called from any thread. Reading and writing of the internal base time is not atomic, so it needs to be externally synchronized with calls to @ref lumSetTime.',
    "Cette fonction peut être appelée depuis n'importe quel thread. La lecture et l'écriture du temps de référence interne ne sont pas atomiques : elles doivent donc être synchronisées de l'extérieur avec les appels à @ref lumSetTime.",
  ],
  [
    'This function may be called from any thread. Reading and writing of the internal base time is not atomic, so it needs to be externally synchronized with calls to @ref lumGetTime.',
    "Cette fonction peut être appelée depuis n'importe quel thread. La lecture et l'écriture du temps de référence interne ne sont pas atomiques : elles doivent donc être synchronisées de l'extérieur avec les appels à @ref lumGetTime.",
  ],

  // ---- errors, remaining ---------------------------------------------------
  [
    'Possible errors include @ref LUM_PLATFORM_UNAVAILABLE and @ref LUM_PLATFORM_ERROR.',
    'Les erreurs possibles incluent @ref LUM_PLATFORM_UNAVAILABLE et @ref LUM_PLATFORM_ERROR.',
  ],
  ['Possible errors include @ref LUM_PLATFORM_ERROR.', 'Les erreurs possibles incluent @ref LUM_PLATFORM_ERROR.'],
  [
    'Possible errors include @ref LUM_INVALID_ENUM and @ref LUM_INVALID_VALUE.',
    'Les erreurs possibles incluent @ref LUM_INVALID_ENUM et @ref LUM_INVALID_VALUE.',
  ],
  ['Possible errors include @ref LUM_INVALID_VALUE.', 'Les erreurs possibles incluent @ref LUM_INVALID_VALUE.'],
  ['Possible errors include @ref LUM_INVALID_ENUM.', 'Les erreurs possibles incluent @ref LUM_INVALID_ENUM.'],
  [
    'Possible errors include @ref LUM_NOT_INITIALIZED, @ref LUM_INVALID_ENUM, @ref LUM_INVALID_VALUE, @ref LUM_API_UNAVAILABLE, @ref LUM_VERSION_UNAVAILABLE, @ref LUM_FORMAT_UNAVAILABLE and @ref LUM_PLATFORM_ERROR.',
    'Les erreurs possibles incluent @ref LUM_NOT_INITIALIZED, @ref LUM_INVALID_ENUM, @ref LUM_INVALID_VALUE, @ref LUM_API_UNAVAILABLE, @ref LUM_VERSION_UNAVAILABLE, @ref LUM_FORMAT_UNAVAILABLE et @ref LUM_PLATFORM_ERROR.',
  ],
  [
    'Possible errors include @ref LUM_NOT_INITIALIZED and @ref LUM_PLATFORM_ERROR. @par Thread Safety This function may only be called from the main thread.',
    'Les erreurs possibles incluent @ref LUM_NOT_INITIALIZED et @ref LUM_PLATFORM_ERROR. @par Sûreté des threads Cette fonction ne peut être appelée que depuis le thread principal.',
  ],
  [
    'Possible errors include @ref LUM_NOT_INITIALIZED, @ref LUM_INVALID_ENUM, @ref LUM_INVALID_VALUE and @ref LUM_PLATFORM_ERROR.',
    'Les erreurs possibles incluent @ref LUM_NOT_INITIALIZED, @ref LUM_INVALID_ENUM, @ref LUM_INVALID_VALUE et @ref LUM_PLATFORM_ERROR.',
  ],
  [
    'Possible errors include @ref LUM_NOT_INITIALIZED, @ref LUM_INVALID_ENUM, @ref LUM_PLATFORM_ERROR and @ref LUM_FEATURE_UNAVAILABLE (see above).',
    'Les erreurs possibles incluent @ref LUM_NOT_INITIALIZED, @ref LUM_INVALID_ENUM, @ref LUM_PLATFORM_ERROR et @ref LUM_FEATURE_UNAVAILABLE (voir ci-dessus).',
  ],
  [
    'Possible errors include @ref LUM_NOT_INITIALIZED, @ref LUM_INVALID_ENUM, @ref LUM_CURSOR_UNAVAILABLE and @ref LUM_PLATFORM_ERROR.',
    'Les erreurs possibles incluent @ref LUM_NOT_INITIALIZED, @ref LUM_INVALID_ENUM, @ref LUM_CURSOR_UNAVAILABLE et @ref LUM_PLATFORM_ERROR.',
  ],
  [
    'Possible errors include @ref LUM_NOT_INITIALIZED, @ref LUM_FORMAT_UNAVAILABLE and @ref LUM_PLATFORM_ERROR.',
    'Les erreurs possibles incluent @ref LUM_NOT_INITIALIZED, @ref LUM_FORMAT_UNAVAILABLE et @ref LUM_PLATFORM_ERROR.',
  ],
  [
    'Possible errors include @ref LUM_NOT_INITIALIZED, @ref LUM_NO_CURRENT_CONTEXT, @ref LUM_INVALID_VALUE and @ref LUM_PLATFORM_ERROR.',
    'Les erreurs possibles incluent @ref LUM_NOT_INITIALIZED, @ref LUM_NO_CURRENT_CONTEXT, @ref LUM_INVALID_VALUE et @ref LUM_PLATFORM_ERROR.',
  ],
  [
    'Possible errors include @ref LUM_NOT_INITIALIZED, @ref LUM_API_UNAVAILABLE and @ref LUM_PLATFORM_ERROR.',
    'Les erreurs possibles incluent @ref LUM_NOT_INITIALIZED, @ref LUM_API_UNAVAILABLE et @ref LUM_PLATFORM_ERROR.',
  ],
  [
    'Possible errors include @ref LUM_NOT_INITIALIZED, @ref LUM_API_UNAVAILABLE, @ref LUM_PLATFORM_ERROR and @ref LUM_INVALID_VALUE',
    'Les erreurs possibles incluent @ref LUM_NOT_INITIALIZED, @ref LUM_API_UNAVAILABLE, @ref LUM_PLATFORM_ERROR et @ref LUM_INVALID_VALUE',
  ],

  // ---- note ----------------------------------------------------------------
  [
    'The context of the specified window must not be current on any other thread when this function is called.',
    "Le contexte de la fenêtre indiquée ne doit être courant sur aucun autre thread au moment de l'appel.",
  ],

  // ---- group briefs and descriptions --------------------------------------
  ['Joystick hat states.', 'États de croix directionnelle de manette.'],
  ['Keyboard key IDs.', 'Identifiants de touches du clavier.'],
  ['Modifier key flags.', 'Indicateurs de touches de modification.'],
  ['Mouse button IDs.', 'Identifiants de boutons de souris.'],
  ['Joystick IDs.', 'Identifiants de manettes.'],
  ['Gamepad buttons.', 'Boutons de manette de jeu.'],
  ['Gamepad axes.', 'Axes de manette de jeu.'],
  ['Error codes.', "Codes d'erreur."],
  ['Standard system cursor shapes.', 'Formes de curseur standard du système.'],
  [
    'Functions related to accessing native handles.',
    "Fonctions d'accès aux handles natifs.",
  ],
  [
    'This is the reference documentation for window related functions and types, including creation, deletion and event polling. For more task-oriented information, see the @ref window_guide.',
    "Documentation de référence des fonctions et types liés aux fenêtres : création, destruction et interrogation des événements. Pour une approche par tâches, voyez @ref window_guide.",
  ],
  [
    'See [joystick hat input](@ref joystick_hat) for how these are used.',
    "Voyez la [saisie des croix de manette](@ref joystick_hat) pour leur usage.",
  ],
  [
    'See [key input](@ref input_key) for how these are used. These key codes are inspired by the _USB HID Usage Tables v1.12_ (p. 53-60), but re-arranged to map to 7-bit ASCII for printable keys (function keys are put in the 256+ range). The naming of the key codes follow these rules: - The US keyboard layout is used - Names of printable alphanumeric characters are used (e.g. "A", "R", "3", etc.) - For non-alphanumeric characters, Unicode:ish names are used (e.g. "COMMA", "LEFT_SQUARE_BRACKET", etc.). Note that some names do not correspond to the Unicode standard (usually for brevity) - Keys that lack a clear US mapping are named "WORLD_x" - For non-printable keys, custom names are used (e.g. "F4", "BACKSPACE", etc.)',
    'Voyez la [saisie des touches](@ref input_key) pour leur usage. Ces codes de touche s\'inspirent des _USB HID Usage Tables v1.12_ (p. 53-60), réarrangés pour correspondre à l\'ASCII 7 bits pour les touches imprimables (les touches de fonction sont placées au-delà de 256). Leur nommage suit ces règles : - La disposition clavier américaine sert de référence - Les caractères alphanumériques imprimables donnent leur nom (par exemple "A", "R", "3") - Pour les caractères non alphanumériques, des noms d\'inspiration Unicode sont employés (par exemple "COMMA", "LEFT_SQUARE_BRACKET"). Certains ne correspondent pas au standard Unicode, en général par souci de concision - Les touches sans correspondance américaine claire sont nommées "WORLD_x" - Pour les touches non imprimables, des noms propres sont employés (par exemple "F4", "BACKSPACE")',
  ],
  [
    'See [key input](@ref input_key) for how these are used.',
    'Voyez la [saisie des touches](@ref input_key) pour leur usage.',
  ],
  [
    'See [mouse button input](@ref input_mouse_button) for how these are used.',
    'Voyez la [saisie des boutons de souris](@ref input_mouse_button) pour leur usage.',
  ],
  [
    'See [joystick input](@ref joystick) for how these are used.',
    'Voyez les [entrées de manette](@ref joystick) pour leur usage.',
  ],
  [
    'See [error handling](@ref error_handling) for how these are used.',
    'Voyez la [gestion des erreurs](@ref error_handling) pour leur usage.',
  ],
  [
    'These are the [standard cursor shapes](@ref cursor_standard) that can be requested from the platform (window system).',
    'Ce sont les [formes de curseur standard](@ref cursor_standard) que l\'on peut demander à la plateforme, c\'est-à-dire au système de fenêtrage.',
  ],
  [
    "**By using the native access functions you assert that you know what you're doing and how to fix problems caused by using them. If you don't, you shouldn't be using them.** Before the inclusion of @ref lumina_native.h, you may define zero or more window system API macro and zero or more context creation API macros. The chosen backends must match those the library was compiled for. Failure to do this will cause a link-time error. The available window API macros are: * `LUM_EXPOSE_NATIVE_WIN32` * `LUM_EXPOSE_NATIVE_COCOA` * `LUM_EXPOSE_NATIVE_X11` * `LUM_EXPOSE_NATIVE_WAYLAND` The available context API macros are: * `LUM_EXPOSE_NATIVE_WGL` * `LUM_EXPOSE_NATIVE_NSGL` * `LUM_EXPOSE_NATIVE_GLX` * `LUM_EXPOSE_NATIVE_EGL` * `LUM_EXPOSE_NATIVE_OSMESA` These macros select which of the native access functions that are declared and which platform-specific headers to include. It is then up your (by definition platform-specific) code to handle which of these should be defined.",
    "**En utilisant les fonctions d'accès natif, vous affirmez savoir ce que vous faites et savoir corriger les problèmes qu'elles peuvent causer. Si ce n'est pas le cas, ne les utilisez pas.** Avant d'inclure @ref lumina_native.h, vous pouvez définir zéro macro d'API de système de fenêtrage ou plus, et zéro macro d'API de création de contexte ou plus. Les backends choisis doivent correspondre à ceux pour lesquels la bibliothèque a été compilée, sans quoi vous obtiendrez une erreur d'édition de liens. Les macros d'API de fenêtrage disponibles sont : * `LUM_EXPOSE_NATIVE_WIN32` * `LUM_EXPOSE_NATIVE_COCOA` * `LUM_EXPOSE_NATIVE_X11` * `LUM_EXPOSE_NATIVE_WAYLAND` Les macros d'API de contexte disponibles sont : * `LUM_EXPOSE_NATIVE_WGL` * `LUM_EXPOSE_NATIVE_NSGL` * `LUM_EXPOSE_NATIVE_GLX` * `LUM_EXPOSE_NATIVE_EGL` * `LUM_EXPOSE_NATIVE_OSMESA` Ces macros déterminent quelles fonctions d'accès natif sont déclarées et quels en-têtes propres à la plateforme sont inclus. C'est ensuite à votre code — spécifique à la plateforme par définition — de décider lesquelles définir.",
  ],

  // ---- struct members ------------------------------------------------------
  ['The width, in screen coordinates, of the video mode.', "La largeur du mode vidéo, en coordonnées d'écran."],
  ['The height, in screen coordinates, of the video mode.', "La hauteur du mode vidéo, en coordonnées d'écran."],
  ['The bit depth of the red channel of the video mode.', 'La profondeur de bits du canal rouge du mode vidéo.'],
  ['The bit depth of the green channel of the video mode.', 'La profondeur de bits du canal vert du mode vidéo.'],
  ['The bit depth of the blue channel of the video mode.', 'La profondeur de bits du canal bleu du mode vidéo.'],
  ['The refresh rate, in Hz, of the video mode.', 'Le taux de rafraîchissement du mode vidéo, en Hz.'],
  [
    'An array of value describing the response of the red channel.',
    'Un tableau de valeurs décrivant la réponse du canal rouge.',
  ],
  [
    'An array of value describing the response of the green channel.',
    'Un tableau de valeurs décrivant la réponse du canal vert.',
  ],
  [
    'An array of value describing the response of the blue channel.',
    'Un tableau de valeurs décrivant la réponse du canal bleu.',
  ],
  ['The number of elements in each array.', "Le nombre d'éléments de chaque tableau."],
  ['The width, in pixels, of this image.', 'La largeur de cette image, en pixels.'],
  ['The height, in pixels, of this image.', 'La hauteur de cette image, en pixels.'],
  [
    'The pixel data of this image, arranged left-to-right, top-to-bottom.',
    'Les données de pixels de cette image, rangées de gauche à droite et de haut en bas.',
  ],
  [
    'The states of each [gamepad button](@ref gamepad_buttons), `LUM_PRESS` or `LUM_RELEASE`.',
    "L'état de chaque [bouton de manette](@ref gamepad_buttons) : `LUM_PRESS` ou `LUM_RELEASE`.",
  ],
  [
    'The states of each [gamepad axis](@ref gamepad_axes), in the range -1.0 to 1.0 inclusive.',
    "L'état de chaque [axe de manette](@ref gamepad_axes), dans l'intervalle -1.0 à 1.0 inclus.",
  ],

  // ---- pointer lifetime, remaining ----------------------------------------
  [
    'The specified allocator is copied before this function returns.',
    "L'allocateur indiqué est copié avant que cette fonction ne rende la main.",
  ],
  [
    'The returned string is static and compile-time generated.',
    'La chaîne retournée est statique et produite à la compilation.',
  ],
  [
    'The returned string is allocated and freed by Lumina. You should not free it yourself. It is guaranteed to be valid only until the next error occurs or the library is terminated.',
    "La chaîne retournée est allouée et libérée par Lumina. Vous ne devez pas la libérer vous-même. Elle n'est garantie valide que jusqu'à l'erreur suivante ou la terminaison de la bibliothèque.",
  ],
  [
    'The returned array is allocated and freed by Lumina. You should not free it yourself. It is guaranteed to be valid only until the monitor configuration changes or the library is terminated.',
    "Le tableau retourné est alloué et libéré par Lumina. Vous ne devez pas le libérer vous-même. Il n'est garanti valide que jusqu'à un changement de configuration des écrans ou la terminaison de la bibliothèque.",
  ],
  [
    'The returned string is allocated and freed by Lumina. You should not free it yourself. It is valid until the specified monitor is disconnected or the library is terminated.',
    "La chaîne retournée est allouée et libérée par Lumina. Vous ne devez pas la libérer vous-même. Elle reste valide jusqu'à la déconnexion de l'écran indiqué ou la terminaison de la bibliothèque.",
  ],
  [
    'The returned array is allocated and freed by Lumina. You should not free it yourself. It is valid until the specified monitor is disconnected, this function is called again for that monitor or the library is terminated.',
    "Le tableau retourné est alloué et libéré par Lumina. Vous ne devez pas le libérer vous-même. Il reste valide jusqu'à la déconnexion de l'écran indiqué, un nouvel appel de cette fonction pour cet écran, ou la terminaison de la bibliothèque.",
  ],
  [
    'The returned array is allocated and freed by Lumina. You should not free it yourself. It is valid until the specified monitor is disconnected or the library is terminated.',
    "Le tableau retourné est alloué et libéré par Lumina. Vous ne devez pas le libérer vous-même. Il reste valide jusqu'à la déconnexion de l'écran indiqué ou la terminaison de la bibliothèque.",
  ],
  [
    'The returned structure and its arrays are allocated and freed by Lumina. You should not free them yourself. They are valid until the specified monitor is disconnected, this function is called again for that monitor or the library is terminated.',
    "La structure retournée et ses tableaux sont alloués et libérés par Lumina. Vous ne devez pas les libérer vous-même. Ils restent valides jusqu'à la déconnexion de l'écran indiqué, un nouvel appel de cette fonction pour cet écran, ou la terminaison de la bibliothèque.",
  ],
  [
    'The specified gamma ramp is copied before this function returns.',
    'La rampe gamma indiquée est copiée avant que cette fonction ne rende la main.',
  ],
  [
    'The returned string is allocated and freed by Lumina. You should not free it yourself. It is valid until the library is terminated.',
    "La chaîne retournée est allouée et libérée par Lumina. Vous ne devez pas la libérer vous-même. Elle reste valide jusqu'à la terminaison de la bibliothèque.",
  ],
  [
    'The returned array is allocated and freed by Lumina. You should not free it yourself. It is valid until the specified joystick is disconnected, this function is called again for that joystick or the library is terminated.',
    "Le tableau retourné est alloué et libéré par Lumina. Vous ne devez pas le libérer vous-même. Il reste valide jusqu'à la déconnexion de la manette indiquée, un nouvel appel de cette fonction pour cette manette, ou la terminaison de la bibliothèque.",
  ],
  [
    'The returned string is allocated and freed by Lumina. You should not free it yourself. It is valid until the specified joystick is disconnected, the gamepad mappings are updated or the library is terminated.',
    "La chaîne retournée est allouée et libérée par Lumina. Vous ne devez pas la libérer vous-même. Elle reste valide jusqu'à la déconnexion de la manette indiquée, une mise à jour des correspondances de manette, ou la terminaison de la bibliothèque.",
  ],
  [
    'The returned string is allocated and freed by Lumina. You should not free it yourself. It is valid until the next call to @ref lumGetClipboardString or @ref lumSetClipboardString, or until the library is terminated.',
    "La chaîne retournée est allouée et libérée par Lumina. Vous ne devez pas la libérer vous-même. Elle reste valide jusqu'au prochain appel à @ref lumGetClipboardString ou @ref lumSetClipboardString, ou jusqu'à la terminaison de la bibliothèque.",
  ],
  [
    'The returned function pointer is valid until the context is destroyed or the library is terminated.',
    "Le pointeur de fonction retourné reste valide jusqu'à la destruction du contexte ou la terminaison de la bibliothèque.",
  ],
  [
    'The returned array is allocated and freed by Lumina. You should not free it yourself. It is guaranteed to be valid only until the library is terminated.',
    "Le tableau retourné est alloué et libéré par Lumina. Vous ne devez pas le libérer vous-même. Il n'est garanti valide que jusqu'à la terminaison de la bibliothèque.",
  ],
  [
    'The returned function pointer is valid until the library is terminated.',
    "Le pointeur de fonction retourné reste valide jusqu'à la terminaison de la bibliothèque.",
  ],
  [
    'The returned string is allocated and freed by Lumina. You should not free it yourself. It is valid until the next call to @ref lumGetX11SelectionString or @ref lumSetX11SelectionString, or until the library is terminated.',
    "La chaîne retournée est allouée et libérée par Lumina. Vous ne devez pas la libérer vous-même. Elle reste valide jusqu'au prochain appel à @ref lumGetX11SelectionString ou @ref lumSetX11SelectionString, ou jusqu'à la terminaison de la bibliothèque.",
  ],
  [
    'The specified memory block will not be accessed by Lumina after this function is called.',
    "Lumina n'accédera plus au bloc mémoire indiqué après l'appel de cette fonction.",
  ],
  [
    'The error description string is valid until the callback function returns.',
    "La chaîne de description de l'erreur reste valide jusqu'au retour de la fonction de rappel.",
  ],
  [
    'The path array and its strings are valid until the callback function returns.',
    'Le tableau de chemins et ses chaînes restent valides jusqu\'au retour de la fonction de rappel.',
  ],
];
