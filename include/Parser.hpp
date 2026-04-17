#pragma once
#include "Lexer.hpp"
#include "AST.hpp"
#include <vector>
#include <stdexcept>

class Parser {
public:
    Parser(const std::vector<FSToken>& tokens);
    FormNode parse();

private:
    std::vector<FSToken> tokens;
    size_t pos;

    FSToken peek() const;
    FSToken consume();
    FSToken expect(FSTokenType type, const std::string& message);
    bool match(FSTokenType type);

    FormNode parseForm();
    SectionNode parseSection();
    FieldNode parseField();
    Attribute parseAttribute();
    std::string parseValue();
    std::vector<std::string> parseList();
};
