#include <emscripten/bind.h>
#include <string>
#include <sstream>
#include "json.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "SemanticAnalyzer.hpp"
#include "IRGenerator.hpp"
#include "Optimizer.hpp"

using json = nlohmann::json;
using namespace emscripten;

// ============================================================================
// WebAssembly Bindings — Browser Entry Point
// ============================================================================
// This file is the bridge between the C++ compiler core and JavaScript.
// It orchestrates the full 6-phase compilation pipeline and returns the
// final JSON schema as a string to the browser environment.
// ============================================================================

/**
 * Generate JSON from the optimized IR program (same logic as main.cpp).
 * Reconstructs the hierarchical JSON structure from flat IR instructions.
 */
json generateJsonFromIR(const std::vector<IRInstruction>& program) {
    json root;
    std::unordered_map<int, json> temps;

    for (const auto& instr : program) {
        switch (instr.opcode) {
            case IROpCode::FORM_DEF:
                temps[instr.resultTemp] = json{
                    {"title", instr.arg1},
                    {"id", instr.arg2},
                    {"sections", json::array()}
                };
                break;

            case IROpCode::SECTION_DEF:
                temps[instr.resultTemp] = json{
                    {"title", instr.arg1},
                    {"fields", json::array()}
                };
                break;

            case IROpCode::FIELD_DEF: {
                std::string typeLower = instr.arg1;
                for (auto& c : typeLower) c = std::tolower(c);
                temps[instr.resultTemp] = json{
                    {"type", typeLower},
                    {"label", instr.arg2},
                    {"id", instr.arg3}
                };
                break;
            }

            case IROpCode::SET_ATTR_STRING:
                temps[instr.targetTemp][instr.arg1] = instr.arg2;
                break;

            case IROpCode::SET_ATTR_INT:
                temps[instr.targetTemp][instr.arg1] = instr.intValue;
                break;

            case IROpCode::SET_ATTR_BOOL:
                temps[instr.targetTemp][instr.arg1] = instr.boolValue;
                break;

            case IROpCode::SET_ATTR_LIST:
                temps[instr.targetTemp][instr.arg1] = instr.listValue;
                break;

            case IROpCode::ATTACH_FIELD:
                temps[instr.targetTemp]["fields"].push_back(temps[instr.intValue]);
                break;

            case IROpCode::ATTACH_SECTION:
                temps[instr.targetTemp]["sections"].push_back(temps[instr.intValue]);
                break;

            case IROpCode::SET_SUBMIT:
                temps[instr.targetTemp]["submit"] = json{{"label", instr.arg1}};
                break;

            case IROpCode::FORM_END:
                root = temps[instr.targetTemp];
                break;
        }
    }

    return root;
}

/**
 * The main export for WebAssembly — full 6-phase compilation pipeline.
 * Phase 1: Lexical Analysis  -> Token stream
 * Phase 2: Syntax Analysis   -> AST
 * Phase 3: Semantic Analysis  -> Validated AST
 * Phase 4: IR Generation      -> Three-Address Code
 * Phase 5: Optimization       -> Optimized IR
 * Phase 6: Code Generation    -> JSON output string
 */
std::string compileToSchema(const std::string& source) {
    try {
        // Phase 1: Lexical Analysis
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        // Phase 2: Syntax Analysis
        Parser parser(tokens);
        FormNode form = parser.parse();

        // Phase 3: Semantic Analysis
        SemanticAnalyzer analyzer;
        analyzer.analyze(form);

        // Phase 4: IR Generation
        IRGenerator irGen;
        auto irProgram = irGen.generate(form);

        // Phase 5: Optimization
        Optimizer optimizer;
        optimizer.optimize(irProgram);

        // Phase 6: Code Generation (IR -> JSON)
        json output = generateJsonFromIR(irProgram);
        return output.dump();

    } catch (const std::exception& e) {
        json errorJson;
        errorJson["error"] = e.what();
        return errorJson.dump();
    }
}

// Emscripten bindings — export compileToSchema to JavaScript
EMSCRIPTEN_BINDINGS(Lexiform_module) {
    function("compileToSchema", &compileToSchema);
}
