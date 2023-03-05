#pragma once

#include "BasicLexer.h"
#include "BasicParser.h"
#include "core/stl.h"
#include "fmt/format.h"

#include <any>
#include <deque>
#include <iosfwd>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace wwivbasic {

class Value {
public:
  enum class Type { BOOLEAN, INTEGER, STRING };

  Value() : value_(std::string()), type(Type::STRING) { debug = ""; }
  explicit Value(bool b) : value_(b), type(Type::BOOLEAN) { debug = fmt::format("{}", b); }
  explicit Value(int i) : value_(i), type(Type::INTEGER) { debug = fmt::format("{}", i); }
  explicit Value(const std::string& s) : value_(s), type(Type::STRING) { debug = s; }
  explicit Value(const char* s) : value_(std::string(s)), type(Type::STRING) { debug = s; }
  explicit Value(const std::any& a);

  int set(int i) {
    value_ = i;
    type = Type::INTEGER;
    debug = fmt::format("{}", i);
    return i;
  }

  std::string set(std::string_view sv) {
    auto s = std::string(sv);
    value_ = s;
    type = Type::STRING;
    debug = fmt::format("{}", s);
    return s;
  }

  bool toBool() const;
  int toInt() const;
  std::string toString() const;
  std::any toAny() const;

  template <typename T> 
  T get() const { return std::get<T>(value_); }

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

private:
  std::variant<bool, int, std::string> value_;
  Type type;
  std::string debug;

};

std::ostream& operator<<(std::ostream& os, const Value& v);


} // namespace wwivbasic

template <> class fmt::formatter<wwivbasic::Value> {
public:
  constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }
  template <typename Context>
  constexpr auto format(wwivbasic::Value const& v, Context& ctx) const {
    return format_to(ctx.out(), "{}", v.toString());
  }
};
