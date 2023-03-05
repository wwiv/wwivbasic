#include "execution_context.h"
#include "core/stl.h"
#include "executor.h"
#include "fmt/format.h"

#include <any>
#include <map>
#include <stack>
#include <string>
#include <vector>

using namespace wwiv::stl;

namespace wwivbasic {

Value::Value(const std::any& a) {
  if (!a.has_value()) {
    return;
  }
  value = a;
  if (a.type() == typeid(bool)) {
    type = Type::BOOLEAN;
  } else if (a.type() == typeid(int)) {
    type = Type::INTEGER;
  } else if (a.type() == typeid(std::string)) {
    type = Type::STRING;
  } else {
    type = Type::STRING;
    std::cout << "WTF";
  }
}

bool Value::toBool() const {
  switch (type) {
  case Type::BOOLEAN: {
    return std::any_cast<bool>(value);
  } break;
  case Type::INTEGER: {
    const auto i = std::any_cast<int>(value);
    return i != 0;
  } break;
  case Type::STRING: {
    const auto s = std::any_cast<std::string>(value);
    return s == "TRUE";
  } break;
  }
  return false;
}

int Value::toInt() const {
  switch (type) {
  case Type::BOOLEAN: {
    const auto b = std::any_cast<bool>(value);
    return b ? 1 : 0;
  } break;
  case Type::INTEGER: {
    return std::any_cast<int>(value);
  } break;
  case Type::STRING: {
    const auto s = std::any_cast<std::string>(value);
    try {
      return std::stoi(s);
    } catch (const std::exception&) {
      return 0;
    }
  } break;
  }
  return 0;
}

std::string Value::toString() const {
  switch (type) {
  case Type::BOOLEAN: {
    const auto b = std::any_cast<bool>(value);
    return b ? "TRUE" : "FALSE";
  } break;
  case Type::INTEGER: {
    return std::to_string(std::any_cast<int>(value));
  } break;
  case Type::STRING: {
    return std::any_cast<std::string>(value);
  } break;
  }
  return {};
}

std::any Value::toAny() const { return value; }

Value Value::operator+(const Value& that) const {
  switch (type) {
  case Type::BOOLEAN:
    return Value(toBool() || that.toBool());
  case Type::INTEGER:
    return Value(toInt() + that.toInt());
  case Type::STRING:
    return Value(fmt::format("{}{}", toString(), that.toString()));
  }
  return Value(false);
}

Value Value::operator-(const Value& that) const {
  switch (type) {
  case Type::BOOLEAN:
    return Value(!(toBool() && that.toBool()));
  case Type::INTEGER:
    return Value(toInt() - that.toInt());
  case Type::STRING:
    return Value(fmt::format("{}{}", toString(), that.toString()));
  }
  return Value(false);
}

Value Value::operator*(const Value& that) const {
  switch (type) {
  case Type::BOOLEAN:
    return Value(toBool() * that.toBool());
  case Type::INTEGER:
    return Value(toInt() * that.toInt());
  case Type::STRING:
    return Value(fmt::format("{}{}", toString(), that.toString())); // ????!
  }
  return Value(false);
}

Value Value::operator/(const Value& that) const {
  switch (type) {
  case Type::BOOLEAN:
    return Value(false); // TODO WARN
  case Type::INTEGER:
    return Value(toInt() / that.toInt());
  case Type::STRING:
    return Value(fmt::format("{}{}", toString(), that.toString()));
  }
  return Value(false);
}

Value Value::operator%(const Value& that) const {
  switch (type) {
  case Type::BOOLEAN:
    return Value(false); // TODO WARN
  case Type::INTEGER:
    return Value(toInt() % that.toInt());
  case Type::STRING:
    return Value(fmt::format("{}{}", toString(), that.toString()));
  }
  return Value(false);
}

bool Value::operator<(const Value& that) const {
  switch (type) {
  case Type::BOOLEAN:
    return (toBool() < that.toBool());
  case Type::INTEGER:
    return (toInt() < that.toInt());
  case Type::STRING:
    return (toString() < that.toString());
  }
  return (false);
}

bool Value::operator>(const Value& that) const {
  switch (type) {
  case Type::BOOLEAN:
    return (toBool() > that.toBool());
  case Type::INTEGER:
    return (toInt() > that.toInt());
  case Type::STRING:
    return (toString() > that.toString());
  }
  return (false);
}

bool Value::operator==(const Value& that) const {
  switch (type) {
  case Type::BOOLEAN:
    return (toBool() == that.toBool());
  case Type::INTEGER:
    return (toInt() == that.toInt());
  case Type::STRING:
    return (toString() == that.toString());
  }
  return (false);
}

Value Value::operator||(const Value& that) const {
  switch (type) {
  case Type::BOOLEAN:
    return Value(toBool() || that.toBool());
  case Type::INTEGER:
    return Value(toInt() || that.toInt());
  case Type::STRING:
    return Value(fmt::format("{}{}", toString(), that.toString())); // Is this right
  }
  return Value(false);
}

Value Value::operator&&(const Value& that) const {
  switch (type) {
  case Type::BOOLEAN:
    return Value(toBool() && that.toBool());
  case Type::INTEGER:
    return Value(toInt() && that.toInt());
  case Type::STRING:
    return Value(fmt::format("{}{}", toString(), that.toString())); // Is this right
  }
  return Value(false);
}

// Execution Context


void Module::upsert(const std::string& name, const Value& value) {
  for (auto it = std::rbegin(scopes); it != std::rend(scopes); it++) {
    if (contains(it->local_vars, name)) {
      auto& var = it->local_vars.at(name);
      var.value = value;
      // updated existing.
      return;
    }
  }

  auto& scope = scopes.back();
  scope.local_vars.emplace(name, Var(name, value));
}

Value Module::var(const std::string& name) {
  for (auto it = std::rbegin(scopes); it != std::rend(scopes); it++) {
    if (contains(it->local_vars, name)) {
      const auto& var = it->local_vars.at(name);
      return var.value;
    }
  }
  std::cout << "UNKNOWN VARIABLE REFERENCED: " << name << std::endl;
  return {};
}

bool Module::has_var(const std::string& name) const {
  for (auto it = std::rbegin(scopes); it != std::rend(scopes); it++) {
    if (contains(it->local_vars, name)) {
      return true;
    }
  }
  return false;
}

bool Module::has_fn(const std::string& name) const {
  return contains(functions, name);
}

Value Module::call(const std::string& function_name, const std::vector<Value>& params,
                             ExecutionVisitor* visitor) {
  if (!contains(functions, function_name)) {
    std::cout << "Unknown function: " << function_name << std::endl;
    return Value(false);
  }

  auto& fn = functions.at(function_name);

  // Validate params
  const auto want_count = fn.params.size();
  const auto have_count = params.size();
  if (fn.type == BasicFunction::Type::BASIC && have_count != want_count) {
    // Only bail on wrong args on BASIC functions.
    std::cout << "Wrong number of parameter to function: " << function_name << std::endl;
    std::cout << "have: " << have_count << std::endl;
    std::cout << "want: " << want_count << std::endl;
    std::cout << std::endl;
    return Value(false);
  }

  // Create a new scope for the function body
  Scope fnscope(function_name);

  // Add variables for parameters
  if (fn.type == BasicFunction::Type::BASIC) {
    for (int i = 0; i < want_count; i++) {
      const auto& n = fn.params.at(i);
      const auto& v = params.at(i);
      fnscope.local_vars.insert_or_assign(n, Var(n, v));
    }
  }

  // Put the scope on top fo the stack
  scopes.push_back(fnscope);

  Value result;
  if (fn.type == BasicFunction::Type::BASIC) {
    // Visit the body of the function call
    result = Value(visitor->visit(fn.def_fn->statements()));
  } else if (fn.type == BasicFunction::Type::NATIVE) {
    result = fn.cpp_fn(params);
  }

  std::cout << "debug: " << Value(result).toString() << std::endl;

  // remove latest scope.
  scopes.pop_back();
  return result;
}

ExecutionContext::ExecutionContext() {
  // Start off with only global scope
  modules.emplace("", Module("") );
  modules.at("").scopes.emplace_back("<GLOBAL>");
  root = module = &modules.at("");
}

// Splits a package off from identifier, i.e "foo.bar.baz" -> {"foo.bar", "baz"}
std::tuple<std::string, std::string> split_package_from_id(const std::string& s) {
  if (const auto idx = s.rfind('.'); idx != std::string::npos) {
    const auto pkg = s.substr(0, idx);
    const auto id = s.substr(idx + 1);
    return std::make_tuple(pkg, id);
  }
  return std::make_tuple("", s);
}

void ExecutionContext::upsert(const std::string& name, const Value& value) {
  
  if (const auto [pkg, id] = split_package_from_id(name); !pkg.empty()) {
    // fully qualified
    if (contains(modules, pkg)) {
      // we have a module.
      modules.at(pkg).upsert(id, value);
      return;
    }
    // TODO(rushfan): Error that we don't have a module loaded for this.
    return;
  }

  // Not fully qualified case.
  if (!module->has_var(name) && root->has_var(name)) {
    // No existing variable in current module, but one at the root, update it.
    root->upsert(name, value);
  }
  else {
    // Add it to the current module.
    module->upsert(name, value);
  }
}

Value ExecutionContext::var(const std::string& name) {
  
  if (const auto [pkg, id] = split_package_from_id(name); !pkg.empty()) {
    // fully qualified
    if (contains(modules, pkg)) {
      // we have a module.
      return modules.at(pkg).var(id);
    }
    // TODO(rushfan): Error that we don't have a module loaded for this.
    return Value(false);
  }
  // Not fully qualified case.
  if (!module->has_var(name) && root->has_var(name)) {
    // No existing variable in current module, but one at the root, update it.
    return root->var(name);
  }
  return module->var(name);
}

Value ExecutionContext::call(const std::string& function_name, const std::vector<Value>& params,
                             ExecutionVisitor* visitor) {
  
  if (const auto [pkg, id] = split_package_from_id(function_name); !pkg.empty()) {
    // fully qualified
    if (contains(modules, pkg)) {
      // we have a module.
      return modules.at(pkg).call(id, params, visitor);
    }
    // TODO(rushfan): Error that we don't have a module loaded for this.
    return Value(false);
  }
  // Not fully qualified case.
  if (!module->has_fn(function_name) && root->has_fn(function_name)) {
    // No existing variable in current module, but one at the root, update it.
    return root->call(function_name, params, visitor);
  }
  return module->call(function_name, params, visitor);

}

} // namespace wwivbasic