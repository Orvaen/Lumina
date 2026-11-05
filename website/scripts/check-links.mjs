#!/usr/bin/env node
// Verifies every internal link in the built site, fragments included.
//
// The reference and the guides cross-link heavily and all of it is generated,
// so a renamed section or a dropped anchor would otherwise rot unnoticed.
// Run after `npm run build`.

import { readFileSync, readdirSync, statSync } from 'node:fs';
import { dirname, join, posix, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const site = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const dist = resolve(site, 'dist');

function walk(dir) {
  const found = [];
  for (const entry of readdirSync(dir)) {
    const full = join(dir, entry);
    if (statSync(full).isDirectory()) found.push(...walk(full));
    else if (entry.endsWith('.html')) found.push(full);
  }
  return found;
}

let pages;
try {
  pages = walk(dist);
} catch {
  console.error('No dist/ directory. Run `npm run build` first.');
  process.exit(1);
}

/** url path -> set of ids present on that page */
const idsByPath = new Map();
/** url path of every page that exists */
const existing = new Set();

const pathOf = (file) => {
  const rel = file.slice(dist.length).split('\\').join('/');
  return rel.replace(/\/index\.html$/, '/').replace(/\.html$/, '');
};

for (const file of pages) {
  const html = readFileSync(file, 'utf8');
  const urlPath = pathOf(file) || '/';
  existing.add(urlPath);
  const ids = new Set();
  for (const match of html.matchAll(/\sid="([^"]+)"/g)) ids.add(match[1]);
  for (const match of html.matchAll(/\sname="([^"]+)"/g)) ids.add(match[1]);
  idsByPath.set(urlPath, ids);
}

let checked = 0;
const missingPages = new Map();
const missingFragments = new Map();

for (const file of pages) {
  const html = readFileSync(file, 'utf8');
  const from = pathOf(file) || '/';

  for (const match of html.matchAll(/\shref="([^"]+)"/g)) {
    const href = match[1];
    if (/^(https?:|mailto:|tel:|data:|#|\/_astro\/|\/pagefind\/)/.test(href)) continue;

    // A relative href is almost always a leftover Doxygen page link such as
    // `modules.html`, which has no equivalent here. Flag it rather than skip.
    if (!href.startsWith('/')) {
      checked += 1;
      record(missingPages, `(relative) ${href}  <- ${from}`);
      continue;
    }

    checked += 1;
    const [rawPath, fragment] = href.split('#');
    const target = rawPath === '' ? from : rawPath;
    const normalised = target.endsWith('/') || /\.[a-z0-9]+$/i.test(target)
      ? target
      : `${target}/`;

    // Static assets live in dist but are not HTML pages.
    const isAsset = /\.[a-z0-9]+$/i.test(normalised);
    if (isAsset) continue;

    if (!existing.has(normalised)) {
      record(missingPages, `${normalised}  <- ${from}`);
      continue;
    }
    if (fragment && !idsByPath.get(normalised).has(fragment)) {
      record(missingFragments, `${normalised}#${fragment}  <- ${from}`);
    }
  }
}

function record(map, key) {
  map.set(key, (map.get(key) ?? 0) + 1);
}

console.log(`${pages.length} pages, ${checked} internal links checked`);

if (missingPages.size) {
  console.log(`\n${missingPages.size} link(s) to a missing page:`);
  for (const key of [...missingPages.keys()].slice(0, 40)) console.log(`  ${key}`);
}

if (missingFragments.size) {
  console.log(`\n${missingFragments.size} link(s) to a missing fragment:`);
  for (const key of [...missingFragments.keys()].slice(0, 40)) console.log(`  ${key}`);
}

if (!missingPages.size && !missingFragments.size) {
  console.log('all internal links resolve');
} else {
  process.exitCode = 1;
}
