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
#include <typeindex>
#include <typeinfo>
#include <vector>

using namespace wwiv::stl;

namespace wwivbasic {

static value_type_t* int_fns;
static value_type_t* string_fns;
static value_type_t* bool_fns;

//static 
std::map<std::string, value_type_t*> Value::types_;
std::map<std::type_index, std::string> Value::typeinfo_map_;


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
    return  std::any_cast<bool>(s) ? "TRUE" : "FALSE";
  };
  i->to_int = [](const std::any& s) -> int { return std::any_cast<bool>(s) ? 1 : 0; };
  i->to_bool = [](const std::any& s) -> bool { return std::any_cast<bool>(s); };

  i->from_string = [](const std::string& s) -> std::any {
    return wwiv::strings::iequals(s, "TRUE");
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

std::once_flag types_initialized;

//static 
void Value::RegisterType(std::string_view name, const std::type_info& type,
  value_type_t* value_type) {
  types_.insert_or_assign(std::string(name), value_type);
  typeinfo_map_.insert_or_assign(type, std::string(name));
}


//static 
void Value::InitalizeDefaultTypes() {
  std::call_once(types_initialized, []() { 
    int_fns = make_value_type_int();
    RegisterType("INTEGER", typeid(int), int_fns);

    bool_fns = make_value_type_boolean();
    RegisterType("BOOLEAN", typeid(bool), bool_fns);

    string_fns = make_value_type_string();
    RegisterType("STRING", typeid(std::string), string_fns);
  });
}

Value::Value(const std::any& a, const std::string& t, const std::string& d, value_type_t* f)
    : value_(a), type(t), debug(d), fns(f) {
  if (debug.empty()) {
    debug = fmt::format("{}", toString());
  }
  if (auto it = types_.find(type); it != std::end(types_)) {
    fns = types_.find(type)->second;
  }
  else {
    LOG(ERROR) << "No type function for type: " << type;
    fns = types_.find("STRING")->second;
  }
}

Value::Value(const std::any& a) {
  if (!a.has_value()) {
    debug = "WTF";
    value_ = false;
    type = "BOOLEAN";
    fns = types_.find("BOOLEAN")->second;
    return;
  }

  if (a.type() == typeid(Value)) {
    auto other = std::any_cast<Value>(a);
    value_ = other.value_;
    type = other.type;
    debug = other.debug;
    fns = other.fns;
  }

  value_ = a;
  const auto& ti = a.type();
  const auto ti_name = ti.name();
  if (auto it = typeinfo_map_.find(a.type()); it != std::end(typeinfo_map_)) {
    type = it->second;
    fns = types_.find(type)->second;
    debug = fmt::format("{} ({})", toString(), type);
    return;
  }


  type = "STRING";
  fns = types_.find(type)->second;
  debug = fmt::format("UNKNOWN TYPE: {}", a.type().name());

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