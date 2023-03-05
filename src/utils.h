#pragma once

#include <map>
#include <string>
#include <tuple>
#include <vector>

std::string remove_quotes(std::string s);
std::tuple<std::string, std::string> split_package_from_id(const std::string& s);

