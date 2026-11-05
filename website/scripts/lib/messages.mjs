// Every string the generator writes itself, per locale.
//
// Prose that comes from the C headers or the .dox guides is not here: it is
// translated by providing a translated source, never by patching generated
// output. See translations/README.md.

/** @typedef {'en' | 'fr'} Locale */

const en = {
  // Field labels inside an API entry
  parameters: 'Parameters',
  returns: 'Returns',
  errors: 'Errors',
  threadSafety: 'Thread safety',
  pointerLifetime: 'Pointer lifetime',
  reentrancy: 'Reentrancy',
  analysis: 'Analysis',
  callbackSignature: 'Callback signature',
  seeAlso: 'See also',
  since: 'Since',
  members: 'Members',

  // Aside titles
  deprecated: 'Deprecated',
  warning: 'Warning',
  platformNotes: 'Platform notes',
  note: 'Note',

  // Table headers
  thParameter: 'Parameter',
  thDir: 'Dir',
  thDescription: 'Description',
  thConstant: 'Constant',
  thValue: 'Value',
  thMember: 'Member',
  thFunction: 'Function',
  thArea: 'Area',

  // Parameter direction
  dirIn: 'in',
  dirOut: 'out',
  dirInOut: 'in, out',
  dirNone: '—',

  // Kind of a type
  kindHandle: 'Opaque handle',
  kindCallback: 'Callback type',
  kindProc: 'Function pointer type',
  kindStruct: 'Struct',
  kindEnum: 'Enumeration',
  kindAlias: 'Type alias',
  kindOther: 'Type',

  // Headings
  headingTypes: 'Types',
  headingFunctions: 'Functions',
  headingByArea: 'By area',

  // Page titles
  titleAllFunctions: 'All functions',
  titleConstants: 'Constants',

  // Sentences
  leadAllFunctions: (n) =>
    `All ${n} functions of the Lumina API, in alphabetical order.`,
  leadConstants: 'Every constant in the Lumina API, grouped as the headers group them.',
  descAllFunctions: (n) =>
    `Alphabetical index of all ${n} functions in the Lumina API.`,
  descConstants: (n) =>
    `All ${n} constants of the Lumina API: key codes, modifier flags, window hints, error codes and more.`,
  descGroup: (label, fns, types) =>
    `${label}: ${fns} functions and ${types} types in the Lumina API.`,
  descFallback: (label) => `${label} — Lumina documentation.`,
  cardCounts: (fns, types) => `${fns} functions · ${types} types`,
  cardConstants: (n) => `${n} constants`,
  exposeNote: (macro) =>
    `Define \`${macro}\` before including \`<Lumina/lumina_native.h>\` to get this function.`,

  // Only shown in a non-default locale.
  untranslatedTitle: 'Not translated yet',
  untranslatedBody: 'This page is still shown in English.',
};

/**
 * French. Identifiers, macro names and header paths stay in English, because
 * they are what the compiler and the error messages use. Technical terms give
 * the English word in brackets at first use within a page, so the reader can
 * follow the English reference without friction.
 */
const fr = {
  parameters: 'Paramètres',
  returns: 'Valeur de retour',
  errors: 'Erreurs',
  threadSafety: 'Sûreté des threads',
  pointerLifetime: 'Durée de vie du pointeur',
  reentrancy: 'Réentrance',
  analysis: 'Analyse',
  callbackSignature: 'Signature de la fonction de rappel',
  seeAlso: 'Voir aussi',
  since: 'Depuis',
  members: 'Membres',

  deprecated: 'Obsolète',
  warning: 'Avertissement',
  platformNotes: 'Notes par plateforme',
  note: 'Note',

  thParameter: 'Paramètre',
  thDir: 'Sens',
  thDescription: 'Description',
  thConstant: 'Constante',
  thValue: 'Valeur',
  thMember: 'Membre',
  thFunction: 'Fonction',
  thArea: 'Domaine',

  dirIn: 'entrée',
  dirOut: 'sortie',
  dirInOut: 'entrée, sortie',
  dirNone: '—',

  kindHandle: 'Handle opaque',
  kindCallback: 'Type de fonction de rappel',
  kindProc: 'Type de pointeur de fonction',
  kindStruct: 'Structure',
  kindEnum: 'Énumération',
  kindAlias: 'Alias de type',
  kindOther: 'Type',

  headingTypes: 'Types',
  headingFunctions: 'Fonctions',
  headingByArea: 'Par domaine',

  titleAllFunctions: 'Toutes les fonctions',
  titleConstants: 'Constantes',

  leadAllFunctions: (n) =>
    `Les ${n} fonctions de l'API Lumina, par ordre alphabétique.`,
  leadConstants:
    "Toutes les constantes de l'API Lumina, regroupées comme les en-têtes les regroupent.",
  descAllFunctions: (n) =>
    `Index alphabétique des ${n} fonctions de l'API Lumina.`,
  descConstants: (n) =>
    `Les ${n} constantes de l'API Lumina : codes de touches, indicateurs de modificateurs, indices de fenêtre, codes d'erreur et le reste.`,
  descGroup: (label, fns, types) =>
    `${label} : ${fns} fonctions et ${types} types de l'API Lumina.`,
  descFallback: (label) => `${label} — documentation Lumina.`,
  cardCounts: (fns, types) => `${fns} fonctions · ${types} types`,
  cardConstants: (n) => `${n} constantes`,
  exposeNote: (macro) =>
    `Définissez \`${macro}\` avant d'inclure \`<Lumina/lumina_native.h>\` pour disposer de cette fonction.`,

  untranslatedTitle: 'Pas encore traduit',
  untranslatedBody:
    "Ce guide est encore affiché en anglais. L'interface, la navigation et les liens restent en français.",
};

const CATALOGUE = { en, fr };

/** @param {Locale} locale */
export function messages(locale) {
  return CATALOGUE[locale] ?? CATALOGUE.en;
}

export const LOCALE_CODES = Object.keys(CATALOGUE);
