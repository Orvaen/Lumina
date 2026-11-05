import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';

import { buildSidebar } from './scripts/lib/structure.mjs';

export default defineConfig({
  // Set `site` once Lumina has a public URL; it enables canonical links and
  // the sitemap. Everything else, search included, works without it.
  // site: 'https://lumina.example',

  integrations: [
    starlight({
      title: 'Lumina',
      description:
        'Lumina is a multi-platform library for OpenGL, OpenGL ES and Vulkan ' +
        'application development: windows, contexts, surfaces, input and events.',
      logo: {
        src: './src/assets/lumina.svg',
        alt: 'Lumina',
      },
      favicon: '/favicon.svg',
      // English stays at the root so existing links keep working; French is
      // served under /fr. Starlight ships its own French UI strings, and
      // falls back to the English page for anything not translated yet.
      defaultLocale: 'root',
      locales: {
        root: { label: 'English', lang: 'en' },
        fr: { label: 'Français', lang: 'fr' },
      },
      // Self-hosted through Fontsource, so there is no request to a font CDN
      // and no flash of unstyled text. Each file is subset with unicode-range,
      // so a browser only fetches the ranges a page actually uses.
      customCss: [
        '@fontsource-variable/instrument-sans/index.css',
        '@fontsource-variable/instrument-sans/wght-italic.css',
        '@fontsource-variable/space-grotesk/index.css',
        '@fontsource-variable/jetbrains-mono/index.css',
        './src/styles/custom.css',
      ],
      sidebar: buildSidebar(),
      // The page frame override appends the site footer below the article.
      components: {
        PageFrame: './src/components/PageFrame.astro',
      },
      // Add the repository here to turn on "Edit this page" links:
      // editLink: { baseUrl: 'https://github.com/<owner>/lumina/edit/master/' },
      // social: [{ icon: 'github', label: 'GitHub', href: 'https://github.com/<owner>/lumina' }],
      tableOfContents: { minHeadingLevel: 2, maxHeadingLevel: 3 },
      expressiveCode: {
        themes: ['github-dark-default', 'github-light'],
        styleOverrides: {
          borderRadius: '0.5rem',
          codeFontSize: '0.875rem',
          // Expressive Code keeps its own selection colour. Point it at the
          // same violet as the rest of the site; translucent, so the syntax
          // highlighting still shows through.
          codeSelectionBackground: 'rgba(76, 29, 149, 0.7)',
          uiSelectionBackground: 'rgba(76, 29, 149, 0.7)',
        },
      },
      lastUpdated: false,
      credits: false,
    }),
  ],
});
