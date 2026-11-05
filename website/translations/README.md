# Guide translations

One directory per locale, mirroring the file names in `../../docs`. A file
here is a translation of the `.dox` guide of the same name.

```
translations/fr/quick.dox   <- translates docs/quick.dox
```

## Why `.dox` and not Markdown

Because the translation then goes through exactly the same pipeline as the
English source. Keeping the Doxygen markup means:

- `@ref` cross-references resolve themselves, so links never rot
- `@section` / `@subsection` / `@anchor` ids stay identical, so every deep
  link and every reference from the C headers lands on the right spot in the
  translated page
- code blocks, tables and asides come out the same on both sides

Translating generated Markdown instead would lose all of that on the next
`npm run generate`.

## Rules

1. **Keep every id exactly as in the English file.** Translate the title that
   follows it, never the id.

   ```
   @section quick_steps Step by step      ->  @section quick_steps Pas à pas
   @anchor LUM_RED_BITS                   ->  @anchor LUM_RED_BITS
   ```

2. **Keep `@ref` targets as they are.** Only translate the link text.

   ```
   [window hints](@ref window_hints)  ->  [indices de fenêtre](@ref window_hints)
   ```

3. **Do not translate code.** Identifiers, macro names, header paths and the
   contents of `@code` blocks stay as they are; a compiler reads them.
   Comments inside a code block may be translated.

4. **Technical vocabulary**: give the English term in brackets the first time
   it appears on a page, then use the French term alone.

   > Le tampon arrière (back buffer) est échangé avec le tampon avant.

5. A missing file is fine. The page is then served in English for that locale,
   with the translated interface around it.

## Checking a translation

```sh
npm run build        # reports guides translated per locale
npm run check:links  # every internal link, fragments included
```

The generator reports unresolved `@ref` targets per locale, so a mistyped id
in a translation shows up immediately.
