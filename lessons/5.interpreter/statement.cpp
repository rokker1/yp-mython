#include "statement.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace ast {

using runtime::Closure;
using runtime::Context;
using runtime::ObjectHolder;

namespace {
const string ADD_METHOD = "__add__"s;
const string INIT_METHOD = "__init__"s;
}  // namespace

ObjectHolder Assignment::Execute(Closure& closure, Context& context) {
    ObjectHolder holder = statement_ptr_.get()->Execute(closure, context);
    closure.insert_or_assign(
        var_,
        holder
    );
    return holder;
}

Assignment::Assignment(std::string var, std::unique_ptr<Statement> rv) 
    : var_(var)
    , statement_ptr_(std::move(rv)) // ?? сомнительное решение
{
}


VariableValue::VariableValue(const std::string& var_name) 
   : var_name_(var_name) {}


VariableValue::VariableValue(std::vector<std::string> dotted_ids) 
    : dotted_ids_(dotted_ids) {}

ObjectHolder VariableValue::Execute(Closure& closure, Context& /*context*/) {
    if(!var_name_.empty()) {
        if(closure.count(var_name_)) {
            return closure.at(var_name_);
        }
    } else if (!dotted_ids_.empty()) {
        if(auto it = closure.find(dotted_ids_[0]); it != closure.end()) {
            // в таблице символов найдено имя первого объекта
            ObjectHolder object = it->second;

            if(dotted_ids_.size() > 1) {
                for(size_t i = 1; i + 1 < dotted_ids_.size(); ++i) {
                    if(auto* instance_ptr = object.TryAs<runtime::ClassInstance>()) {
                        if(auto item = instance_ptr->Fields().find(dotted_ids_[i]); item != instance_ptr->Fields().end()) {
                            object = item->second;
                        } else {throw std::runtime_error("invalid variable"s);}
                    } else {throw std::runtime_error("invalid variable"s);}
                }

                if(auto* p = object.TryAs<runtime::ClassInstance>()){
                    if(auto last = p->Fields().find(dotted_ids_.back()); last != p->Fields().end()) {
                        return last->second;
                    }
                } else {throw std::runtime_error("invalid variable"s);}
            } else {
                return object;
            }
        }
    }
    
    throw std::runtime_error("invalid variable"s);
}

unique_ptr<Print> Print::Variable(const std::string& name) {
    auto s = Print();
}

Print::Print(unique_ptr<Statement> /*argument*/) {
    // Заглушка, реализуйте метод самостоятельно
}

Print::Print(vector<unique_ptr<Statement>> /*args*/) {
    // Заглушка, реализуйте метод самостоятельно
}

ObjectHolder Print::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

MethodCall::MethodCall(std::unique_ptr<Statement> /*object*/, std::string /*method*/,
                       std::vector<std::unique_ptr<Statement>> /*args*/) {
    // Заглушка. Реализуйте метод самостоятельно
}

ObjectHolder MethodCall::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ObjectHolder Stringify::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ObjectHolder Add::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ObjectHolder Sub::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ObjectHolder Mult::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ObjectHolder Div::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ObjectHolder Compound::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ObjectHolder Return::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ClassDefinition::ClassDefinition(ObjectHolder /*cls*/) {
    // Заглушка. Реализуйте метод самостоятельно
}

ObjectHolder ClassDefinition::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

FieldAssignment::FieldAssignment(VariableValue object, std::string field_name,
                                 std::unique_ptr<Statement> rv) 
    : object_(std::move(object)), field_name_(field_name), statement_ptr_(std::move(rv))
{
}

ObjectHolder FieldAssignment::Execute(Closure& closure, Context& context) {
    if(!statement_ptr_) {
        return {};
    }
    auto* h = object_.Execute(closure, context).TryAs<runtime::ClassInstance>();
    if(!h) {
        throw std::runtime_error("invalid variable"s);
    }

    h->Fields()[field_name_] = statement_ptr_->Execute(closure, context);
    return h->Fields()[field_name_];

}

IfElse::IfElse(std::unique_ptr<Statement> /*condition*/, std::unique_ptr<Statement> /*if_body*/,
               std::unique_ptr<Statement> /*else_body*/) {
    // Реализуйте метод самостоятельно
}

ObjectHolder IfElse::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ObjectHolder Or::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ObjectHolder And::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

ObjectHolder Not::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

Comparison::Comparison(Comparator /*cmp*/, unique_ptr<Statement> lhs, unique_ptr<Statement> rhs)
    : BinaryOperation(std::move(lhs), std::move(rhs)) {
    // Реализуйте метод самостоятельно
}

ObjectHolder Comparison::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

NewInstance::NewInstance(const runtime::Class& class_, std::vector<std::unique_ptr<Statement>> args)
    : class_(class_), args_(std::move(args))
{
    // Заглушка. Реализуйте метод самостоятельно
}

NewInstance::NewInstance(const runtime::Class& class_) 
    : class_(class_)
{
    // Заглушка. Реализуйте метод самостоятельно
}

ObjectHolder NewInstance::Execute(Closure& closure, Context& context) {
    if(args_.empty()) {
        const runtime::Method* constructor = class_.GetMethod("__init__");
        if(constructor) {
            runtime::ObjectHolder obj = constructor->body.get()->Execute(closure, context);
            return obj;
        } else {
            runtime::ClassInstance instance(class_);
            ObjectHolder holder = ObjectHolder::Own<runtime::ClassInstance>(
                std::forward<runtime::ClassInstance>(instance));
            //ObjectHolder holder = ObjectHolder::Share(instance);
            

            return holder;
            
        }
    } else {
        // TODO конструктор с параметрами
        ;
    }
    return {};
}

MethodBody::MethodBody(std::unique_ptr<Statement>&& /*body*/) {
}

ObjectHolder MethodBody::Execute(Closure& /*closure*/, Context& /*context*/) {
    // Заглушка. Реализуйте метод самостоятельно
    return {};
}

}  // namespace ast