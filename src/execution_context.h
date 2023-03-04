#pragma once

#include "BasicParser.h"
#include <any>
#include <deque>
#include <functional>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

namespace wwivbasic {

class Value {
public:
  enum class Type { BOOLEAN, INTEGER, STRING };
  std::any value;
  Type type;

  Value() : value(std::string()), type(Type::STRING) {}
  Value(bool b) : value(b), type(Type::BOOLEAN) {}
  Value(int i) : value(i), type(Type::INTEGER) {}
  Value(const std::string& s) : value(s), type(Type::STRING) {}
  Value(const std::any& a);

  bool toBool() const;
  int toInt() const;
  std::string toString() const;
  std::any toAny() const;

  // operators
  Value operator+(const Value& that) const;
  Value operator-(const Value& that) const;
  Value operator*(const Value& that) const;
  Value operator/(const Value& that) const;
  Value operator%(const Value& that) const;

  Value operator||(const Value& that) const;
  Value operator&&(const Value& that) const;

  bool operator<(const Value& that) const;
  bool operator>(const Value& that) const;
  bool operator==(const Value& that) const;
  bool operator!=(const Value& that) const { return !(*this == that); }
};

class Var {
public:
  Var(const std::string& n, const Value& v) : name(n), value(v) {}

  std::string name;
  Value value;
};

class Scope {
public:
  Scope(const std::string& name) : fn_name(name) {}
  std::string fn_name;
  std::map<std::string, Var> local_vars;
};

typedef std::function<Value(std::vector<Value>)> basic_function_fn;

class BasicFunction {
public:
  enum class Type { NATIVE, BASIC };

  BasicFunction(const std::string& n, BasicParser::ProcedureDefinitionContext* fn,
                const std::vector<std::string>& p)
      : name(n), type(Type::BASIC), def_fn(fn), params(p) {}

  BasicFunction(const std::string& n, const basic_function_fn& fn,
                const std::vector<std::string>& p)
      : name(n), type(Type::NATIVE), cpp_fn(fn), params(p) {}

  std::string name;
  Type type{Type::BASIC};
  BasicParser::ProcedureDefinitionContext* def_fn{nullptr};
  basic_function_fn cpp_fn;
  std::vector<std::string> params;
};

class ExecutionVisitor;

class ExecutionContext {
public:
  ExecutionContext();

  // Creates a variable at the top scope or assigns a value to an existing
  // variable.
  void upsert(const std::string& name, const Value& value);
  Value var(const std::string& name);
  Value call(const std::string& function_name, const std::vector<Value>& params,
             ExecutionVisitor* visitor);

  void native_function(const std::string& name, const basic_function_fn& fn,
                       const std::vector<std::string>& params) {
    functions.insert_or_assign(name, BasicFunction(name, fn, params));
  }

  void native_function(const std::string& name, const basic_function_fn& fn) {
    std::vector<std::string> v;
    native_function(name, fn, v);
  }

  std::deque<Scope> scopes;
  std::map<std::string, Var> global_vars;
  std::map<std::string, BasicFunction> functions;
};

} // namespace wwivbasic