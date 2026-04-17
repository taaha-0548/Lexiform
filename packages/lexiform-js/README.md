# Lexiform JS Core

The official JavaScript/TypeScript bindings for the **Lexiform C++ Core Compiler**. This library allows you to run the exact same, high-performance form validation and schema generation logic in the browser using WebAssembly.

## 🚀 Features

- **C++ Performance:** Uses the real C++ Lexer, Parser, and Semantic Analyzer.
- **WASM Powered:** Compiled to WebAssembly for near-native speed.
- **Type Safe:** Full TypeScript support for Schemas and Data.
- **React Ready:** Includes a specialized hook for seamless integration.

## 📦 Installation

```bash
npm install lexiform
```

## 🛠️ Usage

### Using the React Hook

The easiest way to use Lexiform in a React application is the `useLexiform` hook. It handles the asynchronous loading of the WebAssembly module automatically.

```tsx
import { useLexiform } from 'lexiform';

const source = `
  FORM "Newsletter" news-id
  SECTION "Subscriber Details"
    EMAIL "Your Email" email [REQUIRED]
  SUBMIT "Join"
  END
`;

function App() {
  const { schema, isReady, compilerError } = useLexiform(source);

  if (!isReady) return <div>Loading C++ Engine...</div>;
  if (compilerError) return <div>Error: {compilerError}</div>;

  return (
    <form>
      <h1>{schema.title}</h1>
      {/* Render your form using the schema object */}
    </form>
  );
}
```

### Direct Engine Usage

For non-React environments, you can use the `LexiformEngine` directly.

```typescript
import { LexiformEngine } from 'lexiform';

async function compileForm() {
  // 1. Initialize the WASM module
  await LexiformEngine.initWasm();

  // 2. Parse the source string into a JSON Schema
  const schema = LexiformEngine.parse(mySourceString);
  
  // 3. Validate user data
  const errors = LexiformEngine.validate(schema, { email: "invalid-email" });
}
```

## 🏗️ Architecture

This package bundles:
1.  **TypeScript Distribution** (`dist/`): The high-level JS API.
2.  **WebAssembly Binaries** (`src/wasm/`): The compiled C++ core.

When `initWasm()` is called, the library dynamically imports the `.wasm` binary and instantiates the C++ environment.

## 📄 License

MIT
