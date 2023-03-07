#include "stdlib/strings.h"
#include <cstdint>

namespace wwivbasic::stdlib {

int asc(std::string s) {
  return s.empty() ? 0 : static_cast<int>(static_cast<uint8_t>(s.front()));
}

std::string chr(int c) {
  if (c < 0 || c > 255) {
    return {};
  }
  return std::string(1, static_cast<int>(c & 0xff));
}

std::string left(std::string s, int len) {
  if (len >= s.size()) {
    return s;
  }
  return s.substr(0, len);
}

std::string right(std::string s, int len) {
  if (len >= s.size()) {
    return s;
  }
  return s.substr(s.size() - len);
}

Value mid(std::vector<Value> args) {
  // string, start, [len]
  if (args.size() < 1) {
    return {};
  }
  const auto s = args.at(0).toString();
  if (args.size() < 2) {
    return Value(s);
  }
  if (args.size() == 2) {
    if (const auto start = args.at(1).toInt(); start < s.size()) {
      return Value(s.substr(start));
    }
    return Value(s);
  }
  const auto start = args.at(1).toInt();
  if (start < s.size()) {
    const int len = args.at(2).toInt();
    return Value(s.substr(start, len));
  }
  return Value(s);
}


}
