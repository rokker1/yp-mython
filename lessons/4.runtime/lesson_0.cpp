#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <memory>

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

struct Object {

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

class Executable {
public:
    virtual ~Executable() = default;
    virtual ObjectHolder Execute(Closure& closure, Context& context) = 0;
};

struct Method {
    // имя метода
    std::string name;
    // имена формальных параметров метода
    std::vector<std::string> formal_params;
    // тело метода
    std::unique_ptr<Executable> bode;

};

class Class : public Object {
public:
    explicit Class(std::string name, std::vector<Method> methods, const Class* parent);
    [[nodiscard]] const Method* GetMethod(const std::string& name) const;
};

class ClassInstance : public Object {
public:
    explicit ClassInstance(const Class& cls);
    ObjectHolder Call(const std::string& method, 
        const std::vector<ObjectHolder>& actual_args,
        Context& context
    );
    [[nodiscard]] bool HasMethod(const std::string& method, 
                size_t argument_count) const;

    [[nodiscard]] Closure& Fields();
    [[nodiscard]] const Closure& Fields() const;

};

int main() {
    return 0;
}