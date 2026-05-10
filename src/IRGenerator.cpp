#include "IRGenerator.hpp"
#include <iostream>
#include <iomanip>

// ============================================================================
// Phase 4: Intermediate Representation (IR) — Implementation
// ============================================================================
// This file implements the AST-to-IR lowering pass. The IRGenerator walks the
// AST in a single top-down traversal and emits a flat list of Three-Address
// Code instructions. Each AST node (Form, Section, Field) is assigned a fresh
// temporary register, and relationships are expressed via ATTACH instructions.
// ============================================================================

// --- Temporary register allocator -------------------------------------------

int IRGenerator::freshTemp() {
    return nextTemp++;
}

// --- Field type to string mapping -------------------------------------------

std::string IRGenerator::fieldTypeToIRString(FieldType type) {
    switch (type) {
        case FieldType::TEXT:       return "TEXT";
        case FieldType::EMAIL:      return "EMAIL";
        case FieldType::PHONE:      return "PHONE";
        case FieldType::NUMBER:     return "NUMBER";
        case FieldType::DATE:       return "DATE";
        case FieldType::TEXTAREA:   return "TEXTAREA";
        case FieldType::DROPDOWN:   return "DROPDOWN";
        case FieldType::RADIO:      return "RADIO";
        case FieldType::CHECKBOX:   return "CHECKBOX";
        case FieldType::FILE:       return "FILE";
        default:                    return "UNKNOWN";
    }
}

// --- Main IR generation pass ------------------------------------------------

std::vector<IRInstruction> IRGenerator::generate(const FormNode& form) {
    nextTemp = 0;  // Reset register counter for each compilation
    std::vector<IRInstruction> program;

    // FORM_DEF: allocate a temporary for the root form node
    int formTemp = freshTemp();
    {
        IRInstruction instr;
        instr.opcode = IROpCode::FORM_DEF;
        instr.resultTemp = formTemp;
        instr.arg1 = form.title;
        instr.arg2 = form.id;
        program.push_back(instr);
    }

    // Walk each section in the form
    for (const auto& section : form.sections) {
        int sectionTemp = freshTemp();

        // SECTION_DEF: allocate a temporary for this section
        {
            IRInstruction instr;
            instr.opcode = IROpCode::SECTION_DEF;
            instr.resultTemp = sectionTemp;
            instr.arg1 = section.title;
            program.push_back(instr);
        }

        // Walk each field in the section
        for (const auto& field : section.fields) {
            int fieldTemp = freshTemp();

            // FIELD_DEF: allocate a temporary for this field
            {
                IRInstruction instr;
                instr.opcode = IROpCode::FIELD_DEF;
                instr.resultTemp = fieldTemp;
                instr.arg1 = fieldTypeToIRString(field.type);
                instr.arg2 = field.label;
                instr.arg3 = field.id;
                program.push_back(instr);
            }

            // Emit SET_ATTR for each attribute on this field
            for (const auto& attr : field.attributes) {
                IRInstruction instr;
                instr.resultTemp = -1;       // Attribute ops don't produce a new temp
                instr.targetTemp = fieldTemp; // They modify an existing temp
                instr.arg1 = attr.name;

                // Use std::visit to determine the attribute value type
                std::visit([&instr](auto&& val) {
                    using T = std::decay_t<decltype(val)>;
                    if constexpr (std::is_same_v<T, std::string>) {
                        instr.opcode = IROpCode::SET_ATTR_STRING;
                        instr.arg2 = val;
                    } else if constexpr (std::is_same_v<T, int>) {
                        instr.opcode = IROpCode::SET_ATTR_INT;
                        instr.intValue = val;
                    } else if constexpr (std::is_same_v<T, bool>) {
                        instr.opcode = IROpCode::SET_ATTR_BOOL;
                        instr.boolValue = val;
                    } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
                        instr.opcode = IROpCode::SET_ATTR_LIST;
                        instr.listValue = val;
                    }
                }, attr.value);

                program.push_back(instr);
            }

            // ATTACH_FIELD: link this field to its parent section
            {
                IRInstruction instr;
                instr.opcode = IROpCode::ATTACH_FIELD;
                instr.resultTemp = -1;
                instr.targetTemp = sectionTemp;
                instr.intValue = fieldTemp;
                program.push_back(instr);
            }
        }

        // ATTACH_SECTION: link this section to the root form
        {
            IRInstruction instr;
            instr.opcode = IROpCode::ATTACH_SECTION;
            instr.resultTemp = -1;
            instr.targetTemp = formTemp;
            instr.intValue = sectionTemp;
            program.push_back(instr);
        }
    }

    // SET_SUBMIT: set the submit button label
    {
        IRInstruction instr;
        instr.opcode = IROpCode::SET_SUBMIT;
        instr.resultTemp = -1;
        instr.targetTemp = formTemp;
        instr.arg1 = form.submitLabel;
        program.push_back(instr);
    }

    // FORM_END: marks the end of the form definition
    {
        IRInstruction instr;
        instr.opcode = IROpCode::FORM_END;
        instr.resultTemp = -1;
        instr.targetTemp = formTemp;
        program.push_back(instr);
    }

    return program;
}

// --- Human-readable IR instruction formatting --------------------------------

std::string IRInstruction::toString() const {
    std::ostringstream ss;

    switch (opcode) {
        case IROpCode::FORM_DEF:
            ss << "t" << resultTemp << " = FORM_DEF \"" << arg1 << "\" " << arg2;
            break;
        case IROpCode::SECTION_DEF:
            ss << "t" << resultTemp << " = SECTION_DEF \"" << arg1 << "\"";
            break;
        case IROpCode::FIELD_DEF:
            ss << "t" << resultTemp << " = FIELD_DEF " << arg1 << " \"" << arg2 << "\" " << arg3;
            break;
        case IROpCode::SET_ATTR_STRING:
            ss << "    SET_ATTR t" << targetTemp << " " << arg1 << " = \"" << arg2 << "\"";
            break;
        case IROpCode::SET_ATTR_INT:
            ss << "    SET_ATTR t" << targetTemp << " " << arg1 << " = " << intValue;
            break;
        case IROpCode::SET_ATTR_BOOL:
            ss << "    SET_ATTR t" << targetTemp << " " << arg1 << " = " << (boolValue ? "true" : "false");
            break;
        case IROpCode::SET_ATTR_LIST:
            ss << "    SET_ATTR t" << targetTemp << " " << arg1 << " = [";
            for (size_t i = 0; i < listValue.size(); i++) {
                if (i > 0) ss << ", ";
                ss << "\"" << listValue[i] << "\"";
            }
            ss << "]";
            break;
        case IROpCode::ATTACH_FIELD:
            ss << "    ATTACH_FIELD t" << targetTemp << " <- t" << intValue;
            break;
        case IROpCode::ATTACH_SECTION:
            ss << "    ATTACH_SECTION t" << targetTemp << " <- t" << intValue;
            break;
        case IROpCode::SET_SUBMIT:
            ss << "    SET_SUBMIT t" << targetTemp << " \"" << arg1 << "\"";
            break;
        case IROpCode::FORM_END:
            ss << "FORM_END t" << targetTemp;
            break;
    }
    return ss.str();
}

// --- Full program pretty-printer ---------------------------------------------

std::string IRGenerator::printIR(const std::vector<IRInstruction>& program) {
    std::ostringstream ss;
    ss << "=== Lexiform IR (Three-Address Code) ===" << std::endl;
    ss << "Total instructions: " << program.size() << std::endl;
    ss << std::string(42, '-') << std::endl;
    for (size_t i = 0; i < program.size(); i++) {
        ss << std::setw(4) << i << ": " << program[i].toString() << std::endl;
    }
    ss << std::string(42, '-') << std::endl;
    return ss.str();
}
