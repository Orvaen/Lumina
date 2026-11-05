// Converts the Doxygen markup used in Lumina's headers and .dox guides into
// the Markdown that Astro Starlight consumes.
//
// Only the subset of Doxygen that Lumina actually uses is handled; the
// generator reports anything it does not recognise instead of silently
// dropping it, so new markup cannot slip through unnoticed.

import { readFileSync } from 'node:fs';

/** `@win32` and friends are Doxyfile ALIASES that expand to a bold label. */
const PLATFORM_LABELS = {
  win32: 'Windows',
  macos: 'macOS',
  x11: 'X11',
  wayland: 'Wayland',
  linux: 'Linux',
  lumina: 'Lumina',
  glfw: 'GLFW',
};

/** Tags that start a new field in a documentation comment. */
const FIELD_TAGS = new Set([
  'brief',
  'param',
  'return',
  'returns',
  'errors',
  'remark',
  'remarks',
  'note',
  'warning',
  'deprecated',
  'thread_safety',
  'pointer_lifetime',
  'reentrancy',
  'analysis',
  'callback_signature',
  'sa',
  'see',
  'since',
  'ingroup',
  'defgroup',
  'addtogroup',
  'name',
  'file',
  'mainpage',
  'page',
  'tableofcontents',
]);

const PLACEHOLDER = '\u0000';

/** Strips `/*!`, the leading ` * ` of each line, and the closing marker. */
export function stripCommentMarkers(raw) {
  return raw
    .replace(/\r\n/g, '\n')
    .replace(/^\s*\/\*[!*]?/, '')
    .replace(/\*\/\s*$/, '')
    .split('\n')
    // The header's comment prefix is ` *  `; dropping both spaces puts the
    // prose at column 0 while preserving any deliberate extra indentation.
    .map((line) => line.replace(/^[ \t]*\*[ \t]{0,2}/, ''))
    .join('\n')
    .replace(/^\n+/, '')
    .replace(/\s+$/, '');
}

/** Removes the smallest common indentation from a block of lines. */
function dedent(block) {
  const lines = block.split('\n');
  const indents = lines
    .filter((l) => l.trim() !== '')
    .map((l) => l.match(/^[ \t]*/)[0].length);
  const cut = indents.length ? Math.min(...indents) : 0;
  return lines.map((l) => l.slice(cut)).join('\n');
}

/** GitHub-slugger compatible enough for the identifiers and titles we feed it. */
export function slug(text) {
  return String(text)
    .trim()
    .toLowerCase()
    .replace(/[^\w\s-]/g, '')
    .replace(/\s+/g, '-');
}

/**
 * Splits a documentation comment body into its description and fields.
 * Code blocks are protected first so that a `@param` inside one cannot be
 * mistaken for a field.
 */
export function parseDocComment(body) {
  const blocks = [];
  const protectedBody = body.replace(
    /@code(?:\{[^}]*\})?[\s\S]*?@endcode/g,
    (match) => {
      blocks.push(match);
      return `${PLACEHOLDER}CODE${blocks.length - 1}${PLACEHOLDER}`;
    },
  );

  const restore = (text) =>
    text.replace(
      new RegExp(`${PLACEHOLDER}CODE(\\d+)${PLACEHOLDER}`, 'g'),
      (_m, i) => blocks[Number(i)],
    );

  const doc = {
    brief: '',
    description: '',
    params: [],
    returns: '',
    errors: '',
    remarks: [],
    notes: [],
    warnings: [],
    deprecated: '',
    threadSafety: '',
    pointerLifetime: '',
    reentrancy: '',
    analysis: '',
    callbackSignature: '',
    see: [],
    since: '',
    groups: [],
    defgroup: null,
    name: null,
    page: null,
    unknownTags: [],
  };

  // Group-structure markers carry no prose.
  let text = protectedBody.replace(/^\s*@\{\s*$/gm, '').replace(/^\s*@\}\s*$/gm, '');

  const lines = text.split('\n');
  const chunks = [];
  let current = { tag: null, lines: [] };

  for (const line of lines) {
    const match = line.match(/^\s*@([a-z_]+[0-9]*)\b(.*)$/);
    const tag = match?.[1];
    if (tag && FIELD_TAGS.has(tag)) {
      chunks.push(current);
      current = { tag, lines: [match[2].replace(/^\s/, '')] };
    } else {
      current.lines.push(line);
    }
  }
  chunks.push(current);

  const descriptionParts = [];

  for (const chunk of chunks) {
    // `raw` still has code blocks as placeholders, so splitting it on blank
    // lines cannot cut a block in half.
    const raw = chunk.lines.join('\n').trim();
    const value = restore(raw).trim();
    switch (chunk.tag) {
      case null:
        if (value) descriptionParts.push(value);
        break;
      case 'brief': {
        // Doxygen's @brief runs to the end of its paragraph; everything after
        // it is the detailed description, which carries no tag of its own.
        // Keeping them apart matters: the brief is what the alphabetical
        // index and the page summaries show.
        const [summary, ...rest] = raw.split(/\n[ \t]*\n/);
        doc.brief = restore(summary).trim();
        const detail = rest.join('\n\n').trim();
        if (detail) descriptionParts.push(restore(detail).trim());
        break;
      }
      case 'param': {
        const m = value.match(/^(?:\[([a-z,]+)\])?\s*(\S+)\s*([\s\S]*)$/);
        if (m) {
          doc.params.push({
            direction: m[1] ?? '',
            name: m[2],
            text: m[3].trim(),
          });
        }
        break;
      }
      case 'return':
      case 'returns':
        doc.returns = value;
        break;
      case 'errors':
        doc.errors = value;
        break;
      case 'remark':
      case 'remarks':
        doc.remarks.push(value);
        break;
      case 'note':
        doc.notes.push(value);
        break;
      case 'warning':
        doc.warnings.push(value);
        break;
      case 'deprecated':
        doc.deprecated = value;
        break;
      case 'thread_safety':
        doc.threadSafety = value;
        break;
      case 'pointer_lifetime':
        doc.pointerLifetime = value;
        break;
      case 'reentrancy':
        doc.reentrancy = value;
        break;
      case 'analysis':
        doc.analysis = value;
        break;
      case 'callback_signature':
        doc.callbackSignature = value;
        break;
      case 'sa':
      case 'see':
        doc.see.push(value);
        break;
      case 'since':
        doc.since = value;
        break;
      case 'ingroup':
        doc.groups.push(value.trim());
        break;
      case 'defgroup':
      case 'addtogroup': {
        const m = value.match(/^(\S+)\s*([\s\S]*)$/);
        if (m) doc.defgroup = { id: m[1], title: m[2].trim() };
        break;
      }
      case 'name':
        doc.name = value;
        break;
      case 'page':
      case 'mainpage': {
        const m = value.match(/^(\S+)\s*([\s\S]*)$/);
        doc.page = chunk.tag === 'mainpage'
          ? { id: 'index', title: value.trim() }
          : { id: m?.[1] ?? '', title: (m?.[2] ?? '').trim() };
        break;
      }
      case 'tableofcontents':
      case 'file':
        break;
      default:
        doc.unknownTags.push(chunk.tag);
        break;
    }
  }

  doc.description = descriptionParts.join('\n\n').trim();
  return doc;
}

/**
 * Renders Doxygen inline markup as Markdown.
 *
 * @param {string} input
 * @param {{resolve: (id: string) => string | null, onUnresolved?: (id: string) => void,
 *          snippetRoot?: string, onNote?: (msg: string) => void}} ctx
 */
export function renderMarkup(input, ctx) {
  if (!input) return '';

  const vault = [];
  const stash = (value) => {
    vault.push(value);
    return `${PLACEHOLDER}${vault.length - 1}${PLACEHOLDER}`;
  };

  let text = input.replace(/\r\n/g, '\n');

  // `@snippet file id` pulls a marked region out of an example program.
  text = text.replace(/@snippet\s+(\S+)\s+(\S+)/g, (match, file, id) => {
    const code = readSnippet(ctx.snippetRoot, file, id, ctx);
    return code ? stash(`\n\`\`\`c\n${code}\n\`\`\`\n`) : match;
  });

  // Fenced code. `.unparsed` has no language; everything else maps directly.
  text = text.replace(
    /@code(?:\{\.?([a-z]+)\})?[ \t]*\n?([\s\S]*?)@endcode[ \t]*/g,
    (_match, lang, body) => {
      const language = !lang || lang === 'unparsed' ? 'c' : lang;
      const code = dedent(body.replace(/^\n+/, '').replace(/\s+$/, ''));
      return stash(`\n\`\`\`${language}\n${code}\n\`\`\`\n`);
    },
  );

  // Inline code spans and complete HTML tags pass through untouched.
  text = text.replace(/`[^`\n]*`/g, (m) => stash(m));
  text = text.replace(/<\/?[a-z][a-z0-9]*(?:\s[^<>]*?)?\/?>/gi, (m) => stash(m));

  // In a guide, `@note` and `@warning` appear inline and run to the end of
  // their paragraph. (In a header they are fields, handled by
  // parseDocComment, so this only ever fires for guide prose.)
  text = convertParagraphCommands(text);

  // `[label](@ref id)` keeps the author's own link text. The target may be a
  // file reference such as `lumina.h`; without that alternative the file rule
  // below would fire inside the link and nest one link in another.
  text = text.replace(
    /\[([^\]]+)\]\(\s*@ref\s+([A-Za-z0-9_]+(?:\.h)?)\s*\)/g,
    (_m, label, id) => {
      const target = ctx.resolve(id);
      if (target) return `[${label}](${target.url})`;
      ctx.onUnresolved?.(id);
      return label;
    },
  );
  // `@ref lumina.h` is a Doxygen file reference, not an identifier.
  text = text.replace(/@ref\s+([A-Za-z0-9_]+\.h)\b/g, (_m, file) => {
    const target = ctx.resolve(file);
    if (target) return `[${target.label}](${target.url})`;
    ctx.onUnresolved?.(file);
    return stash(`\`${file}\``);
  });

  // A bare `@ref id` shows the target's own title, the way Doxygen does;
  // the raw id would read as "see context_sharing".
  text = text.replace(/@ref\s+([A-Za-z0-9_]+)/g, (_m, id) => {
    const target = ctx.resolve(id);
    if (target) return `[${target.label}](${target.url})`;
    ctx.onUnresolved?.(id);
    return stash(`\`${id}\``);
  });

  // Doxyfile aliases for platform-specific notes.
  text = text.replace(/@(win32|macos|x11|wayland|linux|lumina|glfw)\b:?/g, (_m, key) =>
    stash(`**${PLATFORM_LABELS[key]}:**`),
  );

  // `@par Title` opens a titled paragraph; bare `@par` is just a break.
  text = text.replace(/^[ \t]*@par[ \t]+(.+)$/gm, (_m, title) =>
    stash(`\n**${title.trim().replace(/:$/, '')}**\n`),
  );
  text = text.replace(/^[ \t]*@par[ \t]*$/gm, '');

  // Small inline commands.
  text = text.replace(/@b\s+(\S+)/g, (_m, word) => stash(`**${word}**`));
  text = text.replace(/@[ce]\s+(\S+)/g, (_m, word) => stash(`\`${word}\``));
  text = text.replace(/@n\b/g, '  \n');
  text = text.replace(/@(anchor|tableofcontents)\b[^\n]*/g, '');

  // Any `<` left over is literal text, not a tag.
  text = text.replace(/</g, '&lt;');

  // Doxygen escapes that Markdown does not need.
  text = text.replace(/@@/g, '@');

  for (let i = vault.length - 1; i >= 0; i -= 1) {
    text = text.split(`${PLACEHOLDER}${i}${PLACEHOLDER}`).join(vault[i]);
  }

  return text
    .split('\n')
    .map((line) => line.replace(/[ \t]+$/, (ws) => (ws.includes('  ') ? '  ' : '')))
    .join('\n')
    .replace(/\n{3,}/g, '\n\n')
    .trim();
}

const PARAGRAPH_COMMANDS = {
  note: { type: 'note', title: 'Note' },
  warning: { type: 'caution', title: 'Warning' },
  attention: { type: 'caution', title: 'Attention' },
};

/** Turns paragraph-scoped Doxygen commands into Starlight asides. */
function convertParagraphCommands(text) {
  const lines = text.split('\n');
  const out = [];

  for (let i = 0; i < lines.length; i += 1) {
    const match = lines[i].match(/^[ \t]*@(note|warning|attention)\b[ \t]*(.*)$/);
    if (!match) {
      out.push(lines[i]);
      continue;
    }

    const body = [match[2]];
    while (i + 1 < lines.length && lines[i + 1].trim() !== '') {
      i += 1;
      body.push(lines[i]);
    }

    const { type, title } = PARAGRAPH_COMMANDS[match[1]];
    // Emitted as plain text so the body still goes through the rest of the
    // pipeline and its own @ref links get resolved.
    out.push('', `:::${type}[${title}]`, body.join('\n').trim(), ':::', '');
  }

  return out.join('\n');
}

const snippetCache = new Map();

function readSnippet(root, file, id, ctx) {
  if (!root) return null;
  const key = `${file}:${id}`;
  if (snippetCache.has(key)) return snippetCache.get(key);

  let source;
  try {
    source = readFileSync(`${root}/${file}`, 'utf8');
  } catch {
    ctx.onNote?.(`snippet source not found: ${file}`);
    snippetCache.set(key, null);
    return null;
  }

  const marker = `//! [${id}]`;
  const start = source.indexOf(marker);
  const end = source.indexOf(marker, start + marker.length);
  if (start === -1 || end === -1) {
    ctx.onNote?.(`snippet markers not found: ${file} [${id}]`);
    snippetCache.set(key, null);
    return null;
  }

  const code = source
    .slice(start + marker.length, end)
    .replace(/^\n+/, '')
    .replace(/\s+$/, '');
  snippetCache.set(key, code);
  return code;
}

/** Wraps text in a Starlight aside directive. */
export function aside(type, title, body) {
  if (!body?.trim()) return '';
  const head = title ? `:::${type}[${title}]` : `:::${type}`;
  return `${head}\n${body.trim()}\n:::`;
}
