#include "value.h"
#include "core/textfile.h"
#include "core/stl.h"
#include "executor.h"
#include "utils.h"
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
    debug = "WTF";
    return;
  }
  value = a;
  if (a.type() == typeid(bool)) {
    type = Type::BOOLEAN;
    debug = fmt::format("{}", std::any_cast<bool>(value));
  } else if (a.type() == typeid(int)) {
    type = Type::INTEGER;
    debug = fmt::format("{}", std::any_cast<int>(value));
  } else if (a.type() == typeid(std::string)) {
    type = Type::STRING;
    debug = fmt::format("{}", std::any_cast<std::string>(value));
  } else {
    type = Type::STRING;
    debug = fmt::format("WTF! {}", std::any_cast<std::string>(value));
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

} // namespace wwivbasic