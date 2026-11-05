// The Lumina icon set.
//
// Drawn in the language of the logo: two tones of violet, flat fills with no
// outlines, generously rounded corners, and folded planes where depth helps.
// Polygons get their rounded corners from a same-coloured stroke with
// `stroke-linejoin: round`, which is far simpler than arc paths and keeps the
// shapes editable.
//
// Colours come from CSS custom properties so the set can be retuned per theme
// in one place; the literal fallbacks keep the icons correct if the stylesheet
// has not loaded.

const DEEP = 'var(--lum-icon-deep, #7c3aed)';
const MAIN = 'var(--lum-icon-main, #8b5cf6)';
const LIGHT = 'var(--lum-icon-light, #a78bfa)';
const PANE = 'var(--lum-icon-pane, #f5f3ff)';

/** A polygon with rounded corners. */
const poly = (d, fill, round = 3) =>
  `<path d="${d}Z" fill="${fill}" stroke="${fill}" stroke-width="${round}" stroke-linejoin="round"/>`;

const rect = (x, y, w, h, r, fill) =>
  `<rect x="${x}" y="${y}" width="${w}" height="${h}" rx="${r}" fill="${fill}"/>`;

export const icons = {
  /** The logo's own motif: a frame of panes. */
  window: [
    rect(5, 7, 38, 34, 7, MAIN),
    rect(11, 13, 10.5, 10.5, 2.5, PANE),
    rect(26.5, 13, 10.5, 10.5, 2.5, PANE),
    rect(11, 28.5, 10.5, 10.5, 2.5, PANE),
    rect(26.5, 28.5, 10.5, 10.5, 2.5, PANE),
  ].join(''),

  /** One lit pane with a play mark: run your first window. */
  firstwindow: [
    rect(5, 7, 38, 34, 7, MAIN),
    rect(11, 13, 26, 22, 4, PANE),
    poly('M20 18.5 L30.5 24 L20 29.5 ', DEEP, 2.5),
  ].join(''),

  /** A keycap and a pointer. */
  input: [
    rect(5, 9, 24, 24, 6, LIGHT),
    rect(11, 15, 12, 12, 3, PANE),
    poly('M26 24 L43 31 L35.5 33.5 L39.5 42 L34.5 44 L30.5 35.5 L25 41 ', MAIN, 2.5),
  ].join(''),

  /** A display on its stand. */
  monitor: [
    rect(4, 8, 40, 27, 6, MAIN),
    rect(10, 14, 28, 15, 3, PANE),
    rect(20, 35, 8, 5, 1.5, LIGHT),
    rect(13, 39, 22, 5, 2.5, LIGHT),
  ].join(''),

  /** Two surfaces, one drawn over the other. */
  context: [
    poly('M24 19 L42 28 L24 37 L6 28 ', MAIN),
    poly('M24 6 L42 15 L24 24 L6 15 ', LIGHT),
  ].join(''),

  /** A rendering primitive. */
  vulkan: [
    poly('M24 6 L43 40 L5 40 ', MAIN, 4),
    poly('M24 19 L33 34 L15 34 ', PANE, 2.5),
  ].join(''),

  /** Power: the library coming up. */
  init: [
    rect(5, 5, 38, 38, 10, LIGHT),
    `<path d="M16.5 18.5a10.5 10.5 0 1 0 15 0" fill="none" stroke="${DEEP}" stroke-width="4.5" stroke-linecap="round"/>`,
    `<path d="M24 11 L24 23" fill="none" stroke="${DEEP}" stroke-width="4.5" stroke-linecap="round"/>`,
  ].join(''),

  /** A chip, for platform handles. */
  native: [
    rect(21, 3, 6, 7, 2, LIGHT),
    rect(21, 38, 6, 7, 2, LIGHT),
    rect(3, 21, 7, 6, 2, LIGHT),
    rect(38, 21, 7, 6, 2, LIGHT),
    rect(10, 10, 28, 28, 7, MAIN),
    rect(18, 18, 12, 12, 3, PANE),
  ].join(''),

  /** A set of values. */
  constants: [
    rect(5, 5, 11.5, 11.5, 3, MAIN),
    rect(18.5, 5, 11.5, 11.5, 3, LIGHT),
    rect(32, 5, 11.5, 11.5, 3, MAIN),
    rect(5, 18.5, 11.5, 11.5, 3, LIGHT),
    rect(18.5, 18.5, 11.5, 11.5, 3, MAIN),
    rect(32, 18.5, 11.5, 11.5, 3, LIGHT),
    rect(5, 32, 11.5, 11.5, 3, MAIN),
    rect(18.5, 32, 11.5, 11.5, 3, LIGHT),
    rect(32, 32, 11.5, 11.5, 3, MAIN),
  ].join(''),

  /** Run your first program. */
  start: [
    rect(5, 5, 38, 38, 10, MAIN),
    poly('M19 15 L35 24 L19 33 ', PANE, 3),
  ].join(''),

  /** Pieces coming together. */
  build: [
    rect(16, 4, 16, 16, 4.5, LIGHT),
    rect(4, 26, 16, 16, 4.5, MAIN),
    rect(28, 26, 16, 16, 4.5, MAIN),
  ].join(''),

  /** Something to ship. */
  package: [
    poly('M6 14 L24 23 L24 43 L6 34 ', MAIN),
    poly('M42 14 L42 34 L24 43 L24 23 ', DEEP),
    poly('M24 4 L42 14 L24 23 L6 14 ', LIGHT),
  ].join(''),

  /** The same code on several systems. */
  platforms: [
    rect(5, 5, 23, 23, 6.5, LIGHT),
    rect(20, 20, 23, 23, 6.5, MAIN),
  ].join(''),

  /** Callbacks arriving. */
  events: [
    `<circle cx="13" cy="35" r="5.5" fill="${MAIN}"/>`,
    `<path d="M13 23a12 12 0 0 1 12 12" fill="none" stroke="${LIGHT}" stroke-width="4.5" stroke-linecap="round"/>`,
    `<path d="M13 11a24 24 0 0 1 24 24" fill="none" stroke="${LIGHT}" stroke-width="4.5" stroke-linecap="round"/>`,
  ].join(''),

  /** The catalogue of the API. */
  reference: [
    rect(7, 4, 34, 40, 7, MAIN),
    rect(13, 12, 22, 4.5, 2.25, PANE),
    rect(13, 21.5, 22, 4.5, 2.25, PANE),
    rect(13, 31, 13, 4.5, 2.25, PANE),
  ].join(''),

  /** Release notes and other reading. */
  notes: [
    rect(8, 4, 32, 40, 7, LIGHT),
    rect(14, 12, 20, 4.5, 2.25, DEEP),
    rect(14, 21.5, 20, 4.5, 2.25, DEEP),
    rect(14, 31, 12, 4.5, 2.25, DEEP),
  ].join(''),

  /** Moving code from one API to the other. */
  migrate: [
    rect(2, 11, 17, 26, 5, LIGHT),
    rect(29, 11, 17, 26, 5, MAIN),
    poly('M20.5 20 L27.5 24 L20.5 28 ', DEEP, 2),
  ].join(''),

  /** Nothing to install. */
  bare: [
    rect(5, 5, 38, 38, 10, LIGHT),
    rect(13, 22, 22, 5.5, 2.75, DEEP),
  ].join(''),
};

export const ICON_NAMES = Object.keys(icons);

/**
 * Inline SVG markup for an icon.
 * Returns an empty string for an unknown name, so a typo degrades to a gap
 * rather than breaking the page.
 */
export function renderIcon(name, { size = 40, className = 'lum-icon' } = {}) {
  const body = icons[name];
  if (!body) return '';
  return (
    `<svg class="${className}" viewBox="0 0 48 48" width="${size}" height="${size}"` +
    ` fill="none" aria-hidden="true" focusable="false">${body}</svg>`
  );
}
