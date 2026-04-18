# Lexiform
**The High-Performance Form Markup Language & Compiler**

Lexiform is a unified, developer-first platform for defining, validating, and rendering web forms. It combines a human-readable Domain-Specific Language (DSL) with a high-performance C++ compiler, making form definitions version-controllable, semantically sound, and cross-platform.

## Project Overview

Lexiform is structured as a monorepo containing the following components:

- **C++ Core Compiler:** The heart of Lexiform. It performs lexical analysis, parsing, and semantic validation.
- **WebAssembly Bridge:** A binding layer that compiles the C++ core into WASM for use in browsers and Node.js.
- **`lexiform` (NPM Package):** The official JavaScript/TypeScript library that provides a high-level API and React hooks for the WASM engine.
- **CLI Tool:** A command-line interface for compiling `.form` files to JSON directly from the terminal.

## Why Lexiform?

- **Human-Readable DSL:** Replace massive, brittle JSON schemas with clean, intuitive markup.
- **Semantic Safety:** The C++ compiler catches structural errors, duplicate IDs, and invalid attribute combinations before your form even renders.
- **Unified Logic:** Use the exact same compiler engine across your CLI, backend, and frontend.
- **Performance:** Complex parsing and validation are offloaded to WebAssembly for near-native speed.
- **Git Friendly:** `.form` files are plain text, making them easy to version, diff, and review.

## Repository Structure

```text
Lexiform/
├── src/                # C++ Core Source (Lexer, Parser, Analyzer)
├── include/            # C++ Headers
├── packages/
│   └── lexiform-js/    # Official JS/TS Library (WASM-powered)
├── tests/              # Cross-platform test suites
├── build/              # Build artifacts (CMake)
└── emsdk/              # Emscripten SDK for WASM compilation
```

## Getting Started

### For Users (Web Development)

If you just want to use Lexiform in your React or TypeScript project, install the NPM package:

```bash
npm install lexiform
```

See the [lexiform-js README](./packages/lexiform-js/README.md) for detailed usage instructions and React examples.

### For Contributors (Building from Source)

To build the entire project, including the C++ core and WebAssembly modules:

1. **Prerequisites:**
   - CMake (3.10+)
   - C++17 Compiler
   - Node.js (18+) & npm
   - Emscripten SDK (included in `emsdk/`)

2. **Full Build:**
   ```bash
   npm run build
   ```

3. **Running CLI Tests:**
   ```bash
   node tests/test_runner.js
   ```

## License

MIT - See [LICENSE](./LICENSE) for details.
