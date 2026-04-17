import { FormSchema, FormDataValue } from './types';

// Declare the Emscripten module interface
interface LexiformWasmModule {
  compileToSchema: (source: string) => string;
}

let wasmModuleInstance: LexiformWasmModule | null = null;

export class LexiformEngine {
  /**
   * Initialize the WebAssembly module built from the C++ Core Compiler.
   * This MUST be called before using parse().
   */
  static async initWasm(moduleLoader?: () => Promise<any>): Promise<void> {
    if (wasmModuleInstance) return;
    
    try {
      if (moduleLoader) {
        wasmModuleInstance = await moduleLoader();
      } else {
        // Dynamic import for the pre-built WASM glue code
        // We use 'as any' to bypass TS check for missing file during dev
        const moduleFactory = (await import('./lexiform.js' as any)).default;
        wasmModuleInstance = await moduleFactory();
      }
      console.log("Lexiform C++ Engine (WASM) initialized successfully.");
    } catch (e) {
      console.error("CRITICAL: Failed to load Lexiform C++ WebAssembly module.", e);
      console.error("Ensure that 'lexiform.wasm' is available in the same directory as the JS bundle.");
      throw new Error("Lexiform WASM Initialization Failed. The C++ core could not be loaded.");
    }
  }

  /**
   * Passes the raw Lexiform string to the C++ Compiler in WebAssembly memory.
   * The C++ Core handles Lexical Analysis, Parsing, and Semantic Analysis.
   */
  static parse(source: string): FormSchema {
    if (!wasmModuleInstance) {
      throw new Error("LexiformEngine not initialized. Call initWasm() first.");
    }

    // Call the C++ function exported via Embind
    const jsonString = wasmModuleInstance.compileToSchema(source);
    const parsed = JSON.parse(jsonString);

    if (parsed.error) {
      throw new Error(`Compiler Error: ${parsed.error}`);
    }

    return parsed as FormSchema;
  }

  /**
   * Runtime Validation (Runs entirely in JS for speed during typing)
   */
  static validate(schema: FormSchema, data: Record<string, FormDataValue>): Record<string, string> {
    const errors: Record<string, string> = {};
    schema.sections.forEach(section => {
      section.fields.forEach(field => {
        const value = data[field.id];
        if (field.REQUIRED && (!value || (Array.isArray(value) && value.length === 0))) {
          errors[field.id] = `${field.label} is required.`;
        }
        if (field.type === 'textarea' && field.MAX_WORDS && typeof value === 'string') {
          const wordCount = value.trim().split(/\s+/).length;
          if (wordCount > field.MAX_WORDS) errors[field.id] = `Max ${field.MAX_WORDS} words.`;
        }
      });
    });
    return errors;
  }
}
