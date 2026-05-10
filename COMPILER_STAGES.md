# Lexiform Compiler: Inner Workings & Stages

This document explains the lifecycle of a Lexiform (`.form`) file as it travels through all 6 compiler phases.

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

**Token stream output for example above:**
```
[0] Type=FORM       Value="FORM"         (line 1, col 1)
[1] Type=STRING     Value="Contact Us"   (line 1, col 6)
[2] Type=IDENTIFIER Value="contact-form" (line 1, col 19)
[3] Type=SECTION    Value="SECTION"      (line 2, col 1)
[4] Type=STRING     Value="Identity"     (line 2, col 9)
[5] Type=TEXT       Value="TEXT"          (line 3, col 5)
[6] Type=STRING     Value="Full Name"    (line 3, col 10)
...
```

---

## Stage 2: Syntax Analysis (The Parser)
**Goal:** Organize tokens into a logical tree structure called an **Abstract Syntax Tree (AST)**.

*   **Files:** `src/Parser.cpp`, `include/Parser.hpp`, `include/AST.hpp`
*   **Key Function:** `Parser::parse()`
    *   Uses **Recursive Descent** (calling `parseForm()`, `parseSection()`, and `parseField()`) to build a hierarchy of `Node` objects representing the form's structure.

**AST output:**
```
[AST] FORM: "Contact Us" (ID: contact-form)
  ├── SECTION: "Identity"
  │   ├── FIELD: [TEXT] "Full Name" (ID: name)
  │   │   └── ATTRIBUTE: REQUIRED
  │   │   └── ATTRIBUTE: PLACEHOLDER
  │   ├── FIELD: [EMAIL] "Email" (ID: email)
  │   │   └── ATTRIBUTE: REQUIRED
  └── SUBMIT: "Send Message"
```

---

## Stage 3: Semantic Analysis (The Checker)
**Goal:** Verify that the "meaning" of the code is valid.

*   **Files:** `src/SemanticAnalyzer.cpp`, `include/SemanticAnalyzer.hpp`
*   **Key Function:** `SemanticAnalyzer::analyze(const FormNode& form)`
    *   Traverses the AST and calls `validateField()` to enforce rules like ID uniqueness and attribute compatibility (e.g., ensuring `OPTIONS` is only used on selection fields).

**Symbol Table after analysis:**
```
┌──────────────┬─────────┐
│ ID           │ Context │
├──────────────┼─────────┤
│ contact-form │ FORM    │
│ name         │ FIELD   │
│ email        │ FIELD   │
└──────────────┴─────────┘
Status: Passed (no semantic errors)
```

---

## Stage 4: Intermediate Representation (IR Generation)
**Goal:** Flatten the hierarchical AST into a linear Three-Address Code (TAC) format.

*   **Files:** `src/IRGenerator.cpp`, `include/IRGenerator.hpp`
*   **Key Function:** `IRGenerator::generate(const FormNode& form)`
    *   Walks the AST top-down, assigns fresh temporary registers to each structural node, and emits flat instructions with at most 3 operands each.

**IR output (Three-Address Code):**
```
=== Lexiform IR (Three-Address Code) ===
Total instructions: 12
------------------------------------------
   0: t0 = FORM_DEF "Contact Us" contact-form
   1: t1 = SECTION_DEF "Identity"
   2: t2 = FIELD_DEF TEXT "Full Name" name
   3:     SET_ATTR t2 REQUIRED = true
   4:     SET_ATTR t2 PLACEHOLDER = "Enter your name"
   5:     ATTACH_FIELD t1 <- t2
   6: t3 = FIELD_DEF EMAIL "Email" email
   7:     SET_ATTR t3 REQUIRED = true
   8:     ATTACH_FIELD t1 <- t3
   9:     ATTACH_SECTION t0 <- t1
  10:     SET_SUBMIT t0 "Send Message"
  11: FORM_END t0
------------------------------------------
```

---

## Stage 5: Optimization
**Goal:** Simplify the IR to produce cleaner output.

*   **Files:** `src/Optimizer.cpp`, `include/Optimizer.hpp`
*   **Key Function:** `Optimizer::optimize(std::vector<IRInstruction>& program)`
    *   **Pass 1 — Dead Attribute Elimination:** Removes SET_ATTR instructions where the attribute has no effect on the field type (e.g., PLACEHOLDER on CHECKBOX).
    *   **Pass 2 — Duplicate Attribute Folding:** When the same attribute is set multiple times on the same field, only the last assignment survives.

**Before/After example (with `optimization_demo.form`):**
```
BEFORE optimization (18 instructions):
  ...
  t2 = FIELD_DEF TEXT "Username" username
      SET_ATTR t2 REQUIRED = true
      SET_ATTR t2 PLACEHOLDER = "Enter username"   ← dead store (overwritten)
      SET_ATTR t2 PLACEHOLDER = "Your name"         ← kept (last write)
  ...
  t4 = FIELD_DEF CHECKBOX "Preferences" prefs
      SET_ATTR t4 OPTIONS = ["Dark Mode", "Notifications"]
      SET_ATTR t4 PLACEHOLDER = "Ignored"           ← dead attr (CHECKBOX)
  ...

AFTER optimization (16 instructions):
  - 1 dead attribute removed (PLACEHOLDER on CHECKBOX)
  - 1 duplicate attribute folded (double PLACEHOLDER on TEXT)
```

---

## Stage 6: Code Generation (The Backend)
**Goal:** Transform the optimized IR into a usable format (JSON).

*   **Files:** `src/main.cpp`
*   **Key Function:** `generateJsonFromIR(const std::vector<IRInstruction>& program)`
    *   Walks the optimized IR, reconstructs the hierarchical JSON using a temporary register map, and outputs the final `.json` file.

**JSON output:**
```json
{
  "title": "Contact Us",
  "id": "contact-form",
  "sections": [
    {
      "title": "Identity",
      "fields": [
        { "type": "text", "label": "Full Name", "id": "name", "REQUIRED": true, "PLACEHOLDER": "Enter your name" },
        { "type": "email", "label": "Email", "id": "email", "REQUIRED": true }
      ]
    }
  ],
  "submit": { "label": "Send Message" }
}
```

---

## Stage 7: The WASM Bridge (Portability)
**Goal:** Run the C++ compiler inside a web browser.

*   **Files:** `src/WasmBindings.cpp`
*   **Key Function:** `compileToSchema(const std::string& source)`
    *   Acts as the entry point for the browser. It orchestrates all 6 phases (Lexer → Parser → Analyzer → IR → Optimizer → CodeGen) in sequence, then returns the final JSON as a string to the JavaScript environment.
    *   The `EMSCRIPTEN_BINDINGS` macro exports this function so it can be called directly from JS via `Module.compileToSchema()`.
