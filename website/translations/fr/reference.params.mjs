// Parameter descriptions and callback signatures.
//
// A callback-signature entry keeps its `@code` block as code and translates
// only the sentence after it; the French value is written with real newlines
// so the block still renders as a code block.

export default [
  // ---- init ----------------------------------------------------------------
  ['The [init hint](@ref init_hints) to set.', "L'[indice d'initialisation](@ref init_hints) à régler."],
  ['The new value of the init hint.', "La nouvelle valeur de l'indice d'initialisation."],
  [
    'The allocator to use at the next initialization, or `NULL` to use the default one.',
    "L'allocateur à employer à la prochaine initialisation, ou `NULL` pour celui par défaut.",
  ],
  [
    'The address of the function to use, or `NULL`. @par Loader function signature @code PFN_vkVoidFunction vkGetInstanceProcAddr(VkInstance instance, const char* name) @endcode For more information about this function, see the [Vulkan Registry](https://www.khronos.org/registry/vulkan/).',
    "L'adresse de la fonction à employer, ou `NULL`.\n@par Signature de la fonction de chargement\n@code\nPFN_vkVoidFunction vkGetInstanceProcAddr(VkInstance instance, const char* name)\n@endcode\nPour en savoir plus sur cette fonction, voyez le [registre Vulkan](https://www.khronos.org/registry/vulkan/).",
  ],
  ['Where to store the major version number, or `NULL`.', 'Où stocker le numéro de version majeure, ou `NULL`.'],
  ['Where to store the minor version number, or `NULL`.', 'Où stocker le numéro de version mineure, ou `NULL`.'],
  ['Where to store the revision number, or `NULL`.', 'Où stocker le numéro de révision, ou `NULL`.'],
  [
    'Where to store the error description pointer, or `NULL`.',
    "Où stocker le pointeur vers la description de l'erreur, ou `NULL`.",
  ],
  ['The platform to query.', 'La plateforme à interroger.'],

  // ---- monitor -------------------------------------------------------------
  [
    'Where to store the number of monitors in the returned array. This is set to zero if an error occurred.',
    "Où stocker le nombre d'écrans du tableau retourné.  Mis à zéro si une erreur est survenue.",
  ],
  ['Where to store the monitor width, or `NULL`.', "Où stocker la largeur de l'écran, ou `NULL`."],
  ['Where to store the monitor height, or `NULL`.', "Où stocker la hauteur de l'écran, ou `NULL`."],
  [
    "Where to store the width, in millimetres, of the monitor's display area, or `NULL`.",
    "Où stocker la largeur, en millimètres, de la zone d'affichage de l'écran, ou `NULL`.",
  ],
  [
    "Where to store the height, in millimetres, of the monitor's display area, or `NULL`.",
    "Où stocker la hauteur, en millimètres, de la zone d'affichage de l'écran, ou `NULL`.",
  ],
  ['The monitor whose pointer to set.', "L'écran dont on règle le pointeur."],
  ['The monitor whose pointer to return.', "L'écran dont on retourne le pointeur."],
  [
    'Where to store the number of video modes in the returned array. This is set to zero if an error occurred.',
    'Où stocker le nombre de modes vidéo du tableau retourné.  Mis à zéro si une erreur est survenue.',
  ],
  ['The desired exponent.', "L'exposant voulu."],
  ['The gamma ramp to use.', 'La rampe gamma à employer.'],

  // ---- window --------------------------------------------------------------
  [
    'The desired width, in screen coordinates, of the window. This must be greater than zero.',
    "La largeur voulue de la fenêtre, en coordonnées d'écran.  Doit être strictement positive.",
  ],
  [
    'The desired height, in screen coordinates, of the window. This must be greater than zero.',
    "La hauteur voulue de la fenêtre, en coordonnées d'écran.  Doit être strictement positive.",
  ],
  ['The initial, UTF-8 encoded window title.', 'Le titre initial de la fenêtre, encodé en UTF-8.'],
  [
    'The monitor to use for full screen mode, or `NULL` for windowed mode.',
    "L'écran à utiliser en mode plein écran, ou `NULL` pour le mode fenêtré.",
  ],
  [
    'The window whose context to share resources with, or `NULL` to not share resources.',
    'La fenêtre dont le contexte partagera ses ressources, ou `NULL` pour ne rien partager.',
  ],
  ['The window to destroy.', 'La fenêtre à détruire.'],
  ['The window whose flag to change.', "La fenêtre dont on modifie l'indicateur."],
  ['The window whose title to change.', 'La fenêtre dont on modifie le titre.'],
  ['The UTF-8 encoded window title.', 'Le titre de la fenêtre, encodé en UTF-8.'],
  ['The window whose icon to set.', "La fenêtre dont on règle l'icône."],
  [
    'The number of images in the specified array, or zero to revert to the default window icon.',
    "Le nombre d'images du tableau indiqué, ou zéro pour revenir à l'icône par défaut.",
  ],
  [
    'The images to create the icon from. This is ignored if count is zero.',
    "Les images à partir desquelles créer l'icône.  Ignoré si count vaut zéro.",
  ],
  [
    'Where to store the x-coordinate of the upper-left corner of the content area, or `NULL`.',
    "Où stocker l'abscisse du coin supérieur gauche de la zone de contenu, ou `NULL`.",
  ],
  [
    'Where to store the y-coordinate of the upper-left corner of the content area, or `NULL`.',
    "Où stocker l'ordonnée du coin supérieur gauche de la zone de contenu, ou `NULL`.",
  ],
  [
    'The x-coordinate of the upper-left corner of the content area.',
    "L'abscisse du coin supérieur gauche de la zone de contenu.",
  ],
  [
    'The y-coordinate of the upper-left corner of the content area.',
    "L'ordonnée du coin supérieur gauche de la zone de contenu.",
  ],
  ['The window whose size to retrieve.', 'La fenêtre dont on récupère la taille.'],
  [
    'Where to store the width, in screen coordinates, of the content area, or `NULL`.',
    "Où stocker la largeur, en coordonnées d'écran, de la zone de contenu, ou `NULL`.",
  ],
  [
    'Where to store the height, in screen coordinates, of the content area, or `NULL`.',
    "Où stocker la hauteur, en coordonnées d'écran, de la zone de contenu, ou `NULL`.",
  ],
  [
    'The minimum width, in screen coordinates, of the content area, or `LUM_DONT_CARE`.',
    "La largeur minimale, en coordonnées d'écran, de la zone de contenu, ou `LUM_DONT_CARE`.",
  ],
  [
    'The minimum height, in screen coordinates, of the content area, or `LUM_DONT_CARE`.',
    "La hauteur minimale, en coordonnées d'écran, de la zone de contenu, ou `LUM_DONT_CARE`.",
  ],
  [
    'The maximum width, in screen coordinates, of the content area, or `LUM_DONT_CARE`.',
    "La largeur maximale, en coordonnées d'écran, de la zone de contenu, ou `LUM_DONT_CARE`.",
  ],
  [
    'The maximum height, in screen coordinates, of the content area, or `LUM_DONT_CARE`.',
    "La hauteur maximale, en coordonnées d'écran, de la zone de contenu, ou `LUM_DONT_CARE`.",
  ],
  [
    'The numerator of the desired aspect ratio, or `LUM_DONT_CARE`.',
    'Le numérateur de la proportion voulue, ou `LUM_DONT_CARE`.',
  ],
  [
    'The denominator of the desired aspect ratio, or `LUM_DONT_CARE`.',
    'Le dénominateur de la proportion voulue, ou `LUM_DONT_CARE`.',
  ],
  ['The window to resize.', 'La fenêtre à redimensionner.'],
  [
    'The desired width, in screen coordinates, of the window content area.',
    "La largeur voulue, en coordonnées d'écran, de la zone de contenu de la fenêtre.",
  ],
  [
    'The desired height, in screen coordinates, of the window content area.',
    "La hauteur voulue, en coordonnées d'écran, de la zone de contenu de la fenêtre.",
  ],
  ['The window whose framebuffer to query.', "La fenêtre dont on interroge le tampon d'image."],
  [
    'Where to store the width, in pixels, of the framebuffer, or `NULL`.',
    "Où stocker la largeur, en pixels, du tampon d'image, ou `NULL`.",
  ],
  [
    'Where to store the height, in pixels, of the framebuffer, or `NULL`.',
    "Où stocker la hauteur, en pixels, du tampon d'image, ou `NULL`.",
  ],
  ['The window whose frame size to query.', 'La fenêtre dont on interroge la taille du cadre.'],
  [
    'Where to store the size, in screen coordinates, of the left edge of the window frame, or `NULL`.',
    "Où stocker la taille, en coordonnées d'écran, du bord gauche du cadre de la fenêtre, ou `NULL`.",
  ],
  [
    'Where to store the size, in screen coordinates, of the top edge of the window frame, or `NULL`.',
    "Où stocker la taille, en coordonnées d'écran, du bord supérieur du cadre de la fenêtre, ou `NULL`.",
  ],
  [
    'Where to store the size, in screen coordinates, of the right edge of the window frame, or `NULL`.',
    "Où stocker la taille, en coordonnées d'écran, du bord droit du cadre de la fenêtre, ou `NULL`.",
  ],
  [
    'Where to store the size, in screen coordinates, of the bottom edge of the window frame, or `NULL`.',
    "Où stocker la taille, en coordonnées d'écran, du bord inférieur du cadre de la fenêtre, ou `NULL`.",
  ],
  ['The window to set the opacity for.', "La fenêtre dont on règle l'opacité."],
  ['The desired opacity of the specified window.', "L'opacité voulue pour la fenêtre indiquée."],
  ['The window to iconify.', 'La fenêtre à réduire en icône.'],
  ['The window to restore.', 'La fenêtre à restaurer.'],
  ['The window to maximize.', 'La fenêtre à maximiser.'],
  ['The window to make visible.', 'La fenêtre à rendre visible.'],
  ['The window to hide.', 'La fenêtre à masquer.'],
  ['The window to give input focus.', 'La fenêtre à qui donner le focus.'],
  ['The window to request attention to.', "La fenêtre sur laquelle attirer l'attention."],
  [
    'The window whose monitor, size or video mode to set.',
    "La fenêtre dont on règle l'écran, la taille ou le mode vidéo.",
  ],
  ['The desired monitor, or `NULL` to set windowed mode.', "L'écran voulu, ou `NULL` pour passer en mode fenêtré."],
  [
    'The desired x-coordinate of the upper-left corner of the content area.',
    "L'abscisse voulue du coin supérieur gauche de la zone de contenu.",
  ],
  [
    'The desired y-coordinate of the upper-left corner of the content area.',
    "L'ordonnée voulue du coin supérieur gauche de la zone de contenu.",
  ],
  [
    'The desired with, in screen coordinates, of the content area or video mode.',
    "La largeur voulue, en coordonnées d'écran, de la zone de contenu ou du mode vidéo.",
  ],
  [
    'The desired height, in screen coordinates, of the content area or video mode.',
    "La hauteur voulue, en coordonnées d'écran, de la zone de contenu ou du mode vidéo.",
  ],
  [
    'The desired refresh rate, in Hz, of the video mode, or `LUM_DONT_CARE`.',
    'Le taux de rafraîchissement voulu, en Hz, du mode vidéo, ou `LUM_DONT_CARE`.',
  ],
  [
    'The [window attribute](@ref window_attribs) whose value to return.',
    "L'[attribut de fenêtre](@ref window_attribs) dont on retourne la valeur.",
  ],
  ['The window to set the attribute for.', "La fenêtre dont on règle l'attribut."],
  ['A supported window attribute.', 'Un attribut de fenêtre pris en charge.'],
  ['`LUM_TRUE` or `LUM_FALSE`.', '`LUM_TRUE` ou `LUM_FALSE`.'],
  ['The window whose pointer to set.', 'La fenêtre dont on règle le pointeur.'],
  ['The window whose pointer to return.', 'La fenêtre dont on retourne le pointeur.'],
  ['The maximum amount of time, in seconds, to wait.', "La durée d'attente maximale, en secondes."],

  // ---- input ---------------------------------------------------------------
  ['The window whose input mode to set.', "La fenêtre dont on règle le mode d'entrée."],
  ['The new value of the specified input mode.', "La nouvelle valeur du mode d'entrée indiqué."],
  ['The key to query, or `LUM_KEY_UNKNOWN`.', 'La touche à interroger, ou `LUM_KEY_UNKNOWN`.'],
  ['The scancode of the key to query.', 'Le scancode de la touche à interroger.'],
  ['Any [named key](@ref keys).', "N'importe quelle [touche nommée](@ref keys)."],
  [
    'The desired [keyboard key](@ref keys). `LUM_KEY_UNKNOWN` is not a valid key for this function.',
    "La [touche](@ref keys) voulue.  `LUM_KEY_UNKNOWN` n'est pas une touche valide pour cette fonction.",
  ],
  ['The desired [mouse button](@ref buttons).', 'Le [bouton de souris](@ref buttons) voulu.'],
  [
    'Where to store the cursor x-coordinate, relative to the left edge of the content area, or `NULL`.',
    "Où stocker l'abscisse du curseur, relative au bord gauche de la zone de contenu, ou `NULL`.",
  ],
  [
    'Where to store the cursor y-coordinate, relative to the to top edge of the content area, or `NULL`.',
    "Où stocker l'ordonnée du curseur, relative au bord supérieur de la zone de contenu, ou `NULL`.",
  ],
  [
    'The desired x-coordinate, relative to the left edge of the content area.',
    "L'abscisse voulue, relative au bord gauche de la zone de contenu.",
  ],
  [
    'The desired y-coordinate, relative to the top edge of the content area.',
    "L'ordonnée voulue, relative au bord supérieur de la zone de contenu.",
  ],
  ['The desired cursor image.', "L'image de curseur voulue."],
  [
    'The desired x-coordinate, in pixels, of the cursor hotspot.',
    "L'abscisse voulue, en pixels, du point chaud du curseur.",
  ],
  [
    'The desired y-coordinate, in pixels, of the cursor hotspot.',
    "L'ordonnée voulue, en pixels, du point chaud du curseur.",
  ],
  ['One of the [standard shapes](@ref shapes).', "L'une des [formes standard](@ref shapes)."],
  ['The cursor object to destroy.', "L'objet curseur à détruire."],
  ['The window to set the cursor for.', 'La fenêtre dont on règle le curseur.'],
  [
    'The cursor to set, or `NULL` to switch back to the default arrow cursor.',
    'Le curseur à poser, ou `NULL` pour revenir au curseur fléché par défaut.',
  ],
  [
    'The new key callback, or `NULL` to remove the currently set callback.',
    'La nouvelle fonction de rappel de clavier, ou `NULL` pour retirer celle installée.',
  ],
  [
    'The new scroll callback, or `NULL` to remove the currently set callback.',
    'La nouvelle fonction de rappel de défilement, ou `NULL` pour retirer celle installée.',
  ],
  [
    'The new file drop callback, or `NULL` to remove the currently set callback.',
    'La nouvelle fonction de rappel de dépôt de fichiers, ou `NULL` pour retirer celle installée.',
  ],

  // ---- joystick ------------------------------------------------------------
  [
    'Where to store the number of axis values in the returned array. This is set to zero if the joystick is not present or an error occurred.',
    "Où stocker le nombre de valeurs d'axes du tableau retourné.  Mis à zéro si la manette est absente ou si une erreur est survenue.",
  ],
  [
    'Where to store the number of button states in the returned array. This is set to zero if the joystick is not present or an error occurred.',
    "Où stocker le nombre d'états de boutons du tableau retourné.  Mis à zéro si la manette est absente ou si une erreur est survenue.",
  ],
  [
    'Where to store the number of hat states in the returned array. This is set to zero if the joystick is not present or an error occurred.',
    "Où stocker le nombre d'états de croix du tableau retourné.  Mis à zéro si la manette est absente ou si une erreur est survenue.",
  ],
  ['The joystick whose pointer to set.', 'La manette dont on règle le pointeur.'],
  ['The joystick whose pointer to return.', 'La manette dont on retourne le pointeur.'],
  ['The string containing the gamepad mappings.', 'La chaîne contenant les correspondances de manette.'],
  ['The gamepad input state of the joystick.', "L'état d'entrée de manette de jeu de la manette."],

  // ---- time, context, vulkan ----------------------------------------------
  ['The new value, in seconds.', 'La nouvelle valeur, en secondes.'],
  [
    'The window whose context to make current, or `NULL` to detach the current context.',
    'La fenêtre dont le contexte doit devenir courant, ou `NULL` pour détacher le contexte courant.',
  ],
  ['The window whose buffers to swap.', 'La fenêtre dont on échange les tampons.'],
  [
    'The minimum number of screen updates to wait for until the buffers are swapped by @ref lumSwapBuffers.',
    "Le nombre minimal de rafraîchissements d'écran à attendre avant que @ref lumSwapBuffers n'échange les tampons.",
  ],
  ['The ASCII encoded name of the extension.', "Le nom de l'extension, encodé en ASCII."],
  [
    'Where to store the number of extensions in the returned array. This is set to zero if an error occurred.',
    "Où stocker le nombre d'extensions du tableau retourné.  Mis à zéro si une erreur est survenue.",
  ],
  [
    'The Vulkan instance to query, or `NULL` to retrieve functions related to instance creation.',
    "L'instance Vulkan à interroger, ou `NULL` pour récupérer les fonctions liées à la création d'instance.",
  ],
  ['The instance that the physical device belongs to.', "L'instance à laquelle appartient le périphérique physique."],
  [
    'The physical device that the queue family belongs to.',
    'Le périphérique physique auquel appartient la famille de files.',
  ],
  ['The index of the queue family to query.', "L'indice de la famille de files à interroger."],
  ['The Vulkan instance to create the surface in.', "L'instance Vulkan dans laquelle créer la surface."],
  ['The window to create the surface for.', 'La fenêtre pour laquelle créer la surface.'],
  [
    'The allocator to use, or `NULL` to use the default allocator.',
    "L'allocateur à employer, ou `NULL` pour celui par défaut.",
  ],
  [
    'Where to store the handle of the surface. This is set to `VK_NULL_HANDLE` if an error occurred.',
    'Où stocker le handle de la surface.  Mis à `VK_NULL_HANDLE` si une erreur est survenue.',
  ],

  // ---- OSMesa --------------------------------------------------------------
  ['The window whose color buffer to retrieve.', 'La fenêtre dont on récupère le tampon de couleur.'],
  [
    'Where to store the width of the color buffer, or `NULL`.',
    'Où stocker la largeur du tampon de couleur, ou `NULL`.',
  ],
  [
    'Where to store the height of the color buffer, or `NULL`.',
    'Où stocker la hauteur du tampon de couleur, ou `NULL`.',
  ],
  [
    'Where to store the OSMesa pixel format of the color buffer, or `NULL`.',
    'Où stocker le format de pixel OSMesa du tampon de couleur, ou `NULL`.',
  ],
  [
    'Where to store the address of the color buffer, or `NULL`.',
    "Où stocker l'adresse du tampon de couleur, ou `NULL`.",
  ],
  ['The window whose depth buffer to retrieve.', 'La fenêtre dont on récupère le tampon de profondeur.'],
  [
    'Where to store the width of the depth buffer, or `NULL`.',
    'Où stocker la largeur du tampon de profondeur, ou `NULL`.',
  ],
  [
    'Where to store the height of the depth buffer, or `NULL`.',
    'Où stocker la hauteur du tampon de profondeur, ou `NULL`.',
  ],
  [
    'Where to store the number of bytes per depth buffer element, or `NULL`.',
    "Où stocker le nombre d'octets par élément du tampon de profondeur, ou `NULL`.",
  ],
  [
    'Where to store the address of the depth buffer, or `NULL`.',
    "Où stocker l'adresse du tampon de profondeur, ou `NULL`.",
  ],

  // ---- callback parameters -------------------------------------------------
  ['The minimum size, in bytes, of the memory block.', 'La taille minimale du bloc mémoire, en octets.'],
  ['The address of the memory block to reallocate.', "L'adresse du bloc mémoire à réallouer."],
  ['The new minimum size, in bytes, of the memory block.', 'La nouvelle taille minimale du bloc mémoire, en octets.'],
  ['The address of the memory block to deallocate.', "L'adresse du bloc mémoire à libérer."],
  [
    'An [error code](@ref errors). Future releases may add more error codes.',
    "Un [code d'erreur](@ref errors).  De futures versions pourront en ajouter d'autres.",
  ],
  [
    'A UTF-8 encoded string describing the error.',
    "Une chaîne encodée en UTF-8 décrivant l'erreur.",
  ],
  [
    'The new x-coordinate, in screen coordinates, of the upper-left corner of the content area of the window.',
    "La nouvelle abscisse, en coordonnées d'écran, du coin supérieur gauche de la zone de contenu de la fenêtre.",
  ],
  [
    'The new y-coordinate, in screen coordinates, of the upper-left corner of the content area of the window.',
    "La nouvelle ordonnée, en coordonnées d'écran, du coin supérieur gauche de la zone de contenu de la fenêtre.",
  ],
  [
    'The x-coordinate of mouse, in screen coordinates.',
    "L'abscisse de la souris, en coordonnées d'écran.",
  ],
  [
    'The y-coordinate of mouse, in screen coordinates.',
    "L'ordonnée de la souris, en coordonnées d'écran.",
  ],
  ["'true' or '1' if mouse hovering titlebar.", "'true' ou '1' si la souris survole la barre de titre."],
  ['The window that was resized.', 'La fenêtre qui a été redimensionnée.'],
  [
    'The new width, in screen coordinates, of the window.',
    "La nouvelle largeur de la fenêtre, en coordonnées d'écran.",
  ],
  [
    'The new height, in screen coordinates, of the window.',
    "La nouvelle hauteur de la fenêtre, en coordonnées d'écran.",
  ],
  ['The window that the user attempted to close.', "La fenêtre que l'utilisateur a tenté de fermer."],
  ['The window whose content needs to be refreshed.', 'La fenêtre dont le contenu doit être rafraîchi.'],
  ['The window that gained or lost input focus.', 'La fenêtre qui a gagné ou perdu le focus.'],
  [
    '`LUM_TRUE` if the window was given input focus, or `LUM_FALSE` if it lost it.',
    "`LUM_TRUE` si la fenêtre a reçu le focus, `LUM_FALSE` si elle l'a perdu.",
  ],
  ['The window that was iconified or restored.', 'La fenêtre qui a été réduite en icône ou restaurée.'],
  [
    '`LUM_TRUE` if the window was iconified, or `LUM_FALSE` if it was restored.',
    '`LUM_TRUE` si la fenêtre a été réduite en icône, `LUM_FALSE` si elle a été restaurée.',
  ],
  ['The window that was maximized or restored.', 'La fenêtre qui a été maximisée ou restaurée.'],
  [
    '`LUM_TRUE` if the window was maximized, or `LUM_FALSE` if it was restored.',
    '`LUM_TRUE` si la fenêtre a été maximisée, `LUM_FALSE` si elle a été restaurée.',
  ],
  ['The window whose framebuffer was resized.', "La fenêtre dont le tampon d'image a été redimensionné."],
  ['The new width, in pixels, of the framebuffer.', "La nouvelle largeur du tampon d'image, en pixels."],
  ['The new height, in pixels, of the framebuffer.', "La nouvelle hauteur du tampon d'image, en pixels."],
  ['The window whose content scale changed.', "La fenêtre dont le facteur d'échelle du contenu a changé."],
  [
    'The new x-axis content scale of the window.',
    "Le nouveau facteur d'échelle du contenu de la fenêtre sur l'axe X.",
  ],
  [
    'The new y-axis content scale of the window.',
    "Le nouveau facteur d'échelle du contenu de la fenêtre sur l'axe Y.",
  ],
  [
    'The [mouse button](@ref buttons) that was pressed or released.',
    'Le [bouton de souris](@ref buttons) qui a été enfoncé ou relâché.',
  ],
  [
    'One of `LUM_PRESS` or `LUM_RELEASE`. Future releases may add more actions.',
    "L'un de `LUM_PRESS` ou `LUM_RELEASE`.  De futures versions pourront ajouter d'autres actions.",
  ],
  [
    'The new cursor x-coordinate, relative to the left edge of the content area.',
    'La nouvelle abscisse du curseur, relative au bord gauche de la zone de contenu.',
  ],
  [
    'The new cursor y-coordinate, relative to the top edge of the content area.',
    'La nouvelle ordonnée du curseur, relative au bord supérieur de la zone de contenu.',
  ],
  [
    "`LUM_TRUE` if the cursor entered the window's content area, or `LUM_FALSE` if it left it.",
    "`LUM_TRUE` si le curseur est entré dans la zone de contenu de la fenêtre, `LUM_FALSE` s'il en est sorti.",
  ],
  ['The scroll offset along the x-axis.', "Le décalage de défilement sur l'axe X."],
  ['The scroll offset along the y-axis.', "Le décalage de défilement sur l'axe Y."],
  [
    'The [keyboard key](@ref keys) that was pressed or released.',
    'La [touche](@ref keys) qui a été enfoncée ou relâchée.',
  ],
  ['The platform-specific scancode of the key.', 'Le scancode de la touche, propre à la plateforme.'],
  [
    '`LUM_PRESS`, `LUM_RELEASE` or `LUM_REPEAT`. Future releases may add more actions.',
    "`LUM_PRESS`, `LUM_RELEASE` ou `LUM_REPEAT`.  De futures versions pourront ajouter d'autres actions.",
  ],
  ['The number of dropped paths.', 'Le nombre de chemins déposés.'],
  [
    'The UTF-8 encoded file and/or directory path names.',
    'Les chemins de fichiers ou de répertoires, encodés en UTF-8.',
  ],
  ['The monitor that was connected or disconnected.', "L'écran qui a été connecté ou déconnecté."],
  ['The joystick that was connected or disconnected.', 'La manette qui a été connectée ou déconnectée.'],

  // ---- callback signatures -------------------------------------------------
  [
    '@code void callback_name(int error_code, const char* description) @endcode For more information about the callback parameters, see the [callback pointer type](@ref LUMerrorfun).',
    '@code\nvoid callback_name(int error_code, const char* description)\n@endcode\nPour en savoir plus sur les paramètres, voyez le [type de pointeur de rappel](@ref LUMerrorfun).',
  ],
  [
    '@code void function_name(LUMmonitor* monitor, int event) @endcode For more information about the callback parameters, see the [function pointer type](@ref LUMmonitorfun).',
    '@code\nvoid function_name(LUMmonitor* monitor, int event)\n@endcode\nPour en savoir plus sur les paramètres, voyez le [type de pointeur de fonction](@ref LUMmonitorfun).',
  ],
  [
    '@code void function_name(LUMwindow* window, int xpos, int ypos) @endcode For more information about the callback parameters, see the [function pointer type](@ref LUMwindowposfun).',
    '@code\nvoid function_name(LUMwindow* window, int xpos, int ypos)\n@endcode\nPour en savoir plus sur les paramètres, voyez le [type de pointeur de fonction](@ref LUMwindowposfun).',
  ],
  [
    '@code void function_name(LUMwindow* window, int xpos, int ypos, int* hit) @endcode For more information about the callback parameters, see the [function pointer type](@ref LUMtitlebarhittestfun).',
    '@code\nvoid function_name(LUMwindow* window, int xpos, int ypos, int* hit)\n@endcode\nPour en savoir plus sur les paramètres, voyez le [type de pointeur de fonction](@ref LUMtitlebarhittestfun).',
  ],
  [
    '@code void function_name(LUMwindow* window, int width, int height) @endcode For more information about the callback parameters, see the [function pointer type](@ref LUMwindowsizefun).',
    '@code\nvoid function_name(LUMwindow* window, int width, int height)\n@endcode\nPour en savoir plus sur les paramètres, voyez le [type de pointeur de fonction](@ref LUMwindowsizefun).',
  ],
  [
    '@code void function_name(LUMwindow* window) @endcode For more information about the callback parameters, see the [function pointer type](@ref LUMwindowclosefun).',
    '@code\nvoid function_name(LUMwindow* window)\n@endcode\nPour en savoir plus sur les paramètres, voyez le [type de pointeur de fonction](@ref LUMwindowclosefun).',
  ],
  [
    '@code void function_name(LUMwindow* window); @endcode For more information about the callback parameters, see the [function pointer type](@ref LUMwindowrefreshfun).',
    '@code\nvoid function_name(LUMwindow* window);\n@endcode\nPour en savoir plus sur les paramètres, voyez le [type de pointeur de fonction](@ref LUMwindowrefreshfun).',
  ],
  [
    '@code void function_name(LUMwindow* window, int focused) @endcode For more information about the callback parameters, see the [function pointer type](@ref LUMwindowfocusfun).',
    '@code\nvoid function_name(LUMwindow* window, int focused)\n@endcode\nPour en savoir plus sur les paramètres, voyez le [type de pointeur de fonction](@ref LUMwindowfocusfun).',
  ],
  [
    '@code void function_name(LUMwindow* window, int iconified) @endcode For more information about the callback parameters, see the [function pointer type](@ref LUMwindowiconifyfun).',
    '@code\nvoid function_name(LUMwindow* window, int iconified)\n@endcode\nPour en savoir plus sur les paramètres, voyez le [type de pointeur de fonction](@ref LUMwindowiconifyfun).',
  ],
  [
    '@code void function_name(LUMwindow* window, int maximized) @endcode For more information about the callback parameters, see the [function pointer type](@ref LUMwindowmaximizefun).',
    '@code\nvoid function_name(LUMwindow* window, int maximized)\n@endcode\nPour en savoir plus sur les paramètres, voyez le [type de pointeur de fonction](@ref LUMwindowmaximizefun).',
  ],
  [
    '@code void function_name(LUMwindow* window, int width, int height) @endcode For more information about the callback parameters, see the [function pointer type](@ref LUMframebuffersizefun).',
    '@code\nvoid function_name(LUMwindow* window, int width, int height)\n@endcode\nPour en savoir plus sur les paramètres, voyez le [type de pointeur de fonction](@ref LUMframebuffersizefun).',
  ],
  [
    '@code void function_name(LUMwindow* window, float xscale, float yscale) @endcode For more information about the callback parameters, see the [function pointer type](@ref LUMwindowcontentscalefun).',
    '@code\nvoid function_name(LUMwindow* window, float xscale, float yscale)\n@endcode\nPour en savoir plus sur les paramètres, voyez le [type de pointeur de fonction](@ref LUMwindowcontentscalefun).',
  ],
  [
    '@code void function_name(LUMwindow* window, int key, int scancode, int action, int mods) @endcode For more information about the callback parameters, see the [function pointer type](@ref LUMkeyfun).',
    '@code\nvoid function_name(LUMwindow* window, int key, int scancode, int action, int mods)\n@endcode\nPour en savoir plus sur les paramètres, voyez le [type de pointeur de fonction](@ref LUMkeyfun).',
  ],
  [
    '@code void function_name(LUMwindow* window, unsigned int codepoint) @endcode For more information about the callback parameters, see the [function pointer type](@ref LUMcharfun).',
    '@code\nvoid function_name(LUMwindow* window, unsigned int codepoint)\n@endcode\nPour en savoir plus sur les paramètres, voyez le [type de pointeur de fonction](@ref LUMcharfun).',
  ],
  [
    '@code void function_name(LUMwindow* window, unsigned int codepoint, int mods) @endcode For more information about the callback parameters, see the [function pointer type](@ref LUMcharmodsfun).',
    '@code\nvoid function_name(LUMwindow* window, unsigned int codepoint, int mods)\n@endcode\nPour en savoir plus sur les paramètres, voyez le [type de pointeur de fonction](@ref LUMcharmodsfun).',
  ],
  [
    '@code void function_name(LUMwindow* window, int button, int action, int mods) @endcode For more information about the callback parameters, see the [function pointer type](@ref LUMmousebuttonfun).',
    '@code\nvoid function_name(LUMwindow* window, int button, int action, int mods)\n@endcode\nPour en savoir plus sur les paramètres, voyez le [type de pointeur de fonction](@ref LUMmousebuttonfun).',
  ],
  [
    '@code void function_name(LUMwindow* window, double xpos, double ypos); @endcode For more information about the callback parameters, see the [function pointer type](@ref LUMcursorposfun).',
    '@code\nvoid function_name(LUMwindow* window, double xpos, double ypos);\n@endcode\nPour en savoir plus sur les paramètres, voyez le [type de pointeur de fonction](@ref LUMcursorposfun).',
  ],
  [
    '@code void function_name(LUMwindow* window, int entered) @endcode For more information about the callback parameters, see the [function pointer type](@ref LUMcursorenterfun).',
    '@code\nvoid function_name(LUMwindow* window, int entered)\n@endcode\nPour en savoir plus sur les paramètres, voyez le [type de pointeur de fonction](@ref LUMcursorenterfun).',
  ],
  [
    '@code void function_name(LUMwindow* window, double xoffset, double yoffset) @endcode For more information about the callback parameters, see the [function pointer type](@ref LUMscrollfun).',
    '@code\nvoid function_name(LUMwindow* window, double xoffset, double yoffset)\n@endcode\nPour en savoir plus sur les paramètres, voyez le [type de pointeur de fonction](@ref LUMscrollfun).',
  ],
  [
    '@code void function_name(LUMwindow* window, int path_count, const char* paths[]) @endcode For more information about the callback parameters, see the [function pointer type](@ref LUMdropfun).',
    '@code\nvoid function_name(LUMwindow* window, int path_count, const char* paths[])\n@endcode\nPour en savoir plus sur les paramètres, voyez le [type de pointeur de fonction](@ref LUMdropfun).',
  ],
  [
    '@code void function_name(int jid, int event) @endcode For more information about the callback parameters, see the [function pointer type](@ref LUMjoystickfun).',
    '@code\nvoid function_name(int jid, int event)\n@endcode\nPour en savoir plus sur les paramètres, voyez le [type de pointeur de fonction](@ref LUMjoystickfun).',
  ],
];
