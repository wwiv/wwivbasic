#include "utils.h"

std::string remove_quotes(std::string s) {
  if (s.size() < 2) {
    return s;
  }
  if (s.size() == 2) {
    return {};
  }
  s.pop_back();
  return s.substr(1);
}
