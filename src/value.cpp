#include "value.h"
#include "core/textfile.h"
#include "core/stl.h"
#include "executor.h"
#include "utils.h"
#include "fmt/format.h"

#include <any>
#include <iostream>
#include <map>
#include <stack>
#include <string>
#include <vector>

using namespace wwiv::stl;

namespace wwivbasic {

Value::Value(const std::any& a) {
  if (!a.has_value()) {
    debug = "WTF";
    return;
  }
  if (a.type() == typeid(bool)) {
    type = Type::BOOLEAN;
    value_ = std::any_cast<bool>(a);
    debug = fmt::format("{}", std::get<bool>(value_));
  } else if (a.type() == typeid(int)) {
    type = Type::INTEGER;
    value_ = std::any_cast<int>(a);
    debug = fmt::format("{}", std::get<int>(value_));
  } else if (a.type() == typeid(std::string)) {
    type = Type::STRING;
    value_ = std::any_cast<std::string>(a);
    debug = fmt::format("{}", std::get<std::string>(value_));
  } else {
    type = Type::STRING;
    debug = fmt::format("WTF! {}", a.type().name());
    value_ = std::any_cast<std::string>(a);
  }
}

bool Value::toBool() const {
  switch (type) {
  case Type::BOOLEAN: {
    return std::get<bool>(value_);
  } break;
  case Type::INTEGER: {
    const auto i = std::get<int>(value_);
    return i != 0;
  } break;
  case Type::STRING: {
    const auto& s = std::get<std::string>(value_);
    return s == "TRUE";
  } break;
  }
  return false;
}

int Value::toInt() const {
  switch (type) {
  case Type::BOOLEAN: {
    const auto b = std::get<bool>(value_);
    return b ? 1 : 0;
  } break;
  case Type::INTEGER: {
    return std::get<int>(value_);
  } break;
  case Type::STRING: {
    const auto s = std::get<std::string>(value_);
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
    const auto b = std::get<bool>(value_);
    return b ? "TRUE" : "FALSE";
  } break;
  case Type::INTEGER: {
    return std::to_string(std::get<int>(value_));
  } break;
  case Type::STRING: {
    return std::get<std::string>(value_);
  } break;
  }
  return {};
}

std::any Value::toAny() const { 
  switch (type) {
  case Type::BOOLEAN: return std::make_any<bool>(std::get<bool>(value_));
  case Type::INTEGER: return std::make_any<int>(std::get<int>(value_));
  case Type::STRING:return std::make_any<std::string>(std::get<std::string>(value_));
  }
  return {};
}

Value Value::operator+(const Value& that) const {
  switch (type) {
  case Type::BOOLEAN:
    return Value(toBool() || that.toBool());
  case Type::INTEGER:
    return Value(toInt() + that.toInt());
  case Type::STRING:
    return Value(fmt::format("{}{}", toString(), that));
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
    return Value(fmt::format("{}{}", toString(), that));
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
    return Value(fmt::format("{}{}", toString(), that)); // ????!
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
    return Value(fmt::format("{}{}", toString(), that));
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
    return Value(fmt::format("{}{}", toString(), that));
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
    return Value(fmt::format("{}{}", toString(), that)); // Is this right
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
    return Value(fmt::format("{}{}", toString(), that)); // Is this right
  }
  return Value(false);
}

std::ostream& operator<<(std::ostream& os, const Value& v) {
  os << v.toString();
  return os;
}

} // namespace wwivbasic