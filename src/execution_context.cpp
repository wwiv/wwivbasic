#include "execution_context.h"
#include "executor.h"
#include "fmt/format.h"

#include <any>
#include <map>
#include <stack>
#include <string>
#include <vector>

namespace wwivbasic {
  // Helpers from STL
  template <typename C>
  bool contains(C const& container, typename C::const_reference key) {
    return std::find(std::begin(container), std::end(container), key) != std::end(container);
  }

  template <typename K, typename V, typename C, typename A>
  bool contains(std::map<K, V, C, A> const& m, K const& key) {
    return m.find(key) != std::end(m);
  }

  // Partial specialization for maps with string keys (allows using const char* for lookup values)
  template <typename V, typename C, typename A>
  bool contains(std::map<std::string, V, C, A> const& m, const std::string& key) {
    return m.find(key) != std::end(m);
  }

  // Partial specialization for maps with const string keys.
  template <typename V, typename C, typename A>
  bool contains(std::map<const std::string, V, C, A> const& m, const std::string& key) {
    return m.find(key) != std::end(m);
  }


  int to_int(const std::string& s) {
    try {
      return std::stoi(s);
    }
    catch (const std::exception&) {
      return 0;
    }
  }


  Value::Value(const std::any& a) {
    if (!a.has_value()) {
      return;
    }
    value = a;
    if (a.type() == typeid(bool)) {
      type = Type::BOOLEAN;
    } else if (a.type() == typeid(int)) {
      type = Type::INTEGER;
    }
    else if (a.type() == typeid(std::string)) {
      type = Type::STRING;
    }
    else {
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
    } catch (const std::exception &) {
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

std::any Value::toAny() const {
  return value;
}

Value Value::operator+(const Value &that) const {
  switch (type) {
  case Type::BOOLEAN:  return toBool() || that.toBool();
  case Type::INTEGER:  return toInt() + that.toInt();
  case Type::STRING:   return fmt::format("{}{}", toString(), that.toString());
  }
  return {};
}

Value Value::operator-(const Value &that) const {
  switch (type) {
  case Type::BOOLEAN:  return !(toBool() && that.toBool());
  case Type::INTEGER:  return toInt() - that.toInt();
  case Type::STRING:   return fmt::format("{}{}", toString(), that.toString());
  }
  return {};
}

Value Value::operator*(const Value& that) const {
  switch (type) {
  case Type::BOOLEAN:  return (toBool() * that.toBool());
  case Type::INTEGER:  return toInt() * that.toInt();
  case Type::STRING:   return fmt::format("{}{}", toString(), that.toString()); // ????!
  }
  return {};
}

Value Value::operator/(const Value& that) const {
  switch (type) {
  case Type::BOOLEAN:  return false; // TODO WARN
  case Type::INTEGER:  return toInt() / that.toInt();
  case Type::STRING:   return fmt::format("{}{}", toString(), that.toString());
  }
  return {};
}

Value Value::operator%(const Value& that) const {
  switch (type) {
  case Type::BOOLEAN:  return false; // TODO WARN
  case Type::INTEGER:  return toInt() % that.toInt();
  case Type::STRING:   return fmt::format("{}{}", toString(), that.toString());
  }
  return {};
}


bool Value::operator<(const Value& that) const {
  switch (type) {
  case Type::BOOLEAN: return toBool()   < that.toBool();
  case Type::INTEGER: return toInt()    < that.toInt();
  case Type::STRING:  return toString() < that.toString();
  }
  return false;
}

bool Value::operator>(const Value& that) const {
  switch (type) {
  case Type::BOOLEAN: return toBool()   > that.toBool();
  case Type::INTEGER: return toInt()    > that.toInt();
  case Type::STRING:  return toString() > that.toString();
  }
  return false;
}

bool Value::operator==(const Value& that) const {
  switch (type) {
  case Type::BOOLEAN: return toBool()   == that.toBool();
  case Type::INTEGER: return toInt()    == that.toInt();
  case Type::STRING:  return toString() == that.toString();
  }
  return false;
}

Value Value::operator||(const Value& that) const {
  switch (type) {
  case Type::BOOLEAN: return toBool() || that.toBool();
  case Type::INTEGER: return toInt() || that.toInt();
  case Type::STRING:  return fmt::format("{}{}", toString(), that.toString()); // Is this right
  }
  return false;
}

Value Value::operator&&(const Value& that) const {
  switch (type) {
  case Type::BOOLEAN: return toBool() && that.toBool();
  case Type::INTEGER: return toInt() && that.toInt();
  case Type::STRING:  return fmt::format("{}{}", toString(), that.toString()); // Is this right
  }
  return false;
}


// Execution Context

ExecutionContext::ExecutionContext() {
  // Start off with only global scope
  scopes.emplace_back("GLOBAL");
}


bool ExecutionContext::upsert(const std::string& name, const Value& value) {
  for (auto it = std::rbegin(scopes); it != std::rend(scopes); it++) {
    if (contains(it->local_vars, name)) {
      auto& var = it->local_vars.at(name);
      var.value = value;
      // updated existing.
      return true;
    }
  }

  auto& scope = scopes.back();
  scope.local_vars.emplace(name, Var(name, value));
  return true;
}


Value ExecutionContext::var(const std::string& name) {
  for (auto it = std::rbegin(scopes); it != std::rend(scopes); it++) {
    if (contains(it->local_vars, name)) {
      auto& var = it->local_vars.at(name);
      return var.value;
    }
  }
  std::cout << "UNKNOWN VARIABLE REFERENCED: " << name << std::endl;
  return {};
}

Value ExecutionContext::call(const std::string &function_name,
                             const std::vector<Value> &params,
                             ExecutionVisitor *visitor) {
  if (!contains(functions, function_name)) {
    std::cout << "Unknown function: " << function_name << std::endl;
    return false;
  }

  auto& fn = functions.at(function_name);

  // Validate params
  const auto want_count = fn.params.size();
  const auto have_count = params.size();
  if (fn.type == basic_function_t::Type::BASIC && have_count != want_count) {
    // Only bail on wrong args on BASIC functions.
    std::cout << "Wrong number of parameter to function: " << function_name << std::endl;
    std::cout << "have: " << have_count << std::endl;
    std::cout << "want: " << want_count << std::endl;
    std::cout << std::endl;
    return false;
  }

  // Create a new scope for the function body
  Scope fnscope(function_name);

  // Add variables for parameters
  if (fn.type == basic_function_t::Type::BASIC) {
    for (int i = 0; i < want_count; i++) {
      const auto& n = fn.params.at(i);
      const auto& v = params.at(i);
      fnscope.local_vars.insert_or_assign(n, Var(n, v));
    }
  }
  
  // Put the scope on top fo the stack
  scopes.push_back(fnscope);

  Value result;
  if (fn.type == basic_function_t::Type::BASIC) {
    // Visit the body of the function call
    result = Value(visitor->visit(fn.fn->statements()));
  }
  else if (fn.type == basic_function_t::Type::NATIVE) {
    result = fn.cpp_fn(params);
  }

  std::cout << "debug: " << Value(result).toString() << std::endl;

  // remove latest scope.
  scopes.pop_back();
  return result;
}

} // namespace wwivbasic