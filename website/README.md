# Lumina documentation site

An [Astro Starlight](https://starlight.astro.build/) site. Almost none of its
content is written by hand: it is generated from the library sources, so the
documentation cannot describe an API that no longer exists.

```sh
npm install
npm run dev       # generate, then serve with hot reload on :4321
npm run build     # generate, then build the static site into dist/
npm run preview   # serve dist/
npm run check:links
```

## Languages

English is served at the root, French under `/fr`. Starlight supplies its own
French interface strings, so the chrome — search, "Sur cette page", pagination,
the language switcher — is translated with no work on our side.

Everything the generator writes itself goes through `scripts/lib/messages.mjs`:
field labels, table headers, aside titles, page titles and descriptions.

Guides are translated by adding a `.dox` file under `translations/<locale>/`
with the same name as its English source. See `translations/README.md`.

**The reference is generated from the C headers**, so it cannot be translated
by editing its output. Instead each documentation field is looked up, by its
exact English text, in `translations/<locale>/reference*.mjs` — a list of
`[english, french]` pairs. Keying on the text rather than on the symbol is
what makes this affordable: the reference is formulaic, and 1587 field
occurrences come down to 951 distinct strings (`@thread_safety` alone is 150
occurrences of 7 sentences). It is also self-healing: reword a header comment
and its entry stops matching, so the field falls back to English and the
generator reports it rather than shipping a stale translation.

Every run rewrites `translations/<locale>/reference.todo.txt` with what is
left, sorted by how often each string occurs — translate from the top for the
most coverage per word.

**Every page is emitted in every locale**, with the English source standing in
where there is no translation yet, under a short notice saying so. Leaving
those pages to Starlight's own fallback would serve the English file verbatim,
and its links point outside the locale — one click and the reader is back in
English for good.

The generator checks that each translation keeps every section and anchor id of
its source, and fails the build if not: those ids are what `@ref` and every
deep link resolve against, so a renamed one would break links silently.

Current coverage is printed by `npm run build`:

```
[en] guides 14/14 (100%) · reference 1587 fields · 846 ids indexed
[fr] guides 14/14 (100%) · reference 1419/1587 fields (89%) · 846 ids indexed
```

The guides, the landing page and the 404 are fully French. The reference is at
89% of its fields; what remains is the long per-symbol descriptions, listed in
`translations/fr/reference.todo.txt`.

## Where the content comes from

| Source | Becomes |
| ------ | ------- |
| `include/Lumina/lumina.h` | `/reference/*` — 124 functions, 34 types, 323 constants |
| `include/Lumina/lumina_native.h` | `/reference/native/` — 24 platform functions |
| `docs/*.dox` | `/overview`, `/guides/*`, `/about/*` |
| `docs/spaces.svg` | copied to `public/` |

`npm run generate` rewrites these directories from scratch on every run, so
they are not committed:

- `src/content/docs/overview.md`
- `src/content/docs/guides/`
- `src/content/docs/about/`
- `src/content/docs/reference/`

## Typography and colour

Three self-hosted typefaces, one per job, wired through Starlight's
`--sl-font` / `--sl-font-mono` tokens in `src/styles/custom.css`:

| Role | Family | Used for |
| ---- | ------ | -------- |
| Body and UI | Instrument Sans | prose, tables, sidebar, search |
| Display | Space Grotesk | all headings, the site title, the hero |
| Mono | JetBrains Mono | code blocks, signatures, identifiers |

They come from Fontsource as npm packages, so nothing is requested from a font
CDN at runtime and there is no flash of unstyled text. The files are subset
with `unicode-range`; an English page fetches only the latin ranges. Code
ligatures are disabled, because documentation should show the exact characters
a compiler sees.

To swap a family: install its `@fontsource-variable/…` package, replace the
entry in `customCss` in `astro.config.mjs`, and change the one `--sl-font*`
token that names it.

Colour follows the logo: violet, with `--sl-color-accent*` set in `:root`.
Those declarations are deliberately **unlayered**, which is how they beat
Starlight's own defaults inside `@layer starlight.base` — source order would
not be enough.

Selection comes in two strengths: `--lum-selection-bg` (solid, for prose) and
`--lum-selection-soft` (translucent, for code, so syntax highlighting survives
being selected). Expressive Code keeps its own selection colour, so it is set
to the same violet through `expressiveCode.styleOverrides` in
`astro.config.mjs`.

## Icons

`scripts/lib/icons.mjs` holds the whole set as inline SVG, drawn in the logo's
language: two tones of violet, flat fills, no outlines, generously rounded
corners, folded planes where depth helps. Polygons get their rounded corners
from a same-coloured stroke with `stroke-linejoin: round`, which keeps the
shapes readable and editable.

The colours are CSS custom properties (`--lum-icon-deep`, `--lum-icon-main`,
`--lum-icon-light`, `--lum-icon-pane`) with literal fallbacks, so the set
retunes per theme from one place in `custom.css` and still renders correctly
if the stylesheet has not loaded.

One module serves both consumers, so they cannot drift:

- `src/components/LumIcon.astro` and `LumCard.astro` use it on the landing page
- `scripts/generate.mjs` uses it for the page icons and the "By area" card grid
  on `/reference/`, emitted as plain HTML (Markdown passes an HTML block
  through untouched as long as it contains no blank line)

Each page's icon is set in `scripts/lib/structure.mjs`, next to its slug and
label. To add an icon, add it to `icons.mjs`; `ICON_NAMES` lists what exists,
and an unknown name renders as nothing rather than breaking the page.

Card grids carry Starlight's `not-content` class, which opts them out of its
Markdown link and spacing styles.

## Authored files

Everything else under `src/` is authored and safe to edit:

- `src/content/docs/index.mdx` — the landing page
- `src/content/docs/404.md`
- `src/styles/custom.css` — theme tokens, icon colours, cards, reference layout
- `src/components/LumIcon.astro`, `src/components/LumCard.astro`
- `src/assets/lumina.svg` — the logo mark; `public/favicon.svg` is a heavier
  cut of it with larger panes, so it still reads at 16 px
- `astro.config.mjs`
- `scripts/lib/structure.mjs` — page order, slugs, icons and the sidebar
- `scripts/lib/icons.mjs` — the icon set

## How the generator works

`scripts/generate.mjs` runs in three phases, because cross-references have to
be resolvable before anything is rendered:

1. **Parse.** `lib/parse-header.mjs` walks the headers into a model of groups,
   functions, types and constants, each with its documentation comment.
   `lib/parse-guides.mjs` reads the `.dox` guides and turns Doxygen's
   sectioning commands into Markdown headings.
2. **Index.** Every page, section, anchor, function, type and constant id is
   collected into one map of Doxygen id to URL. Each heading is preceded by an
   explicit `<a id="…">` carrying its original Doxygen id, so `@ref` never has
   to guess how a heading was slugified, and links from the old Doxygen output
   keep working.
3. **Render.** `lib/doxygen.mjs` converts Doxygen markup to Markdown: `@ref`
   becomes a link labelled with the target's own title, `@code` becomes a
   fenced block, `@note` and `@remark` become Starlight asides, and the
   platform aliases (`@win32`, `@macos`, `@x11`, `@wayland`) become bold
   labels.

The generator reports what it could not place — unresolved `@ref` targets,
unhandled Doxygen tags, ids documented in two places — rather than dropping
them silently. A clean run currently reports two unresolved targets,
`LUM_TITLEBAR_hint` and `LUM_TITLEBAR_attrib`: the `LUM_TITLEBAR` window hint
references documentation anchors that were never written in `docs/window.dox`.
They render as plain code instead of broken links.

## Before publishing

Three things in `astro.config.mjs` are left commented out because they need
URLs this project does not have yet:

- `site` — enables canonical links and the sitemap
- `editLink.baseUrl` — adds "Edit this page" links
- `social` — adds the repository icon to the header

The pages also still link to `glfw.org` wherever the guides did, since the
upstream documentation describes the same API under the old names.
