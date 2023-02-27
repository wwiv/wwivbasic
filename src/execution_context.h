#pragma once

#include "BasicParser.h"
#include <any>
#include <deque>
#include <string>
#include <map>
#include <vector>
#include <type_traits>

namespace wwivbasic {

int to_int(const std::string& s); 

class Value {
public:
  enum class Type { BOOLEAN, INTEGER, STRING };
  std::any value;
  Type type;

  Value() : value(std::string()) {}
  Value(bool b) : value(b), type(Type::BOOLEAN) {}
  Value(int i) : value(i), type(Type::INTEGER) {}
  Value(const std::string& s) : value(s), type(Type::STRING) {}
  Value(const std::any& a);

  bool toBool() const;
  int toInt() const;
  std::string toString() const;
  std::any toAny() const;
  template<typename T> 
  T get() const {
    if constexpr (std::is_same_v<T, bool>) {
      return toBool();
    } else if constexpr (std::is_same_v<T, int>) {
      return toInt();
    }
    else if constexpr(std::is_same_v<T, std::string>) {
      return toString();
    } else {
      static_assert("boo");
    }
  }
  
  // operators
  Value operator+(const Value& that);
  Value operator-(const Value& that);
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

struct basic_function_t {
  std::string name;
  BasicParser::ProcedureDefinitionContext* fn;
  std::vector<std::string> params;
};

class ExecutionVisitor;

class ExecutionContext {
public:
  ExecutionContext();

  // Creates a variable at the top scope or assigns a value to an existing
  // variable.
  bool upsert(const std::string& name, const Value& value);
  Value var(const std::string& name);
  Value call(const std::string& function_name, const std::vector<Value>& params, ExecutionVisitor* visitor);

  std::deque<Scope> scopes;
  std::map<std::string, Var> global_vars;
  std::map<std::string, basic_function_t> functions;
};

}