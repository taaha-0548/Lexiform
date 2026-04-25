# Lexiform Compiler: Inner Workings & Stages

This document explains the lifecycle of a Lexiform (`.form`) file as it travels through the compiler to become a functional web form.

## Example Source Code (`contact.form`)
```lexiform
FORM "Contact Us" contact-form
SECTION "Identity"
    TEXT "Full Name" name [REQUIRED, PLACEHOLDER="Enter your name"]
    EMAIL "Email" email [REQUIRED]
SUBMIT "Send Message"
END
```

---

## Stage 1: Lexical Analysis (The Lexer)
**Goal:** Break the raw text into "tokens" (the atoms of the language).

*   **Files:** `src/Lexer.cpp`, `include/Lexer.hpp`
*   **Key Function:** `Lexer::tokenize()`
    *   Iteratively calls `readIdentifierOrKeyword()`, `readString()`, and `readNumber()` to categorize every character in the source file.

---

## Stage 2: Syntax Analysis (The Parser)
**Goal:** Organize tokens into a logical tree structure called an **Abstract Syntax Tree (AST)**.

*   **Files:** `src/Parser.cpp`, `include/Parser.hpp`, `include/AST.hpp`
*   **Key Function:** `Parser::parse()`
    *   Uses **Recursive Descent** (calling `parseForm()`, `parseSection()`, and `parseField()`) to build a hierarchy of `Node` objects representing the form's structure.

---

## Stage 3: Semantic Analysis (The Checker)
**Goal:** Verify that the "meaning" of the code is valid.

*   **Files:** `src/SemanticAnalyzer.cpp`, `include/SemanticAnalyzer.hpp`
*   **Key Function:** `SemanticAnalyzer::analyze(const FormNode& form)`
    *   Traverses the AST and calls `validateField()` to enforce rules like ID uniqueness and attribute compatibility (e.g., ensuring `OPTIONS` is only used on selection fields).

---

## Stage 4: Target Generation (The Backend)
**Goal:** Transform the validated AST into a usable format (JSON).

*   **Files:** `src/main.cpp`
*   **Key Function:** `generateJson(const FormNode& form)`
    *   Uses the `nlohmann/json` library to map AST nodes to JSON objects, which are then saved to disk as an `.json` file.

---

## Stage 5: The WASM Bridge (Portability)
**Goal:** Run the C++ compiler inside a web browser.

*   **Files:** `src/WasmBindings.cpp`
*   **Key Function:** `compileToSchema(const std::string& source)`
    *   Acts as the entry point for the browser. It orchestrates the Lexer, Parser, and Analyzer in sequence, then returns the final JSON as a string to the JavaScript environment.
    *   The `EMSCRIPTEN_BINDINGS` macro exports this function so it can be called directly from JS via `Module.compileToSchema()`.
