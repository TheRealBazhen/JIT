#include "parser/parser.h"

#include <cctype>
#include <stack>

namespace JIT {
    namespace parser {
        const char* missing_operator::what() const noexcept {
            return "Missing binary operation";
        }

        const char* missing_operand::what() const noexcept {
            return "Missing variable, number or function";
        }

        const char* missing_open_bracket::what() const noexcept {
            return "Missing '('";
        }

        const char* missing_close_bracket::what() const noexcept {
            return "Missing ')'";
        }

        unknown_symbol::unknown_symbol(char symb) {
            err_msg[16] = symb;
        }

        const char* unknown_symbol::what() const noexcept {
            return err_msg;
        }

        std::vector<Token> SplitToTokens(const std::string& input) {
            uint32_t pos = 0;
            std::vector<Token> result;

            while (pos < input.size()) {
                // Skip spaces
                while (pos < input.size() && isspace(input[pos])) {
                    ++pos;
                }
                if (pos >= input.size()) {
                    break;
                }

                // Symbol name
                if (isalpha(input[pos])) {
                    uint32_t name_size = 0;
                    while (pos + name_size < input.size() &&
                      (isalpha(input[pos + name_size]) || isdigit(input[pos + name_size]))) {
                        ++name_size;
                    }
                    std::string name = input.substr(pos, name_size);
                    pos += name_size;
                    if (pos >= input.size() || input[pos] != '(') {
                        // Variable
                        Token tok;
                        tok.type = Token::VARIABLE;
                        tok.variable.name = name;
                        result.push_back(tok);
                    } else {
                        // Function
                        // Count number of arguments
                        uint32_t scan_pos = pos + 1, depth = 1;
                        uint32_t num_arguments = 0;
                        uint32_t additional_content = 0; 
                        while (depth > 0 && scan_pos < input.size()) {
                            if (input[scan_pos] == '(') {
                                ++depth;
                            } else if (input[scan_pos] == ')') {
                                --depth;
                            } else {
                                if (input[scan_pos] == ',' && depth == 1) {
                                    ++num_arguments;
                                }
                                ++additional_content;
                            }
                            ++scan_pos;
                        }
                        if (depth > 0) {
                            throw missing_close_bracket();
                        }
                        if (additional_content > 0) {
                            ++num_arguments;
                        }

                        Token tok;
                        tok.type = Token::FUNCTION;
                        tok.function.name = name;
                        tok.function.num_arguments = num_arguments;
                        result.push_back(tok);
                    }
                } else if (isdigit(input[pos])) {
                    // Number
                    uint32_t number_size = 0;
                    while (pos + number_size < input.size() && isdigit(input[pos + number_size])) {
                        ++number_size;
                    }
                    int32_t number = std::atoi(input.substr(pos, number_size).c_str());
                    pos += number_size;
                    Token tok;
                    tok.type = Token::NUMBER;
                    tok.number = number;
                    result.push_back(tok); 
                } else {
                    if (input[pos] != '+' && input[pos] != '-' && input[pos] != '*' &&
                        input[pos] != '(' && input[pos] != ')' && input[pos] != ',') {
                        throw unknown_symbol(input[pos]);
                    }

                    Token tok;
                    tok.type = Token::OPERATION;
                    tok.operation = static_cast<Operation>(input[pos]);
                    result.push_back(tok);
                    ++pos;
                }
            }
            return result;
        }

        uint32_t GetPriority(Operation operation) {
            switch (operation) {
            case Operation::OPEN_BRACKET:
                return 0;

            case Operation::CLOSE_BRACKET:
            case Operation::COMMA:
                return 1;

            case Operation::PLUS:
            case Operation::MINUS:
                return 2;

            case Operation::MULTIPLY:
                return 3;

            case Operation::UNARY_MINUS:
                return 4;

            default:
                return 5;
            }
        }

        void DropOperators(std::vector<Token>& result, std::stack<Token>& operators, Operation oper) {
            while (!operators.empty()) {
                if (GetPriority(operators.top().operation) >= GetPriority(oper)) {
                    result.push_back(operators.top());
                    operators.pop();
                } else {
                    break;
                }
            }
        }

        std::vector<Token> ConvertToPostfixNotation(const std::vector<Token>& input) {
            enum State {
                WAIT_OPERATOR,
                WAIT_OPERAND
            } state = WAIT_OPERAND;
            std::vector<Token> result;
            std::stack<Token> operators;
            
            for (auto token : input) {
                if (state == WAIT_OPERAND) {
                    if (token.type == Token::OPERATION) {
                        // Bracket or unary minus
                        if (token.operation == Operation::MINUS) {
                            token.operation = Operation::UNARY_MINUS;
                        } else if (token.operation != Operation::OPEN_BRACKET &&
                                   token.operation != Operation::CLOSE_BRACKET) {
                            throw missing_operand();
                        }
                        operators.push(token);
                    } else if (token.type == Token::FUNCTION) {
                        operators.push(token);
                    } else {
                        // Symbol or number
                        result.push_back(token);
                        state = WAIT_OPERATOR;
                    }
                } else {
                    if (token.type != Token::OPERATION) {
                        throw missing_operator();
                    }
                    DropOperators(result, operators, token.operation);
                    if (token.operation == Operation::CLOSE_BRACKET) {
                        // Extract open bracket
                        if (operators.empty() ||
                            operators.top().type != Token::OPERATION ||
                            operators.top().operation != Operation::OPEN_BRACKET) {
                            throw missing_open_bracket();
                        }
                        operators.pop();
                        // Move function symbol (if it exists)
                        if (!operators.empty() && operators.top().type == Token::FUNCTION) {
                            result.push_back(operators.top());
                            operators.pop();
                        }
                        continue;
                    }
                    if (token.operation != Operation::COMMA) {
                        operators.push(token);
                    }
                    state = WAIT_OPERAND;
                }
            }
            DropOperators(result, operators, Operation::CLOSE_BRACKET);
            if (!operators.empty()) {
                throw missing_close_bracket();
            }
            return result;
        }
    } // namespace parser
} // namespace JIT