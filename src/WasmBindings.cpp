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
json generateJsonFromIR(const std::vector<IRInstruction> &program)
{
    json root;
    std::unordered_map<int, json> temps;

    for (const auto &instr : program)
    {
        switch (instr.opcode)
        {
        case IROpCode::FORM_DEF:
            temps[instr.resultTemp] = json{
                {"title", instr.arg1},
                {"id", instr.arg2},
                {"sections", json::array()}};
            break;

        case IROpCode::SECTION_DEF:
            temps[instr.resultTemp] = json{
                {"title", instr.arg1},
                {"fields", json::array()}};
            break;

        case IROpCode::FIELD_DEF:
        {
            std::string typeLower = instr.arg1;
            for (auto &c : typeLower)
                c = std::tolower(c);
            temps[instr.resultTemp] = json{
                {"type", typeLower},
                {"label", instr.arg2},
                {"id", instr.arg3}};
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
std::string compileToSchema(const std::string &source)
{
    try
    {
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
    }
    catch (const std::exception &e)
    {
        json errorJson;
        errorJson["error"] = e.what();
        return errorJson.dump();
    }
}

/**
 * Debug version: Return all compilation stages in one JSON object
 */
std::string compileWithStages(const std::string &source)
{
    try
    {
        json stages;
        stages["success"] = true;

        // Phase 1: Lexical Analysis
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        json tokenJson = json::array();
        for (const auto &token : tokens)
        {
            tokenJson.push_back({{"type", token.type},
                                 {"value", token.value}});
        }
        stages["phase1_lexing"] = {
            {"name", "Lexical Analysis"},
            {"description", "Tokenization via handwritten finite automaton"},
            {"tokens", tokenJson}};

        // Phase 2: Syntax Analysis
        Parser parser(tokens);
        FormNode form = parser.parse();
        stages["phase2_parsing"] = {
            {"name", "Syntax Analysis"},
            {"description", "Recursive descent parser → AST"},
            {"ast", "Form node structure parsed successfully"}};

        // Phase 3: Semantic Analysis
        SemanticAnalyzer analyzer;
        analyzer.analyze(form);
        stages["phase3_semantic"] = {
            {"name", "Semantic Analysis"},
            {"description", "Symbol table, type checking, scope validation"},
            {"result", "Semantic validation completed"}};

        // Phase 4: IR Generation
        IRGenerator irGen;
        auto irProgram = irGen.generate(form);
        json irJson = json::array();
        auto getOpcodeName = [](IROpCode op) {
            switch (op) {
                case IROpCode::FORM_DEF: return "FORM_DEF";
                case IROpCode::SECTION_DEF: return "SECTION_DEF";
                case IROpCode::FIELD_DEF: return "FIELD_DEF";
                case IROpCode::SET_ATTR_STRING: return "SET_ATTR_STRING";
                case IROpCode::SET_ATTR_INT: return "SET_ATTR_INT";
                case IROpCode::SET_ATTR_BOOL: return "SET_ATTR_BOOL";
                case IROpCode::SET_ATTR_LIST: return "SET_ATTR_LIST";
                case IROpCode::ATTACH_FIELD: return "ATTACH_FIELD";
                case IROpCode::ATTACH_SECTION: return "ATTACH_SECTION";
                case IROpCode::SET_SUBMIT: return "SET_SUBMIT";
                case IROpCode::FORM_END: return "FORM_END";
                default: return "UNKNOWN";
            }
        };

        for (const auto &instr : irProgram)
        {
            irJson.push_back({{"opcode", getOpcodeName(instr.opcode)},
                              {"arg1", instr.arg1},
                              {"arg2", instr.arg2},
                              {"arg3", instr.arg3}});
        }
        stages["phase4_ir_generation"] = {
            {"name", "IR Generation"},
            {"description", "AST → Three-Address Code (TAC)"},
            {"instructions", irJson},
            {"count", irProgram.size()}};

        // Phase 5: Optimization
        Optimizer optimizer;
        optimizer.optimize(irProgram);
        json optIrJson = json::array();
        for (const auto &instr : irProgram)
        {
            optIrJson.push_back({{"opcode", getOpcodeName(instr.opcode)},
                                 {"arg1", instr.arg1},
                                 {"arg2", instr.arg2},
                                 {"arg3", instr.arg3}});
        }
        stages["phase5_optimization"] = {
            {"name", "Optimization"},
            {"description", "Dead attribute elimination + duplicate folding"},
            {"instructions", optIrJson},
            {"optimized_count", irProgram.size()}};

        // Phase 6: Code Generation
        json output = generateJsonFromIR(irProgram);
        stages["phase6_codegen"] = {
            {"name", "Code Generation"},
            {"description", "Optimized IR → JSON schema"},
            {"result", output}};

        return stages.dump();
    }
    catch (const std::exception &e)
    {
        json errorJson;
        errorJson["success"] = false;
        errorJson["error"] = e.what();
        return errorJson.dump();
    }
}

// Emscripten bindings — export compileToSchema to JavaScript
EMSCRIPTEN_BINDINGS(Lexiform_module)
{
    function("compileToSchema", &compileToSchema);
    function("compileWithStages", &compileWithStages);
}
