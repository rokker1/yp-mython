#include <iostream>
#include <sstream>
#include <unordered_map>

class Context {
public:
    virtual std::ostream& GetOutputStream() = 0;
protected:
    ~Context() = default;
};

struct DummyContext : Context {
    std::ostream& GetOutputStream() override {
        return output;
    }

    std::ostringstream output; // Юнит-тесты могут проверять содержимое потока output
};

class ObjectHolder {
public:
    ObjectHolder() = default;

    template <typename T>
    [[nodiscard]] static ObjectHolder Own(T&& object);

    [[nodiscard]] static ObjectHolder Share(Object& object);
    [[nodiscard]] static ObjectHolder None();

    Object& operator*() const;
    Object* operator->() const;
    [[nodiscard]] Object* Get() const;

    template <typename T>
    [[nodiscard]] T* TryAs() const;

    explicit operator bool() const;
    
private:

};

using Closure = std::unordered_map<std::string, ObjectHolder>;