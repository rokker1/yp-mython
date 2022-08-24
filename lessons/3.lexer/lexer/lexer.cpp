#include "lexer.h"

#include <algorithm>
#include <charconv>


using namespace std;

namespace parse {


static const unordered_map<std::string, Token> mython_keywords = {
    {"class"s, Token{token_type::Class{}}},
    {"return"s, Token{token_type::Return{}}}, 
    {"if"s, Token{token_type::If{}}}, 
    {"else"s, Token{token_type::Else{}}}, 
    {"def"s, Token{token_type::Def{}}}, 
    {"print"s, Token{token_type::Print{}}}, 
    {"or"s, Token{token_type::Or{}}}, 
    {"None"s, Token{token_type::None{}}}, 
    {"and"s, Token{token_type::And{}}}, 
    {"not"s, Token{token_type::Not{}}}, 
    {"True"s, Token{token_type::True{}}}, 
    {"False"s, Token{token_type::False{}}}
};

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
    , current_token_(LoadToken(input))
{
    
}

const Token& Lexer::CurrentToken() const {
    return current_token_;
}

Token Lexer::NextToken() {
    current_token_ = LoadToken(input_);
    return current_token_;
}

Token LoadString(std::istream& input, char exit_quote) {
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    
    while(true) {
        if(it  == end) {
            throw LexerError("String parsing error"s);
        }
        const char ch = *it;
        if (ch == exit_quote) {
            ++it;
            break;
        } else if (ch == '\\') {
            ++it;
            if(it  == end) {
                throw LexerError("String parsing error"s);
            }
            const char escaped_char = *(it);
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case '\'':
                    s.push_back('\'');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                
                default:
                    throw LexerError("Unrecognized escape sequence \\"s + escaped_char);
            }

        } else if (ch == '\n' || ch == '\r') {
            throw LexerError("Unexpected end of line"s);
        } else {
            s.push_back(ch);
        }
        ++it;
    }

    return Token(token_type::String{std::move(s)});
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
    std::string s;
    char c;
    while (c = input.peek(), std::isalnum(c) || c == '_') {
        s.push_back(static_cast<char>(input.get()));
    }
    if(mython_keywords.count(s)) {
        return mython_keywords.at(s);
    }
    return Token(token_type::Id{s});
}

Token LoadToken(std::istream& input) {
    // если токен пустой (читаем первый токен)
    // или последний - текущий токен - новая линия - 
    // прочитать отступ
    // сравнить с хранимым отступом
    // обновить хранимый отступ
    // вернуть токен с отступом

    char c;
    if(input.peek() == '\n') {
        input.get();
        return Token(token_type::Newline{});
    }
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

    if(std::isalpha(c) || c == '_') {
        input.putback(c);
        return LoadId(input);
    }

    if(c == '-') {
        return Token(token_type::Char{'-'});
    }

    if(c == '\'') {
        return LoadString(input, '\'');
    } 

    if(c == '"') {
        return LoadString(input, '"');
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
    if(c == '!') {
        if(input.peek() == '=') {
            input.get();
            return Token(token_type::NotEq{});
        } else {
            return Token(token_type::Char{'!'});
        }
    }
    return Token(token_type::Id{});
}

} // namespace parse 