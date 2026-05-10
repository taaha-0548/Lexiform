// ============================================================================
// Phase 3: Semantic Analysis (The "Brain")
// ============================================================================
// The Semantic Analyzer is the third phase of the compiler pipeline. It
// traverses the AST produced by the Parser and enforces context-sensitive
// rules that cannot be expressed in a context-free grammar.
//
// Implementation: AST Tree Traversal with Symbol Table
//   - Symbol Table: an unordered_map<string, string> mapping IDs to their
//     context type ("FORM" or "FIELD"). Used for uniqueness checking.
//   - Type Checking: validates that attributes are only applied to compatible
//     field types (e.g., MAX_WORDS only on TEXTAREA).
//   - Completeness: ensures mandatory attributes are present (e.g., OPTIONS
//     on DROPDOWN/RADIO fields).
//
// Errors are reported as exceptions with descriptive messages including the
// offending field ID and the violated rule.
// ============================================================================

#include "SemanticAnalyzer.hpp"
#include <iostream>

void SemanticAnalyzer::analyze(const FormNode& form) {
    symbolTable.clear();
    checkDuplicateID(form.id, "FORM");

    for (const auto& section : form.sections) {
        for (const auto& field : section.fields) {
            checkDuplicateID(field.id, "FIELD");
            validateField(field);
        }
    }
}

void SemanticAnalyzer::checkDuplicateID(const std::string& id, const std::string& context) {
    if (symbolTable.find(id) != symbolTable.end()) {
        throw std::runtime_error("Semantic Error: Duplicate ID '" + id + "' found in " + context);
    }
    symbolTable[id] = context;
}

void SemanticAnalyzer::validateField(const FieldNode& field) {
    for (const auto& attr : field.attributes) {
        if (attr.name == "MAX_WORDS") {
            if (field.type != FieldType::TEXTAREA) {
                throw std::runtime_error("Semantic Error: Field '" + field.id + "' is not a TEXTAREA but uses MAX_WORDS");
            }
        } else if (attr.name == "OPTIONS") {
            if (field.type != FieldType::DROPDOWN && field.type != FieldType::RADIO && field.type != FieldType::CHECKBOX) {
                throw std::runtime_error("Semantic Error: Field '" + field.id + "' is not a selection type but uses OPTIONS");
            }
        } else if (attr.name == "MIN" || attr.name == "MAX") {
            if (field.type != FieldType::NUMBER) {
                throw std::runtime_error("Semantic Error: Field '" + field.id + "' is not a NUMBER but uses MIN/MAX");
            }
        }
        // Add more checks as needed
    }
    
    // Check mandatory attributes
    if (field.type == FieldType::DROPDOWN || field.type == FieldType::RADIO) {
        bool hasOptions = false;
        for (const auto& attr : field.attributes) {
            if (attr.name == "OPTIONS") hasOptions = true;
        }
        if (!hasOptions) {
            throw std::runtime_error("Semantic Error: Field '" + field.id + "' (DROPDOWN/RADIO) requires OPTIONS");
        }
    }
}
