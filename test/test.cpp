#include <sys/mman.h>

#include "gtest/gtest.h"

#include "translator/translator.h"

typedef int (*function_t)();

TEST(TokenSplitter, NumberParse) {
    auto first_expr = JIT::parser::SplitToTokens("1");
    EXPECT_EQ(first_expr.size(), 1);
    EXPECT_EQ(first_expr[0].type, JIT::parser::Token::NUMBER);
    EXPECT_EQ(first_expr[0].number, 1);
}

TEST(TokenSplitter, OperationParse) {
    char opers[] = {'+', '-', '*', '(', ')'};
    for (int i = 0; i < 5; ++i) {
        char source_expr[2] = {opers[i], 0};
        auto first_expr = JIT::parser::SplitToTokens(source_expr);
        EXPECT_EQ(first_expr.size(), 1);
        EXPECT_EQ(first_expr[0].type, JIT::parser::Token::OPERATION);
        EXPECT_EQ(static_cast<char>(first_expr[0].operation), opers[i]);
    }
}

TEST(TokenSplitter, FunctionParse) {
    auto first_expr = JIT::parser::SplitToTokens("f()");
    EXPECT_GT(first_expr.size(), 1);
    EXPECT_EQ(first_expr[0].type, JIT::parser::Token::FUNCTION);
    EXPECT_EQ(first_expr[0].function.name, "f");
    EXPECT_EQ(first_expr[0].function.num_arguments, 0);
    auto second_expr = JIT::parser::SplitToTokens("f(1, 2)");
    EXPECT_GT(second_expr.size(), 1);
    EXPECT_EQ(second_expr[0].type, JIT::parser::Token::FUNCTION);
    EXPECT_EQ(second_expr[0].function.name, "f");
    EXPECT_EQ(second_expr[0].function.num_arguments, 2);
}

TEST(TokenSplitter, Trash) {
    EXPECT_ANY_THROW(JIT::parser::SplitToTokens("1+f(2,3"));
    EXPECT_ANY_THROW(JIT::parser::SplitToTokens("+@"));
}

TEST(Parser, SimpleOperations) {
    auto splitted = JIT::parser::SplitToTokens("1+2*3");
    auto postfix = JIT::parser::ConvertToPostfixNotation(splitted);

    EXPECT_EQ(postfix.size(), 5);
    EXPECT_EQ(postfix[0].type, JIT::parser::Token::NUMBER);
    EXPECT_EQ(postfix[0].number, 1);
    EXPECT_EQ(postfix[1].type, JIT::parser::Token::NUMBER);
    EXPECT_EQ(postfix[1].number, 2);
    EXPECT_EQ(postfix[2].type, JIT::parser::Token::NUMBER);
    EXPECT_EQ(postfix[2].number, 3);
    EXPECT_EQ(postfix[3].type, JIT::parser::Token::OPERATION);
    EXPECT_EQ(postfix[3].operation, JIT::parser::Operation::MULTIPLY);
    EXPECT_EQ(postfix[4].type, JIT::parser::Token::OPERATION);
    EXPECT_EQ(postfix[4].operation, JIT::parser::Operation::PLUS);
}

TEST(Parser, UnaryMinus) {
    auto splitted = JIT::parser::SplitToTokens("-(1+2*3)");
    auto postfix = JIT::parser::ConvertToPostfixNotation(splitted);
    EXPECT_EQ(postfix.back().type, JIT::parser::Token::OPERATION);
    EXPECT_EQ(postfix.back().operation, JIT::parser::Operation::UNARY_MINUS);
}

TEST(Parser, Brackets) {
    auto splitted = JIT::parser::SplitToTokens("(1+2)*3");
    auto postfix = JIT::parser::ConvertToPostfixNotation(splitted);

    EXPECT_EQ(postfix.size(), 5);
    EXPECT_EQ(postfix[0].type, JIT::parser::Token::NUMBER);
    EXPECT_EQ(postfix[0].number, 1);
    EXPECT_EQ(postfix[1].type, JIT::parser::Token::NUMBER);
    EXPECT_EQ(postfix[1].number, 2);
    EXPECT_EQ(postfix[2].type, JIT::parser::Token::OPERATION);
    EXPECT_EQ(postfix[2].operation, JIT::parser::Operation::PLUS);
    EXPECT_EQ(postfix[3].type, JIT::parser::Token::NUMBER);
    EXPECT_EQ(postfix[3].number, 3);
    EXPECT_EQ(postfix[4].type, JIT::parser::Token::OPERATION);
    EXPECT_EQ(postfix[4].operation, JIT::parser::Operation::MULTIPLY);
}

TEST(Parser, Functions) {
    auto splitted = JIT::parser::SplitToTokens("f(1+2,3)");
    auto postfix = JIT::parser::ConvertToPostfixNotation(splitted);

    EXPECT_EQ(postfix.size(), 5);
    EXPECT_EQ(postfix[0].type, JIT::parser::Token::NUMBER);
    EXPECT_EQ(postfix[0].number, 1);
    EXPECT_EQ(postfix[1].type, JIT::parser::Token::NUMBER);
    EXPECT_EQ(postfix[1].number, 2);
    EXPECT_EQ(postfix[2].type, JIT::parser::Token::OPERATION);
    EXPECT_EQ(postfix[2].operation, JIT::parser::Operation::PLUS);
    EXPECT_EQ(postfix[3].type, JIT::parser::Token::NUMBER);
    EXPECT_EQ(postfix[3].number, 3);
    EXPECT_EQ(postfix[4].type, JIT::parser::Token::FUNCTION);
    EXPECT_EQ(postfix[4].function.name, "f");
    EXPECT_EQ(postfix[4].function.num_arguments, 2);
}

TEST(Parser, MissingBrackets) {
    std::string incorrect_samples[] = {"(((1+2)*3+4)", "(1+2*3", "1+2)*3"};

    for (const auto& sample : incorrect_samples) {
        EXPECT_ANY_THROW(JIT::parser::ConvertToPostfixNotation(JIT::parser::SplitToTokens(sample)));
    }
}

TEST(Parser, MissingOperator) {
    std::string incorrect_samples[] = {"1+2 3", "f(a 1, 2)"};

    for (const auto& sample : incorrect_samples) {
        EXPECT_ANY_THROW(JIT::parser::ConvertToPostfixNotation(JIT::parser::SplitToTokens(sample)));
    }
}

TEST(Parser, MissingOperand) {
    std::string incorrect_samples[] = {"1+*3", "f(a-, 2)"};

    for (const auto& sample : incorrect_samples) {
        EXPECT_ANY_THROW(JIT::parser::ConvertToPostfixNotation(JIT::parser::SplitToTokens(sample)));
    }
}

int32_t a = 0, b = 1, c = 2, d = 239;

int32_t sum(int32_t a, int32_t b, int32_t c) {
    return a + b + c;
}

int32_t dec(int32_t a) {
    return a - 1;
}

symbol_t symbols[] =
{
    {"a", &a},
    {"b", &b},
    {"c", &c},
    {"d", &d},
    {"sum", reinterpret_cast<void*>(sum)},
    {"dec", reinterpret_cast<void*>(dec)},
    {nullptr, nullptr}
};

static void* InitCodeBuffer()
{
    void* result = mmap(0,
                        4096,
                        PROT_READ | PROT_WRITE | PROT_EXEC,
                        MAP_PRIVATE | MAP_ANON,
                        0,
                        0);
    if (result == nullptr) {
        throw std::runtime_error("Failed to init code buffer");
    }
    return result;
}

void FreeCodeBuffer(void* buf) {
    munmap(buf, 4096);
}

int32_t Execute(const std::string &expr) {
    void* buf = InitCodeBuffer();
    try {
        jit_compile_expression_to_arm(expr.c_str(), symbols, buf);
        function_t func = reinterpret_cast<function_t>(buf);
        int32_t result = func();
        FreeCodeBuffer(buf);
        return result;
    } catch (...) {
        FreeCodeBuffer(buf);
        throw;
    }
}

TEST(Translator, Corectness) {
    EXPECT_EQ(Execute("sum(2+3*dec(d), a)-(-c)"), 718);
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}