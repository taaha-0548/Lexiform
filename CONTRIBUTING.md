# Contributing to Lexiform

First off, thanks for taking the time to contribute! 🎉

## How to Contribute

### 1. Development Environment
*   **C++ Compiler:** Requires C++17 or higher.
*   **Build System:** CMake 3.10+.
*   **Frontend:** Node.js 18+ and npm.

### 2. Workflow
1.  Fork the repository.
2.  Create a new branch (`git checkout -b feature/amazing-feature`).
3.  **Build everything:** `npm run build`
4.  Make your changes.
5.  **Important:** Run the test suites!
    *   Full suite: `npm test`
    *   Compiler tests: `node tests/test_runner.js`
    *   Engine tests: `npm test -w packages/lexiform-js`
6.  Commit your changes (`git commit -m 'Add amazing feature'`).
7.  Push to the branch (`git push origin feature/amazing-feature`).
8.  Open a Pull Request.

### 3. Code Standards
*   **C++:** Follow a consistent style (similar to LLVM or Google style).
*   **TypeScript:** Use strict typing; avoid `any`.
*   **DSL:** Ensure new keywords are documented in the README.

## Bug Reports
If you find a bug, please open an issue and include:
*   A minimal `.form` file that reproduces the error.
*   The expected output vs the actual output.
*   Your operating system and compiler version.
