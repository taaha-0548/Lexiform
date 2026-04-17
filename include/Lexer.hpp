#pragma once
#include <string>
#include <vector>

enum class FSTokenType {
    // Keywords
    FORM, SECTION, SUBMIT, END,
    
    // Field Types
    TEXT, EMAIL, PHONE, NUMBER, DATE, TEXTAREA, 
    DROPDOWN, RADIO, CHECKBOX, FILE,

    // Attributes
    ATTR_ID, REQUIRED, ATTR_MIN, ATTR_MAX, MAX_WORDS, OPTIONS, PLACEHOLDER,

    // Literals & Identifiers
    STRING_LITERAL, IDENTIFIER, NUMBER_LITERAL,

    // Symbols
    EQUALS, LBRACKET, RBRACKET, COMMA,

    // End of file
    EOF_TOKEN,
    INVALID
};

struct FSToken {
    FSTokenType type;
    std::string value;
    int line;
    int column;
};

class Lexer {
public:
    Lexer(const std::string& source);
    std::vector<FSToken> tokenize();

private:
    std::string source;
    size_t pos;
    int line;
    int column;

    char peek() const;
    char consume();
    void skipWhitespace();
    FSToken readString();
    FSToken readIdentifierOrKeyword();
    FSToken readNumber();
};
