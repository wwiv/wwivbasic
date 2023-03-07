#include "stdlib/strings.h"
#include <cstdint>
#include "core/stl.h"

namespace wwivbasic::stdlib {

Value val(const std::vector<Value>& v) {
  if (v.empty()) {
    return Value(0);
  }
  return Value(v.front().toInt());
}

Value len(const std::vector<Value>& v) {
  if (v.empty()) {
    return Value(0);
  }
  return Value(wwiv::stl::size_int(v.front().toString()));
}

}
