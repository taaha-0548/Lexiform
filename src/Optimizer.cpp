#include "Optimizer.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>

// ============================================================================
// Phase 5: Optimization Passes — Implementation
// ============================================================================
// Two optimization passes operate on the flat IR instruction list:
//   Pass 1 — Dead Attribute Elimination (type-based pruning)
//   Pass 2 — Duplicate Attribute Folding (dead store elimination)
// Both passes remove instructions from the IR, reducing the work the code
// generator must perform and producing cleaner output.
// ============================================================================

// --- Helper: resolve what field type a temporary register holds ---------------

std::string Optimizer::resolveFieldType(const std::vector<IRInstruction>& program, int temp) {
    for (const auto& instr : program) {
        if (instr.opcode == IROpCode::FIELD_DEF && instr.resultTemp == temp) {
            return instr.arg1;  // arg1 is the field type string (e.g., "TEXT")
        }
    }
    return "";
}

// --- Helper: attribute-type compatibility table ------------------------------

bool Optimizer::isAttributeValidForType(const std::string& attr, const std::string& fieldType) {
    // PLACEHOLDER is only meaningful for input-like fields
    if (attr == "PLACEHOLDER") {
        return (fieldType == "TEXT" || fieldType == "EMAIL" ||
                fieldType == "PHONE" || fieldType == "NUMBER" ||
                fieldType == "TEXTAREA");
    }

    // MAX_WORDS only applies to TEXTAREA
    if (attr == "MAX_WORDS") {
        return (fieldType == "TEXTAREA");
    }

    // MIN and MAX only apply to NUMBER
    if (attr == "MIN" || attr == "MAX") {
        return (fieldType == "NUMBER");
    }

    // OPTIONS only applies to selection types
    if (attr == "OPTIONS") {
        return (fieldType == "DROPDOWN" || fieldType == "RADIO" || fieldType == "CHECKBOX");
    }

    // REQUIRED is valid on all field types
    if (attr == "REQUIRED") {
        return true;
    }

    // Unknown attributes are kept (conservative)
    return true;
}

// --- Pass 1: Dead Attribute Elimination --------------------------------------

int Optimizer::eliminateDeadAttributes(std::vector<IRInstruction>& program) {
    int removed = 0;

    // Identify SET_ATTR instructions that target an incompatible field type
    program.erase(
        std::remove_if(program.begin(), program.end(),
            [this, &program, &removed](const IRInstruction& instr) {
                // Only consider SET_ATTR_* instructions
                bool isSetAttr = (instr.opcode == IROpCode::SET_ATTR_STRING ||
                                  instr.opcode == IROpCode::SET_ATTR_INT ||
                                  instr.opcode == IROpCode::SET_ATTR_BOOL ||
                                  instr.opcode == IROpCode::SET_ATTR_LIST);
                if (!isSetAttr) return false;

                std::string fieldType = resolveFieldType(program, instr.targetTemp);
                if (fieldType.empty()) return false;  // Can't resolve — keep it

                if (!isAttributeValidForType(instr.arg1, fieldType)) {
                    removed++;
                    return true;  // Dead attribute — remove it
                }
                return false;
            }
        ),
        program.end()
    );

    return removed;
}

// --- Pass 2: Duplicate Attribute Folding (Dead Store Elimination) ------------

int Optimizer::foldDuplicateAttributes(std::vector<IRInstruction>& program) {
    int folded = 0;

    // Track which (temp, attrName) pairs we've seen, scanning backwards.
    // The LAST write to a given (temp, attr) wins; earlier writes are dead stores.
    std::unordered_set<std::string> seen;
    std::vector<bool> keep(program.size(), true);

    // Reverse scan: mark earlier duplicates for removal
    for (int i = static_cast<int>(program.size()) - 1; i >= 0; i--) {
        const auto& instr = program[i];
        bool isSetAttr = (instr.opcode == IROpCode::SET_ATTR_STRING ||
                          instr.opcode == IROpCode::SET_ATTR_INT ||
                          instr.opcode == IROpCode::SET_ATTR_BOOL ||
                          instr.opcode == IROpCode::SET_ATTR_LIST);

        if (isSetAttr) {
            // Composite key: "t<N>:<attrName>"
            std::string key = "t" + std::to_string(instr.targetTemp) + ":" + instr.arg1;

            if (seen.count(key)) {
                // This is a dead store — a later instruction overwrites it
                keep[i] = false;
                folded++;
            } else {
                seen.insert(key);
            }
        }
    }

    // Compact the program, removing dead stores
    std::vector<IRInstruction> optimized;
    optimized.reserve(program.size() - folded);
    for (size_t i = 0; i < program.size(); i++) {
        if (keep[i]) {
            optimized.push_back(program[i]);
        }
    }
    program = std::move(optimized);

    return folded;
}

// --- Main optimization pipeline ----------------------------------------------

OptimizationStats Optimizer::optimize(std::vector<IRInstruction>& program) {
    OptimizationStats stats;
    stats.totalInstructionsBefore = static_cast<int>(program.size());

    // Pass 1: Dead Attribute Elimination
    stats.deadAttrsRemoved = eliminateDeadAttributes(program);

    // Pass 2: Duplicate Attribute Folding
    stats.duplicateAttrsFolded = foldDuplicateAttributes(program);

    stats.totalInstructionsAfter = static_cast<int>(program.size());
    return stats;
}

// --- Statistics printer -------------------------------------------------------

std::string Optimizer::printStats(const OptimizationStats& stats) {
    std::ostringstream ss;
    ss << "=== Optimization Report ===" << std::endl;
    ss << "Instructions before: " << stats.totalInstructionsBefore << std::endl;
    ss << "Instructions after:  " << stats.totalInstructionsAfter << std::endl;
    ss << "Pass 1 — Dead Attribute Elimination:  " << stats.deadAttrsRemoved << " removed" << std::endl;
    ss << "Pass 2 — Duplicate Attribute Folding:  " << stats.duplicateAttrsFolded << " folded" << std::endl;
    int totalSaved = stats.totalInstructionsBefore - stats.totalInstructionsAfter;
    ss << "Total instructions eliminated: " << totalSaved << std::endl;
    ss << std::string(28, '-') << std::endl;
    return ss.str();
}
