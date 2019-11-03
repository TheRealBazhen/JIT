#ifndef PARSER_H_
#define PARSER_H_

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace JIT {
    namespace parser {
        // Parser errors
        class missing_operator : public std::exception {
        public:
            const char* what() const noexcept override;
        };

        class missing_operand : public std::exception {
        public:
            const char* what() const noexcept override;
        };

        class missing_open_bracket : public std::exception {
        public:
            const char* what() const noexcept override;
        };

        class missing_close_bracket : public std::exception {
        public:
            const char* what() const noexcept override;
        };

        class unknown_symbol : public std::exception {
        public:
            unknown_symbol(char symb);
            const char* what() const noexcept override;
        private:
            char err_msg[19] = "Unknown symbol ' '";
        };

        struct Variable {
            std::string name;
        };

        struct Function {
            std::string name;
            uint32_t num_arguments;
        };

        enum struct Operation {
            PLUS = '+',
            MINUS = '-',
            MULTIPLY = '*',
            UNARY_MINUS = '@', // Just something
            OPEN_BRACKET = '(',
            CLOSE_BRACKET = ')',
            COMMA = ','
        };

        struct Token {
            enum Type {
                VARIABLE,
                FUNCTION,
                NUMBER,
                OPERATION
            } type;
            Variable variable;
            Function function;
            int32_t number;
            Operation operation;
        };

        std::vector<Token> SplitToTokens(const std::string& input);
        std::vector<Token> ConvertToPostfixNotation(const std::vector<Token>& input);
    } // namespace parser
} // namespace JIT

#endif // PARSER_H_