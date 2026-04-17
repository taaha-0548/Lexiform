#pragma once
#include <string>
#include <vector>
#include <memory>
#include <variant>

enum class FieldType {
    TEXT, EMAIL, PHONE, NUMBER, DATE, TEXTAREA, 
    DROPDOWN, RADIO, CHECKBOX, FILE
};

struct Attribute {
    std::string name;
    std::variant<std::string, int, std::vector<std::string>, bool> value;
};

struct FieldNode {
    FieldType type;
    std::string label;
    std::string id;
    std::vector<Attribute> attributes;
};

struct SectionNode {
    std::string title;
    std::vector<FieldNode> fields;
};

struct FormNode {
    std::string title;
    std::string id;
    std::vector<SectionNode> sections;
    std::string submitLabel;

    void print() const; // For AST dumping
};
