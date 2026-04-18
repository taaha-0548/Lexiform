# lexiform (JS/TS Library)
**Official JavaScript and TypeScript bindings for the Lexiform C++ Core Compiler.**

This package allows you to run the Lexiform compiler directly in the browser or Node.js using WebAssembly. It provides a clean, type-safe API and a specialized React hook for seamless integration.

## Features

- **C++ Performance:** Powered by the native Lexiform C++ Lexer, Parser, and Semantic Analyzer.
- **WebAssembly Powered:** High-speed form compilation with 100% feature parity with the CLI tool.
- **React Ready:** Includes the `useLexiform` hook for managing compiler state and form data.
- **Zero Framework Lock-in:** Compiles to standard JSON schemas that can be rendered by any framework.

## Installation

```bash
npm install lexiform
```

## Usage

### ⚛️ Using in React

The `useLexiform` hook is the recommended way to use Lexiform in React. 

#### Option A: Inline Strings
```tsx
const source = `
  FORM "Newsletter" news-id
  SECTION "Subscriber Details"
    EMAIL "Your Email" email [REQUIRED]
  SUBMIT "Join"
  END
`;
const { schema } = useLexiform(source);
```

#### Option B: Actual `.form` Files (Recommended)
For professional projects, keep your markup in separate `.form` files. 

**1. TypeScript Setup**
Add Lexiform's client types to your `tsconfig.json` so your IDE recognizes `.form` files:
```json
{
  "compilerOptions": {
    "types": ["lexiform/client"]
  }
}
```

**2. Vite Setup**
To ensure WebAssembly loads correctly and to treat `.form` files as raw text, update your `vite.config.ts`:

```typescript
// vite.config.ts
export default defineConfig({
  plugins: [react()],
  optimizeDeps: {
    exclude: ['lexiform'] // 🔴 CRITICAL: Prevents Vite from mangling WASM paths
  },
  assetsInclude: ['**/*.form'] // Allows importing .form files
});
```

Now you can import them cleanly:
```tsx
import contactSource from './contact.form'; // No ?raw needed!
```

### React Implementation Example
```tsx
import { useLexiform } from 'lexiform';
import source from './my-form.form';

function App() {
  const { 
    schema, 
    isReady, 
    compilerError, 
    data, 
    handleChange, 
    validate, 
    errors 
  } = useLexiform(source);

  if (!isReady) return <div>Loading C++ Engine...</div>;
  if (compilerError) return <div>Compiler Error: {compilerError}</div>;

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    if (validate()) {
      console.log("Form Submitted:", data);
    }
  };

  return (
    <form onSubmit={handleSubmit}>
      <h1>{schema?.title}</h1>
      {schema?.sections.map((section) => (
         <fieldset key={section.title}>
            <legend>{section.title}</legend>
            {section.fields.map((field) => (
               <div key={field.id}>
                 <label>{field.label}</label>
                 <input
                   type={field.type}
                   value={(data[field.id] as string) || ''}
                   onChange={(e) => handleChange(field.id, e.target.value)}
                   required={Boolean(field.REQUIRED)}
                 />
                 {errors[field.id] && <span style={{color: 'red'}}>{errors[field.id]}</span>}
               </div>
            ))}
         </fieldset>
      ))}
      <button type="submit">{schema?.submit.label}</button>
    </form>
  );
}
```

### 🚀 Direct Engine Usage (Node.js/Vanilla JS)

For non-React environments, use the `LexiformEngine` directly.

```typescript
import { LexiformEngine } from 'lexiform';

async function runCompiler() {
  // 1. Initialize the WASM module
  await LexiformEngine.initWasm();

  // 2. Compile source markup to JSON Schema
  try {
    const schema = LexiformEngine.parse('FORM "Example" ex ...');
    console.log("Generated Schema:", schema);
    
    // 3. Validate data at runtime
    const errors = LexiformEngine.validate(schema, { some_field: "" });
  } catch (err) {
    console.error(err.message);
  }
}
```

## API Reference

### `useLexiform(source: string, moduleLoader?: () => Promise<any>)`
Returns:
- `schema`: The compiled `FormSchema` object.
- `isReady`: Boolean indicating if WASM is loaded and parsing is complete.
- `compilerError`: String containing any errors caught by the C++ analyzer.
- `data`: Current form state object.
- `errors`: Current validation errors.
- `handleChange(id, value)`: Helper to update `data` state.
- `validate()`: Runs JS-side validation against the schema.
- `isValid`: Boolean indicating if there are currently validation errors.

### `LexiformEngine.initWasm(moduleLoader?)`
Initializes the WASM environment. Must be called before `parse()`.

### `LexiformEngine.parse(source: string)`
Calls the C++ core to tokenize, parse, and analyze the source. Returns a `FormSchema`.

### `LexiformEngine.validate(schema, data)`
Runs a fast JS-side validation of the provided data against the schema constraints.

## License

MIT
