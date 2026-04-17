#pragma once
#include "AST.hpp"
#include <string>
#include <unordered_map>
#include <stdexcept>

class SemanticAnalyzer {
public:
    void analyze(const FormNode& form);

private:
    std::unordered_map<std::string, std::string> symbolTable; // ID -> Type (as string)

    void checkDuplicateID(const std::string& id, const std::string& context);
    void validateField(const FieldNode& field);
};
