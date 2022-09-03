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
    return make_unique<Print>(make_unique<VariableValue>(name));
}

Print::Print(unique_ptr<Statement> argument) 
{
    args_.push_back(std::move(argument));
}

Print::Print(vector<unique_ptr<Statement>> args) 
    : args_(std::move(args))
{

}

ObjectHolder Print::Execute(Closure& closure, Context& context) {
    bool is_first = true;
    for(const auto &statement : args_) {
        if(!is_first) {
            context.GetOutputStream() << ' ';
        }
        is_first = false;
        ObjectHolder holder = statement->Execute(closure, context); // здесь
        if(holder) {
            holder.Get()->Print(context.GetOutputStream(), context);
        } else {
            context.GetOutputStream() << "None"s; // здесь
        }
    }
    context.GetOutputStream() << '\n';
    return {};
}

MethodCall::MethodCall(std::unique_ptr<Statement> object, std::string method,
                       std::vector<std::unique_ptr<Statement>> args) 
    : object_(std::move(object))
    , method_(std::move(method))
    , args_(std::move(args)) {}

ObjectHolder MethodCall::Execute(Closure& closure, Context& context) {
    runtime::ClassInstance* instance_ptr = object_->Execute(closure, context).TryAs<runtime::ClassInstance>();
    if(instance_ptr) {
        std::vector<ObjectHolder> actual_args;
        for(const auto& arg : args_) {
            actual_args.push_back(std::move(arg->Execute(closure, context)));
        }
        return instance_ptr->Call(method_, std::move(actual_args), context);
    }
    throw std::runtime_error("invalid something"s);

}

ObjectHolder Stringify::Execute(Closure& closure, Context& context) {
    ObjectHolder a = argument_->Execute(closure, context);
    ostringstream s;
    if(!a) {
        s << "None"s;
        
    } else {
        a.Get()->Print(s, context);
    }

    runtime::String value(s.str());
    auto holder = ObjectHolder::Own<runtime::String>(std::forward<runtime::String>(value));
    return holder;
}

ObjectHolder Add::Execute(Closure& closure, Context& context) {
    const ObjectHolder l_holder = lhs_->Execute(closure, context);
    const ObjectHolder r_holder = rhs_->Execute(closure, context);
    if(auto l = l_holder.TryAs<runtime::Number>()) {
        if(auto r = r_holder.TryAs<runtime::Number>()) {
            return ObjectHolder::Own<runtime::Number>(std::forward<runtime::Number>(l->GetValue() + r->GetValue()));
        }
    } else if (auto l = l_holder.TryAs<runtime::String>()) {
        if(auto r = r_holder.TryAs<runtime::String>()) {
            return ObjectHolder::Own<runtime::String>(std::forward<runtime::String>(l->GetValue() + r->GetValue()));
        }
    } else if (auto instance_ptr = l_holder.TryAs<runtime::ClassInstance>()) {
        if (instance_ptr->HasMethod("__add__"s, 1)) {
            std::vector<ObjectHolder> actual_args;
            actual_args.push_back(std::move(r_holder));
            return instance_ptr->Call("__add__"s, std::move(actual_args), context);
        }
    } 
    throw std::runtime_error("can't add"s);
}

ObjectHolder Sub::Execute(Closure& closure, Context& context) {
    const ObjectHolder l_holder = lhs_->Execute(closure, context);
    
    if(auto l = l_holder.TryAs<runtime::Number>()) {
        const ObjectHolder r_holder = rhs_->Execute(closure, context);
        if(auto r = r_holder.TryAs<runtime::Number>()) {
            return ObjectHolder::Own<runtime::Number>(std::forward<runtime::Number>(l->GetValue() - r->GetValue()));
        }
    } 
    throw std::runtime_error("can't sub"s);
}

ObjectHolder Mult::Execute(Closure& closure, Context& context) {
    const ObjectHolder l_holder = lhs_->Execute(closure, context);
    
    if(auto l = l_holder.TryAs<runtime::Number>()) {
        const ObjectHolder r_holder = rhs_->Execute(closure, context);
        if(auto r = r_holder.TryAs<runtime::Number>()) {
            return ObjectHolder::Own<runtime::Number>(std::forward<runtime::Number>(l->GetValue() * r->GetValue()));
        }
    } 
    throw std::runtime_error("can't mult"s);
}

ObjectHolder Div::Execute(Closure& closure, Context& context) {
    const ObjectHolder l_holder = lhs_->Execute(closure, context);
    
    if(auto l = l_holder.TryAs<runtime::Number>()) {
        const ObjectHolder r_holder = rhs_->Execute(closure, context);

        if(auto r = r_holder.TryAs<runtime::Number>()) {
            if(r->GetValue() == 0) { throw std::runtime_error("can't divide by zero!"s); }
            return ObjectHolder::Own<runtime::Number>(std::forward<runtime::Number>(l->GetValue() * r->GetValue()));
        }
    } 
    throw std::runtime_error("can't div"s);
}

ObjectHolder Compound::Execute(Closure& closure, Context& context) {

    for (auto&& s : statements_)
    {
        if(s) {
            s->Execute(closure, context);
        }

    }




    return {};
}

ObjectHolder Return::Execute(Closure& closure, Context& context) {
    throw statement_->Execute(closure, context);
}

ClassDefinition::ClassDefinition(ObjectHolder cls) 
    : cls_(std::move(cls))
{
}

ObjectHolder ClassDefinition::Execute(Closure& closure, Context& context) {
    runtime::Class* class_holder = cls_.TryAs<runtime::Class>();
    std::string class_name = class_holder->GetName();
    closure.insert(make_pair(class_name, cls_));
    return {}; // ??? или здесь надо вернуть только что вставленный класс
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

IfElse::IfElse(std::unique_ptr<Statement> condition, std::unique_ptr<Statement> if_body,
               std::unique_ptr<Statement> else_body) 
    : condition_(std::move(condition))
    , if_body_(std::move(if_body))
    , else_body_(std::move(else_body))               
{
    // Реализуйте метод самостоятельно
}

ObjectHolder IfElse::Execute(Closure& closure, Context& context) {
    runtime::Bool* condition_result = condition_->Execute(closure, context).TryAs<runtime::Bool>();
    if(condition_result) {
        if(condition_result->GetValue()) {
            try {
                // if_body_.Execute(closure, context);
                // return {};
                // alt ?
                return if_body_.Execute(closure, context);
                
            } catch (ObjectHolder h) {
                return h;
            }
        } else {
            try {
                // else_body_.Execute(closure, context);
                // return {};
                // alt ?
                return else_body_.Execute(closure, context);
                
            } catch (ObjectHolder h) {
                return h;
            }
        }
    }
    throw runtime_error("cant execute the condition!"s);
}

ObjectHolder Or::Execute(Closure& closure, Context& context) {

    runtime::Bool* l = lhs_->Execute(closure, context).TryAs<runtime::Bool>();
    if(l != nullptr) {
        if(l->GetValue()) {
            return ObjectHolder::Own<runtime::Bool>(std::forward<runtime::Bool>({true}));
        } else {
            runtime::Bool* r = rhs_->Execute(closure, context).TryAs<runtime::Bool>();
            return ObjectHolder::Own<runtime::Bool>(std::forward<runtime::Bool>({r->GetValue()}));
        }
    }
    return {};
}

ObjectHolder And::Execute(Closure& closure, Context& context) {
    runtime::Bool* l = lhs_->Execute(closure, context).TryAs<runtime::Bool>();
    if(l != nullptr) {
        if(!(l->GetValue())) {
            return ObjectHolder::Own<runtime::Bool>(std::forward<runtime::Bool>({false}));
        } else {
            runtime::Bool* r = rhs_->Execute(closure, context).TryAs<runtime::Bool>();
            return ObjectHolder::Own<runtime::Bool>(std::forward<runtime::Bool>({r->GetValue()}));
        }
    }
    return {};
}

ObjectHolder Not::Execute(Closure& closure, Context& context) {
    return ObjectHolder::Own<runtime::Bool>(std::forward<runtime::Bool>({
        !argument_->Execute(closure, context).TryAs<runtime::Bool>()->GetValue()
    }));
}

Comparison::Comparison(Comparator cmp, unique_ptr<Statement> lhs, unique_ptr<Statement> rhs)
    : BinaryOperation(std::move(lhs), std::move(rhs)) 
    , cmp_(cmp)
{
    
}

ObjectHolder Comparison::Execute(Closure& closure, Context& context) {
    ObjectHolder l = lhs_->Execute(closure, context);
    ObjectHolder r = rhs_->Execute(closure, context);
    auto res = cmp_(l, r, context);
    return ObjectHolder::Own<runtime::Bool>(std::forward<runtime::Bool>(res));
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
    const runtime::Method* constructor = class_.GetMethod("__init__");
    if(constructor) {
        if(args_.empty()) {
            runtime::ObjectHolder obj = constructor->body.get()->Execute(closure, context);
            return obj;
        } else {
            vector<ObjectHolder> actual_args;
            for(const auto& arg : args_) {
                actual_args.push_back(std::move(arg->Execute(closure, context)));
            }

            runtime::ClassInstance instance(class_);
            instance.Call("__init__"s, std::move(actual_args), context);
            ObjectHolder holder = ObjectHolder::Own<runtime::ClassInstance>(
                std::forward<runtime::ClassInstance>(instance));
            return holder;
        }
    } else {
        runtime::ClassInstance instance(class_);
        ObjectHolder holder = ObjectHolder::Own<runtime::ClassInstance>(
            std::forward<runtime::ClassInstance>(instance));
        return holder;
    }

    return {};
}

MethodBody::MethodBody(std::unique_ptr<Statement>&& body) 
    : body_(std::move(body))
{

}

ObjectHolder MethodBody::Execute(Closure& closure, Context& context) {
    try {
        body_.Execute(closure, context);
        return {};
        /* alt 
        return body_.Execute(closure, context);
        */
    } catch (ObjectHolder h) {
        return h;
    }
    return {};
}

}  // namespace ast