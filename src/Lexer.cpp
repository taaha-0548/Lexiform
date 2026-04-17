#include "Lexer.hpp"
#include <cctype>
#include <unordered_map>
#include <iostream>

static const std::unordered_map<std::string, FSTokenType> KEYWORDS = {
    {"FORM", FSTokenType::FORM},
    {"SECTION", FSTokenType::SECTION},
    {"SUBMIT", FSTokenType::SUBMIT},
    {"END", FSTokenType::END},
    {"TEXT", FSTokenType::TEXT},
    {"EMAIL", FSTokenType::EMAIL},
    {"PHONE", FSTokenType::PHONE},
    {"NUMBER", FSTokenType::NUMBER},
    {"DATE", FSTokenType::DATE},
    {"TEXTAREA", FSTokenType::TEXTAREA},
    {"DROPDOWN", FSTokenType::DROPDOWN},
    {"RADIO", FSTokenType::RADIO},
    {"CHECKBOX", FSTokenType::CHECKBOX},
    {"FILE", FSTokenType::FILE},
    {"ID", FSTokenType::ATTR_ID},
    {"REQUIRED", FSTokenType::REQUIRED},
    {"MIN", FSTokenType::ATTR_MIN},
    {"MAX", FSTokenType::ATTR_MAX},
    {"MAX_WORDS", FSTokenType::MAX_WORDS},
    {"OPTIONS", FSTokenType::OPTIONS},
    {"PLACEHOLDER", FSTokenType::PLACEHOLDER}
};

Lexer::Lexer(const std::string& src) : source(src), pos(0), line(1), column(1) {}

char Lexer::peek() const {
    if (pos >= source.size()) return '\0';
    return source[pos];
}

char Lexer::consume() {
    if (pos >= source.size()) return '\0';
    char c = source[pos++];
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return c;
}

void Lexer::skipWhitespace() {
    while (std::isspace(peek())) {
        consume();
    }
}

FSToken Lexer::readString() {
    consume(); // skip "
    int start_line = line;
    int start_col = column - 1;
    std::string val;
    while (peek() != '"' && peek() != '\0') {
        val += consume();
    }
    if (peek() == '"') consume(); // consume "
    return {FSTokenType::STRING_LITERAL, val, start_line, start_col};
}

FSToken Lexer::readIdentifierOrKeyword() {
    int start_line = line;
    int start_col = column;
    std::string val;
    while (std::isalnum(peek()) || peek() == '_' || peek() == '-') {
        val += consume();
    }
    
    auto it = KEYWORDS.find(val);
    if (it != KEYWORDS.end()) {
        return {it->second, val, start_line, start_col};
    }
    return {FSTokenType::IDENTIFIER, val, start_line, start_col};
}

FSToken Lexer::readNumber() {
    int start_line = line;
    int start_col = column;
    std::string val;
    while (std::isdigit(peek())) {
        val += consume();
    }
    return {FSTokenType::NUMBER_LITERAL, val, start_line, start_col};
}

std::vector<FSToken> Lexer::tokenize() {
    std::vector<FSToken> tokens;
    while (true) {
        skipWhitespace();
        char c = peek();
        if (c == '\0') break;

        if (c == '"') {
            tokens.push_back(readString());
        } else if (std::isalpha(c) || c == '_' || c == '-') {
            tokens.push_back(readIdentifierOrKeyword());
        } else if (std::isdigit(c)) {
            tokens.push_back(readNumber());
        } else if (c == '=') {
            tokens.push_back({FSTokenType::EQUALS, std::string(1, consume()), line, column - 1});
        } else if (c == '[') {
            tokens.push_back({FSTokenType::LBRACKET, std::string(1, consume()), line, column - 1});
        } else if (c == ']') {
            tokens.push_back({FSTokenType::RBRACKET, std::string(1, consume()), line, column - 1});
        } else if (c == ',') {
            tokens.push_back({FSTokenType::COMMA, std::string(1, consume()), line, column - 1});
        } else {
            // Error handling could be improved
            std::cerr << "Lexer error at line " << line << ", column " << column << ": unexpected character '" << c << "'" << std::endl;
            consume();
        }
    }
    tokens.push_back({FSTokenType::EOF_TOKEN, "", line, column});
    return tokens;
}
