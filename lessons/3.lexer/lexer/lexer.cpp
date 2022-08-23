#include "lexer.h"

#include <algorithm>
#include <charconv>
#include <unordered_map>

using namespace std;

namespace parse {

bool operator==(const Token& lhs, const Token& rhs) {
    using namespace token_type;

    if(lhs.index() != rhs.index()) {
        return false;
    }
    if (lhs.Is<Char>()) {
        return lhs.As<Char>().value == rhs.As<Char>().value;
    }
    if (lhs.Is<Number>()) {
        return lhs.As<Number>().value == rhs.As<Number>().value;
    }
    if (lhs.Is<String>()) {
        return lhs.As<String>().value == rhs.As<String>().value;
    }
    if (lhs.Is<Id>()) {
        return lhs.As<Id>().value == rhs.As<Id>().value;
    }
    return true;
}

bool operator!=(const Token& lhs, const Token& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, const Token& rhs) {
    using namespace token_type;

#define VALUED_OUTPUT(type) \
    if (auto p = rhs.TryAs<type>()) return os << #type << '{' << p->value << '}';

    VALUED_OUTPUT(Number);
    VALUED_OUTPUT(Id);
    VALUED_OUTPUT(String);
    VALUED_OUTPUT(Char);

#undef VALUED_OUTPUT

#define UNVALUED_OUTPUT(type) \
    if(rhs.Is<type>()) return os << #type;

    UNVALUED_OUTPUT(Class);
    UNVALUED_OUTPUT(Return);
    UNVALUED_OUTPUT(If);
    UNVALUED_OUTPUT(Else);
    UNVALUED_OUTPUT(Def);
    UNVALUED_OUTPUT(Newline);
    UNVALUED_OUTPUT(Print);
    UNVALUED_OUTPUT(Indent);
    UNVALUED_OUTPUT(Dedent);
    UNVALUED_OUTPUT(And);
    UNVALUED_OUTPUT(Or);
    UNVALUED_OUTPUT(Not);
    UNVALUED_OUTPUT(Eq);
    UNVALUED_OUTPUT(NotEq);
    UNVALUED_OUTPUT(LessOrEq);
    UNVALUED_OUTPUT(GreaterOrEq);
    UNVALUED_OUTPUT(None);
    UNVALUED_OUTPUT(True);
    UNVALUED_OUTPUT(False);
    UNVALUED_OUTPUT(Eof);

#undef UNVALUED_OUTPUT

    return os << "Unknown token :("sv;
}

Lexer::Lexer(std::istream& input)
    : input_(input)
{}

const Token& Lexer::CurrentToken() const {
    // Заглушка. Реализуйте метод самостоятельно
    throw std::logic_error("Not implemented"s);
}

Token Lexer::NextToken() {
    // Заглушка. Реализуйте метод самостоятельно
    throw std::logic_error("Not implemented"s);
}

Token LoadString(std::istream& input, bool is_double_quote) {
    return Token(token_type::String{});
}
Token LoadNumber(std::istream& input) {
    std::string parsed_sum;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_sum, &input] {
        parsed_sum += static_cast<char>(input.get());
        if(!input) {
            throw LexerError("Failed to read number from stream"s);
        }
    };

    auto read_digits = [&input, read_char] {
        if(!std::isdigit(input.peek())) {
            throw LexerError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    read_digits();

    try
    {
        return Token(token_type::Number{std::stoi(parsed_sum)});
    }
    catch(...)
    {
        throw LexerError("Failed to convert "s + parsed_sum + " to number"s);
    }
}

Token LoadId(std::istream& input) {

    return Token(token_type::Id{});
}

Token LoadToken(std::istream& input) {
    char c;
    if(!(input >> c)) {
        return Token{token_type::Eof()};
    }

    if(c == '\n') {
        return Token(token_type::Newline{});
    }

    if(std::isdigit(c)) {
        input.putback(c);
        return LoadNumber(input);
    }
    if(std::isalnum(c) || c == '_') {
        input.putback(c);
        return LoadId(input);
    }

    if(c == '-') {
        return LoadNumber(input);
    }

    if(c == '\'') {
        return LoadString(input, false);
    } 

    if(c == '\"') {
        return LoadString(input, true);
    }
        
    if(c == '+') {
        return Token(token_type::Char{'+'});
    }

    if(c == '-') {
        return Token(token_type::Char{'-'});
    }

    if(c == '*') {
        return Token(token_type::Char{'*'});
    }

    if(c == '/') {
        return Token(token_type::Char{'/'});
    }

    if(c == '>') {
        if(input.peek() == '=') {
            input.get();
            return Token(token_type::GreaterOrEq{});
        } else {
            return Token(token_type::Char{'>'});
        }
    }

    if(c == '<') {
        if(input.peek() == '=') {
            input.get();
            return Token(token_type::LessOrEq{});
        } else if (input.peek() == '>'){
            input.get();
            return Token(token_type::NotEq{});
        } else {
            return Token(token_type::Char{'<'});
        }
    }

    if(c == '=') {
        if(input.peek() == '=') {
            input.get();
            return Token(token_type::Eq{});
        } else {
            return Token(token_type::Char{'='});
        }
    }
    return Token(token_type::Id{});
}

} // namespace parse 