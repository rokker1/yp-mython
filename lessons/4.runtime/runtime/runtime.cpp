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
    // поиск у базового класса
    if(!m) {
        if(cls_.GetParent()) {
            m = cls_.GetParent()->GetMethod(method);
        }

    }
    
    if(m) {
        if(m->formal_params.size() == argument_count) {
            return true;
        } else {
            return false;
        }
    }

   throw runtime_error("e"s);
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
    
    fields_.clear();
    fields_.insert({"self", ObjectHolder::Share(*this)});

    size_t args_count = 0;
    if(!actual_args.empty()) {
        args_count = actual_args.size();
    }

    if(HasMethod(method, args_count)) {
        const Method* m = cls_.GetMethod(method);
        if(!m) {
            if(cls_.GetParent()) {
                m = cls_.GetParent()->GetMethod(method);
            }

        }
        size_t params_count = m->formal_params.size();
        for(size_t i = 0; i < params_count; ++i) {
            fields_.insert({m->formal_params.at(i), actual_args.at(i)});
        }
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
    // if(parent) {
    //     for (const Method& m : parent->methods_) {

    //         auto it = std::find_if(methods_.begin(), methods_.end(), [&m](const Method& local_method){
    //             return local_method.name == m.name;
    //         });
    //         if(it == methods_.end()) {
    //             methods_.push_back({
    //                 m.name,
    //                 m.formal_params,

    //             });
    //         }
    //     }
    // }

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

[[nodiscard]] const std::string& Class::GetName() const {
    return name_;
}

void Class::Print(ostream& os, [[maybe_unused]] Context& context) {
    os << "Class "s << name_ ;
}

void Bool::Print(std::ostream& os, [[maybe_unused]] Context& context) {
    os << (GetValue() ? "True"sv : "False"sv);
}

bool Equal(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
    if(Number* n_left = lhs.TryAs<Number>()) {
        if (Number* n_right = rhs.TryAs<Number>()) {
            return n_left->GetValue() == n_right->GetValue();
        }
    }
    if(String* s_left = lhs.TryAs<String>()) {
        if (String* s_right = rhs.TryAs<String>()) {
            return s_left->GetValue() == s_right->GetValue();
        }
    }
    if(Bool* b_left = lhs.TryAs<Bool>()) {
        if (Bool* b_right = rhs.TryAs<Bool>()) {
            return b_left->GetValue() == b_right->GetValue();
        }
    }

    if(ClassInstance* left = lhs.TryAs<ClassInstance>()) {
        if(left->HasMethod("__eq__", 1)) {
            ObjectHolder result = left->Call("__eq__", {rhs}, context);
            if(result) {
                Bool* r = result.TryAs<Bool>();
                return r->GetValue();
            } 
        }
    }

    if(!lhs && !rhs) {
        return true;
    }

    throw std::runtime_error("Cannot compare objects for equality"s);
}

bool Less(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
    if(Number* n_left = lhs.TryAs<Number>()) {
        if (Number* n_right = rhs.TryAs<Number>()) {
            return n_left->GetValue() < n_right->GetValue();
        }
    }
    if(String* s_left = lhs.TryAs<String>()) {
        if (String* s_right = rhs.TryAs<String>()) {
            return s_left->GetValue() < s_right->GetValue();
        }
    }
    if(Bool* b_left = lhs.TryAs<Bool>()) {
        if (Bool* b_right = rhs.TryAs<Bool>()) {
            return b_left->GetValue() < b_right->GetValue();
        }
    }
    if(ClassInstance* left = lhs.TryAs<ClassInstance>()) {
        if(left->HasMethod("__lt__", 1)) {
            ObjectHolder result = left->Call("__lt__", {rhs}, context);
            if(result) {
                Bool* r = result.TryAs<Bool>();
                return r->GetValue();
            } 
        }
    }

    throw std::runtime_error("Cannot compare objects for less"s);
}

bool NotEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
    return !(Equal(lhs, rhs, context));
}

bool Greater(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
    return (!(Less(lhs, rhs, context)) && NotEqual(lhs, rhs, context));
    throw std::runtime_error("Cannot compare objects for equality"s);
}

bool LessOrEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
    return !Greater(lhs, rhs, context);
    throw std::runtime_error("Cannot compare objects for equality"s);
}

bool GreaterOrEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
    return !Less(lhs, rhs, context);
    throw std::runtime_error("Cannot compare objects for equality"s);
}

}  // namespace runtime