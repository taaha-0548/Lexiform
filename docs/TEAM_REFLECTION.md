# Team Reflection

**Course:** Compiler Construction  
**Project:** Lexiform — A Form Markup Language & Compiler  
**Date:** May 2026

---

## 1. Challenges Faced and Solutions

### Understanding Compiler Theory vs. Practice
The biggest challenge was bridging the gap between textbook compiler theory and actually implementing it. In lectures, concepts like lexical analysis and recursive descent parsing felt abstract — but when we had to write a working lexer from scratch in C++, we realized how many edge cases exist. For example, our lexer initially couldn't handle hyphens in identifiers (like `contact-form`), which broke everything. The fix was simple (adding `-` to the valid character set in `readIdentifierOrKeyword()`), but finding the bug took hours of debugging.

### Choosing the Right Scope
Early on, we were tempted to build a general-purpose language, but we quickly realized that would be far too ambitious. Narrowing our scope to a domain-specific language (DSL) for web forms was one of our best decisions. It gave us a clear, bounded problem to solve while still requiring all six compiler phases.

### WebAssembly Integration
Compiling our C++ core to WebAssembly using Emscripten was unexpectedly difficult. The Emscripten documentation is extensive but overwhelming for beginners. We spent almost two full days debugging why our WASM module wouldn't load in the browser — the issue turned out to be Vite's dependency pre-bundling mangling the WASM file paths. The solution was adding `optimizeDeps: { exclude: ['lexiform'] }` to the Vite config, a one-line fix that took two days to find.

### Building the IR Phase
Adding the Intermediate Representation (IR) phase after already having a working AST-to-JSON pipeline was challenging because we had to refactor the code generation to consume IR instructions instead of AST nodes directly. We chose a Three-Address Code style IR where each instruction operates on temporary registers. This made the optimizer passes much cleaner since they could work on a flat list of instructions instead of recursing through a tree.

### Semantic Analysis Edge Cases
Our semantic analyzer initially only checked for duplicate IDs. We had to iteratively add more checks (attribute compatibility, mandatory OPTIONS for DROPDOWN/RADIO) as we discovered more invalid `.form` files that should be rejected. This taught us that semantic analysis is never truly "done" — there's always another rule to enforce.

---

## 2. What We Learned About Compiler Design

### Compilers Are Pipelines
The most fundamental lesson was that a compiler is a pipeline of transformations, where each phase takes one representation and produces another. Understanding this made the entire project manageable — we could build and test each phase independently before connecting them.

### The Importance of Good Data Structures
Our AST design (`FormNode → SectionNode → FieldNode → Attribute`) directly influenced how easy or hard every subsequent phase was. When we got the AST right, the parser practically wrote itself. The IR design had a similar cascading effect on the optimizer and code generator.

### Error Messages Matter
We learned that a compiler's error messages are arguably more important than its success case. Users spend most of their time fixing errors, so producing clear messages with line numbers and column positions (e.g., `"Semantic Error: Field 'title' is not a TEXTAREA but uses MAX_WORDS"`) was critical for usability.

### Testing Is Non-Negotiable
Our test suite saved us countless times. Every time we added a new feature or refactored something, running the test suite immediately told us if we'd broken something. Writing test cases for *invalid* inputs (semantic errors, duplicate IDs) was just as important as testing valid ones.

### Optimization Is About Trade-offs
Implementing the optimizer taught us that optimization is always about trade-offs. Our dead attribute elimination pass removes attributes that have no effect on certain field types (like PLACEHOLDER on a CHECKBOX). This makes the output cleaner, but we had to be careful not to remove attributes that might be used by custom renderers we haven't thought of yet. We chose a conservative approach — only removing attributes we were certain were dead.

---

## 3. Future Improvements

If we had more time, we would pursue the following enhancements:

1. **Comments in the DSL:** Adding `//` single-line comments and `/* */` block comments to make `.form` files more maintainable.

2. **Conditional Logic:** Supporting constructs like `SHOW FIELD IF other-field IS "value"` to create dynamic forms where fields appear/disappear based on user input.

3. **Nested Sections:** Allowing sections within sections for more complex form hierarchies.

4. **Multi-target Code Generation:** Instead of only outputting JSON, we could generate HTML+CSS, Flutter widgets, or SwiftUI views directly from the IR.

5. **VS Code Extension:** Syntax highlighting, autocomplete, and inline error reporting for `.form` files would make the developer experience much smoother.

6. **A Visual Form Designer:** A drag-and-drop GUI that outputs valid `.form` markup, making Lexiform accessible to non-programmers.

7. **More Optimization Passes:** We only implemented two passes (dead attribute elimination and duplicate attribute folding). Additional passes like common sub-expression elimination for repeated attribute patterns and form-level optimizations (merging single-field sections) would further improve the output.

---

## 4. Individual Contributions

| Team Member | Contribution |
|-------------|-------------|
| **Taaha** | C++ core compiler (Lexer, Parser, AST design), CMake build system, CLI interface (--debug, --interactive) |
| **Raahim** | Semantic Analyzer, IR Generator, Optimizer, WebAssembly bridge (Emscripten bindings), NPM package (`lexiform`) |
| **Maaz** | React frontend, test suite design, documentation (Language Reference, Architecture docs), example programs |

All members participated in code reviews, debugging sessions, and the final demo preparation. The project was managed using Git with meaningful commit messages and feature branches.

---

*This reflection was written collaboratively by all team members.*
