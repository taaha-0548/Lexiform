# Lexiform: Compiler Architecture

This document describes the formal phases of the Lexiform compiler, implemented in C++.

## Compilation Pipeline Overview

```
Source Code (.form)
       │
       ▼
┌─────────────────┐
│ Phase 1: Lexer  │  → Token Stream
└────────┬────────┘
         ▼
┌─────────────────┐
│ Phase 2: Parser │  → Abstract Syntax Tree (AST)
└────────┬────────┘
         ▼
┌─────────────────────────┐
│ Phase 3: Semantic       │  → Validated AST + Symbol Table
│         Analyzer        │
└────────┬────────────────┘
         ▼
┌─────────────────────────┐
│ Phase 4: IR Generator   │  → Three-Address Code (TAC)
└────────┬────────────────┘
         ▼
┌─────────────────────────┐
│ Phase 5: Optimizer      │  → Optimized IR
│  • Dead Attr Elim       │
│  • Duplicate Attr Fold  │
└────────┬────────────────┘
         ▼
┌─────────────────────────┐
│ Phase 6: Code Generator │  → JSON Schema / WASM output
└─────────────────────────┘
```

## 1. Lexical Analysis (The Scanner)
**File:** `Lexer.cpp` / `Lexer.hpp`
- **Goal:** Tokenization.
- **Method:** Handwritten Finite Automaton.
- **Process:** Converts the raw character stream into a sequence of `FSToken` objects. It handles whitespace stripping, string literal extraction, keyword identification via a static keyword lookup table, and tracks line/column for error reporting.

## 2. Syntax Analysis (The Parser)
**File:** `Parser.cpp` / `Parser.hpp`
- **Goal:** Hierarchical structure validation.
- **Method:** Recursive Descent (Top-Down Parsing).
- **Process:** Consumes tokens and builds a tree-like structure (AST) based on the Lexiform grammar. It enforces the `FORM → SECTION → FIELD` hierarchy with one token of lookahead.

## 3. Semantic Analysis (The "Brain")
**File:** `SemanticAnalyzer.cpp` / `SemanticAnalyzer.hpp`
- **Goal:** Context-sensitive validation.
- **Method:** Tree Traversal with Symbol Table.
- **Checks:**
  - **Uniqueness:** Ensures all IDs (Form and Field) are globally unique.
  - **Type Compatibility:** Validates that attributes (like `MAX_WORDS`) are only applied to valid types (like `TEXTAREA`).
  - **Completeness:** Ensures mandatory attributes are present (e.g., `OPTIONS` on `DROPDOWN`/`RADIO`).

## 4. Intermediate Representation (IR Generation)
**File:** `IRGenerator.cpp` / `IRGenerator.hpp`
- **Goal:** Linearize the AST into a flat, optimizable instruction list.
- **Method:** Single-pass AST traversal with temporary register allocation.
- **Output:** Three-Address Code (TAC) where each instruction operates on at most three operands. Structural nodes (Form, Section, Field) are assigned temporary registers, and relationships are expressed via `ATTACH` instructions.
- **IR Opcodes:** `FORM_DEF`, `SECTION_DEF`, `FIELD_DEF`, `SET_ATTR_*`, `ATTACH_FIELD`, `ATTACH_SECTION`, `SET_SUBMIT`, `FORM_END`

## 5. Optimization
**File:** `Optimizer.cpp` / `Optimizer.hpp`
- **Goal:** Simplify and reduce the IR before code generation.
- **Method:** Two-pass optimization on the flat IR instruction list.
- **Pass 1 — Dead Attribute Elimination:** Removes `SET_ATTR` instructions where the attribute has no semantic effect on the target field type (e.g., `PLACEHOLDER` on `CHECKBOX`, `MAX_WORDS` on `TEXT`).
- **Pass 2 — Duplicate Attribute Folding:** When the same attribute is set multiple times on the same temporary (dead stores), only the last assignment is kept.

## 6. Code Generation (The Backend)
**File:** `main.cpp` (`generateJsonFromIR`)
- **Goal:** Target Translation.
- **Method:** IR-to-JSON Mapping.
- **Target:** Framework-agnostic JSON Schema.
- **Process:** Walks the optimized IR instruction list, reconstructs the hierarchical JSON structure using a temporary register map, and outputs the final JSON via the `nlohmann/json` library.

## 7. WebAssembly & JavaScript Layer (The Bridge)
**File:** `WasmBindings.cpp` / `packages/lexiform-js/`
- **Goal:** Browser & Runtime Portability.
- **Method:** Emscripten Compilation & JS Glue Code.
- **Process:** 
  - The C++ core (all 6 phases) is compiled to **WebAssembly (WASM)** using `emcc`.
  - Emscripten generates a **JavaScript Glue Module** (`lexiform.js`) that handles WASM instantiation and memory management.
  - The `lexiform` NPM package provides a clean TypeScript wrapper (`LexiformEngine`) and a React hook (`useLexiform`) around this module.
