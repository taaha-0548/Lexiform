#pragma once
#include "IRGenerator.hpp"
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>

// ============================================================================
// Phase 5: Optimization Passes
// ============================================================================
// The Optimizer operates on the linearized IR (Three-Address Code) produced by
// Phase 4. It applies a sequence of optimization passes that simplify, clean,
// and reduce the IR before it is consumed by the code generator.
//
// Currently implemented optimizations:
//   1. Dead Attribute Elimination  — Removes SET_ATTR instructions whose
//      attribute has no semantic effect on the target field type (e.g.,
//      PLACEHOLDER on a CHECKBOX, MAX_WORDS on a TEXT field).
//   2. Duplicate Attribute Folding — When the same attribute is set multiple
//      times on the same field, only the last assignment is kept (the earlier
//      ones are "dead stores" that get folded away).
// ============================================================================

/**
 * Stores statistics about what each optimization pass removed, used to
 * display before/after comparisons in --debug output.
 */
struct OptimizationStats {
    int deadAttrsRemoved = 0;
    int duplicateAttrsFolded = 0;
    int totalInstructionsBefore = 0;
    int totalInstructionsAfter = 0;
};

/**
 * Applies optimization passes to the IR instruction list.
 * Each pass takes the IR by reference and modifies it in-place.
 */
class Optimizer {
public:
    /**
     * Run all optimization passes in sequence.
     * @param program  The IR instruction list (modified in-place)
     * @return         Statistics about what was optimized
     */
    OptimizationStats optimize(std::vector<IRInstruction>& program);

    /**
     * Format optimization statistics as a human-readable report.
     */
    static std::string printStats(const OptimizationStats& stats);

private:
    /**
     * Pass 1: Dead Attribute Elimination
     * Removes SET_ATTR instructions where the attribute is not applicable
     * to the field type it targets. This catches cases the semantic analyzer
     * might allow but that produce no useful output.
     *
     * Examples of dead attributes:
     *   - PLACEHOLDER on CHECKBOX, RADIO, FILE, DROPDOWN
     *   - MAX_WORDS on anything except TEXTAREA
     *   - MIN/MAX on anything except NUMBER
     */
    int eliminateDeadAttributes(std::vector<IRInstruction>& program);

    /**
     * Pass 2: Duplicate Attribute Folding (Dead Store Elimination)
     * When the same attribute is set multiple times on the same temporary
     * register, only the last assignment survives. Earlier assignments are
     * redundant "dead stores" that can be safely removed.
     *
     * Example:
     *   SET_ATTR t2 PLACEHOLDER = "Hello"   <-- removed (dead store)
     *   SET_ATTR t2 PLACEHOLDER = "World"   <-- kept (last write wins)
     */
    int foldDuplicateAttributes(std::vector<IRInstruction>& program);

    // Helper: resolve the field type string for a given temp register
    std::string resolveFieldType(const std::vector<IRInstruction>& program, int temp);

    // Helper: check if an attribute is valid for a given field type
    bool isAttributeValidForType(const std::string& attr, const std::string& fieldType);
};
