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
#include <typeindex>
#include <variant>
#include <vector>

namespace wwivbasic {


struct value_type_t;

class Value {
public:
  Value(const std::any& a, const std::string & type, const std::string& debug, value_type_t* f);
  explicit Value(const std::any& a);

  Value() : Value(false) {}
  explicit Value(bool b) : Value(std::make_any<bool>(b)) {}
  explicit Value(int i) : Value(std::make_any<int>(i)) {}
  explicit Value(const std::string& s) : Value(std::make_any<std::string>(s)) {}
  explicit Value(const char* s) : Value(std::make_any<std::string>(std::string(s))) {}

  int set(int i) {
    value_ = i;
    debug = fmt::format("{}", i);
    return i;
  }

  std::string set(std::string_view sv) {
    auto s = std::string(sv);
    value_ = s;
    debug = fmt::format("{}", s);
    return s;
  }

  bool toBool() const;
  int toInt() const;
  std::string toString() const;
  std::any toAny() const;

  template <typename T> 
  T get() const { return std::any_cast<T>(value_); }

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
  /**
   * @brief  Registers STRING, INT, and BOOLEAN types.
  */
  static void InitalizeDefaultTypes();
  static void RegisterType(std::string_view name, const std::type_info& type,
                           value_type_t* value_type);

private:
  std::any value_;
  std::string type;
  std::string debug;

  // name -> value_type_t
  static std::map<std::string, value_type_t*> types_;
  // type -> name
  static std::map<std::type_index, std::string> typeinfo_map_;
  value_type_t* fns{ nullptr };
};

std::ostream& operator<<(std::ostream& os, const Value& v);


struct value_type_t {
  value_type_t() {}
  std::string name;

  typedef std::function<Value(const Value&, const Value&)> op_fn;
  typedef std::function<bool(const Value&, const Value&)> rel_fn;
  typedef std::function<std::string(const std::any&)> to_string_fn;
  typedef std::function<std::any(const std::string&)> from_string_fn;
  typedef std::function<int(const std::any&)> to_int_fn;
  typedef std::function<std::any(int)> from_int_fn;
  typedef std::function<bool(const std::any&)> to_bool_fn;
  typedef std::function<std::any(bool)> from_bool_fn;
  typedef std::function<int(const std::any&)> val_len_fn;
  std::function<bool(rel_fn&, rel_fn&, const Value&, const Value&)> default_or_fn =
    [=](rel_fn& fl, rel_fn& fr, const Value& l, const Value& r) -> bool {
    if (!fl || !fr) {
      return false;
    }
    return fl(l, r) || fr(l, r);
  };

  rel_fn lt;
  rel_fn gt;
  rel_fn eq;
  rel_fn ne = [=](const Value& l, const Value& r) -> bool { return !eq(l, r); };
  rel_fn ge = [=](const Value& l, const Value& r) -> bool { return default_or_fn(eq, gt, l, r); };
  rel_fn le = [=](const Value& l, const Value& r) -> bool { return default_or_fn(eq, lt, l, r); };
  rel_fn and = [](const Value& l, const Value& r) -> bool { return l.toBool() && r.toBool(); };
  rel_fn or = [](const Value& l, const Value& r) -> bool { return l.toBool() || r.toBool(); };

  op_fn add = [](const Value& l, const Value& r) -> Value { return Value(l.toInt() + r.toInt()); };
  op_fn sub = [](const Value& l, const Value& r)->Value { return Value(l.toInt() - r.toInt()); };
  op_fn mul = [](const Value& l, const Value& r)->Value { return Value(l.toInt() * r.toInt()); };
  op_fn div = [](const Value& l, const Value& r)->Value { return Value(l.toInt() / r.toInt()); };
  op_fn mod = [](const Value& l, const Value& r)->Value { return Value(l.toInt() % r.toInt()); };

  to_string_fn to_string;
  from_string_fn from_string;
  to_int_fn to_int;
  from_int_fn from_int;
  to_bool_fn to_bool;
  from_bool_fn from_bool;

  val_len_fn val;
  val_len_fn len;
};


} // namespace wwivbasic

template <> class fmt::formatter<wwivbasic::Value> {
public:
  constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }
  template <typename Context>
  constexpr auto format(wwivbasic::Value const& v, Context& ctx) const {
    return format_to(ctx.out(), "{}", v.toString());
  }
};
