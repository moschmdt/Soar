# Constant Match Test Implementation Plan

## Overview
This document outlines the implementation of a new test type in Soar called the "constant match test" that uses the syntax `$ <variable>`. This test is designed to force literalization in chunking when a variable matches constant-typed values (strings, integers, floats).

## Feature Requirements

### Syntax
- Test syntax: `$ <variable>`
- Can only appear in attribute and/or value fields, never in ID fields
- Cannot appear on RHS of rules
- Example usage: `^attribute { <var> $ <var> }`

### Semantics
- Test evaluates to true if the variable matches a constant-typed symbol (STR_CONSTANT_SYMBOL_TYPE, INT_CONSTANT_SYMBOL_TYPE, FLOAT_CONSTANT_SYMBOL_TYPE)
- Sets a "force-literalize" flag on the test structure
- In chunking, causes the identity of the variable to be set to NULL, propagating literalization similar to singleton processing

## Implementation Components

### 1. Lexer Changes (lexer.h/lexer.cpp)
**Files to modify:**
- `Core/SoarKernel/src/parsing/lexer.h`
- `Core/SoarKernel/src/parsing/lexer.cpp`

**Changes needed:**
1. Add new lexeme type `DOLLAR_LEXEME` to enum in lexer.h (around line 51)
2. Add lexer routine `lex_dollar()` to handle '$' character
3. Update lexer_routines initialization in `init()` to map '$' to `lex_dollar`
4. Add '$' to constituent_char array if needed

**Implementation pattern:** Follow existing patterns like `lex_at()` for '@' symbol

### 2. Test Type Definition (enums.h)
**File to modify:**
- `Core/SoarKernel/src/shared/enums.h`

**Changes needed:**
1. Add new TestType: `CONSTANT_MATCH_TEST = 16` to enum (around line 368)
2. Update `NUM_TEST_TYPES` accordingly

### 3. Test Structure Extension (test.h)
**File to modify:**
- `Core/SoarKernel/src/soar_representation/test.h`

**Changes needed:**
1. Add `force_literalize` flag to test_struct (around line 38)
2. Update inline functions to handle new test type

### 4. Parser Changes (parser.cpp)
**File to modify:**
- `Core/SoarKernel/src/parsing/parser.cpp`

**Changes needed:**
1. Modify `parse_relational_test()` to handle DOLLAR_LEXEME (around line 366)
2. Add case for DOLLAR_LEXEME that:
   - Consumes the '$' token
   - Reads the following variable
   - Creates test with CONSTANT_MATCH_TEST type
   - Sets force_literalize flag to true

**Implementation pattern:** Follow AT_LEXEME/SMEM_LINK_TEST pattern (around line 408)

### 5. Test Creation and Management (test.cpp)
**File to modify:**
- `Core/SoarKernel/src/soar_representation/test.cpp`

**Changes needed:**
1. Update `hash_test()` to handle CONSTANT_MATCH_TEST (around line 538)
2. Update `tests_are_equal()` to handle new test type
3. Update `copy_test()` to preserve force_literalize flag
4. Update `deallocate_test()` if needed

### 6. Rete Integration (rete.h/rete.cpp)
**Files to modify:**
- `Core/SoarKernel/src/decision_process/rete.h`
- `Core/SoarKernel/src/decision_process/rete.cpp`

**Changes needed:**
1. Add relational rete test types in rete.h:
   - `RELATIONAL_CONSTANT_MATCH_RETE_TEST`
2. Add conversion functions for test type mapping
3. Add rete test routines in `init_rete()` (around line 9518)
4. Implement evaluation routines that check if symbol is constant
5. Update `add_rete_tests_for_test()` to handle CONSTANT_MATCH_TEST

### 7. Test Evaluation Logic
**New functions needed in rete.cpp:**
- `constant_match_rete_test_routine()` - checks if constant matches constant
- `variable_constant_match_rete_test_routine()` - checks if variable value is constant

**Implementation:**
```cpp
bool evaluate_constant_match_test(Symbol* sym) {
    return (sym->symbol_type == STR_CONSTANT_SYMBOL_TYPE ||
            sym->symbol_type == INT_CONSTANT_SYMBOL_TYPE ||
            sym->symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE);
}
```

### 8. Chunking Integration (ebc_unify.cpp, ebc_backtrace.cpp) - **CORRECTED**

**Critical Insight Discovered During Implementation:**
The constant match test (`$ <var>`) is a **constraint** on a variable, not a **binding**. In conjunctive tests like `{<> bart <x> $ <x>}`, there are separate test objects:
- EQUALITY_TEST for `<x>` (performs the actual variable binding)  
- CONSTANT_MATCH_TEST for `$ <x>` (constrains what `<x>` can bind to)

**Original Implementation Error:**
Initially attempted to literalize the identity of the CONSTANT_MATCH_TEST itself, which was incorrect.

**Corrected Implementation:**
The chunking logic must identify variables constrained by `$ <var>` tests and literalize the identities of the **EQUALITY_TEST** objects for those variables.

**Files to modify:**
- `Core/SoarKernel/src/explanation_based_chunking/ebc_backtrace.cpp`
- `Core/SoarKernel/src/explanation_based_chunking/ebc_unify.cpp`
- `Core/SoarKernel/src/explanation_based_chunking/ebc.h`

**Changes needed:**

1. **In ebc_backtrace.cpp** - Modified `add_to_grounds()`:
   - Added `check_for_constant_match_literalization(pCond)` call after singleton unification check

2. **In ebc_unify.cpp** - Implemented corrected algorithm:
   - `check_for_constant_match_literalization()` - Entry point that checks all condition tests
   - `has_constant_match_test()` - Recursively searches for CONSTANT_MATCH_TEST with force_literalize
   - `literalize_constant_match_tests()` - **Two-pass algorithm**:
     
     **Pass 1:** Collect all symbols referenced by `$ <var>` tests
     **Pass 2:** Find EQUALITY_TEST objects for those symbols and literalize their identities  
     **Pass 3:** Recursively process nested conjunctive tests

3. **In ebc.h** - Added function declarations

4. **In identity_record.cpp** - Added IDS_literalized_constant_match case handling

**Algorithm Details:**
```cpp
// Pass 1: Identify constrained variables
std::set<Symbol*> constrained_symbols;
for (cons* c = t->data.conjunct_list; c; c = c->rest) {
    if (sub_test->type == CONSTANT_MATCH_TEST && sub_test->force_literalize) {
        constrained_symbols.insert(sub_test->data.referent);
    }
}

// Pass 2: Literalize equality tests for constrained variables
for (cons* c = t->data.conjunct_list; c; c = c->rest) {
    if (sub_test->type == EQUALITY_TEST && sub_test->identity) {
        if (constrained_symbols.contains(sub_test->data.referent)) {
            sub_test->identity->literalize();
        }
    }
}
```

**Integration point:** Called from `add_to_grounds()` similar to `check_for_singleton_unification()`

**Expected Result:** 
For `^foo {<> bart <x> $ <x>}` where `<x>` matches "simpson":
- Chunk will contain `^foo simpson` (literalized)
- Not `^foo { $ <f2> <f2> <> bart <> simpson }` (variable preserved)

### 9. Rule Printing (soar_to_string.cpp)
**File to modify:**
- `Core/SoarKernel/src/output_manager/soar_to_string.cpp`

**Changes needed:**
1. Add case for CONSTANT_MATCH_TEST in `test_to_string()` (around line 145)
2. Add case for CONSTANT_MATCH_TEST in `test_type_to_string()` (around line 62)

**Implementation:**
```cpp
case CONSTANT_MATCH_TEST:
    destString += "$ ";
    destString += t->data.referent->to_string(true);
    break;
```

### 10. Production Reordering (production_reorder.cpp)
**File to modify:**
- `Core/SoarKernel/src/soar_representation/production_reorder.cpp`

**Changes needed:**
1. Update `reverse_direction_of_relational_test()` to handle CONSTANT_MATCH_TEST (around line 438)

## Implementation Phases

### Phase 1: Basic Parsing and Structure
1. Add lexer support for '$' token
2. Add test type definition
3. Add test structure extensions
4. Add basic parser support
5. Add rule printing support

### Phase 2: Rete Integration
1. Add rete test types and conversion functions
2. Implement evaluation routines
3. Update rete initialization

### Phase 3: Chunking Integration
1. Add constant match literalization logic
2. Integrate with add_to_grounds processing
3. Add identity nullification handling

### Phase 4: Testing and Validation
1. Create unit tests for parsing
2. Create integration tests for chunking behavior
3. Validate literalization behavior matches specification

## Testing Strategy

### Unit Tests
1. **Parser tests** - Verify '$' token parsing and test creation
2. **Rete tests** - Verify evaluation of constant match conditions
3. **Chunking tests** - Verify literalization behavior

### Integration Tests
1. **End-to-end test** comparing:
   - Rule with `^color { <var> $ <var> }` matching constant "red"
   - Expected chunking result equivalent to `^color red`

### Test Cases
```soar
# Test basic parsing
sp {test-constant-match-parsing
   (state <s> ^attribute { <var> $ <var> })
-->
   (<s> ^result success)
}

# Test chunking behavior
sp {test-constant-match-chunking
   (state <s> ^superstate <ss>)
   (<ss> ^attribute { <var> $ <var> })
-->
   (<s> ^result <var>)
}
```

## Error Handling

### Parse-time Errors
1. '$' without following variable
2. '$' in ID field
3. '$' on RHS

### Runtime Considerations
1. Variable not matching constant should make test fail normally
2. No special error handling beyond normal test evaluation

## Performance Considerations

### Minimal Impact
- New test type follows existing patterns
- Evaluation is simple constant type check
- Chunking impact only when test matches and literalization occurs

### Memory Usage
- Single additional flag per test structure
- No significant memory overhead

## Backward Compatibility

### Fully Compatible
- No changes to existing test types or behavior
- New syntax is additive only
- Existing rules unaffected

## Future Extensions

### Possible Enhancements
1. Support for specific constant types (e.g., `$int <var>`, `$string <var>`)
2. Negated version (matches non-constants)
3. Integration with other chunking mechanisms

## Dependencies

### Internal Dependencies
- Requires understanding of test evaluation in rete
- Requires understanding of identity propagation in chunking
- Requires coordination with existing literalization mechanisms

### External Dependencies
- None - purely internal Soar feature

## Documentation Updates

### Files to Update
1. Soar manual - add syntax documentation
2. Chunking documentation - explain literalization behavior
3. Grammar specification - add new test syntax

This implementation plan provides a comprehensive roadmap for adding the constant match test feature to Soar while maintaining compatibility and following existing architectural patterns.
