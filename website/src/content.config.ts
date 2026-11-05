import { defineCollection } from 'astro:content';
import { docsLoader } from '@astrojs/starlight/loaders';
import { docsSchema } from '@astrojs/starlight/schema';

// Starlight notes that the optional "i18n" collection is absent. The site is
// English-only, so that is expected; declaring an empty collection produces
// more warnings than it removes.
export const collections = {
  docs: defineCollection({ loader: docsLoader(), schema: docsSchema() }),
};
