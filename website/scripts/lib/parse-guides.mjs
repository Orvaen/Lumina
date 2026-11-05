// Parses the Doxygen .dox guides into page records.
//
// Runs before any rendering, because the cross-reference index has to know
// every page, section and anchor id before `@ref` can be resolved.

import { existsSync, readFileSync } from 'node:fs';
import { stripCommentMarkers } from './doxygen.mjs';

/**
 * @param {string} dir directory holding the .dox files
 * @param {Array<{file: string, slug: string}>} pages
 * @param {{optional?: boolean}} [options] when `optional`, pages whose source
 *   is missing are skipped instead of throwing. Used for translations, which
 *   are allowed to be incomplete.
 */
export function parseGuides(dir, pages, options = {}) {
  const sources = options.optional
    ? pages.filter((page) => existsSync(`${dir}/${page.file}`))
    : pages;

  return sources.map((page) => {
    const raw = readFileSync(`${dir}/${page.file}`, 'utf8');
    let text = stripCommentMarkers(raw);

    // The page title comes from `@page id Title`. `@mainpage notitle` has
    // none, and the caller falls back to the label from structure.mjs.
    let pageId = null;
    let title = null;

    text = text.replace(/^\s*@page\s+(\S+)\s+(.+)$/m, (_m, id, pageTitle) => {
      pageId = id;
      title = pageTitle.trim();
      return '';
    });
    text = text.replace(/^\s*@mainpage\b.*$/m, (match) => {
      pageId ??= 'mainpage';
      return match.includes('notitle') ? '' : '';
    });

    text = text.replace(/^\s*@tableofcontents\s*$/gm, '');

    const ids = [];
    const body = convertStructure(text, ids);

    return { ...page, pageId, title, body, ids };
  });
}

/**
 * Turns Doxygen's sectioning commands into Markdown headings, each preceded by
 * an explicit anchor carrying the original Doxygen id. Keeping the id means
 * every `@ref` resolves without having to guess how headings are slugified.
 */
function convertStructure(text, ids) {
  const out = [];

  for (const line of text.split('\n')) {
    const heading =
      line.match(/^\s*@(section|subsection|subsubsection)\s+(\S+)\s+(.+?)\s*$/);
    if (heading) {
      const level = { section: '##', subsection: '###', subsubsection: '####' }[
        heading[1]
      ];
      ids.push({ id: heading[2], title: heading[3] });
      out.push('', `<a id="${heading[2]}"></a>`, '', `${level} ${heading[3]}`, '');
      continue;
    }

    const anchor = line.match(/^\s*@anchor\s+(\S+)\s*$/);
    if (anchor) {
      // A bare anchor has no title of its own.
      ids.push({ id: anchor[1], title: null });
      out.push('', `<a id="${anchor[1]}"></a>`, '');
      continue;
    }

    out.push(line);
  }

  return out.join('\n');
}
