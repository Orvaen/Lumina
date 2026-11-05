// Translation memory for the API reference.
//
// The reference is generated from the C headers, so it cannot be translated
// by editing its output. Instead each documentation field is looked up, by its
// exact English text, in a per-locale catalogue of [english, french] pairs.
//
// Keying on the text rather than on the symbol is what makes this cheap: the
// reference is highly formulaic, and 1529 field occurrences come down to 951
// distinct strings — `@thread_safety` alone is 150 occurrences of 7 sentences.
// It is also self-healing: if a header comment is reworded, its entry simply
// stops matching and the field falls back to English, and the generator
// reports it instead of shipping a stale translation.

/** Keys ignore how the source happens to be line-wrapped. */
export const normalise = (text) => text.replace(/\s+/g, ' ').trim();

/** @param {Array<[string, string]>} entries */
export function buildMemory(entries) {
  const map = new Map();
  for (const [english, french] of entries) {
    if (french) map.set(normalise(english), french);
  }
  return map;
}

/**
 * Wraps a memory with the bookkeeping the generator reports:
 * how many fields were translated, and which strings are still missing.
 */
export function createTranslator(memory) {
  const missing = new Map();
  let hits = 0;
  let total = 0;

  /** Translates one field, falling back to the original. */
  function translate(text) {
    if (!text || !text.trim()) return text;
    total += 1;
    if (!memory) return text;

    const key = normalise(text);
    const found = memory.get(key);
    if (found) {
      hits += 1;
      return found;
    }
    missing.set(key, (missing.get(key) ?? 0) + 1);
    return text;
  }

  /** Returns a copy of a parsed doc comment with every prose field translated. */
  function translateDoc(doc) {
    if (!doc) return doc;
    return {
      ...doc,
      brief: translate(doc.brief),
      description: translate(doc.description),
      returns: translate(doc.returns),
      errors: translate(doc.errors),
      threadSafety: translate(doc.threadSafety),
      pointerLifetime: translate(doc.pointerLifetime),
      reentrancy: translate(doc.reentrancy),
      analysis: translate(doc.analysis),
      callbackSignature: translate(doc.callbackSignature),
      since: translate(doc.since),
      params: doc.params?.map((p) => ({ ...p, text: translate(p.text) })),
      remarks: doc.remarks?.map(translate),
      notes: doc.notes?.map(translate),
      warnings: doc.warnings?.map(translate),
      deprecated: translate(doc.deprecated),
      // `see` entries are bare @ref links; they resolve on their own.
      see: doc.see,
    };
  }

  const stats = () => ({
    hits,
    total,
    missing,
    percent: total ? Math.round((hits / total) * 100) : 100,
  });

  return { translate, translateDoc, stats };
}
