#pragma once

#include "value.h"

namespace wwivbasic::stdlib {

int asc(std::string s);
std::string chr(int c);

std::string left(std::string s, int len);
std::string right(std::string s, int len);
Value mid(std::vector<Value> args);


} // namespace wwivbasic::stdlib
