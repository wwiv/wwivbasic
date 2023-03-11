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

value_type_t* make_value_type_int() {
  value_type_t* i = new value_type_t();
  i->name = "INTEGER";
  i->lt = [](const Value& l, const Value& r) -> bool {
    return std::any_cast<int>(l) < std::any_cast<int>(r);
  };
  i->gt = [](const Value& l, const Value& r) -> bool {
    return bool(std::any_cast<int>(l) > std::any_cast<int>(r));
  };
  i->eq = [](const Value& l, const Value& r) -> bool {
    return bool(std::any_cast<int>(l) == std::any_cast<int>(r));
  };
  i->to_string = [](const std::any& s) -> std::string {
    return fmt::format("{:d}", std::any_cast<int>(s));
  };
  i->to_int = [](const std::any& s) -> int { return std::any_cast<int>(s); };
  i->to_bool = [](const std::any& s) -> bool { return std::any_cast<int>(s) != 0; };

  i->from_string = [](const std::string& s) -> std::any {
    try {
      return std::stoi(s);
    }
    catch (const std::exception&) {
      return 0;
    }
  };
  i->from_int = [](int i) -> std::any { return i; };
  i->from_bool = [](bool b) -> std::any { return b ? 1 : 0; };
  i->val = [](const std::any& s) -> int { return std::any_cast<int>(s); };
  i->len = [](const std::any& s) -> int {
    int i = std::any_cast<int>(s);
    return static_cast<int>(std::floor(std::log10(i))) + 1;
  };

  return i;
}

value_type_t* make_value_type_boolean() {
  value_type_t* i = new value_type_t();
  i->name = "BOOLEAN";
  i->lt = [](const Value& l, const Value& r) -> bool {
    return bool(std::any_cast<bool>(l) < std::any_cast<bool>(r));
  };
  i->gt = [](const Value& l, const Value& r) -> bool {
    return bool(std::any_cast<bool>(l) > std::any_cast<bool>(r));
  };
  i->eq = [](const Value& l, const Value& r) -> bool {
    return bool(std::any_cast<bool>(l) == std::any_cast<bool>(r));
  };
  i->to_string = [](const std::any& s) -> std::string {
    return fmt::format("{:d}", std::any_cast<bool>(s));
  };
  i->to_int = [](const std::any& s) -> int { return std::any_cast<bool>(s) ? 1 : 0; };
  i->to_bool = [](const std::any& s) -> bool { return std::any_cast<bool>(s); };

  i->from_string = [](const std::string& s) -> std::any {
    return std::any_cast<bool>(s) ? "TRUE" : "FALSE";
  };
  i->from_int = [](int i) -> std::any { return i != 0; };
  i->from_bool = [](bool b) -> std::any { return b; };
  i->val = [](const std::any& s) -> int { return std::any_cast<bool>(s) ? 1 : 0; };
  i->len = [](const std::any& s) -> int { return 1; };

  return i;
}


value_type_t* make_value_type_string() {
  value_type_t* i = new value_type_t();
  i->name = "STRING";
  i->lt = [](const Value& l, const Value& r) -> bool {
    return bool(std::any_cast<std::string>(l) < std::any_cast<std::string>(r));
  };
  i->gt = [](const Value& l, const Value& r) -> bool {
    return bool(std::any_cast<std::string>(l) > std::any_cast<std::string>(r));
  };
  i->eq = [](const Value& l, const Value& r) -> bool {
    return bool(std::any_cast<std::string>(l) == std::any_cast<std::string>(r));
  };
  i->to_string = [](const std::any& s) -> std::string {
    return std::any_cast<std::string>(s);
  };
  i->to_int = [](const std::any& s) -> int { 
    try {
      return std::stoi(std::any_cast<std::string>(s));
    }
    catch (const std::exception&) {
      return 0;
    }
  };
  i->to_bool = [](const std::any& s) -> bool { 
    return wwiv::strings::iequals(std::any_cast<std::string>(s), "TRUE");
  };

  i->from_string = [](const std::string& s) -> std::any {
    return s;
  };
  i->from_int = [](int i) -> std::any { return fmt::format("{:d}", i); };
  i->from_bool = [](bool b) -> std::any { return b ? "TRUE" : "FALSE"; };
  i->val = [](const std::any& s) -> int { 
    try {
      return std::stoi(std::any_cast<std::string>(s));
    }
    catch (const std::exception&) {
      return 0;
    }
  };
  i->len = [](const std::any& s) -> int {
    return wwiv::strings::size_int(std::any_cast<std::string>(s));
  };

  return i;
}

static value_type_t* int_fns;
static value_type_t* string_fns;
static value_type_t* bool_fns;

//static 
std::map<std::string, value_type_t*> Value::types;

std::once_flag types_initialized;

//static 
void Value::InitalizeDefaultTypes() {
  std::call_once(types_initialized, []() { 
    int_fns = make_value_type_int();
    string_fns = make_value_type_string();
    bool_fns = make_value_type_boolean();

    types.emplace("INTEGER", int_fns);
    types.emplace("BOOLEAN", bool_fns);
    types.emplace("STRING", string_fns);
  });
}


//Value::Value() : value_(std::string()), type("STRING") { debug = ""; }

Value::Value(const std::any& a, const std::string& t, const std::string& d, value_type_t* f)
    : value_(a), type(t), debug(d), fns(f) {
  if (debug.empty()) {
    debug = fmt::format("{}", toString());
  }
  if (auto it = types.find(type); it != std::end(types)) {
    fns = types.find(type)->second;
  }
  else {
    LOG(ERROR) << "No type function for type: " << type;
    fns = types.find("STRING")->second;
  }

}

/*
Value::Value(bool b) : value_(b), type("BOOLEAN") { debug = fmt::format("{}", b); }

Value::Value(int i) : value_(i), type("INTEGER") { debug = fmt::format("{}", i); }

Value::Value(const std::string& s) : value_(s), type("STRING") { debug = s; }

Value::Value(const char* s) : value_(std::string(s)), type("STRING") { debug = s; }

*/

Value::Value(const std::any& a) {
  if (!a.has_value()) {
    debug = "WTF";
    value_ = false;
    type = "BOOLEAN";
    fns = types.find("BOOLEAN")->second;
    return;
  }

  value_ = a;
  if (a.type() == typeid(bool)) {
    type = "BOOLEAN";
    fns = types.find(type)->second;
    debug = fmt::format("{}", std::any_cast<bool>(value_));
  } else if (a.type() == typeid(int)) {
    type = "INTEGER";
    fns = types.find(type)->second;
    debug = fmt::format("{}", std::any_cast<int>(value_));
  } else if (a.type() == typeid(std::string)) {
    type = "STRING";
    fns = types.find(type)->second;
    debug = fmt::format("{}", std::any_cast<std::string>(value_));
  } else {
    // TODO(rushfan): Look up custom type here
    type = "STRING";
    fns = types.find(type)->second;
    debug = fmt::format("WTF! {}", a.type().name());
  } 

}

bool Value::toBool() const {
  return fns->to_bool(value_);
}

int Value::toInt() const {
  return fns->to_int(value_);
}

std::string Value::toString() const {
  return fns->to_string(value_);
}

std::any Value::toAny() const {
  return value_;
}

Value Value::operator+(const Value& that) const {
  return fns->add(*this, that);
}

Value Value::operator-(const Value& that) const {
  return fns->sub(*this, that);
}

Value Value::operator*(const Value& that) const {
  return fns->mul(*this, that);
}

Value Value::operator/(const Value& that) const {
  return fns->div(*this, that);
}

Value Value::operator%(const Value& that) const {
  return fns->mod(*this, that);
}

bool Value::operator<(const Value& that) const {
  return fns->lt(*this, that);
}

bool Value::operator>(const Value& that) const {
  return fns->gt(*this, that);
}

bool Value::operator==(const Value& that) const {
  return fns->eq(*this, that);
}

Value Value::operator||(const Value& that) const {
  return Value(fns->or(*this, that));
}

Value Value::operator&&(const Value& that) const {
  return Value(fns->and(*this, that));
}

std::ostream& operator<<(std::ostream& os, const Value& v) {
  os << v.toString();
  return os;
}

} // namespace wwivbasic