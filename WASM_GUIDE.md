# Using FormScript C++ Core in the Browser (WebAssembly)

FormScript provides a high-performance C++ compiler for semantic analysis and code generation. To bring this exact same, rigorous C++ logic to your frontend frameworks (React, Vue, Svelte), we compile the C++ core into **WebAssembly (WASM)**.

This ensures that the JavaScript library `@formscript/core` is not just recreating the logic in TypeScript, but is actually running the **real C++ compiler** under the hood inside the user's browser.

---

## 🏗️ 1. Compiling the C++ Core to WebAssembly

We have automated this process. As long as you have [Emscripten](https://emscripten.org/) installed, you can just run:

```bash
npm run build:wasm
```

This will:
1.  Initialize the CMake build for WebAssembly.
2.  Compile the C++ source files using `emcc`.
3.  Generate `formscript.js` and `formscript.wasm`.
4.  Automatically move them to `packages/formscript-js/src/wasm/` for bundling.

---

## ⚛️ 2. How to use the NPM Library

Users can now simply install the library:

```bash
npm install @formscript/core
```

And use the provided React hook:

```tsx
import { useFormScript } from 'formscript';

const { schema, isReady } = useFormScript(source);
```

### **Under the Hood (Library Architecture)**

When you import the library in your frontend, this is what happens:
1. **Load:** The Wasm file is fetched and instantiated by the browser.
2. **Execute:** The TypeScript engine calls `Module.compileToSchema(rawFormString)`.
3. **C++ Processing:** 
   - **Lexer** tokenizes the string.
   - **Parser** builds the AST.
   - **Semantic Analyzer** strictly validates types, IDs, and constraints.
4. **Return:** C++ returns a JSON string, which is parsed back into a TypeScript object for React to render.

---

## 💻 3. Frontend Usage Guide (For Users)

As a frontend developer using React, you don't need to worry about the C++ memory management. The library handles it for you.

### **Installation**
```bash
npm install @formscript/core
```

### **React Example**
```tsx
import React from 'react';
import { useFormScript } from '@formscript/core';

const formSource = `
  FORM "Signup" sign-up
  TEXT "Username" user [REQUIRED]
  SUBMIT "Go"
  END
`;

export default function App() {
  // The hook automatically initializes the C++ Wasm compiler!
  const { schema, isReady, error } = useFormScript(formSource);

  if (!isReady) return <div>Loading C++ Compiler Engine...</div>;
  if (error) return <div className="error">{error}</div>;

  return (
    <form>
      <h1>{schema.title}</h1>
      {/* Map over schema.sections as usual */}
      <button type="submit">{schema.submit.label}</button>
    </form>
  );
}
```

By leveraging WebAssembly, FormScript guarantees **100% feature parity** and identical validation behavior between the CLI backend tool and the frontend browser rendering!
