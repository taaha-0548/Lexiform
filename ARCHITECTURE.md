# Lexiform: Compiler Architecture

This document describes the formal phases of the Lexiform compiler, implemented in C++.

## 1. Lexical Analysis (The Scanner)
**File:** `Lexer.cpp` / `Lexer.hpp`
- **Goal:** Tokenization.
- **Method:** Handwritten Finite Automaton.
- **Process:** Converts the raw character stream into a sequence of `FSToken` objects. It handles whitespace stripping, string literal extraction, and keyword identification.

## 2. Syntax Analysis (The Parser)
**File:** `Parser.cpp` / `Parser.hpp`
- **Goal:** Hierarchical structure validation.
- **Method:** Recursive Descent (Top-Down Parsing).
- **Process:** Consumes tokens and builds a tree-like structure (AST) based on the FormScript grammar. It enforces the `FORM -> SECTION -> FIELD` hierarchy.

## 3. Semantic Analysis (The "Brain")
**File:** `SemanticAnalyzer.cpp` / `SemanticAnalyzer.hpp`
- **Goal:** Context-sensitive validation.
- **Method:** Tree Traversal with Symbol Table.
- **Checks:**
  - **Uniqueness:** Ensures all IDs (Form and Field) are globally unique.
  - **Type Compatibility:** Validates that attributes (like `MAX_WORDS`) are only applied to valid types (like `TEXTAREA`).
  - **Completeness:** Ensures a `SUBMIT` button exists.

## 4. Code Generation (The Backend)
**File:** `main.cpp` (generateJson)
- **Goal:** Target Translation.
- **Method:** AST-to-JSON Mapping.
- **Target:** Framework-agnostic JSON Schema.
- **Process:** Translates the validated AST into the final JSON format using the `nlohmann/json` library.
