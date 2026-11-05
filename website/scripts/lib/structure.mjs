// The shape of the site. Imported by both the generator and astro.config.mjs,
// so the sidebar and the generated pages cannot drift apart.

/**
 * The path the site is served from.
 *
 * GitHub Pages serves a project site under /<repo>/, so every absolute link
 * has to carry that prefix or it lands on the wrong host root. This is the
 * only place it is written: set it to an empty string for a root deployment
 * (a user site, or a custom domain) and nothing else needs to change.
 */
export const BASE = '/Lumina';

/** An absolute site path with the base applied: withBase('/reference/'). */
export const withBase = (path) => `${BASE}${path}`;

/**
 * The locales the site is built in.
 *
 * English lives at the root, so every existing link keeps working; French is
 * served under /fr. A page with no translation is simply not emitted for that
 * locale and Starlight falls back to the English content, which means the
 * translation can land page by page without ever breaking the site.
 */
export const LOCALES = [
  { code: 'en', base: '', dir: '', source: null },
  { code: 'fr', base: '/fr', dir: 'fr', source: 'fr' },
];

export const DEFAULT_LOCALE = 'en';

/** Guide pages, in reading order. `file` is the Doxygen source in /docs. */
export const GUIDES = [
  {
    file: 'start.dox',
    slug: 'start',
    icon: 'terminal',
    labels: { en: 'Start a project', fr: 'Démarrer un projet' },
  },
  {
    file: 'first-window.dox',
    slug: 'first-window',
    icon: 'firstwindow',
    labels: { en: 'Open a window', fr: 'Ouvrir une fenêtre' },
  },
  {
    file: 'intro.dox',
    slug: 'introduction',
    icon: 'init',
    labels: { en: 'Introduction to the API', fr: "Introduction à l'API" },
  },
  {
    file: 'quick.dox',
    slug: 'getting-started',
    icon: 'start',
    labels: { en: 'Getting started', fr: 'Premiers pas' },
  },
  {
    file: 'compile.dox',
    slug: 'compiling-lumina',
    icon: 'build',
    labels: { en: 'Compiling Lumina', fr: 'Compiler Lumina' },
  },
  {
    file: 'build.dox',
    slug: 'building-applications',
    icon: 'package',
    labels: { en: 'Building applications', fr: 'Compiler une application' },
  },
  {
    file: 'window.dox',
    slug: 'window',
    icon: 'window',
    labels: { en: 'Windows', fr: 'Fenêtres' },
  },
  {
    file: 'context.dox',
    slug: 'context',
    icon: 'context',
    labels: { en: 'Contexts', fr: 'Contextes' },
  },
  {
    file: 'vulkan.dox',
    slug: 'vulkan',
    icon: 'vulkan',
    labels: { en: 'Vulkan', fr: 'Vulkan' },
  },
  {
    file: 'monitor.dox',
    slug: 'monitor',
    icon: 'monitor',
    labels: { en: 'Monitors', fr: 'Écrans' },
  },
  {
    file: 'input.dox',
    slug: 'input',
    icon: 'input',
    labels: { en: 'Input', fr: 'Entrées' },
  },
];

/** Pages that are about the project rather than the API. */
export const ABOUT = [
  {
    file: 'moving.dox',
    slug: 'migrating-from-glfw-2',
    icon: 'migrate',
    labels: { en: 'Migrating from GLFW 2', fr: 'Migrer depuis GLFW 2' },
  },
  {
    file: 'compat.dox',
    slug: 'standards-conformance',
    icon: 'platforms',
    labels: { en: 'Standards conformance', fr: 'Conformité aux standards' },
  },
  {
    file: 'internal.dox',
    slug: 'internal-structure',
    icon: 'native',
    labels: { en: 'Internal structure', fr: 'Structure interne' },
  },
  {
    file: 'news.dox',
    slug: 'release-notes',
    icon: 'notes',
    labels: { en: 'Release notes', fr: 'Notes de version' },
  },
];

/** The page built from main.dox; it sits at the root of the content tree. */
export const OVERVIEW = {
  file: 'main.dox',
  slug: 'overview',
  icon: 'reference',
  dir: '',
  labels: { en: 'What is Lumina?', fr: "Qu'est-ce que Lumina ?" },
};

/**
 * Reference pages, one per documented Doxygen group that holds functions and
 * types. Constant groups are collected on a single page instead.
 */
export const REFERENCE_PAGES = [
  {
    id: 'init',
    slug: 'init',
    icon: 'init',
    labels: { en: 'Initialization, version and errors', fr: 'Initialisation, version et erreurs' },
  },
  { id: 'window', slug: 'window', icon: 'window', labels: { en: 'Window', fr: 'Fenêtre' } },
  { id: 'input', slug: 'input', icon: 'input', labels: { en: 'Input', fr: 'Entrées' } },
  { id: 'monitor', slug: 'monitor', icon: 'monitor', labels: { en: 'Monitor', fr: 'Écran' } },
  { id: 'context', slug: 'context', icon: 'context', labels: { en: 'Context', fr: 'Contexte' } },
  {
    id: 'vulkan',
    slug: 'vulkan',
    icon: 'vulkan',
    labels: { en: 'Vulkan support', fr: 'Prise en charge de Vulkan' },
  },
  { id: 'native', slug: 'native', icon: 'native', labels: { en: 'Native access', fr: 'Accès natif' } },
];

/** Constant groups, in the order they appear on the constants page. */
export const CONSTANT_GROUPS = [
  { id: 'keys', labels: { en: 'Keyboard keys', fr: 'Touches du clavier' } },
  { id: 'mods', labels: { en: 'Modifier key flags', fr: 'Indicateurs de touches de modification' } },
  { id: 'buttons', labels: { en: 'Mouse buttons', fr: 'Boutons de la souris' } },
  { id: 'shapes', labels: { en: 'Standard cursor shapes', fr: 'Formes de curseur standard' } },
  { id: 'joysticks', labels: { en: 'Joysticks', fr: 'Manettes' } },
  { id: 'gamepad_buttons', labels: { en: 'Gamepad buttons', fr: 'Boutons de manette' } },
  { id: 'gamepad_axes', labels: { en: 'Gamepad axes', fr: 'Axes de manette' } },
  { id: 'hat_state', labels: { en: 'Joystick hat states', fr: 'États de croix directionnelle' } },
  { id: 'errors', labels: { en: 'Error codes', fr: "Codes d'erreur" } },
  {
    id: 'window',
    labels: { en: 'Window hints and attributes', fr: 'Indices et attributs de fenêtre' },
  },
  {
    id: 'init',
    labels: {
      en: 'Initialization hints and shared values',
      fr: "Indices d'initialisation et valeurs partagées",
    },
  },
  { id: 'input', labels: { en: 'Key and button actions', fr: 'Actions des touches et boutons' } },
  { id: null, labels: { en: 'Other values', fr: 'Autres valeurs' } },
];

export const GUIDE_DIR = 'guides';
export const ABOUT_DIR = 'about';
export const REFERENCE_DIR = 'reference';

/** The label of an entry in one locale, falling back to English. */
export function labelOf(entry, locale) {
  return entry.labels[locale] ?? entry.labels[DEFAULT_LOCALE];
}

/** Starlight's per-entry label translations: { fr: '…' }. */
function translationsOf(entry) {
  const out = {};
  for (const { code } of LOCALES) {
    if (code !== DEFAULT_LOCALE && entry.labels[code]) out[code] = entry.labels[code];
  }
  return out;
}

const item = (entry, slug) => ({
  label: labelOf(entry, DEFAULT_LOCALE),
  translations: translationsOf(entry),
  slug,
});

/**
 * The sidebar handed to Starlight. Slugs are locale-agnostic: Starlight
 * resolves each one inside the locale being rendered.
 */
export function buildSidebar() {
  return [
    {
      label: 'Start here',
      translations: { fr: 'Commencer ici' },
      items: [
        item(OVERVIEW, OVERVIEW.slug),
        ...GUIDES.slice(0, 6).map((page) => item(page, `${GUIDE_DIR}/${page.slug}`)),
      ],
    },
    {
      label: 'Guides',
      translations: { fr: 'Guides' },
      items: GUIDES.slice(6).map((page) => item(page, `${GUIDE_DIR}/${page.slug}`)),
    },
    {
      label: 'API reference',
      translations: { fr: "Référence de l'API" },
      items: [
        {
          label: 'All functions',
          translations: { fr: 'Toutes les fonctions' },
          slug: REFERENCE_DIR,
        },
        ...REFERENCE_PAGES.map((page) => item(page, `${REFERENCE_DIR}/${page.slug}`)),
        {
          label: 'Constants',
          translations: { fr: 'Constantes' },
          slug: `${REFERENCE_DIR}/constants`,
        },
      ],
    },
    {
      label: 'About',
      translations: { fr: 'À propos' },
      items: ABOUT.map((page) => item(page, `${ABOUT_DIR}/${page.slug}`)),
    },
  ];
}
