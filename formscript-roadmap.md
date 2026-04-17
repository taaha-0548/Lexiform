# FormScript Compiler: Comprehensive C++ Implementation Roadmap

**Project Theme:** Document Compilation DSL
**Language Concept:** A developer-first markup language that compiles into a structured, framework-agnostic JSON schema [cite: 8, 9].

This roadmap provides a detailed, step-by-step guide to building the FormScript compiler using modern C++ tools.

---

## Phase 0: Environment Setup & CLI Construction

Before writing compiler logic, you need a solid foundation to handle files and inputs.

1.  **Build System (CMake):** * Set up a `CMakeLists.txt` file. This will handle linking your parser generators and header-only libraries.
    * Target C++17 or C++20 to take advantage of modern features like `std::variant`, `std::optional`, and string views.
2.  **Command-Line Interface (CLI11):**
    * Include the header-only `CLI11` library.
    * Define your compiler's entry point: `formscript <input.form> -o <output.json>`.
    * Set up file streams to read the `.form` source file and prepare an output stream for the JSON.

---

## Phase 1: Lexical Analysis (The Scanner)

**Goal:** Convert the raw `.form` text into a stream of categorized tokens.
**Tool:** ANTLR4 Lexer or Flex.

1.  **Define Structural Keywords:**
    * Tokens for `FORM`, `SECTION`, `SUBMIT`, `END` [cite: 16, 17, 29, 30].
2.  **Define Field Types:**
    * Tokens for `TEXT`, `EMAIL`, `PHONE`, `NUMBER`, `DATE`, `TEXTAREA`, `DROPDOWN`, `RADIO`, `CHECKBOX`, `FILE` [cite: 13].
3.  **Define Attributes:**
    * Tokens for `ID`, `REQUIRED`, `MIN`, `MAX`, `MAX_WORDS`, `OPTIONS`, `PLACEHOLDER` [cite: 13].
4.  **Define Literals & Symbols:**
    * `STRING`: Regex for text in quotes (e.g., `"Job Application"`) [cite: 16].
    * `IDENTIFIER`: Regex for standard variables (e.g., `job-app`, `name`) [cite: 16, 18].
    * `NUMBER_LITERAL`: Digits for min/max values.
    * Symbols: `=`, `[`, `]`, `,`.
5.  **Whitespace & Comments:** Configure the lexer to skip standard whitespaces and line breaks.

---

## Phase 2: Syntax Analysis (The Parser)

**Goal:** Consume tokens to enforce the hierarchical structure and build an Abstract Syntax Tree (AST).
**Tool:** ANTLR4 Parser or Bison.

1.  **Enforce Form Hierarchy:**
    * Rule: A `program` consists of exactly one `form_decl`.
    * Rule: A `form_decl` starts with `FORM`, a `STRING` (title), an `ID`, followed by one or more `section_decl`, a `submit_decl`, and ends with `END` [cite: 16, 30].
2.  **Enforce Section Hierarchy:**
    * Rule: A `section_decl` starts with `SECTION`, a `STRING`, followed by one or more `field_decl` [cite: 17, 18].
3.  **Parse Fields & Attributes:**
    * Rule: A `field_decl` starts with a `field_type`, a `STRING` (label), an `ID`, and a list of zero or more `attribute` tokens [cite: 18, 19].
4.  **Resolve Attribute Grammar Ambiguity (Challenge 1 Prep):**
    * *Important:* Do *not* try to restrict attributes to specific types in the parser rules (e.g., don't make a rule that says `TEXTAREA` specifically takes `MAX_WORDS`) [cite: 55, 56]. Allow *any* field to take *any* attribute at this stage to keep the grammar clean and unambiguous. We will enforce type-safety in Phase 3.

---

## Phase 3: Semantic Analysis (The "Brain")

**Goal:** Traverse the AST to check for logical and semantic correctness [cite: 13]. This is where you write native C++ traversal logic (e.g., using ANTLR Visitors).

1.  **Global Symbol Table & Duplicate IDs (Challenge 2):**
    * Initialize a `std::unordered_map<std::string, FieldASTNode*>` at the start of the traversal.
    * Every time you visit a field or form node, extract its `ID`.
    * Check if the ID exists in the map. If yes, throw a `SemanticError: Duplicate ID detected`. If no, insert it. This ensures global uniqueness across all nested scopes [cite: 58-60].
2.  **Attribute-Type Compatibility (Challenge 1 Resolution):**
    * As you visit each field node, inspect its type and its list of attributes.
    * Implement switch/if logic:
        * If type is `DROPDOWN` or `RADIO`, ensure `OPTIONS` exists.
        * If type is `TEXTAREA`, allow `MAX_WORDS`.
        * If type is `NUMBER`, allow `MIN` and `MAX`.
        * If an illegal attribute is found (e.g., `MIN` on an `EMAIL`), throw a `SemanticError: Attribute mismatch for field type` [cite: 55-57].
3.  **Mandatory Checks:**
    * Ensure the `SUBMIT` node exists [cite: 13].

---

## Phase 4: JSON Code Generation (The Backend)

**Goal:** Translate the validated AST into the final, framework-agnostic JSON schema [cite: 11, 52].
**Tool:** `nlohmann/json` (Header-only C++ library).

1.  **Initialize Schema Base:**
    * Create `nlohmann::json output;`
    * Set `output["id"]` and `output["title"]` from the root AST node [cite: 33, 34].
    * Initialize `output["sections"] = nlohmann::json::array();` [cite: 35].
2.  **Traverse Sections & Fields:**
    * Iterate through section nodes, creating a temporary JSON object for each: `{"title": "...", "fields": []}` [cite: 36-38].
    * Iterate through field nodes within the section. Map AST attributes to JSON keys:
        * `ID` -> `"id"`
        * `type` -> `"type"` (lowercase)
        * `Label` -> `"label"`
        * `REQUIRED` presence -> `"required": true` (default to false if absent) [cite: 39-41].
3.  **Handle Type-Specific JSON Mapping (Challenge 3):**
    * For `DROPDOWN` with `OPTIONS=[A, B, C]`, construct a JSON array and assign it to the `"options"` key [cite: 42, 43].
    * Map `MIN`/`MAX` and `MAX_WORDS` to their respective integer JSON values [cite: 44, 45].
4.  **Final Output:**
    * Map the `SUBMIT` node to `output["submit"] = {"label": "..."};` [cite: 49].
    * Write the JSON object to the designated output file using `std::setw(4)` for clean formatting.

---

## Phase 5: Testing & Refinement
1.  **Positive Test Cases:** Create `.form` files that utilize every feature (like your provided Example Program) [cite: 14-30] and ensure the compiled JSON matches the expected target [cite: 31-50].
2.  **Negative Test Cases:** Write tests specifically designed to fail:
    * Forms with duplicate IDs.
    * Forms using `MAX_WORDS` on a `CHECKBOX`.
    * Forms missing a `SUBMIT` tag.
3.  **Memory Management:** Run the compiler through `Valgrind` (if on Linux/Mac) or address sanitizers to ensure your AST nodes are properly freed if not using smart pointers.
