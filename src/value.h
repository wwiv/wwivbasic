#pragma once

#include "BasicLexer.h"
#include "BasicParser.h"
#include "core/stl.h"
#include "fmt/format.h"

#include <any>
#include <deque>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace wwivbasic {

class Value {
public:
  enum class Type { BOOLEAN, INTEGER, STRING };
  std::any value;
  Type type;
  std::string debug;

  Value() : value(std::string()), type(Type::STRING) { debug = ""; }
  explicit Value(bool b) : value(b), type(Type::BOOLEAN) { debug = fmt::format("{}", b); }
  explicit Value(int i) : value(i), type(Type::INTEGER) { debug = fmt::format("{}", i); }
  explicit Value(const std::string& s) : value(s), type(Type::STRING) { debug = s; }
  explicit Value(const char* s) : value(std::string(s)), type(Type::STRING) { debug = s; }
  explicit Value(const std::any& a);

  int set(int i) {
    value = i;
    type = Type::INTEGER;
    debug = fmt::format("{}", i);
    return i;
  }

  std::string set(std::string_view sv) {
    auto s = std::string(sv);
    value = s;
    type = Type::STRING;
    debug = fmt::format("{}", s);
    return s;
  }

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

} // namespace wwivbasic