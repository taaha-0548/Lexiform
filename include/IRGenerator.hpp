#pragma once
#include "AST.hpp"
#include <string>
#include <vector>
#include <sstream>

// ============================================================================
// Phase 4: Intermediate Representation (IR)
// ============================================================================
// The IR is a linearized, Three-Address Code (TAC) style representation of the
// form structure. Each instruction uses at most three operands and a temporary
// register (tN). This flat format decouples the hierarchical AST from the
// final code generation target (JSON), enabling optimization passes to operate
// on a simple instruction list rather than a recursive tree.
// ============================================================================

/**
 * Operation codes for the Lexiform Intermediate Representation.
 * Each opcode maps to a single logical action in the compilation pipeline.
 */
enum class IROpCode {
    // Structure operations
    FORM_DEF,           // tN = FORM_DEF "title" id
    SECTION_DEF,        // tN = SECTION_DEF "title"
    FIELD_DEF,          // tN = FIELD_DEF <type> "label" id

    // Attribute operations (Three-Address: target, attribute, value)
    SET_ATTR_STRING,    // SET_ATTR tN attrName "stringValue"
    SET_ATTR_INT,       // SET_ATTR tN attrName intValue
    SET_ATTR_BOOL,      // SET_ATTR tN attrName boolValue
    SET_ATTR_LIST,      // SET_ATTR tN attrName [item1, item2, ...]

    // Linking operations (attach child to parent)
    ATTACH_FIELD,       // ATTACH_FIELD parentTemp childTemp
    ATTACH_SECTION,     // ATTACH_SECTION formTemp sectionTemp

    // Terminal
    SET_SUBMIT,         // SET_SUBMIT tN "label"
    FORM_END            // FORM_END tN
};

/**
 * A single IR instruction in Three-Address Code format.
 * Each instruction produces a result in `resultTemp` (if applicable)
 * and operates on up to three arguments.
 */
struct IRInstruction {
    IROpCode opcode;
    int resultTemp;                     // Temporary register index (-1 if none)
    std::string arg1;                   // Primary argument (name, label, type)
    std::string arg2;                   // Secondary argument (ID, attribute name)
    std::string arg3;                   // Tertiary argument (string value)
    int intValue = 0;                   // Integer value for SET_ATTR_INT
    bool boolValue = false;             // Boolean value for SET_ATTR_BOOL
    std::vector<std::string> listValue; // List value for SET_ATTR_LIST
    int targetTemp = -1;                // Target temp for ATTACH operations

    // Human-readable representation of this instruction
    std::string toString() const;
};

/**
 * Generates a linearized Three-Address Code IR from the validated AST.
 *
 * The IR generation process:
 * 1. Walk the AST top-down (Form -> Section -> Field -> Attribute)
 * 2. Assign a fresh temporary register for each structural node
 * 3. Emit SET_ATTR instructions for each attribute
 * 4. Emit ATTACH instructions to link children to parents
 *
 * This produces a flat list of instructions that can be optimized
 * before being consumed by the code generator.
 */
class IRGenerator {
public:
    /**
     * Convert a validated AST (FormNode) into a list of IR instructions.
     * @param form  The root AST node (must have passed semantic analysis)
     * @return      Ordered list of IR instructions
     */
    std::vector<IRInstruction> generate(const FormNode& form);

    /**
     * Pretty-print the entire IR program to a string.
     * Useful for the --debug flag output.
     */
    static std::string printIR(const std::vector<IRInstruction>& program);

private:
    int nextTemp = 0;   // Counter for generating fresh temporary names

    int freshTemp();    // Allocate the next temporary register
    std::string fieldTypeToIRString(FieldType type);
};
