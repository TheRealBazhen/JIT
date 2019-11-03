#include "translator/translator.h"

#include <stack>
#include <vector>

#include <iostream>

namespace JIT {
    namespace translator {
        // ARM assembler code codes
        namespace command_code {
            uint32_t ADD_R0_R0_R1 = 0xE0800001;
            uint32_t SUB_R0_R0_R1 = 0xE0400001;
            uint32_t MUL_R0_R0_R1 = 0xE0000190;
            uint32_t PUSH_R0 = 0xE52D0004;
            // Commands like pop {ri}
            uint32_t POP[4] = {
                0xE49D0004,
                0xE49D1004,
                0xE49D2004,
                0xE49D3004
            };
            // Commands like movw ri, #0 
            uint32_t MOVW[5] = {
                0xE3000000,
                0xE3001000,
                0xE3002000,
                0xE3003000,
                0xE3004000
            };
            // Commands like movt ri, #0
            uint32_t MOVT[5] = {
                0xE3400000,
                0xE3401000,
                0xE3402000,
                0xE3403000,
                0xE3404000
            };
            // Commands like ldr ri, [ri]
            uint32_t LDR[5] = {
                0xE5900000,
                0xE5911000,
                0xE5922000,
                0xE5933000,
                0xE5944000
            };
            uint32_t BLX_R4 = 0xE12FFF34;
            uint32_t PUSH_R4_LR = 0xE92D4010;
            uint32_t POP_R4_LR = 0xE8BD4010;
            uint32_t BX_LR = 0xE12FFF1E;
        } // namespace command_code

        uint32_t AdaptConstantToWrite(uint16_t constant) {
            // separate four bits: 0xabcd -> 0xa0bcd
            return ((constant >> 12) << 16) | (constant & ((1 << 12) - 1)); 
        }

        void SetConstant(std::vector<uint32_t>& command_list, uint32_t reg_number, uint32_t constant) {
            uint32_t upper_part = (constant >> 16),
                     lower_part = (constant & ((1 << 16) - 1));
            upper_part = AdaptConstantToWrite(upper_part);
            lower_part = AdaptConstantToWrite(lower_part);
            command_list.push_back(command_code::MOVW[reg_number] | lower_part);
            command_list.push_back(command_code::MOVT[reg_number] | upper_part);
        }

        void LoadVariable(std::vector<uint32_t>& command_list, uint32_t reg_number, void* var_pointer) {
            SetConstant(command_list, reg_number, reinterpret_cast<uint32_t>(var_pointer));
            command_list.push_back(command_code::LDR[reg_number]);
        }

        void CallFunction(std::vector<uint32_t>& command_list, void* func_pointer) {
            SetConstant(command_list, 4, reinterpret_cast<uint32_t>(func_pointer));
            command_list.push_back(command_code::BLX_R4);
        }

        void CallFunction(std::vector<uint32_t>& command_list, void* func_pointer,
                          uint32_t num_arguments) {
            for (uint32_t i = num_arguments; i > 0; --i) {
                command_list.push_back(command_code::POP[i - 1]);
            }
            CallFunction(command_list, func_pointer);
            // Save result
            command_list.push_back(command_code::PUSH_R0);
        }

        void CompleteBinaryOperation(std::vector<uint32_t>& command_list, parser::Operation operation) {
            command_list.push_back(command_code::POP[1]);
            command_list.push_back(command_code::POP[0]);
            switch (operation) {
            case parser::Operation::PLUS:
                command_list.push_back(command_code::ADD_R0_R0_R1);
                break;

            case parser::Operation::MINUS:
                command_list.push_back(command_code::SUB_R0_R0_R1);;
                break;

            case parser::Operation::MULTIPLY:
                command_list.push_back(command_code::MUL_R0_R0_R1);;
                break;
            }
            // save result
            command_list.push_back(command_code::PUSH_R0);
        }

        void CompleteUnaryMinus(std::vector<uint32_t>& command_list) {
            SetConstant(command_list, 0, 0);
            command_list.push_back(command_code::POP[1]);
            command_list.push_back(command_code::SUB_R0_R0_R1);
            command_list.push_back(command_code::PUSH_R0);
        }

        std::vector<uint32_t> GetARMCommandList(
            const std::vector<parser::Token>& postfix_notation_expression,
            const std::unordered_map<std::string, void*>& external_symbols) {
            std::vector<uint32_t> command_list;

            command_list.push_back(command_code::PUSH_R4_LR);
            for (const auto& token : postfix_notation_expression) {
                if (token.type == parser::Token::NUMBER) {
                    SetConstant(command_list, 0, token.number);
                    command_list.push_back(command_code::PUSH_R0);
                } else if (token.type == parser::Token::VARIABLE) {
                    LoadVariable(command_list, 0, external_symbols.at(token.variable.name));
                    command_list.push_back(command_code::PUSH_R0);
                } else if (token.type == parser::Token::FUNCTION) {
                    CallFunction(command_list, external_symbols.at(token.function.name),
                                 token.function.num_arguments);
                } else {
                    if (token.operation == parser::Operation::UNARY_MINUS) {
                        CompleteUnaryMinus(command_list);
                    } else {
                        CompleteBinaryOperation(command_list, token.operation);
                    }
                }
            }
            //command_list.push_back(command_code::POP[0]);
            command_list.pop_back();
            command_list.push_back(command_code::POP_R4_LR);
            command_list.push_back(command_code::BX_LR);
            return command_list;
        }
    } // namespace translator
} // namespace JIT

extern "C" int
jit_compile_expression_to_arm(const char * expression,
                              const symbol_t * externs,
                              void * out_buffer) {
    try {
        auto splitted_expr = JIT::parser::SplitToTokens(expression);
        auto postfix_notation = JIT::parser::ConvertToPostfixNotation(splitted_expr);
        std::unordered_map<std::string, void*> externs_map;
        while (externs->name != nullptr && externs->pointer != nullptr) {
            externs_map[externs->name] = externs->pointer;
            ++externs;
        }
        auto command_list = JIT::translator::GetARMCommandList(postfix_notation, externs_map);
        uint32_t* out = static_cast<uint32_t*>(out_buffer);
        for (uint32_t i = 0; i < command_list.size(); ++i) {
            *out = command_list[i];
            ++out;
        }
        return 1;
    } catch (std::exception& error) {
        std::cout << "Parser error: " << error.what() << std::endl;
        return 0;
    }
}