#!/usr/bin/env node
// Builds the Starlight content collection from the Lumina sources:
//   include/Lumina/lumina.h          -> the API reference
//   include/Lumina/lumina_native.h   -> the native access reference
//   docs/*.dox                       -> the guides, in English
//   translations/<locale>/*.dox      -> the guides, translated
//   LICENSE.md, CONTRIBUTORS.md, …   -> the project's own documents
//
// Per locale, these directories are owned by this script and rewritten on
// every run: guides/, about/, reference/, project/ and overview.md.
// Everything else under src/ is authored.
//
// A guide with no translation is simply not emitted for that locale, and
// Starlight serves the English page in its place.

import {
  copyFileSync,
  existsSync,
  mkdirSync,
  readFileSync,
  rmSync,
  writeFileSync,
} from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';

import { parseHeader } from './lib/parse-header.mjs';
import { parseGuides } from './lib/parse-guides.mjs';
import { renderMarkup } from './lib/doxygen.mjs';
import { renderIcon } from './lib/icons.mjs';
import { messages } from './lib/messages.mjs';
import { buildMemory, createTranslator } from './lib/translate.mjs';
import {
  ABOUT,
  ABOUT_DIR,
  BASE,
  CONSTANT_GROUPS,
  DEFAULT_LOCALE,
  GUIDES,
  GUIDE_DIR,
  LOCALES,
  OVERVIEW,
  PROJECT,
  PROJECT_DIR,
  REFERENCE_DIR,
  REFERENCE_PAGES,
  REPO,
  labelOf,
} from './lib/structure.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const site = resolve(here, '..');
const repo = resolve(site, '..');
const docsDir = resolve(repo, 'docs');
const includeDir = resolve(repo, 'include/Lumina');
const examplesDir = resolve(repo, 'examples');
const translationsDir = resolve(site, 'translations');
const contentDir = resolve(site, 'src/content/docs');
const publicDir = resolve(site, 'public');

// ---------------------------------------------------------------------------
// 1. Parse the sources once; they are locale-independent
// ---------------------------------------------------------------------------

const mainHeader = parseHeader(resolve(includeDir, 'lumina.h'));
const nativeHeader = parseHeader(resolve(includeDir, 'lumina_native.h'));

const api = {
  groups: new Map([...mainHeader.groups, ...nativeHeader.groups]),
  functions: [...mainHeader.functions, ...nativeHeader.functions],
  types: [...mainHeader.types, ...nativeHeader.types],
  macros: [...mainHeader.macros, ...nativeHeader.macros],
};

const PAGE_LIST = [
  { ...OVERVIEW, dir: '' },
  ...GUIDES.map((page) => ({ ...page, dir: GUIDE_DIR })),
  ...ABOUT.map((page) => ({ ...page, dir: ABOUT_DIR })),
];

/** Parsed guide pages per locale; translations may be partial. */
const parsedByLocale = new Map();
for (const locale of LOCALES) {
  const dir = locale.source ? resolve(translationsDir, locale.source) : docsDir;
  const optional = Boolean(locale.source);
  parsedByLocale.set(
    locale.code,
    existsSync(dir) ? parseGuides(dir, PAGE_LIST, { optional }) : [],
  );
}

const englishPages = parsedByLocale.get(DEFAULT_LOCALE);

/**
 * The reference is generated from the headers, so it is translated through a
 * catalogue of [english, french] pairs rather than by editing its output.
 * A locale with no catalogue simply renders the reference in English.
 */
const memories = new Map();
for (const locale of LOCALES) {
  if (!locale.source) continue;
  const file = resolve(translationsDir, locale.source, 'reference.mjs');
  if (!existsSync(file)) continue;
  const module = await import(pathToFileURL(file).href);
  memories.set(locale.code, buildMemory(module.default));
}

/**
 * A translation must keep every section and anchor id of its source: the ids
 * are what `@ref` and every deep link resolve against, so a renamed one breaks
 * links silently. Compare them rather than trust the translator.
 */
const parityIssues = [];
{
  const englishBySlug = new Map(englishPages.map((page) => [page.slug, page]));
  for (const locale of LOCALES) {
    if (!locale.source) continue;
    for (const page of parsedByLocale.get(locale.code)) {
      const source = englishBySlug.get(page.slug);
      if (!source) continue;

      const where = `${locale.code}/${page.file}`;
      if (source.pageId !== page.pageId) {
        parityIssues.push(`${where}: page id is ${page.pageId}, expected ${source.pageId}`);
      }

      const expected = new Set(source.ids.map((entry) => entry.id));
      const actual = new Set(page.ids.map((entry) => entry.id));
      const missing = [...expected].filter((id) => !actual.has(id));
      const unknown = [...actual].filter((id) => !expected.has(id));
      if (missing.length) parityIssues.push(`${where}: missing ids ${missing.join(', ')}`);
      if (unknown.length) parityIssues.push(`${where}: unknown ids ${unknown.join(', ')}`);
    }
  }
}

// ---------------------------------------------------------------------------
// 2. Shared helpers
// ---------------------------------------------------------------------------

const notes = [];
const unknownTags = new Set();

function write(relativePath, body) {
  const target = resolve(contentDir, relativePath);
  mkdirSync(dirname(target), { recursive: true });
  writeFileSync(target, `${body.replace(/\n{3,}/g, '\n\n').trimEnd()}\n`, 'utf8');
}

function frontmatter(fields) {
  const lines = ['---'];
  for (const [key, value] of Object.entries(fields)) {
    if (value === undefined || value === null || value === '') continue;
    lines.push(typeof value === 'string' ? `${key}: ${JSON.stringify(value)}` : `${key}:`);
    if (typeof value !== 'string') {
      for (const [k, v] of Object.entries(value)) lines.push(`  ${k}: ${JSON.stringify(v)}`);
    }
  }
  lines.push('---', '');
  return lines.join('\n');
}

/** Raw HTML passes through Markdown untouched while it has no blank line. */
function pageIcon(name) {
  const svg = renderIcon(name, { size: 40, className: 'lum-icon' });
  return svg ? `<span class="lum-page-icon">${svg}</span>\n` : '';
}

function cardGrid(cards) {
  const items = cards.map(
    (card) =>
      `<a class="lum-card lum-card--link" href="${card.href}">` +
      `<span class="lum-card__icon">${renderIcon(card.icon, { size: 36 })}</span>` +
      `<span class="lum-card__title">${card.title}</span>` +
      `<span class="lum-card__body">${card.body}</span></a>`,
  );
  // `not-content` is Starlight's opt-out from its Markdown link and spacing
  // styles, so the cards keep their own look.
  return [
    '<div class="lum-grid lum-grid--compact not-content">',
    ...items,
    '</div>',
  ].join('\n');
}

/** First sentence of rendered prose, for the page description. */
function summarise(markdown, fallback) {
  const prose = markdown
    .split('\n')
    .filter((line) => line.trim() && !/^[#`<|:*-]/.test(line.trim()))
    .join(' ');
  const sentence = prose.split(/(?<=\.)\s/)[0] ?? '';
  const text = sentence.replace(/\[([^\]]+)\]\([^)]*\)/g, '$1').replace(/[*`]/g, '').trim();
  return text.length > 20 && text.length < 300 ? text : fallback;
}

function splitParams(text) {
  const parts = [];
  let depth = 0;
  let buffer = '';
  for (const ch of text) {
    if (ch === '(' || ch === '[') depth += 1;
    if (ch === ')' || ch === ']') depth -= 1;
    if (ch === ',' && depth === 0) {
      parts.push(buffer);
      buffer = '';
    } else {
      buffer += ch;
    }
  }
  if (buffer.trim()) parts.push(buffer);
  return parts.map((p) => p.trim());
}

function formatSignature(signature) {
  if (signature.length <= 84) return signature;
  const open = signature.indexOf('(');
  const close = signature.lastIndexOf(')');
  const head = signature.slice(0, open + 1);
  const params = splitParams(signature.slice(open + 1, close));
  const pad = ' '.repeat(head.length);
  return `${head}${params.join(`,\n${pad}`)})`;
}

/**
 * Shows a struct as plain C. The per-member documentation comments would
 * otherwise appear verbatim, Doxygen markup and all, inside the code block;
 * the members table below carries them instead.
 */
function cleanDefinition(definition) {
  return definition
    .replace(/\/\*[!*][\s\S]*?\*\//g, '')
    .split('\n')
    .filter((line) => line.trim() !== '')
    .join('\n')
    .trim();
}

function collectUnknown(doc) {
  for (const tag of doc?.unknownTags ?? []) unknownTags.add(tag);
}

const byName = (a, b) => a.name.localeCompare(b.name);

// ---------------------------------------------------------------------------
// 3. Build one locale
// ---------------------------------------------------------------------------

/**
 * @param {{code: string, base: string, dir: string, source: string | null}} locale
 */
function buildLocale(locale) {
  const t = messages(locale.code);
  const prefix = (path) => `${BASE}${locale.base}${path}`;
  const out = (path) => (locale.dir ? `${locale.dir}/${path}` : path);

  // Localise the API model once, before anything is rendered, so each field
  // is looked up exactly once and the coverage figures stay honest.
  const translator = createTranslator(memories.get(locale.code) ?? null);
  const { translate: tr, translateDoc } = translator;

  const local = {
    groups: new Map(
      [...api.groups].map(([id, group]) => [
        id,
        { ...group, brief: tr(group.brief), description: tr(group.description) },
      ]),
    ),
    functions: api.functions.map((fn) => ({ ...fn, doc: translateDoc(fn.doc) })),
    types: api.types.map((type) => ({
      ...type,
      doc: translateDoc(type.doc),
      members: type.members?.map((m) => ({ ...m, doc: tr(m.doc) })),
    })),
    macros: api.macros.map((macro) => ({ ...macro, doc: translateDoc(macro.doc) })),
  };

  const translated = parsedByLocale.get(locale.code);
  const translatedBySlug = new Map(translated.map((page) => [page.slug, page]));

  // Every page is emitted in every locale, using the translation when there is
  // one and the English source otherwise. Leaving untranslated pages to
  // Starlight's fallback would serve the English file as-is, whose links point
  // outside the locale and would drop the reader back into English for good.
  const pagesToEmit = englishPages.map(
    (page) => translatedBySlug.get(page.slug) ?? page,
  );
  const idSources = pagesToEmit;

  const isTranslated = (page) =>
    locale.code === DEFAULT_LOCALE || translatedBySlug.has(page.slug);

  // ---- cross-reference index --------------------------------------------

  const index = new Map();
  const collisions = [];

  function register(id, url, label, soft = false) {
    const existing = index.get(id);
    if (existing && existing.url !== url) {
      if (soft) {
        collisions.push({ id, kept: 'guide' });
        return;
      }
      collisions.push({ id, kept: 'reference' });
    }
    index.set(id, { url, label });
  }

  const codeLabel = (name) => `\`${name}\``;
  const pagePath = (page) =>
    page.dir ? `/${page.dir}/${page.slug}/` : `/${page.slug}/`;

  // `@mainpage notitle` carries no title of its own. An untranslated page also
  // takes the label from structure.mjs, so the heading agrees with the
  // sidebar instead of showing an English title under a French entry.
  const pageTitle = (page) =>
    page.title && page.title !== 'notitle' && isTranslated(page)
      ? page.title
      : labelOf(page, locale.code);

  for (const page of idSources) {
    const url = prefix(pagePath(page));
    if (page.pageId) register(page.pageId, url, pageTitle(page));
    for (const { id, title: heading } of page.ids) {
      register(id, `${url}#${id}`, heading ?? codeLabel(id));
    }
  }

  const groupUrl = new Map(
    REFERENCE_PAGES.map((page) => [page.id, prefix(`/${REFERENCE_DIR}/${page.slug}/`)]),
  );
  const constantsUrl = prefix(`/${REFERENCE_DIR}/constants/`);
  const referenceUrl = (group) => groupUrl.get(group) ?? groupUrl.get('init');

  for (const page of REFERENCE_PAGES) {
    register(page.id, groupUrl.get(page.id), labelOf(page, locale.code));
  }
  for (const group of CONSTANT_GROUPS) {
    if (group.id && !groupUrl.has(group.id)) {
      register(
        group.id,
        `${constantsUrl}#const-${group.id}`,
        labelOf(group, locale.code).toLowerCase(),
      );
    }
  }
  for (const fn of local.functions) {
    register(fn.name, `${referenceUrl(fn.group)}#${fn.name}`, codeLabel(fn.name));
  }
  for (const type of local.types) {
    register(type.name, `${referenceUrl(type.group)}#${type.name}`, codeLabel(type.name));
  }
  for (const macro of local.macros) {
    register(macro.name, `${constantsUrl}#${macro.name}`, codeLabel(macro.name), true);
  }
  register('lumina.h', prefix(`/${REFERENCE_DIR}/`), codeLabel('lumina.h'));
  register('lumina_native.h', prefix(`/${REFERENCE_DIR}/native/`), codeLabel('lumina_native.h'));

  // ---- rendering context -------------------------------------------------

  const unresolved = new Map();
  const ctx = {
    resolve: (id) => index.get(id) ?? null,
    onUnresolved: (id) => unresolved.set(id, (unresolved.get(id) ?? 0) + 1),
    onNote: (message) => notes.push(message),
    snippetRoot: examplesDir,
  };

  const md = (text) => renderMarkup(text, ctx);
  const cell = (text) =>
    md(text).replace(/\n+/g, ' ').replace(/\|/g, '\\|').trim();

  // ---- wipe what this locale owns ---------------------------------------

  for (const dir of [GUIDE_DIR, ABOUT_DIR, REFERENCE_DIR, PROJECT_DIR]) {
    rmSync(resolve(contentDir, out(dir)), { recursive: true, force: true });
  }
  rmSync(resolve(contentDir, out(`${OVERVIEW.slug}.md`)), { force: true });

  // ---- guides ------------------------------------------------------------

  for (const page of pagesToEmit) {
    let body = md(page.body);
    // intro.dox embeds a diagram that now lives in /public.
    body = body.replace(/src="spaces\.svg"/g, 'src="/spaces.svg"');

    const label = labelOf(page, locale.code);
    const path = out(page.dir ? `${page.dir}/${page.slug}.md` : `${page.slug}.md`);

    const notice = isTranslated(page)
      ? ''
      : `:::caution[${t.untranslatedTitle}]\n${t.untranslatedBody}\n:::\n\n`;

    write(
      path,
      frontmatter({
        title: pageTitle(page),
        description: summarise(body, t.descFallback(label)),
      }) +
        pageIcon(page.icon) +
        notice +
        body,
    );
  }

  // ---- the project's own documents --------------------------------------

  // These are the only pages whose source is not written for the site. The
  // licence, the contribution guide, the support note and the credits are
  // read in the repository by people who never open the site, so the page
  // renders that same file instead of a copy that would quietly drift from
  // it. Its first heading is dropped: Starlight already paints the title.
  for (const page of PROJECT) {
    const source = resolve(repo, page.file);
    if (!existsSync(source)) {
      notes.push(`project document not found: ${page.file}`);
      continue;
    }

    const label = labelOf(page, locale.code);
    const body = readFileSync(source, 'utf8')
      .replace(/^﻿/, '')
      .replace(/^#[ \t]+.*\n+/, '')
      .trim();

    // Say where the text comes from, so a reader who wants to propose a
    // change knows which file to edit.
    const origin =
      `:::note\n${t.fromRepo(page.file)} ` +
      `[${t.fromRepoLink}](${REPO}/blob/master/${page.file})\n:::\n\n`;

    // There is no translation route for these: they are one file, shared with
    // the repository. A locale that is not the default says so.
    const notice =
      locale.code === DEFAULT_LOCALE
        ? ''
        : `:::caution[${t.untranslatedTitle}]\n${t.untranslatedBody}\n:::\n\n`;

    write(
      out(`${PROJECT_DIR}/${page.slug}.md`),
      frontmatter({
        title: label,
        description: summarise(body, t.descFallback(label)),
      }) +
        pageIcon(page.icon) +
        origin +
        notice +
        body,
    );
  }

  // ---- reference: entry renderers ---------------------------------------

  const DIRECTION_LABEL = { in: t.dirIn, out: t.dirOut, 'in,out': t.dirInOut };

  const KIND_LABEL = {
    handle: t.kindHandle,
    callback: t.kindCallback,
    proc: t.kindProc,
    struct: t.kindStruct,
    enum: t.kindEnum,
    alias: t.kindAlias,
  };

  function paramTable(params) {
    const rows = params.map(
      (p) =>
        `| \`${p.name}\` | ${DIRECTION_LABEL[p.direction] ?? t.dirNone} | ${cell(p.text)} |`,
    );
    return [
      `| ${t.thParameter} | ${t.thDir} | ${t.thDescription} |`,
      '| --- | --- | --- |',
      ...rows,
    ].join('\n');
  }

  function labelled(label, text) {
    if (!text?.trim()) return [];
    return [`**${label}**`, '', md(text), ''];
  }

  function asideBlock(type, title, bodies) {
    const content = bodies.filter((b) => b?.trim()).map((b) => md(b));
    if (!content.length) return [];
    return [`:::${type}[${title}]`, content.join('\n\n'), ':::', ''];
  }

  function seeAlso(doc) {
    if (!doc?.see?.length) return [];
    return [`**${t.seeAlso}**`, '', ...doc.see.map((entry) => `- ${cell(entry)}`), ''];
  }

  function renderFunction(fn) {
    const doc = fn.doc;
    collectUnknown(doc);

    const lines = [`<a id="${fn.name}"></a>`, '', `### ${fn.name}`, ''];
    lines.push('```c', formatSignature(fn.signature), '```', '');

    if (doc?.brief) lines.push(md(doc.brief), '');
    if (fn.expose) lines.push(t.exposeNote(fn.expose), '');
    if (doc?.description && doc.description !== doc.brief) lines.push(md(doc.description), '');

    if (doc?.params?.length) lines.push(`**${t.parameters}**`, '', paramTable(doc.params), '');
    lines.push(...labelled(t.returns, doc?.returns));
    lines.push(...labelled(t.errors, doc?.errors));
    lines.push(...asideBlock('caution', t.deprecated, [doc?.deprecated]));
    lines.push(...asideBlock('caution', t.warning, doc?.warnings ?? []));
    lines.push(...asideBlock('note', t.platformNotes, doc?.remarks ?? []));
    lines.push(...asideBlock('tip', t.note, doc?.notes ?? []));
    lines.push(...labelled(t.threadSafety, doc?.threadSafety));
    lines.push(...labelled(t.pointerLifetime, doc?.pointerLifetime));
    lines.push(...labelled(t.reentrancy, doc?.reentrancy));
    lines.push(...labelled(t.analysis, doc?.analysis));
    lines.push(...labelled(t.callbackSignature, doc?.callbackSignature));
    lines.push(...seeAlso(doc));
    lines.push(...labelled(t.since, doc?.since));

    return lines.join('\n');
  }

  function renderType(type) {
    const doc = type.doc;
    collectUnknown(doc);

    const lines = [`<a id="${type.name}"></a>`, '', `### ${type.name}`, ''];
    lines.push(`*${KIND_LABEL[type.kind] ?? t.kindOther}*`, '');
    lines.push('```c', cleanDefinition(type.definition), '```', '');

    if (doc?.brief) lines.push(md(doc.brief), '');
    if (doc?.description && doc.description !== doc.brief) lines.push(md(doc.description), '');

    if (type.members?.length) {
      lines.push(`**${t.members}**`, '');
      lines.push(`| ${t.thMember} | ${t.thDescription} |`, '| --- | --- |');
      for (const member of type.members) {
        lines.push(`| \`${member.declaration.replace(/\|/g, '\\|')}\` | ${cell(member.doc)} |`);
      }
      lines.push('');
    }

    lines.push(...labelled(t.callbackSignature, doc?.callbackSignature));
    lines.push(...asideBlock('note', t.platformNotes, doc?.remarks ?? []));
    lines.push(...asideBlock('tip', t.note, doc?.notes ?? []));
    lines.push(...labelled(t.pointerLifetime, doc?.pointerLifetime));
    lines.push(...seeAlso(doc));
    lines.push(...labelled(t.since, doc?.since));

    return lines.join('\n');
  }

  // ---- reference: group pages -------------------------------------------

  for (const page of REFERENCE_PAGES) {
    const group = local.groups.get(page.id);
    const functions = local.functions.filter((fn) => fn.group === page.id).sort(byName);
    const types = local.types.filter((type) => type.group === page.id).sort(byName);
    const label = labelOf(page, locale.code);

    const lines = [pageIcon(page.icon)];
    if (group?.brief) lines.push(md(group.brief), '');
    if (group?.description) lines.push(md(group.description), '');

    if (types.length) {
      lines.push(`## ${t.headingTypes}`, '');
      lines.push(types.map(renderType).join('\n\n'), '');
    }
    if (functions.length) {
      lines.push(`## ${t.headingFunctions}`, '');
      lines.push(functions.map(renderFunction).join('\n\n'), '');
    }

    write(
      out(`${REFERENCE_DIR}/${page.slug}.md`),
      frontmatter({
        title: label,
        description: t.descGroup(label, functions.length, types.length),
        tableOfContents: { minHeadingLevel: 2, maxHeadingLevel: 3 },
      }) + lines.join('\n'),
    );
  }

  // ---- reference: constants ----------------------------------------------

  const constantsOut = [pageIcon('constants'), t.leadConstants, ''];
  let macroCount = 0;

  for (const group of CONSTANT_GROUPS) {
    const macros = local.macros.filter((macro) => (macro.group ?? null) === group.id);
    if (!macros.length) continue;
    macroCount += macros.length;

    const anchor = `const-${group.id ?? 'other'}`;
    constantsOut.push(`<a id="${anchor}"></a>`, '', `## ${labelOf(group, locale.code)}`, '');

    const definition = local.groups.get(group.id);
    if (definition?.description && group.id && !groupUrl.has(group.id)) {
      constantsOut.push(md(definition.description), '');
    }

    const rows = macros.map((macro) => {
      collectUnknown(macro.doc);
      // Both halves of the comment belong in the cell: a constant's brief is
      // often just its name restated, and the detail that follows is what the
      // reader came for. They stay short enough for a table.
      const parts = [macro.doc?.brief, macro.doc?.description]
        .map((part) => part?.trim())
        .filter(Boolean);
      const unique = parts.filter((part, i) => parts.indexOf(part) === i);
      return {
        macro,
        value: macro.value ? `\`${macro.value.replace(/\|/g, '\\|')}\`` : '',
        description: cell(unique.join(' ')),
      };
    });

    // Key codes and similar tables carry no prose in the header, so the
    // description column would be 122 empty cells.
    const described = rows.some((row) => row.description);

    constantsOut.push(
      described
        ? `| ${t.thConstant} | ${t.thValue} | ${t.thDescription} |`
        : `| ${t.thConstant} | ${t.thValue} |`,
      described ? '| --- | --- | --- |' : '| --- | --- |',
    );
    for (const row of rows) {
      const name = `<a id="${row.macro.name}"></a>\`${row.macro.name}\``;
      constantsOut.push(
        described
          ? `| ${name} | ${row.value} | ${row.description} |`
          : `| ${name} | ${row.value} |`,
      );
    }
    constantsOut.push('');
  }

  write(
    out(`${REFERENCE_DIR}/constants.md`),
    frontmatter({
      title: t.titleConstants,
      description: t.descConstants(macroCount),
      tableOfContents: { minHeadingLevel: 2, maxHeadingLevel: 2 },
    }) + constantsOut.join('\n'),
  );

  // ---- reference: alphabetical index -------------------------------------

  const groupLabel = new Map(
    REFERENCE_PAGES.map((page) => [page.id, labelOf(page, locale.code)]),
  );

  const indexOut = [
    pageIcon('reference'),
    t.leadAllFunctions(local.functions.length),
    '',
    `| ${t.thFunction} | ${t.thArea} | ${t.thDescription} |`,
    '| --- | --- | --- |',
  ];

  for (const fn of [...local.functions].sort(byName)) {
    const url = `${referenceUrl(fn.group)}#${fn.name}`;
    const area = groupLabel.get(fn.group) ?? fn.group ?? '';
    indexOut.push(`| [\`${fn.name}\`](${url}) | ${area} | ${cell(fn.doc?.brief ?? '')} |`);
  }

  indexOut.push('', `## ${t.headingByArea}`, '');
  indexOut.push(
    cardGrid([
      ...REFERENCE_PAGES.map((page) => ({
        href: groupUrl.get(page.id),
        icon: page.icon,
        title: labelOf(page, locale.code),
        body: t.cardCounts(
          local.functions.filter((fn) => fn.group === page.id).length,
          local.types.filter((type) => type.group === page.id).length,
        ),
      })),
      {
        href: constantsUrl,
        icon: 'constants',
        title: t.titleConstants,
        body: t.cardConstants(macroCount),
      },
    ]),
  );

  write(
    out(`${REFERENCE_DIR}/index.md`),
    frontmatter({
      title: t.titleAllFunctions,
      description: t.descAllFunctions(local.functions.length),
      tableOfContents: { minHeadingLevel: 2, maxHeadingLevel: 2 },
    }) + indexOut.join('\n'),
  );

  return {
    locale: locale.code,
    // Every page is emitted; this counts the ones with a real translation.
    guides: pagesToEmit.filter(isTranslated).length,
    guidesTotal: englishPages.length,
    ids: index.size,
    unresolved,
    collisions,
    reference: translator.stats(),
    hasMemory: memories.has(locale.code),
  };
}

// ---------------------------------------------------------------------------
// 4. Run every locale
// ---------------------------------------------------------------------------

mkdirSync(contentDir, { recursive: true });
mkdirSync(publicDir, { recursive: true });

const results = LOCALES.map(buildLocale);

if (existsSync(resolve(docsDir, 'spaces.svg'))) {
  copyFileSync(resolve(docsDir, 'spaces.svg'), resolve(publicDir, 'spaces.svg'));
}

// ---------------------------------------------------------------------------
// 5. Report
// ---------------------------------------------------------------------------

console.log(
  [
    `functions         ${api.functions.length}`,
    `types             ${api.types.length}`,
    `constants         ${api.macros.length}`,
  ].join('\n'),
);

for (const result of results) {
  const pct = Math.round((result.guides / result.guidesTotal) * 100);
  const ref = result.reference;
  const refLabel =
    result.locale === DEFAULT_LOCALE
      ? `reference ${ref.total} fields`
      : `reference ${ref.hits}/${ref.total} fields (${ref.percent}%)`;
  console.log(
    `\n[${result.locale}] guides ${result.guides}/${result.guidesTotal} (${pct}%)` +
      ` · ${refLabel} · ${result.ids} ids indexed`,
  );

  // A working list of what is left to translate, newest run wins.
  if (result.locale !== DEFAULT_LOCALE && ref.missing.size) {
    const dir = LOCALES.find((l) => l.code === result.locale)?.source;
    const file = resolve(translationsDir, dir, 'reference.todo.txt');
    const sorted = [...ref.missing.entries()].sort((a, b) => b[1] - a[1]);
    const words = sorted.reduce((sum, [text]) => sum + text.split(/\s+/).length, 0);
    writeFileSync(
      file,
      `# ${sorted.length} strings left to translate, ${words} words.\n` +
        `# Regenerated by scripts/generate.mjs; add pairs to reference.mjs.\n\n` +
        sorted.map(([text, count]) => `[x${count}] ${text}`).join('\n\n') +
        '\n',
      'utf8',
    );
    console.log(
      `      ${ref.missing.size} string(s) still in English (${words} words)` +
        ` -> translations/${dir}/reference.todo.txt`,
    );
  }

  if (result.collisions.length) {
    const hard = result.collisions.filter((c) => c.kept === 'reference');
    console.log(
      `      ${result.collisions.length} id(s) documented in both a guide and the` +
        ` reference (${result.collisions.length - hard.length} resolved to the guide)`,
    );
    for (const c of hard) console.log(`      overridden: ${c.id}`);
  }

  if (result.unresolved.size) {
    const total = [...result.unresolved.values()].reduce((a, b) => a + b, 0);
    console.log(
      `      ${result.unresolved.size} unresolved @ref target(s), ${total} occurrence(s):`,
    );
    for (const [id, count] of [...result.unresolved.entries()].sort((a, b) => b[1] - a[1])) {
      console.log(`        ${String(count).padStart(4)}  ${id}`);
    }
  }
}

if (parityIssues.length) {
  console.log(`\n${parityIssues.length} translation id mismatch(es):`);
  for (const issue of parityIssues) console.log(`  ${issue}`);
  process.exitCode = 1;
}

if (unknownTags.size) {
  console.log(`\nunhandled Doxygen tags: ${[...unknownTags].join(', ')}`);
}

if (notes.length) {
  console.log(`\n${notes.length} note(s):`);
  for (const note of [...new Set(notes)]) console.log(`  ${note}`);
}
