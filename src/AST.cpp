#include "AST.hpp"
#include <iostream>

static std::string fieldTypeToString(FieldType type) {
    switch (type) {
        case FieldType::TEXT: return "TEXT";
        case FieldType::EMAIL: return "EMAIL";
        case FieldType::PHONE: return "PHONE";
        case FieldType::NUMBER: return "NUMBER";
        case FieldType::DATE: return "DATE";
        case FieldType::TEXTAREA: return "TEXTAREA";
        case FieldType::DROPDOWN: return "DROPDOWN";
        case FieldType::RADIO: return "RADIO";
        case FieldType::CHECKBOX: return "CHECKBOX";
        case FieldType::FILE: return "FILE";
        default: return "UNKNOWN";
    }
}

void FormNode::print() const {
    std::cout << "[AST] FORM: \"" << title << "\" (ID: " << id << ")" << std::endl;
    for (const auto& section : sections) {
        std::cout << "  ├── SECTION: \"" << section.title << "\"" << std::endl;
        for (const auto& field : section.fields) {
            std::cout << "  │   ├── FIELD: [" << fieldTypeToString(field.type) << "] \"" << field.label << "\" (ID: " << field.id << ")" << std::endl;
            for (const auto& attr : field.attributes) {
                std::cout << "  │   │   └── ATTRIBUTE: " << attr.name << std::endl;
            }
        }
    }
    std::cout << "  └── SUBMIT: \"" << submitLabel << "\"" << std::endl;
}
