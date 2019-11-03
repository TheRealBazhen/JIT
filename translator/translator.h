#ifndef TRANSLATOR_H_
#define TRANSLATOR_H_

#include <unordered_map>

#include "parser/parser.h"

namespace JIT {
    namespace translator {
        std::vector<uint32_t> GetARMCommandList(
            const std::vector<parser::Token>& postfix_notation_expression,
            const std::unordered_map<std::string, void*>& external_symbols);
    } // namespace translator
} // namespace JIT


typedef struct {
    const char *name;
    void *pointer;
} symbol_t;

extern "C" int
jit_compile_expression_to_arm(const char * expression,
                              const symbol_t * externs,
                              void * out_buffer);

#endif // TRANSLATOR_H_