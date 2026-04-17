# 📝 Lexiform

**A developer-first Domain Specific Language (DSL) for building and validating cross-platform forms.**

Lexiform allows you to define complex form structures, validation rules, and UI hierarchies in a simple, human-readable format. Our compiler then translates these `.form` scripts into framework-agnostic JSON schemas that power your frontends.

---

## 🚀 Key Features

*   **Custom DSL:** Define forms with a simple, keyword-based syntax.
*   **High Performance:** C++ compiler for rapid translation and semantic analysis.
*   **Shared Engine:** A cross-platform TypeScript library (`LexiformEngine`) for real-time frontend validation.
*   **Framework Agnostic:** Compiles to standard JSON that works with React, Vue, Angular, and more.
*   **Semantic Validation:** Catches duplicate IDs and attribute mismatches at compile time.

---

## 🛠️ Monorepo Structure

-   `src/`: Core C++ Compiler logic (Lexer, Parser, Semantic Analyzer).
-   `packages/Lexiform-js/`: The NPM package providing JS/TS bindings for the C++ core via WebAssembly.
-   `frontend/`: A React demo application (YouTube Studio clone) using the library.
-   `include/`: C++ Header files.
-   `tests/`: Test suites for both C++ and JS.

## 🏗️ Build & Development

We use a unified build system. Ensure you have a C++ compiler and [Emscripten](https://emscripten.org/) installed.

### 1. Build Everything
From the root directory:
```bash
npm run build
```
This will compile the C++ core to WASM, build the JS library, and bundle the frontend.

### 2. Specific Build Tasks
-   **Build C++ CLI:** `npm run build:core`
-   **Build WASM Bridge:** `npm run build:wasm`
-   **Build JS Library:** `npm run build:js`
-   **Run Frontend Dev:** `npm run dev:frontend`

---

## ⚛️ Frontend Integration (The NPM Package)

You can now use the high-performance C++ compiler directly in your frontend apps via the `lexiform` package.

### 1. Installation
```bash
npm install lexiform
```

### 2. Usage with React
```tsx
import { useLexiform } from 'lexiform';
```

---

## 📦 NPM Registry
View the official package here: [https://www.npmjs.com/package/lexiform](https://www.npmjs.com/package/lexiform)

const source = `
  FORM "Signup" signup-id
  TEXT "Name" user [REQUIRED]
  SUBMIT "Go"
  END
`;

function App() {
  const { schema, isReady } = useLexiform(source);
  if (!isReady) return <div>Loading C++ Engine...</div>;
  return <h1>{schema.title}</h1>;
}
```

---

## 📦 NPM Publishing

To publish the library to the NPM registry:

1.  Rebuild the assets: `npm run build:wasm && npm run build:js`
2.  Navigate to the package: `cd packages/Lexiform-js`
3.  Publish: `npm publish`

---

## 📜 DSL Syntax Guide

| Keyword | Description | Example |
| :--- | :--- | :--- |
| `FORM` | Starts a form definition. | `FORM "Title" id` |
| `SECTION` | Group fields together. | `SECTION "Details"` |
| `TEXT` | Standard text input. | `TEXT "Label" id` |
| `TEXTAREA`| Multi-line input. | `TEXTAREA "Bio" id` |
| `DROPDOWN`| Selection menu. | `DROPDOWN "Tag" id [OPTIONS=["A", "B"]]` |
| `REQUIRED`| Marks a field as mandatory. | `[REQUIRED]` |
| `MAX_WORDS`| Limit for textareas. | `[MAX_WORDS=500]` |
| `SUBMIT` | Define the submit button. | `SUBMIT "Text"` |
| `END` | Marks the end of the form. | `END` |

---

## 🤝 Contributing
We welcome contributions! Please feel free to submit a Pull Request.

## 📄 License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
