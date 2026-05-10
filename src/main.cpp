#include <iostream>
#include <fstream>
#include <sstream>
#include "CLI11.hpp"
#include "json.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "SemanticAnalyzer.hpp"
#include "IRGenerator.hpp"
#include "Optimizer.hpp"

using json = nlohmann::json;

// ============================================================================
// Phase 6: Code Generation — JSON Target
// ============================================================================
// The code generator consumes the optimized IR and produces the final JSON
// schema. It reconstructs the hierarchical form structure by interpreting
// the flat IR instruction list and building the JSON tree.
// ============================================================================

/**
 * Generate final JSON output from the optimized IR program.
 * Walks the IR instruction list and builds the nested JSON structure.
 */
json generateJsonFromIR(const std::vector<IRInstruction>& program) {
    json root;
    std::unordered_map<int, json> temps;  // Temporary register -> JSON object

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
                // Convert field type to lowercase for JSON output
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

// --- Legacy direct AST-to-JSON generator (kept for reference) ----------------

json generateJsonDirect(const FormNode& form) {
    json root;
    root["title"] = form.title;
    root["id"] = form.id;
    root["sections"] = json::array();

    for (const auto& section : form.sections) {
        json sectionJson;
        sectionJson["title"] = section.title;
        sectionJson["fields"] = json::array();

        for (const auto& field : section.fields) {
            json fieldJson;
            fieldJson["id"] = field.id;
            fieldJson["label"] = field.label;
            
            // Map field type enum to lowercase string
            static const std::vector<std::string> typeNames = {
                "text", "email", "phone", "number", "date", "textarea", 
                "dropdown", "radio", "checkbox", "file"
            };
            fieldJson["type"] = typeNames[static_cast<int>(field.type)];

            for (const auto& attr : field.attributes) {
                std::visit([&fieldJson, &attr](auto&& arg) {
                    fieldJson[attr.name] = arg;
                }, attr.value);
            }
            sectionJson["fields"].push_back(fieldJson);
        }
        root["sections"].push_back(sectionJson);
    }

    root["submit"] = {{"label", form.submitLabel}};
    return root;
}

// ============================================================================
// Compilation Pipeline
// ============================================================================
// Orchestrates all 6 phases in sequence:
//   Phase 1: Lexical Analysis  (Lexer)
//   Phase 2: Syntax Analysis   (Parser)
//   Phase 3: Semantic Analysis  (SemanticAnalyzer)
//   Phase 4: IR Generation      (IRGenerator)
//   Phase 5: Optimization       (Optimizer)
//   Phase 6: Code Generation    (generateJsonFromIR)
// ============================================================================

/**
 * Run the full 6-phase compilation pipeline on source code.
 * @param source     Raw .form source text
 * @param debug      If true, print all intermediate representations
 * @return           Final JSON output
 */
json compileFull(const std::string& source, bool debug) {

    // --- Phase 1: Lexical Analysis ---
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    if (debug) {
        std::cout << "\n=== Phase 1: Lexical Analysis (Token Stream) ===" << std::endl;
        std::cout << "Total tokens: " << tokens.size() << std::endl;
        std::cout << std::string(48, '-') << std::endl;
        for (size_t i = 0; i < tokens.size(); i++) {
            std::cout << "  [" << i << "] Type=" << static_cast<int>(tokens[i].type)
                      << "  Value=\"" << tokens[i].value << "\""
                      << "  (line " << tokens[i].line << ", col " << tokens[i].column << ")"
                      << std::endl;
        }
        std::cout << std::string(48, '-') << std::endl;
    }

    // --- Phase 2: Syntax Analysis ---
    Parser parser(tokens);
    FormNode form = parser.parse();

    if (debug) {
        std::cout << "\n=== Phase 2: Syntax Analysis (AST) ===" << std::endl;
        form.print();
        std::cout << std::endl;
    }

    // --- Phase 3: Semantic Analysis ---
    SemanticAnalyzer analyzer;
    analyzer.analyze(form);

    if (debug) {
        std::cout << "=== Phase 3: Semantic Analysis ===" << std::endl;
        std::cout << "Symbol Table:" << std::endl;
        std::cout << "  Form ID: \"" << form.id << "\" -> FORM" << std::endl;
        for (const auto& section : form.sections) {
            for (const auto& field : section.fields) {
                std::cout << "  Field ID: \"" << field.id << "\" -> FIELD" << std::endl;
            }
        }
        std::cout << "Semantic analysis passed (no errors)." << std::endl;
        std::cout << std::endl;
    }

    // --- Phase 4: IR Generation ---
    IRGenerator irGen;
    auto irProgram = irGen.generate(form);

    if (debug) {
        std::cout << "\n=== Phase 4: Intermediate Representation (Before Optimization) ===" << std::endl;
        std::cout << IRGenerator::printIR(irProgram);
        std::cout << std::endl;
    }

    // --- Phase 5: Optimization ---
    Optimizer optimizer;
    auto irBeforeOpt = irProgram;  // Keep a copy for before/after comparison
    auto stats = optimizer.optimize(irProgram);

    if (debug) {
        std::cout << "=== Phase 5: Optimization ===" << std::endl;
        std::cout << Optimizer::printStats(stats);

        if (stats.totalInstructionsBefore != stats.totalInstructionsAfter) {
            std::cout << "\n--- IR After Optimization ---" << std::endl;
            std::cout << IRGenerator::printIR(irProgram);
        } else {
            std::cout << "IR unchanged (no optimizations applied)." << std::endl;
        }
        std::cout << std::endl;
    }

    // --- Phase 6: Code Generation ---
    json output = generateJsonFromIR(irProgram);

    if (debug) {
        std::cout << "=== Phase 6: Code Generation (JSON Output) ===" << std::endl;
        std::cout << output.dump(2) << std::endl;
        std::cout << std::endl;
    }

    return output;
}

// ============================================================================
// Interactive REPL Mode
// ============================================================================
// Reads multi-line .form snippets from stdin and compiles them on-the-fly.
// Enter an empty line to compile, or type "exit" / "quit" to leave.
// ============================================================================

void runInteractive() {
    std::cout << "╔══════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║       Lexiform Interactive REPL (v0.1.0)        ║" << std::endl;
    std::cout << "║  Type your .form code, then press Enter twice.  ║" << std::endl;
    std::cout << "║  Type 'exit' or 'quit' to leave.                ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════╝" << std::endl;

    while (true) {
        std::cout << "\nlexiform> ";
        std::string source;
        std::string line;

        // Read multi-line input until an empty line is entered
        while (std::getline(std::cin, line)) {
            if (line.empty()) break;
            if (line == "exit" || line == "quit") {
                std::cout << "Goodbye!" << std::endl;
                return;
            }
            source += line + "\n";
        }

        // Skip if only whitespace was entered
        if (source.empty() || source.find_first_not_of(" \t\n\r") == std::string::npos) {
            continue;
        }

        try {
            json output = compileFull(source, false);
            std::cout << "\n✅ Compilation successful!" << std::endl;
            std::cout << output.dump(2) << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "\n❌ Error: " << e.what() << std::endl;
        }
    }
}

// ============================================================================
// Main Entry Point
// ============================================================================

int main(int argc, char** argv) {
    CLI::App app{"Lexiform Compiler: Compiles .form files to structured JSON schema."};

    std::string input_file;
    std::string output_file = "output.json";
    bool debug_mode = false;
    bool interactive_mode = false;
    bool dump_ast = false;

    app.add_option("input", input_file, "Input .form file")
       ->check(CLI::ExistingFile);
    app.add_option("-o,--output", output_file, "Output .json file")
       ->capture_default_str();
    app.add_flag("--debug", debug_mode, "Show all intermediate representations (tokens, AST, IR, optimized IR, JSON)");
    app.add_flag("--interactive", interactive_mode, "Launch interactive REPL mode");
    app.add_flag("--ast", dump_ast, "Dump the Abstract Syntax Tree to console (legacy, use --debug instead)");

    CLI11_PARSE(app, argc, argv);

    // --- Interactive REPL mode ---
    if (interactive_mode) {
        runInteractive();
        return 0;
    }

    // --- File compilation mode (requires input file) ---
    if (input_file.empty()) {
        std::cerr << "Error: No input file specified. Use --interactive for REPL mode." << std::endl;
        std::cerr << "Usage: lexiform input.form -o output.json" << std::endl;
        return 1;
    }

    std::ifstream input(input_file);
    if (!input.is_open()) {
        std::cerr << "Error: Could not open input file " << input_file << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << input.rdbuf();
    std::string source = buffer.str();

    try {
        // Legacy --ast flag support (just dump AST, use old direct pipeline)
        if (dump_ast && !debug_mode) {
            Lexer lexer(source);
            auto tokens = lexer.tokenize();
            Parser parser(tokens);
            FormNode form = parser.parse();
            form.print();
            SemanticAnalyzer analyzer;
            analyzer.analyze(form);
            json output = generateJsonDirect(form);
            std::ofstream out(output_file);
            if (out.is_open()) {
                out << output.dump(4) << std::endl;
                std::cout << "Successfully compiled " << input_file << " to " << output_file << std::endl;
            }
            return 0;
        }

        // Full 6-phase pipeline
        json output = compileFull(source, debug_mode);

        std::ofstream out(output_file);
        if (out.is_open()) {
            out << output.dump(4) << std::endl;
            std::cout << "Successfully compiled " << input_file << " to " << output_file << std::endl;
        } else {
            std::cerr << "Error: Could not open output file " << output_file << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Compilation failed: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
