#include <emscripten/bind.h>
#include <string>
#include <sstream>
#include "json.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "SemanticAnalyzer.hpp"

using json = nlohmann::json;
using namespace emscripten;

// Reusing the generateJson logic
json generateJsonFromAST(const FormNode& form) {
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

// The main export for WebAssembly
std::string compileToSchema(const std::string& source) {
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        Parser parser(tokens);
        FormNode form = parser.parse();

        SemanticAnalyzer analyzer;
        analyzer.analyze(form);
        
        json output = generateJsonFromAST(form);
        return output.dump(); // Return JSON string
    } catch (const std::exception& e) {
        json errorJson;
        errorJson["error"] = e.what();
        return errorJson.dump();
    }
}

// Bindings
EMSCRIPTEN_BINDINGS(Lexiform_module) {
    function("compileToSchema", &compileToSchema);
}
