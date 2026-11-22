#include "stdlib/strings.h"
#include "fmt/format.h"

#include <algorithm>
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

// LEFT, RIGHT and MID return "" for a negative or zero length, or a start
// past the end of the string, like my_basic. Positions are 0-based.

std::string left(std::string s, int len) {
  if (len <= 0) {
    return {};
  }
  return s.substr(0, std::min(static_cast<size_t>(len), s.size()));
}

std::string right(std::string s, int len) {
  if (len <= 0) {
    return {};
  }
  const auto n = std::min(static_cast<size_t>(len), s.size());
  return s.substr(s.size() - n);
}

// MID(s, start) returns the rest of s from start; MID(s, start, len) returns
// at most len characters.
Value mid(std::vector<Value> args) {
  if (args.size() < 2 || args.size() > 3) {
    throw BasicError(fmt::format("MID: expected 2 or 3 arguments, got {}", args.size()));
  }
  const auto s = args.at(0).toString();
  const auto start = args.at(1).toInt();
  if (start < 0 || static_cast<size_t>(start) >= s.size()) {
    return Value(std::string());
  }
  if (args.size() == 2) {
    return Value(s.substr(static_cast<size_t>(start)));
  }
  const auto len = args.at(2).toInt();
  if (len <= 0) {
    return Value(std::string());
  }
  return Value(s.substr(static_cast<size_t>(start), static_cast<size_t>(len)));
}

}
