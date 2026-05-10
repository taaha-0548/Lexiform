// ============================================================================
// Phase 2: Syntax Analysis (The Parser)
// ============================================================================
// The Parser is the second phase of the compiler pipeline. It consumes the
// token stream produced by the Lexer and validates it against the Lexiform
// grammar, building an Abstract Syntax Tree (AST) in the process.
//
// Implementation: Recursive Descent (Top-Down Parsing)
//   - Each grammar rule maps to a function: parseForm(), parseSection(), etc.
//   - The parser uses a single token of lookahead via peek().
//   - Error recovery is done via exceptions (std::runtime_error).
//
// Grammar handled (simplified):
//   Form    -> FORM STRING ID Section+ SUBMIT STRING END
//   Section -> SECTION STRING Field+
//   Field   -> FieldType STRING ID [ '[' AttrList ']' ]
//   AttrList-> Attr (',' Attr)*
//   Attr    -> NAME [ '=' Value ]
// ============================================================================

#include "Parser.hpp"
#include <iostream>

Parser::Parser(const std::vector<FSToken>& ts) : tokens(ts), pos(0) {}

FSToken Parser::peek() const {
    if (pos >= tokens.size()) return tokens.back();
    return tokens[pos];
}

FSToken Parser::consume() {
    FSToken t = peek();
    if (pos < tokens.size()) pos++;
    return t;
}

FSToken Parser::expect(FSTokenType type, const std::string& message) {
    if (peek().type == type) return consume();
    throw std::runtime_error("Parser error at line " + std::to_string(peek().line) + ": " + message);
}

bool Parser::match(FSTokenType type) {
    if (peek().type == type) {
        consume();
        return true;
    }
    return false;
}

// Entry point for parsing — begins the recursive descent from the top rule
FormNode Parser::parse() {
    return parseForm();
}

// Parse the top-level FORM rule: FORM "title" id <sections> SUBMIT "label" END
FormNode Parser::parseForm() {
    expect(FSTokenType::FORM, "Expected 'FORM'");
    std::string title = expect(FSTokenType::STRING_LITERAL, "Expected form title (string)").value;
    std::string id = expect(FSTokenType::IDENTIFIER, "Expected form ID").value;

    std::vector<SectionNode> sections;
    sections.push_back(parseSection()); // At least one section
    while (peek().type == FSTokenType::SECTION) {
        sections.push_back(parseSection());
    }

    expect(FSTokenType::SUBMIT, "Expected 'SUBMIT'");
    std::string submitLabel = expect(FSTokenType::STRING_LITERAL, "Expected submit label (string)").value;

    expect(FSTokenType::END, "Expected 'END'");

    return {title, id, sections, submitLabel};
}

// Parse a SECTION: SECTION "title" <fields>
// Each section must contain at least one field
SectionNode Parser::parseSection() {
    expect(FSTokenType::SECTION, "Expected 'SECTION'");
    std::string title = expect(FSTokenType::STRING_LITERAL, "Expected section title (string)").value;

    std::vector<FieldNode> fields;
    fields.push_back(parseField()); // At least one field
    while (peek().type >= FSTokenType::TEXT && peek().type <= FSTokenType::FILE) {
        fields.push_back(parseField());
    }

    return {title, fields};
}

// Map token types to AST field types (direct 1:1 mapping)
static FieldType toFieldType(FSTokenType type) {
    switch (type) {
        case FSTokenType::TEXT: return FieldType::TEXT;
        case FSTokenType::EMAIL: return FieldType::EMAIL;
        case FSTokenType::PHONE: return FieldType::PHONE;
        case FSTokenType::NUMBER: return FieldType::NUMBER;
        case FSTokenType::DATE: return FieldType::DATE;
        case FSTokenType::TEXTAREA: return FieldType::TEXTAREA;
        case FSTokenType::DROPDOWN: return FieldType::DROPDOWN;
        case FSTokenType::RADIO: return FieldType::RADIO;
        case FSTokenType::CHECKBOX: return FieldType::CHECKBOX;
        case FSTokenType::FILE: return FieldType::FILE;
        default: throw std::runtime_error("Unknown field type");
    }
}

// Parse a single field: <FieldType> "label" id ['[' attrs ']']
FieldNode Parser::parseField() {
    FSTokenType type = consume().type;
    std::string label = expect(FSTokenType::STRING_LITERAL, "Expected field label (string)").value;
    std::string id = expect(FSTokenType::IDENTIFIER, "Expected field ID").value;

    std::vector<Attribute> attributes;
    if (match(FSTokenType::LBRACKET)) {
        while (peek().type != FSTokenType::RBRACKET) {
            attributes.push_back(parseAttribute());
            if (peek().type == FSTokenType::COMMA) consume();
        }
        expect(FSTokenType::RBRACKET, "Expected ']' after attributes");
    }

    return {toFieldType(type), label, id, attributes};
}

Attribute Parser::parseAttribute() {
    FSToken attrToken = consume();
    std::string name = attrToken.value;
    
    if (match(FSTokenType::EQUALS)) {
        if (peek().type == FSTokenType::LBRACKET) {
            return {name, parseList()};
        } else if (peek().type == FSTokenType::NUMBER_LITERAL) {
            return {name, std::stoi(consume().value)};
        } else if (peek().type == FSTokenType::STRING_LITERAL) {
            return {name, consume().value};
        } else if (peek().type == FSTokenType::IDENTIFIER) {
            std::string val = consume().value;
            if (val == "true") return {name, true};
            if (val == "false") return {name, false};
            return {name, val};
        }
    }
    
    // Flag attributes (like REQUIRED)
    return {name, true};
}

std::vector<std::string> Parser::parseList() {
    expect(FSTokenType::LBRACKET, "Expected '[' for list");
    std::vector<std::string> list;
    while (peek().type != FSTokenType::RBRACKET) {
        if (peek().type == FSTokenType::STRING_LITERAL || peek().type == FSTokenType::IDENTIFIER) {
            list.push_back(consume().value);
        } else {
            throw std::runtime_error("Expected string or identifier in list");
        }
        if (peek().type == FSTokenType::COMMA) consume();
    }
    expect(FSTokenType::RBRACKET, "Expected ']' after list");
    return list;
}
