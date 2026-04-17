#include <iostream>
#include <fstream>
#include <sstream>
#include "CLI11.hpp"
#include "json.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "SemanticAnalyzer.hpp"

using json = nlohmann::json;

json generateJson(const FormNode& form) {
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
            
            // Map field type
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

int main(int argc, char** argv) {
    CLI::App app{"Lexiform Compiler: Compiles .form files to structured JSON schema."};

    std::string input_file;
    std::string output_file = "output.json";
    bool dump_ast = false;

    app.add_option("input", input_file, "Input .form file")
       ->required()
       ->check(CLI::ExistingFile);
    app.add_option("-o,--output", output_file, "Output .json file")
       ->capture_default_str();
    app.add_flag("--ast", dump_ast, "Dump the Abstract Syntax Tree to console");

    CLI11_PARSE(app, argc, argv);

    std::ifstream input(input_file);
    if (!input.is_open()) {
        std::cerr << "Error: Could not open input file " << input_file << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << input.rdbuf();
    std::string source = buffer.str();

    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        Parser parser(tokens);
        FormNode form = parser.parse();

        if (dump_ast) {
            form.print();
        }

        // Phase 3: Semantic Analysis
        SemanticAnalyzer analyzer;
        analyzer.analyze(form);
        
        json output = generateJson(form);

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
