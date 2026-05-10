# Lexiform
**The High-Performance Form Markup Language & Compiler**

Lexiform is a unified, developer-first platform for defining, validating, and rendering web forms. It combines a human-readable Domain-Specific Language (DSL) with a high-performance C++ compiler, making form definitions version-controllable, semantically sound, and cross-platform.

## Project Overview

Lexiform is structured as a monorepo containing the following components:

- **C++ Core Compiler:** The heart of Lexiform. It performs lexical analysis, parsing, semantic validation, IR generation, optimization, and code generation (6 complete compiler phases).
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
├── src/                # C++ Core Source (Lexer, Parser, Analyzer, IR, Optimizer)
├── include/            # C++ Headers
├── packages/
│   └── lexiform-js/    # Official JS/TS Library (WASM-powered)
├── frontend/           # React frontend demo application
├── tests/              # Cross-platform test suites
├── docs/               # Language Reference, Team Reflection
├── build/              # Build artifacts (CMake)
└── emsdk/              # Emscripten SDK for WASM compilation
```

## Compiler Phases

The Lexiform compiler implements a complete 6-phase compilation pipeline:

| Phase | Description | File |
|-------|-------------|------|
| 1. Lexical Analysis | Tokenization via handwritten finite automaton | `Lexer.cpp` |
| 2. Syntax Analysis | Recursive descent parser → AST | `Parser.cpp` |
| 3. Semantic Analysis | Symbol table, type checking, scope validation | `SemanticAnalyzer.cpp` |
| 4. IR Generation | AST → Three-Address Code (TAC) | `IRGenerator.cpp` |
| 5. Optimization | Dead attribute elimination + duplicate folding | `Optimizer.cpp` |
| 6. Code Generation | Optimized IR → JSON schema | `main.cpp` |

## Getting Started

### For Users (Web Development)

If you just want to use Lexiform in your React or TypeScript project, install the NPM package:

```bash
npm install lexiform
```

See the [lexiform-js README](./packages/lexiform-js/README.md) for detailed usage instructions and React examples.

### CLI Usage

```bash
# Basic compilation
lexiform input.form -o output.json

# Debug mode — shows all intermediate representations
lexiform input.form --debug

# Interactive REPL mode
lexiform --interactive
```

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

## Documentation

- [Language Reference Manual](./docs/LANGUAGE_REFERENCE.md) — Complete DSL specification with BNF grammar
- [Compiler Architecture](./ARCHITECTURE.md) — Phase-by-phase implementation overview
- [Compiler Stages (Detailed)](./COMPILER_STAGES.md) — Step-by-step walkthrough with examples
- [Frontend Architecture](./FRONTEND_ARCHITECTURE.md) — React + WASM integration
- [WASM Guide](./WASM_GUIDE.md) — WebAssembly build and usage
- [Team Reflection](./docs/TEAM_REFLECTION.md) — Project retrospective

## License

MIT - See [LICENSE](./LICENSE) for details.
