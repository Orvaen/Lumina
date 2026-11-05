// Parses include/Lumina/lumina.h into a model of the public API: groups,
// functions, types and constants, each with its documentation comment.

import { readFileSync } from 'node:fs';
import { parseDocComment, stripCommentMarkers } from './doxygen.mjs';

const MACRO_NAME = /^LUM_[A-Z0-9_]+$/;

// Internal guards that record whether the header had to define a symbol for
// the platform's OpenGL headers. Not part of the API.
const MACRO_EXCLUDE = /_DEFINED$/;

export function parseHeader(path) {
  const source = readFileSync(path, 'utf8').replace(/\r\n/g, '\n');
  const lines = source.split('\n');

  const groups = new Map();
  const functions = [];
  const types = [];
  const macros = [];

  /** Group ids pushed by `@defgroup`/`@addtogroup` ... `@{` ... `@}`. */
  const groupStack = [];
  const currentGroup = () => {
    for (let i = groupStack.length - 1; i >= 0; i -= 1) {
      if (groupStack[i]) return groupStack[i];
    }
    return null;
  };

  // Tracks `#if defined(LUM_EXPOSE_NATIVE_*)` so each native function can say
  // which macro the user has to define to get it.
  const conditionStack = [];
  const currentExpose = () => {
    for (let i = conditionStack.length - 1; i >= 0; i -= 1) {
      if (conditionStack[i]) return conditionStack[i];
    }
    return null;
  };

  let pending = null;
  let i = 0;

  while (i < lines.length) {
    const line = lines[i];

    const conditional = line.match(/^\s*#\s*(if|ifdef|ifndef|endif)\b(.*)$/);
    if (conditional) {
      if (conditional[1] === 'endif') {
        conditionStack.pop();
      } else {
        const expose = conditional[2].match(/\b(LUM_EXPOSE_NATIVE_[A-Z0-9_]+)\b/);
        conditionStack.push(expose?.[1] ?? null);
      }
      pending = null;
      i += 1;
      continue;
    }

    // ---- documentation comment -------------------------------------
    if (/^\s*\/\*[!*]/.test(line)) {
      const start = i;
      while (i < lines.length && !/\*\//.test(lines[i])) i += 1;
      const raw = lines.slice(start, i + 1).join('\n');
      i += 1;

      const opens = /@\{/.test(raw);
      const closes = /@\}/.test(raw);
      const doc = parseDocComment(stripCommentMarkers(raw));

      if (closes && !opens) {
        groupStack.pop();
        continue;
      }

      if (doc.defgroup) {
        const { id, title } = doc.defgroup;
        if (!groups.has(id)) {
          groups.set(id, {
            id,
            title,
            brief: doc.brief,
            description: doc.description,
            parent: doc.groups[0] ?? null,
          });
        } else if (title && !groups.get(id).title) {
          groups.get(id).title = title;
        }
        if (opens) groupStack.push(id);
        continue;
      }

      if (doc.name !== null) {
        // `@name` is a presentational grouping; it does not introduce a ref id.
        if (opens) groupStack.push(null);
        continue;
      }

      if (opens) {
        groupStack.push(null);
        continue;
      }

      pending = doc;
      continue;
    }

    // ---- blank line: a documentation comment survives it -----------
    if (line.trim() === '') {
      i += 1;
      continue;
    }

    // ---- #define ---------------------------------------------------
    const define = line.match(/^\s*#define\s+([A-Za-z_][A-Za-z0-9_]*)\s*(.*)$/);
    if (define && MACRO_NAME.test(define[1]) && !MACRO_EXCLUDE.test(define[1])) {
      macros.push({
        name: define[1],
        value: define[2].trim(),
        doc: pending,
        group: pending?.groups[0] ?? currentGroup(),
      });
      pending = null;
      i += 1;
      continue;
    }

    // ---- typedef ---------------------------------------------------
    if (/^\s*typedef\b/.test(line)) {
      const start = i;
      let depth = 0;
      let text = '';
      do {
        text += `${lines[i]}\n`;
        depth += (lines[i].match(/\{/g) ?? []).length;
        depth -= (lines[i].match(/\}/g) ?? []).length;
        i += 1;
      } while (i < lines.length && (depth > 0 || !/;\s*$/.test(lines[i - 1])));

      const definition = text.replace(/\s+$/, '');
      const classified = classifyType(definition);
      // Skip compatibility shims such as `typedef void* id;` in the native
      // header; only Lumina's own types are public API.
      if (classified.name.startsWith('LUM')) {
        types.push({
          ...classified,
          definition,
          members: extractMembers(lines.slice(start, i)),
          doc: pending,
          group: pending?.groups[0] ?? currentGroup(),
        });
      }
      pending = null;
      continue;
    }

    // ---- function --------------------------------------------------
    if (/^\s*LUMAPI\b/.test(line)) {
      let text = line;
      while (!/;\s*$/.test(text) && i + 1 < lines.length) {
        i += 1;
        text += ` ${lines[i].trim()}`;
      }
      i += 1;

      const fn = parseFunction(text);
      if (fn) {
        functions.push({
          ...fn,
          doc: pending,
          group: pending?.groups[0] ?? currentGroup(),
          expose: currentExpose(),
        });
      }
      pending = null;
      continue;
    }

    // Anything else (preprocessor conditionals, includes, extern "C") ends
    // the reach of a documentation comment.
    pending = null;
    i += 1;
  }

  return { groups, functions, types, macros };
}

function parseFunction(declaration) {
  const text = declaration
    .replace(/^\s*LUMAPI\s+/, '')
    .replace(/;\s*$/, '')
    .replace(/\s+/g, ' ')
    .trim();

  const open = text.indexOf('(');
  if (open === -1) return null;
  const close = text.lastIndexOf(')');

  const head = text.slice(0, open).trim();
  const paramList = text.slice(open + 1, close).trim();

  const nameMatch = head.match(/([A-Za-z_][A-Za-z0-9_]*)$/);
  if (!nameMatch) return null;
  const name = nameMatch[1];
  const returnType = head.slice(0, head.length - name.length).trim();

  const params =
    paramList === '' || paramList === 'void'
      ? []
      : splitTopLevel(paramList).map((p) => p.trim());

  return { name, returnType, params, signature: `${returnType} ${name}(${paramList})` };
}

/** Splits a parameter list on commas that are not inside brackets. */
function splitTopLevel(text) {
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
  return parts;
}

function classifyType(definition) {
  const flat = definition.replace(/\s+/g, ' ');

  // typedef enum LUMtoggle { ... } LUMtoggle;  -> enumeration
  if (/^typedef\s+enum\b/.test(flat)) {
    const name = flat.match(/\}\s*(\w+);$/);
    return { name: name?.[1] ?? 'unknown', kind: 'enum' };
  }

  // typedef struct LUMwindow LUMwindow;  -> opaque handle
  let m = flat.match(/^typedef struct (\w+) (\w+);$/);
  if (m) return { name: m[2], kind: 'handle' };

  // typedef struct LUMvidmode { ... } LUMvidmode;  -> struct
  m = flat.match(/\}\s*(\w+);$/);
  if (m) return { name: m[1], kind: 'struct' };

  // typedef void (* LUMkeyfun)(...);  -> function pointer
  m = flat.match(/\(\s*\*\s*(\w+)\s*\)/);
  if (m) {
    const name = m[1];
    return { name, kind: /fun$/.test(name) ? 'callback' : 'proc' };
  }

  m = flat.match(/(\w+);$/);
  return { name: m?.[1] ?? 'unknown', kind: 'alias' };
}

/** Pulls `/*! doc *\/` + `type name;` pairs out of a struct body. */
function extractMembers(bodyLines) {
  const members = [];
  let comment = null;

  for (let i = 0; i < bodyLines.length; i += 1) {
    const line = bodyLines[i];
    if (/^\s*\/\*[!*]/.test(line)) {
      const start = i;
      while (i < bodyLines.length && !/\*\//.test(bodyLines[i])) i += 1;
      comment = stripCommentMarkers(bodyLines.slice(start, i + 1).join('\n'));
      continue;
    }
    // Struct fields end in a semicolon, enumerators in a comma or nothing.
    const member =
      line.match(/^\s{2,}(.+?)\s*;\s*$/) ??
      line.match(/^\s{2,}([A-Z][A-Z0-9_]*(?:\s*=\s*[^,]+)?)\s*,?\s*$/);
    if (member && !/^typedef|^\}/.test(member[1])) {
      const declaration = member[1].trim();
      const nameMatch = declaration.match(/([A-Za-z_][A-Za-z0-9_]*)\s*(\[[^\]]*\])?$/);
      members.push({
        declaration,
        name: nameMatch?.[1] ?? declaration,
        doc: comment ?? '',
      });
      comment = null;
    }
  }

  return members;
}
