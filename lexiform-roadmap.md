# Lexiform Project Roadmap

Lexiform is currently in **v0.1.0** (Initial Core Implementation). This roadmap outlines the completed work and the planned features for the project.

## ✅ Completed Milestones

- [x] **Handwritten Lexer & Parser:** Implemented recursive descent parser for the Lexiform DSL.
- [x] **Semantic Analysis:** Type-safe validation (e.g., checking if `MAX_WORDS` is applied to `TEXTAREA`).
- [x] **WebAssembly Bridge:** Embind integration to run the C++ core in JavaScript environments.
- [x] **NPM Package (`lexiform`):** Official TypeScript wrapper with `LexiformEngine` and React hooks.
- [x] **CLI Tool:** Direct compilation from terminal to JSON.

## 🚀 Future Roadmap

### Short-Term (v0.2.0 - v0.3.0)
- [ ] **Expanded DSL Syntax:**
  - Support for `NESTED SECTIONS`.
  - Conditional logic: `SHOW FIELD IF other-field IS "value"`.
- [ ] **Pre-built UI Components:**
  - Official React component library to render the generated JSON schema automatically.
- [ ] **VS Code Extension:**
  - Syntax highlighting and autocomplete for `.form` files.

### Medium-Term (v0.5.0+)
- [ ] **Multi-target Code Generation:**
  - Support for outputting HTML/Tailwind, Flutter, or Swift code instead of just JSON.
- [ ] **Enhanced Semantic Validation:**
  - Cross-field validation (e.g., `START_DATE` must be before `END_DATE`).

### Long-Term (v1.0.0+)
- [ ] **Visual Form Designer:**
  - A drag-and-drop editor that outputs `.form` markup.
- [ ] **Plugin System:**
  - Allow developers to add custom field types and custom semantic analyzers in C++.

## 📋 Maintenance Tasks
- [ ] Clean up legacy references to "FormScript" in internal test runners and scripts.
- [ ] Improve WASM loading speed and memory management.
- [ ] Add comprehensive unit tests for the C++ Lexer and Parser.
