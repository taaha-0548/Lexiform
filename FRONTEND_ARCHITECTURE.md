# Lexiform Frontend Architecture

This document describes how the Lexiform React frontend interacts with the WebAssembly-powered compiler core.

## Overview
The frontend is a Vite + React application that provides a live editor and preview environment for Lexiform DSL (`.form`) files. It utilizes the `@lexiform/js` package to bridge the gap between the browser's JavaScript environment and the C++ compiler.

---

## 1. Engine Initialization
Before any compilation can occur, the WebAssembly module must be loaded and instantiated.

*   **File:** `frontend/src/App.tsx`
*   **Logic:** Uses a `useEffect` hook to call `LexiformEngine.initWasm()`.
*   **State:** `isReady` tracks if the engine is loaded.

```typescript
useEffect(() => {
  async function init() {
    await LexiformEngine.initWasm();
    setIsReady(true);
  }
  init();
}, []);
```

---

## 2. Compilation Flow (DSL to JSON)
When the user clicks "Compile Form" or when the app first loads, the raw text in the editor is sent to the compiler.

*   **Function:** `parseAndSetSchema(dslSource: string)`
*   **Process:**
    1.  Calls `LexiformEngine.parse(dslSource)`.
    2.  This triggers the C++ `compileToSchema` function (via the WASM bridge).
    3.  If successful, it returns a JSON string which is parsed into a JavaScript object (`schema`).
    4.  If it fails, the C++ error message is caught and displayed in the `compileError` state.

---

## 3. Dynamic Rendering
The frontend does not use static forms. Instead, it "renders" the JSON schema returned by the compiler.

*   **Logic:** The `App` component maps over `schema.sections` and `section.fields`.
*   **Field Mapping:** A `switch` or `if/else` block matches the `type` field in the JSON (e.g., `"text"`, `"dropdown"`, `"checkbox"`) to the corresponding HTML element.
*   **Attribute Handling:** Attributes like `PLACEHOLDER` or `OPTIONS` are extracted from the JSON and applied directly to the HTML props.

```tsx
{field.type === 'text' && (
  <input type="text" placeholder={field.PLACEHOLDER} ... />
)}
```

---

## 4. Validation Layer
The frontend also uses the Lexiform engine to validate user input against the generated schema before "submitting."

*   **Function:** `submitMock(event)`
*   **Logic:** Calls `LexiformEngine.validate(schema, formData)`.
*   **Feedback:** If validation fails (e.g., a `REQUIRED` field is empty), the engine returns an errors object which is mapped to the `validationErrors` state and displayed under each field.

---

## Key Files Summary

| File | Responsibility |
| :--- | :--- |
| `src/main.tsx` | Entry point, mounts the React application. |
| `src/App.tsx` | Main application logic: state management, WASM orchestration, and dynamic UI rendering. |
| `src/App.css` | Styles the editor, preview grid, and form components. |
| `src/form-schema.json` | A static example of the target output format. |
