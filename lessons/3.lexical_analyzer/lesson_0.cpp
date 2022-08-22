#include <iostream>
#include <variant>
#include <string>

namespace parse {

namespace token_type {
struct Number {
    // лексема "целое число"
    int value;
};
struct Id { // лексема "идентификатор"
    std::string value;
};

struct Class { // лексема "class"
    
};
struct Return { // лексема "return"

};

} // namespace token_type

using TokenBase = ::std::variant<
    token_type::Number,
    token_type::Id,
    token_type::Class,
    token_type::Return
    
    >;

struct Token : TokenBase {
    using TokenBase::TokenBase;

    template <typename T>
    [[nodiscard]] bool Is() const {
        return std::holds_alternative<T>(*this);
    }

    template <typename T>
    [[nodiscard]] const T& As() const {
        return std::get<T>(*this);
    }

    template <typename T>
    [[nodiscard]] const T& TryAs() const {
        return std::get_if<T>(*this);
    }

};

} // namespace parse

int main() {
    using parse::Token;

    Token t;
    if(t.Is<parse::token_type::Class>()) {
        return 0;
    }

    return 0;
}
