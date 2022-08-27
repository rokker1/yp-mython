#include "runtime.h"

#include <cassert>
#include <optional>
#include <sstream>

using namespace std;

namespace runtime {

ObjectHolder::ObjectHolder(std::shared_ptr<Object> data)
    : data_(std::move(data)) {
}

void ObjectHolder::AssertIsValid() const {
    assert(data_ != nullptr);
}

ObjectHolder ObjectHolder::Share(Object& object) {
    // Возвращаем невладеющий shared_ptr (его deleter ничего не делает)
    return ObjectHolder(std::shared_ptr<Object>(&object, [](auto* /*p*/) { /* do nothing */ }));
}

ObjectHolder ObjectHolder::None() {
    return ObjectHolder();
}

Object& ObjectHolder::operator*() const {
    AssertIsValid();
    return *Get();
}

Object* ObjectHolder::operator->() const {
    AssertIsValid();
    return Get();
}

Object* ObjectHolder::Get() const {
    return data_.get();
}

ObjectHolder::operator bool() const {
    return Get() != nullptr;
}

bool IsTrue(const ObjectHolder& object) {
    // проверяем что object не пуст
    if(object) {
        if(Bool* p = object.TryAs<Bool>()) {
            return p->GetValue();
        } else if (Number* p = object.TryAs<Number>()) {
            return p->GetValue() != 0;
        } else if (String* p = object.TryAs<String>()) {
            return !(p->GetValue().empty());
        }
    }
    return false;
}

void ClassInstance::Print(std::ostream& os, Context& context) {
    // Заглушка, реализуйте метод самостоятельно
    if(HasMethod("__str__", 0)) {
        ObjectHolder string_representation = Call("__str__", {}, context);
        if(string_representation) { // если холдер не пуст
            String* s_object_ptr = string_representation.TryAs<String>();
            s_object_ptr->Print(os, context);
        }
    } else {
        size_t obj_address = *(reinterpret_cast<size_t*>(this));
        os << obj_address;
    }
}

bool ClassInstance::HasMethod(const std::string& method, size_t argument_count) const {
    const Method* m = cls_.GetMethod(method);
    if(m->formal_params.size() == argument_count) {
        return true;
    }
    return false;
}

Closure& ClassInstance::Fields() {
    return fields_;
}

const Closure& ClassInstance::Fields() const {
    return fields_;
}

ClassInstance::ClassInstance(const Class& cls) 
    : cls_(cls)
{
}

ObjectHolder ClassInstance::Call(const std::string& method,
                                 const std::vector<ObjectHolder>& actual_args,
                                 Context& context) {
    if(HasMethod(method, actual_args.size())) {
        const Method* m = cls_.GetMethod(method);
        return m->body.get()->Execute(fields_, context);

    } else {
        throw runtime_error("No such method!"s);
    }
}

Class::Class(std::string name, std::vector<Method> methods, const Class* parent) 
    : name_(name)
    , methods_(std::move(methods))
    , parent_(parent)
{
    // ??? забыл сделать методы базового класса, видимо)
    // Реализуйте метод самостоятельно
}

const Method* Class::GetMethod(const std::string& name) const {
    std::vector<Method>::const_iterator method = std::find_if(
                methods_.begin(), methods_.end(), 
                [&name](const runtime::Method& method){
                    return method.name == name;
                });
    if(method != methods_.end()) {
        return &(*method);
    }
    return nullptr;
}

[[nodiscard]] inline const std::string& Class::GetName() const {
    // Заглушка. Реализуйте метод самостоятельно.
    throw std::runtime_error("Not implemented"s);
}

void Class::Print(ostream& /*os*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
}

void Bool::Print(std::ostream& os, [[maybe_unused]] Context& context) {
    os << (GetValue() ? "True"sv : "False"sv);
}

bool Equal(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/) {
    // Заглушка. Реализуйте функцию самостоятельно
    throw std::runtime_error("Cannot compare objects for equality"s);
}

bool Less(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/) {
    // Заглушка. Реализуйте функцию самостоятельно
    throw std::runtime_error("Cannot compare objects for less"s);
}

bool NotEqual(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/) {
    // Заглушка. Реализуйте функцию самостоятельно
    throw std::runtime_error("Cannot compare objects for equality"s);
}

bool Greater(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/) {
    // Заглушка. Реализуйте функцию самостоятельно
    throw std::runtime_error("Cannot compare objects for equality"s);
}

bool LessOrEqual(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/) {
    // Заглушка. Реализуйте функцию самостоятельно
    throw std::runtime_error("Cannot compare objects for equality"s);
}

bool GreaterOrEqual(const ObjectHolder& /*lhs*/, const ObjectHolder& /*rhs*/, Context& /*context*/) {
    // Заглушка. Реализуйте функцию самостоятельно
    throw std::runtime_error("Cannot compare objects for equality"s);
}

}  // namespace runtime