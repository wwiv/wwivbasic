#pragma once

#include "core/strings.h"
#include <map>
#include <string>
#include <tuple>
#include <vector>

std::string remove_quotes(std::string s);
std::tuple<std::string, std::string> split_package_from_id(const std::string& s);

// Splits a package off from identifier, i.e "foo.bar.baz" -> {"foo.bar", "baz"}
template<typename C>
std::tuple<std::string, std::string> find_package_and_id(C packages, const std::string& s) {
  for (auto it = std::crbegin(packages); it != std::crend(packages); it++) {
    if (wwiv::strings::starts_with(s, *it)) {
      const auto idx = s.rfind('.');
      return std::make_tuple(*it, s.substr(idx + 1));
    }
  }
  return std::make_tuple("", s);
}
