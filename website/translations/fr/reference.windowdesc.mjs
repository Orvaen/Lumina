// The descriptor-based window creation API added in 3.4.

export default [
  // ---- briefs --------------------------------------------------------------
  [
    'Creates a window and its context from a descriptor.',
    'Crée une fenêtre et son contexte à partir d\'un descripteur.',
  ],
  [
    'A three-state switch for window descriptor fields.',
    'Interrupteur à trois états pour les champs de descripteur de fenêtre.',
  ],
  [
    'Window and context creation parameters.',
    'Paramètres de création de fenêtre et de contexte.',
  ],
  [
    'Requests a buffer of exactly zero bits in a window descriptor.',
    'Demande un tampon de zéro bit exactement dans un descripteur de fenêtre.',
  ],

  // ---- descriptions --------------------------------------------------------
  [
    'Fields of a @ref LUMwindowdesc left at zero take the library default, which for several options is "on". A plain `bool` could therefore not express "off" without also meaning "unspecified", so optional switches use this type instead.',
    "Les champs d'un @ref LUMwindowdesc laissés à zéro prennent la valeur par défaut de la bibliothèque, qui vaut « activé » pour plusieurs options.  Un simple `bool` ne saurait donc exprimer « désactivé » sans signifier aussi « non précisé » : les interrupteurs facultatifs emploient donc ce type.",
  ],
  [
    'This describes everything @ref lumCreateWindowEx needs, replacing the global hint state of @ref lumWindowHint. Because it is a plain struct, a window can be created from a single expression and the parameters can be stored, passed around and compared like any other value. __Every field left at zero takes the library default__, so only what differs has to be written: @code LUMwindow* window = lumCreateWindowEx(&(LUMwindowdesc) { .width = 1280, .height = 720, .title = "Lumina", .glMajor = 3, .glMinor = 3, .glProfile = LUM_OPENGL_CORE_PROFILE, .samples = 4, .makeCurrent = LUM_ON }); @endcode The bit depth fields are the one place where zero is ambiguous, because a buffer of zero bits is a meaningful request. Zero means "use the default" there as everywhere else; pass @ref LUM_BITS_NONE to ask for no bits at all, or `LUM_DONT_CARE` to express no preference.',
    "Cette structure décrit tout ce dont @ref lumCreateWindowEx a besoin et remplace l'état global d'indices de @ref lumWindowHint.  Comme c'est une structure ordinaire, une fenêtre se crée en une seule expression, et les paramètres se stockent, se transmettent et se comparent comme n'importe quelle valeur.\n\n__Tout champ laissé à zéro prend la valeur par défaut de la bibliothèque__ : vous n'écrivez donc que ce qui en diffère.\n\n@code\nLUMwindow* window = lumCreateWindowEx(&(LUMwindowdesc)\n{\n    .width       = 1280,\n    .height      = 720,\n    .title       = \"Lumina\",\n    .glMajor     = 3,\n    .glMinor     = 3,\n    .glProfile   = LUM_OPENGL_CORE_PROFILE,\n    .samples     = 4,\n    .makeCurrent = LUM_ON\n});\n@endcode\n\nLes champs de profondeur de bits sont le seul endroit où zéro est ambigu, un tampon de zéro bit étant une demande légitime.  Zéro y signifie « utiliser la valeur par défaut », comme partout ailleurs ; passez @ref LUM_BITS_NONE pour demander aucun bit, ou `LUM_DONT_CARE` pour n'exprimer aucune préférence.",
  ],
  [
    'This function creates a window and, unless the descriptor disables it, an associated OpenGL or OpenGL ES context. Everything is taken from the descriptor, so no global hint state is read and nothing has to be set up beforehand. Every field of the descriptor left at zero takes the library default, so a window can be created from a single expression: @code LUMwindow* window = lumCreateWindowEx(&(LUMwindowdesc) { .width = 1280, .height = 720, .title = "Lumina", .glMajor = 3, .glMinor = 3, .glProfile = LUM_OPENGL_CORE_PROFILE, .samples = 4, .makeCurrent = LUM_ON }); @endcode Apart from where its parameters come from, this function behaves exactly like @ref lumCreateWindow; see that function for the full account of window and context creation, including the platform specific remarks. The descriptor is only read during the call and does not have to outlive it. Setting @ref LUMwindowdesc.makeCurrent to `LUM_ON` makes the new context current on the calling thread before this function returns.',
    "Cette fonction crée une fenêtre et, sauf si le descripteur le désactive, un contexte OpenGL ou OpenGL ES associé.  Tout est pris dans le descripteur : aucun état global d'indices n'est lu et rien n'a besoin d'être préparé au préalable.\n\nTout champ du descripteur laissé à zéro prend la valeur par défaut de la bibliothèque, si bien qu'une fenêtre se crée en une seule expression :\n\n@code\nLUMwindow* window = lumCreateWindowEx(&(LUMwindowdesc)\n{\n    .width       = 1280,\n    .height      = 720,\n    .title       = \"Lumina\",\n    .glMajor     = 3,\n    .glMinor     = 3,\n    .glProfile   = LUM_OPENGL_CORE_PROFILE,\n    .samples     = 4,\n    .makeCurrent = LUM_ON\n});\n@endcode\n\nÀ la provenance de ses paramètres près, cette fonction se comporte exactement comme @ref lumCreateWindow ; voyez celle-ci pour le détail complet de la création de fenêtre et de contexte, remarques par plateforme comprises.\n\nLe descripteur n'est lu que pendant l'appel et n'a pas besoin de lui survivre.  Régler @ref LUMwindowdesc.makeCurrent sur `LUM_ON` rend le nouveau contexte courant sur le thread appelant avant le retour de la fonction.",
  ],
  [
    'In a @ref LUMwindowdesc a bit depth of zero means "use the default", so this is how a buffer is asked for explicitly not to exist. It differs from `LUM_DONT_CARE`, which expresses no preference either way.',
    "Dans un @ref LUMwindowdesc, une profondeur de zéro bit signifie « utiliser la valeur par défaut » : c'est donc ainsi qu'on demande explicitement qu'un tampon n'existe pas.  À distinguer de `LUM_DONT_CARE`, qui n'exprime aucune préférence.",
  ],

  // ---- parameters, returns, lifetime --------------------------------------
  [
    'The window and context parameters. Must not be `NULL`.',
    'Les paramètres de fenêtre et de contexte.  Ne doit pas être `NULL`.',
  ],
  [
    'The strings in the descriptor are copied before this function returns.',
    'Les chaînes du descripteur sont copiées avant que cette fonction ne rende la main.',
  ],

  // ---- deprecation notices -------------------------------------------------
  [
    'Window parameters are now passed in a @ref LUMwindowdesc to @ref lumCreateWindowEx, which needs no global hint state. The hint functions continue to work and are not scheduled for removal.',
    "Les paramètres de fenêtre se passent désormais dans un @ref LUMwindowdesc à @ref lumCreateWindowEx, qui n'a besoin d'aucun état global d'indices.  Les fonctions d'indices continuent de fonctionner et ne sont pas vouées à disparaître.",
  ],
  [
    'Use @ref lumCreateWindowEx, which takes every parameter in a @ref LUMwindowdesc instead of reading the global hint state. This function continues to work and is not scheduled for removal.',
    "Utilisez @ref lumCreateWindowEx, qui prend tous ses paramètres dans un @ref LUMwindowdesc au lieu de lire l'état global d'indices.  Cette fonction continue de fonctionner et n'est pas vouée à disparaître.",
  ],

  // ---- LUMtoggle values ----------------------------------------------------
  ['Use the library default for this option.', 'Utiliser la valeur par défaut de la bibliothèque pour cette option.'],
  ['Enable this option.', 'Activer cette option.'],
  ['Disable this option.', 'Désactiver cette option.'],

  // ---- LUMwindowdesc members ----------------------------------------------
  [
    'The width, in screen coordinates, of the content area. Required.',
    "La largeur de la zone de contenu, en coordonnées d'écran.  Obligatoire.",
  ],
  [
    'The height, in screen coordinates, of the content area. Required.',
    "La hauteur de la zone de contenu, en coordonnées d'écran.  Obligatoire.",
  ],
  [
    'The initial, UTF-8 encoded window title. Required.',
    'Le titre initial de la fenêtre, encodé en UTF-8.  Obligatoire.',
  ],
  [
    'The window whose context to share objects with, or `NULL`.',
    'La fenêtre dont le contexte partagera ses objets, ou `NULL`.',
  ],
  [
    'The desired refresh rate, in Hz, of a full screen window.',
    "Le taux de rafraîchissement voulu, en Hz, d'une fenêtre plein écran.",
  ],
  [
    '`LUM_OFF` creates the window without a client API context, as Vulkan requires. This replaces setting `LUM_CLIENT_API` to `LUM_NO_API`.',
    "`LUM_OFF` crée la fenêtre sans contexte d'API cliente, comme Vulkan l'exige.  Cela remplace le réglage de `LUM_CLIENT_API` sur `LUM_NO_API`.",
  ],
  ['`LUM_OPENGL_API` or `LUM_OPENGL_ES_API`.', '`LUM_OPENGL_API` ou `LUM_OPENGL_ES_API`.'],
  [
    '`LUM_NATIVE_CONTEXT_API`, `LUM_EGL_CONTEXT_API` or `LUM_OSMESA_CONTEXT_API`.',
    '`LUM_NATIVE_CONTEXT_API`, `LUM_EGL_CONTEXT_API` ou `LUM_OSMESA_CONTEXT_API`.',
  ],
  [
    'The minimum major version the context must be compatible with.',
    'La version majeure minimale avec laquelle le contexte doit être compatible.',
  ],
  [
    'The minimum minor version the context must be compatible with.',
    'La version mineure minimale avec laquelle le contexte doit être compatible.',
  ],
  [
    '`LUM_OPENGL_CORE_PROFILE` or `LUM_OPENGL_COMPAT_PROFILE`.',
    '`LUM_OPENGL_CORE_PROFILE` ou `LUM_OPENGL_COMPAT_PROFILE`.',
  ],
  [
    '`LUM_NO_RESET_NOTIFICATION` or `LUM_LOSE_CONTEXT_ON_RESET`.',
    '`LUM_NO_RESET_NOTIFICATION` ou `LUM_LOSE_CONTEXT_ON_RESET`.',
  ],
  [
    '`LUM_RELEASE_BEHAVIOR_FLUSH` or `LUM_RELEASE_BEHAVIOR_NONE`.',
    '`LUM_RELEASE_BEHAVIOR_FLUSH` ou `LUM_RELEASE_BEHAVIOR_NONE`.',
  ],
  [
    'The number of samples to use for multisampling. Zero disables it.',
    "Le nombre d'échantillons pour le multiéchantillonnage.  Zéro le désactive.",
  ],
  [
    'Use full resolution framebuffers on Retina displays. Defaults to on.',
    "Utiliser des tampons d'image en pleine résolution sur les écrans Retina.  Activé par défaut.",
  ],
  [
    'Let the system move the context between GPUs. Defaults to off.',
    "Laisser le système déplacer le contexte d'un GPU à l'autre.  Désactivé par défaut.",
  ],
  [
    'The frame autosave name, or `NULL` to disable frame autosaving.',
    "Le nom d'enregistrement automatique du cadre, ou `NULL` pour le désactiver.",
  ],
  [
    'Allow Alt+Space to open the window menu. Defaults to off.',
    "Autoriser Alt+Espace à ouvrir le menu de la fenêtre.  Désactivé par défaut.",
  ],
  [
    'The class part of the ICCCM `WM_CLASS` window property.',
    'La partie classe de la propriété de fenêtre ICCCM `WM_CLASS`.',
  ],
  [
    'The instance part of the ICCCM `WM_CLASS` window property.',
    'La partie instance de la propriété de fenêtre ICCCM `WM_CLASS`.',
  ],
  [
    '`LUM_ON` makes the new context current on the calling thread, saving a separate call to @ref lumMakeContextCurrent. Ignored when the window has no context.',
    '`LUM_ON` rend le nouveau contexte courant sur le thread appelant, ce qui évite un appel séparé à @ref lumMakeContextCurrent.  Ignoré si la fenêtre n\'a pas de contexte.',
  ],
];
