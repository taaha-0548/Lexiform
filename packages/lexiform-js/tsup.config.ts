import { defineConfig } from 'tsup';

export default defineConfig({
  entry: ['src/index.ts'],
  format: ['cjs', 'esm'],
  dts: true,
  clean: false, // Don't clean to keep WASM files if they were copied early
  minify: true,
  // Ensure the Emscripten glue code is not bundled
  external: [
    'react',
    './lexiform.js',
    '../wasm/lexiform.js'
  ],
  // This ensures that the dynamic import('./lexiform.js') remains in the output
  noExternal: [],
  splitting: false,
  sourcemap: true,
});
