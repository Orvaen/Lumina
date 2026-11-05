// Brief summaries: the one-line description shown at the top of each entry
// and in the alphabetical index. Kept in its own file because there is one
// per symbol and they are the most read strings of the reference.

export default [
  // ---- init ----------------------------------------------------------------
  ['Initializes the Lumina library.', 'Initialise la bibliothèque Lumina.'],
  ['Terminates the Lumina library.', 'Termine la bibliothèque Lumina.'],
  ['Sets the specified init hint to the desired value.', "Règle l'indice d'initialisation indiqué à la valeur voulue."],
  ['Sets the init allocator to the desired value.', "Règle l'allocateur d'initialisation à la valeur voulue."],
  [
    'Sets the desired Vulkan `vkGetInstanceProcAddr` function.',
    'Définit la fonction Vulkan `vkGetInstanceProcAddr` à employer.',
  ],
  ['Retrieves the version of the Lumina library.', 'Récupère la version de la bibliothèque Lumina.'],
  [
    'Returns a string describing the compile-time configuration.',
    'Retourne une chaîne décrivant la configuration de compilation.',
  ],
  [
    'Returns and clears the last error for the calling thread.',
    "Retourne et efface la dernière erreur du thread appelant.",
  ],
  ['Sets the error callback.', "Installe la fonction de rappel d'erreur."],
  ['Returns the currently selected platform.', 'Retourne la plateforme actuellement retenue.'],
  [
    'Returns whether the library includes support for the specified platform.',
    'Indique si la bibliothèque prend en charge la plateforme indiquée.',
  ],

  // ---- monitor -------------------------------------------------------------
  ['Returns the currently connected monitors.', 'Retourne les écrans actuellement connectés.'],
  ['Returns the primary monitor.', "Retourne l'écran principal."],
  [
    "Returns the position of the monitor's viewport on the virtual screen.",
    "Retourne la position de la zone d'affichage de l'écran sur l'écran virtuel.",
  ],
  ['Retrieves the work area of the monitor.', "Récupère la zone de travail de l'écran."],
  ['Returns the physical size of the monitor.', "Retourne la taille physique de l'écran."],
  [
    'Retrieves the content scale for the specified monitor.',
    "Récupère le facteur d'échelle du contenu de l'écran indiqué.",
  ],
  ['Returns the name of the specified monitor.', "Retourne le nom de l'écran indiqué."],
  ['Sets the user pointer of the specified monitor.', "Règle le pointeur utilisateur de l'écran indiqué."],
  ['Returns the user pointer of the specified monitor.', "Retourne le pointeur utilisateur de l'écran indiqué."],
  ['Sets the monitor configuration callback.', "Installe la fonction de rappel de configuration des écrans."],
  [
    'Returns the available video modes for the specified monitor.',
    "Retourne les modes vidéo disponibles de l'écran indiqué.",
  ],
  ['Returns the current mode of the specified monitor.', "Retourne le mode courant de l'écran indiqué."],
  [
    'Generates a gamma ramp and sets it for the specified monitor.',
    "Produit une rampe gamma et l'applique à l'écran indiqué.",
  ],
  ['Returns the current gamma ramp for the specified monitor.', "Retourne la rampe gamma courante de l'écran indiqué."],
  ['Sets the current gamma ramp for the specified monitor.', "Règle la rampe gamma courante de l'écran indiqué."],

  // ---- window --------------------------------------------------------------
  ['Resets all window hints to their default values.', 'Remet tous les indices de fenêtre à leur valeur par défaut.'],
  ['Creates a window and its associated context.', 'Crée une fenêtre et son contexte associé.'],
  ['Destroys the specified window and its context.', 'Détruit la fenêtre indiquée et son contexte.'],
  ['Checks the close flag of the specified window.', "Consulte l'indicateur de fermeture de la fenêtre indiquée."],
  ['Sets the close flag of the specified window.', "Règle l'indicateur de fermeture de la fenêtre indiquée."],
  ['Sets the title of the specified window.', 'Règle le titre de la fenêtre indiquée.'],
  ['Sets the icon for the specified window.', "Règle l'icône de la fenêtre indiquée."],
  [
    'Retrieves the position of the content area of the specified window.',
    'Récupère la position de la zone de contenu de la fenêtre indiquée.',
  ],
  [
    'Sets the position of the content area of the specified window.',
    'Règle la position de la zone de contenu de la fenêtre indiquée.',
  ],
  [
    'Retrieves the size of the content area of the specified window.',
    'Récupère la taille de la zone de contenu de la fenêtre indiquée.',
  ],
  ['Sets the size limits of the specified window.', 'Fixe les limites de taille de la fenêtre indiquée.'],
  ['Sets the aspect ratio of the specified window.', 'Fixe la proportion de la fenêtre indiquée.'],
  [
    'Sets the size of the content area of the specified window.',
    'Règle la taille de la zone de contenu de la fenêtre indiquée.',
  ],
  [
    'Retrieves the size of the framebuffer of the specified window.',
    "Récupère la taille du tampon d'image de la fenêtre indiquée.",
  ],
  ['Retrieves the size of the frame of the window.', 'Récupère la taille du cadre de la fenêtre.'],
  [
    'Retrieves the content scale for the specified window.',
    "Récupère le facteur d'échelle du contenu de la fenêtre indiquée.",
  ],
  ['Returns the opacity of the whole window.', "Retourne l'opacité de la fenêtre entière."],
  ['Sets the opacity of the whole window.', "Règle l'opacité de la fenêtre entière."],
  ['Iconifies the specified window.', 'Réduit la fenêtre indiquée en icône.'],
  ['Restores the specified window.', 'Restaure la fenêtre indiquée.'],
  ['Maximizes the specified window.', 'Maximise la fenêtre indiquée.'],
  ['Makes the specified window visible.', 'Rend la fenêtre indiquée visible.'],
  ['Hides the specified window.', 'Masque la fenêtre indiquée.'],
  [
    'Brings the specified window to front and sets input focus.',
    'Amène la fenêtre indiquée au premier plan et lui donne le focus.',
  ],
  ['Requests user attention to the specified window.', "Demande l'attention de l'utilisateur sur la fenêtre indiquée."],
  [
    'Returns the monitor that the window uses for full screen mode.',
    "Retourne l'écran qu'utilise la fenêtre en mode plein écran.",
  ],
  [
    'Sets the mode, monitor, video mode and placement of a window.',
    "Règle le mode, l'écran, le mode vidéo et le placement d'une fenêtre.",
  ],
  ['Returns an attribute of the specified window.', 'Retourne un attribut de la fenêtre indiquée.'],
  ['Sets an attribute of the specified window.', 'Règle un attribut de la fenêtre indiquée.'],
  ['Sets the user pointer of the specified window.', 'Règle le pointeur utilisateur de la fenêtre indiquée.'],
  ['Returns the user pointer of the specified window.', 'Retourne le pointeur utilisateur de la fenêtre indiquée.'],
  [
    'Sets the position callback for the specified window.',
    'Installe la fonction de rappel de position de la fenêtre indiquée.',
  ],
  [
    'Sets the titlebar hittest callback for the specified window.',
    'Installe la fonction de rappel de test de la barre de titre de la fenêtre indiquée.',
  ],
  [
    'Sets the size callback for the specified window.',
    'Installe la fonction de rappel de taille de la fenêtre indiquée.',
  ],
  [
    'Sets the close callback for the specified window.',
    'Installe la fonction de rappel de fermeture de la fenêtre indiquée.',
  ],
  [
    'Sets the refresh callback for the specified window.',
    'Installe la fonction de rappel de rafraîchissement de la fenêtre indiquée.',
  ],
  [
    'Sets the focus callback for the specified window.',
    'Installe la fonction de rappel de focus de la fenêtre indiquée.',
  ],
  [
    'Sets the iconify callback for the specified window.',
    'Installe la fonction de rappel de réduction en icône de la fenêtre indiquée.',
  ],
  [
    'Sets the maximize callback for the specified window.',
    'Installe la fonction de rappel de maximisation de la fenêtre indiquée.',
  ],
  [
    'Sets the framebuffer resize callback for the specified window.',
    "Installe la fonction de rappel de redimensionnement du tampon d'image de la fenêtre indiquée.",
  ],
  [
    'Sets the window content scale callback for the specified window.',
    "Installe la fonction de rappel de facteur d'échelle du contenu de la fenêtre indiquée.",
  ],

  // ---- events --------------------------------------------------------------
  ['Processes all pending events.', 'Traite tous les événements en attente.'],
  ['Waits until events are queued and processes them.', "Attend que des événements soient en file, puis les traite."],
  [
    'Waits with timeout until events are queued and processes them.',
    "Attend, avec un délai maximal, que des événements soient en file, puis les traite.",
  ],
  ['Posts an empty event to the event queue.', "Poste un événement vide dans la file d'événements."],

  // ---- input ---------------------------------------------------------------
  [
    'Returns the value of an input option for the specified window.',
    "Retourne la valeur d'une option d'entrée de la fenêtre indiquée.",
  ],
  ['Sets an input option for the specified window.', "Règle une option d'entrée de la fenêtre indiquée."],
  ['Returns whether raw mouse motion is supported.', 'Indique si le mouvement de souris brut est pris en charge.'],
  [
    'Returns the layout-specific name of the specified printable key.',
    'Retourne le nom, propre à la disposition clavier, de la touche imprimable indiquée.',
  ],
  [
    'Returns the platform-specific scancode of the specified key.',
    'Retourne le scancode, propre à la plateforme, de la touche indiquée.',
  ],
  [
    'Returns the last reported state of a keyboard key for the specified window.',
    "Retourne le dernier état signalé d'une touche du clavier pour la fenêtre indiquée.",
  ],
  [
    'Returns the last reported state of a mouse button for the specified window.',
    "Retourne le dernier état signalé d'un bouton de souris pour la fenêtre indiquée.",
  ],
  [
    'Retrieves the position of the cursor relative to the content area of the window.',
    'Récupère la position du curseur par rapport à la zone de contenu de la fenêtre.',
  ],
  [
    'Sets the position of the cursor, relative to the content area of the window.',
    'Règle la position du curseur par rapport à la zone de contenu de la fenêtre.',
  ],
  ['Creates a custom cursor.', 'Crée un curseur personnalisé.'],
  ['Creates a cursor with a standard shape.', 'Crée un curseur de forme standard.'],
  ['Destroys a cursor.', 'Détruit un curseur.'],
  ['Sets the cursor for the window.', 'Règle le curseur de la fenêtre.'],
  ['Sets the key callback.', 'Installe la fonction de rappel de clavier.'],
  ['Sets the Unicode character callback.', 'Installe la fonction de rappel de caractère Unicode.'],
  [
    'Sets the Unicode character with modifiers callback.',
    'Installe la fonction de rappel de caractère Unicode avec modificateurs.',
  ],
  ['Sets the mouse button callback.', 'Installe la fonction de rappel de bouton de souris.'],
  ['Sets the cursor position callback.', 'Installe la fonction de rappel de position du curseur.'],
  ['Sets the cursor enter/leave callback.', "Installe la fonction de rappel d'entrée/sortie du curseur."],
  ['Sets the scroll callback.', 'Installe la fonction de rappel de défilement.'],
  ['Sets the path drop callback.', 'Installe la fonction de rappel de dépôt de chemins.'],

  // ---- joystick ------------------------------------------------------------
  ['Returns whether the specified joystick is present.', 'Indique si la manette indiquée est présente.'],
  [
    'Returns the values of all axes of the specified joystick.',
    'Retourne les valeurs de tous les axes de la manette indiquée.',
  ],
  [
    'Returns the state of all buttons of the specified joystick.',
    "Retourne l'état de tous les boutons de la manette indiquée.",
  ],
  [
    'Returns the state of all hats of the specified joystick.',
    "Retourne l'état de toutes les croix directionnelles de la manette indiquée.",
  ],
  ['Returns the name of the specified joystick.', 'Retourne le nom de la manette indiquée.'],
  [
    'Returns the SDL compatible GUID of the specified joystick.',
    'Retourne le GUID compatible SDL de la manette indiquée.',
  ],
  ['Sets the user pointer of the specified joystick.', 'Règle le pointeur utilisateur de la manette indiquée.'],
  ['Returns the user pointer of the specified joystick.', 'Retourne le pointeur utilisateur de la manette indiquée.'],
  [
    'Returns whether the specified joystick has a gamepad mapping.',
    'Indique si la manette indiquée dispose d\'une correspondance de manette de jeu.',
  ],
  ['Sets the joystick configuration callback.', 'Installe la fonction de rappel de configuration des manettes.'],
  [
    'Adds the specified SDL_GameControllerDB gamepad mappings.',
    'Ajoute les correspondances de manette SDL_GameControllerDB indiquées.',
  ],
  [
    'Returns the human-readable gamepad name for the specified joystick.',
    'Retourne le nom lisible de manette de jeu associé à la manette indiquée.',
  ],
  [
    'Retrieves the state of the specified joystick remapped as a gamepad.',
    "Récupère l'état de la manette indiquée, remappé en manette de jeu.",
  ],

  // ---- clipboard, time -----------------------------------------------------
  ['Sets the clipboard to the specified string.', 'Place la chaîne indiquée dans le presse-papiers.'],
  ['Returns the contents of the clipboard as a string.', 'Retourne le contenu du presse-papiers sous forme de chaîne.'],
  ['Returns the Lumina time.', 'Retourne le temps Lumina.'],
  ['Sets the Lumina time.', 'Règle le temps Lumina.'],
  ['Returns the current value of the raw timer.', 'Retourne la valeur courante du minuteur brut.'],
  ['Returns the frequency, in Hz, of the raw timer.', 'Retourne la fréquence du minuteur brut, en Hz.'],

  // ---- context -------------------------------------------------------------
  [
    'Makes the context of the specified window current for the calling thread.',
    'Rend le contexte de la fenêtre indiquée courant pour le thread appelant.',
  ],
  [
    'Returns the window whose context is current on the calling thread.',
    'Retourne la fenêtre dont le contexte est courant sur le thread appelant.',
  ],
  [
    'Swaps the front and back buffers of the specified window.',
    'Échange les tampons avant et arrière de la fenêtre indiquée.',
  ],
  ['Sets the swap interval for the current context.', "Règle l'intervalle d'échange du contexte courant."],
  ['Returns whether the specified extension is available.', "Indique si l'extension indiquée est disponible."],
  [
    'Returns the address of the specified function for the current context.',
    "Retourne l'adresse de la fonction indiquée pour le contexte courant.",
  ],

  // ---- vulkan --------------------------------------------------------------
  [
    'Returns whether the Vulkan loader and an ICD have been found.',
    'Indique si le chargeur Vulkan et un ICD ont été trouvés.',
  ],
  [
    'Returns the Vulkan instance extensions required by Lumina.',
    "Retourne les extensions d'instance Vulkan requises par Lumina.",
  ],
  [
    'Returns the address of the specified Vulkan instance function.',
    "Retourne l'adresse de la fonction d'instance Vulkan indiquée.",
  ],
  [
    'Returns whether the specified queue family can present images.',
    "Indique si la famille de files indiquée peut présenter des images.",
  ],
  ['Creates a Vulkan surface for the specified window.', 'Crée une surface Vulkan pour la fenêtre indiquée.'],

  // ---- native access -------------------------------------------------------
  [
    'Returns the adapter device name of the specified monitor.',
    "Retourne le nom de périphérique de l'adaptateur de l'écran indiqué.",
  ],
  [
    'Returns the display device name of the specified monitor.',
    "Retourne le nom de périphérique d'affichage de l'écran indiqué.",
  ],
  ['Returns the `HWND` of the specified window.', 'Retourne le `HWND` de la fenêtre indiquée.'],
  ['Returns the `HGLRC` of the specified window.', 'Retourne le `HGLRC` de la fenêtre indiquée.'],
  ['Returns the `CGDirectDisplayID` of the specified monitor.', "Retourne le `CGDirectDisplayID` de l'écran indiqué."],
  ['Returns the `NSWindow` of the specified window.', 'Retourne la `NSWindow` de la fenêtre indiquée.'],
  ['Returns the `NSOpenGLContext` of the specified window.', 'Retourne le `NSOpenGLContext` de la fenêtre indiquée.'],
  ['Returns the `Display` used by Lumina.', 'Retourne le `Display` qu\'utilise Lumina.'],
  ['Returns the `RRCrtc` of the specified monitor.', "Retourne le `RRCrtc` de l'écran indiqué."],
  ['Returns the `RROutput` of the specified monitor.', "Retourne le `RROutput` de l'écran indiqué."],
  ['Returns the `Window` of the specified window.', 'Retourne la `Window` de la fenêtre indiquée.'],
  [
    'Sets the current primary selection to the specified string.',
    'Place la chaîne indiquée dans la sélection primaire courante.',
  ],
  [
    'Returns the contents of the current primary selection as a string.',
    'Retourne le contenu de la sélection primaire courante sous forme de chaîne.',
  ],
  ['Returns the `GLXContext` of the specified window.', 'Retourne le `GLXContext` de la fenêtre indiquée.'],
  ['Returns the `GLXWindow` of the specified window.', 'Retourne la `GLXWindow` de la fenêtre indiquée.'],
  ['Returns the `struct wl_display*` used by Lumina.', "Retourne le `struct wl_display*` qu'utilise Lumina."],
  [
    'Returns the `struct wl_output*` of the specified monitor.',
    "Retourne le `struct wl_output*` de l'écran indiqué.",
  ],
  [
    'Returns the main `struct wl_surface*` of the specified window.',
    'Retourne le `struct wl_surface*` principal de la fenêtre indiquée.',
  ],
  ['Returns the `EGLDisplay` used by Lumina.', "Retourne l'`EGLDisplay` qu'utilise Lumina."],
  ['Returns the `EGLContext` of the specified window.', "Retourne l'`EGLContext` de la fenêtre indiquée."],
  ['Returns the `EGLSurface` of the specified window.', "Retourne l'`EGLSurface` de la fenêtre indiquée."],
  [
    'Retrieves the color buffer associated with the specified window.',
    'Récupère le tampon de couleur associé à la fenêtre indiquée.',
  ],
  [
    'Retrieves the depth buffer associated with the specified window.',
    'Récupère le tampon de profondeur associé à la fenêtre indiquée.',
  ],
  ['Returns the `OSMesaContext` of the specified window.', 'Retourne l\'`OSMesaContext` de la fenêtre indiquée.'],

  // ---- types ---------------------------------------------------------------
  ['Client API function pointer type.', "Type de pointeur de fonction de l'API cliente."],
  ['Vulkan API function pointer type.', "Type de pointeur de fonction de l'API Vulkan."],
  [
    'The function pointer type for memory allocation callbacks.',
    "Type de pointeur de fonction des rappels d'allocation mémoire.",
  ],
  [
    'The function pointer type for memory reallocation callbacks.',
    'Type de pointeur de fonction des rappels de réallocation mémoire.',
  ],
  [
    'The function pointer type for memory deallocation callbacks.',
    'Type de pointeur de fonction des rappels de libération mémoire.',
  ],
  [
    'The function pointer type for error callbacks.',
    "Type de pointeur de fonction des rappels d'erreur.",
  ],
  [
    'The function pointer type for window position callbacks.',
    'Type de pointeur de fonction des rappels de position de fenêtre.',
  ],
  [
    'The function pointer type for window titlebar hittest callbacks.',
    'Type de pointeur de fonction des rappels de test de barre de titre.',
  ],
  [
    'The function pointer type for window size callbacks.',
    'Type de pointeur de fonction des rappels de taille de fenêtre.',
  ],
  [
    'The function pointer type for window close callbacks.',
    'Type de pointeur de fonction des rappels de fermeture de fenêtre.',
  ],
  [
    'The function pointer type for window content refresh callbacks.',
    'Type de pointeur de fonction des rappels de rafraîchissement du contenu de fenêtre.',
  ],
  [
    'The function pointer type for window focus callbacks.',
    'Type de pointeur de fonction des rappels de focus de fenêtre.',
  ],
  [
    'The function pointer type for window iconify callbacks.',
    'Type de pointeur de fonction des rappels de réduction en icône.',
  ],
  [
    'The function pointer type for window maximize callbacks.',
    'Type de pointeur de fonction des rappels de maximisation de fenêtre.',
  ],
  [
    'The function pointer type for framebuffer size callbacks.',
    "Type de pointeur de fonction des rappels de taille de tampon d'image.",
  ],
  [
    'The function pointer type for window content scale callbacks.',
    "Type de pointeur de fonction des rappels de facteur d'échelle du contenu.",
  ],
  [
    'The function pointer type for mouse button callbacks.',
    'Type de pointeur de fonction des rappels de bouton de souris.',
  ],
  [
    'The function pointer type for cursor position callbacks.',
    'Type de pointeur de fonction des rappels de position du curseur.',
  ],
  [
    'The function pointer type for cursor enter/leave callbacks.',
    "Type de pointeur de fonction des rappels d'entrée/sortie du curseur.",
  ],
  [
    'The function pointer type for scroll callbacks.',
    'Type de pointeur de fonction des rappels de défilement.',
  ],
  [
    'The function pointer type for keyboard key callbacks.',
    'Type de pointeur de fonction des rappels de touche du clavier.',
  ],
  [
    'The function pointer type for Unicode character callbacks.',
    'Type de pointeur de fonction des rappels de caractère Unicode.',
  ],
  [
    'The function pointer type for Unicode character with modifiers callbacks.',
    'Type de pointeur de fonction des rappels de caractère Unicode avec modificateurs.',
  ],
  [
    'The function pointer type for path drop callbacks.',
    'Type de pointeur de fonction des rappels de dépôt de chemins.',
  ],
  [
    'The function pointer type for monitor configuration callbacks.',
    'Type de pointeur de fonction des rappels de configuration des écrans.',
  ],
  [
    'The function pointer type for joystick configuration callbacks.',
    'Type de pointeur de fonction des rappels de configuration des manettes.',
  ],
  ['Video mode type.', 'Type de mode vidéo.'],
  ['Gamma ramp.', 'Rampe gamma.'],
  ['Image data.', "Données d'image."],
  ['Gamepad input state', "État d'entrée de manette de jeu"],
];
