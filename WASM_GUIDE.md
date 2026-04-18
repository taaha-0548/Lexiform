# Lexiform WebAssembly (WASM) Guide

This guide explains how Lexiform leverages WebAssembly to run its C++ compiler core in the browser and Node.js.

## 🏗️ The Compilation Pipeline

Lexiform's core is written in C++ (Lexer, Parser, Semantic Analyzer). To maintain 100% feature parity between the CLI and the web library, we compile this core to WebAssembly using **Emscripten**.

### 1. Build Process

The build process is managed by CMake and npm. Running `npm run build:wasm` at the root performs the following:

1.  **CMake Configuration:** Uses `emcmake` to configure the build for the Emscripten toolchain.
2.  **Compilation:** Compiles C++ source files into `lexiform.wasm` and generates the `lexiform.js` glue code.
3.  **Deployment:** Copies the generated artifacts to `packages/lexiform-js/dist/` for bundling with the NPM package.

### 2. C++ Bindings (`WasmBindings.cpp`)

We use `emscripten/bind.h` to export the `compileToSchema` function to JavaScript.

```cpp
// src/WasmBindings.cpp
EMSCRIPTEN_BINDINGS(Lexiform_module) {
    function("compileToSchema", &compileToSchema);
}
```

This function takes a raw string (the `.form` source) and returns a JSON string representing the compiled schema or an error object.

---

## ⚛️ The JS/TS Library (`lexiform`)

The `lexiform` package wraps the low-level WASM module in a high-level TypeScript API.

### How it Works

1.  **Initialization:** When `LexiformEngine.initWasm()` is called, it dynamically imports the Emscripten-generated `lexiform.js`.
2.  **Instantiation:** The `lexiform.js` module fetches and instantiates the `lexiform.wasm` binary.
3.  **Execution:** When `LexiformEngine.parse(source)` is called:
    - The source string is passed into the WASM memory space.
    - The C++ `compileToSchema` function is executed.
    - The resulting JSON string is passed back to JavaScript and parsed into a POJO (Plain Old JavaScript Object).

---

## 🛠️ Development & Debugging

### Building WASM locally
If you have the Emscripten SDK installed and activated:
```bash
npm run build:wasm
```

### Manual WASM Loading
If you need to control how WASM is loaded (e.g., in specialized build environments or CDNs), you can pass a custom `moduleLoader` to `initWasm`:

```typescript
import { LexiformEngine } from 'lexiform';
import lexiformModuleFactory from 'lexiform/dist/lexiform.js';

await LexiformEngine.initWasm(async () => {
  return await lexiformModuleFactory({
    locateFile: (path) => `https://my-cdn.com/wasm/${path}`
  });
});
```

### Key Files
- `src/WasmBindings.cpp`: The bridge between C++ and JS.
- `packages/lexiform-js/src/core/engine.ts`: The high-level JS wrapper.
- `packages/lexiform-js/dist/lexiform.js`: Generated Emscripten glue code.
- `packages/lexiform-js/dist/lexiform.wasm`: Compiled C++ core.
