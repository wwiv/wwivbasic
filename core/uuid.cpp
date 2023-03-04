/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*               Copyright (C)2020-2022, WWIV Software Services           */
/*                                                                        */
/*    Licensed  under the  Apache License, Version  2.0 (the "License");  */
/*    you may not use this  file  except in compliance with the License.  */
/*    You may obtain a copy of the License at                             */
/*                                                                        */
/*                http://www.apache.org/licenses/LICENSE-2.0              */
/*                                                                        */
/*    Unless  required  by  applicable  law  or agreed to  in  writing,   */
/*    software  distributed  under  the  License  is  distributed on an   */
/*    "AS IS"  BASIS, WITHOUT  WARRANTIES  OR  CONDITIONS OF ANY  KIND,   */
/*    either  express  or implied.  See  the  License for  the specific   */
/*    language governing permissions and limitations under the License.   */
/*                                                                        */
/**************************************************************************/
#include "core/uuid.h"

#include "core/stl.h"
#include <array>
#include <iomanip>
#include <ios>
#include <random>
#include <sstream>

namespace wwiv::core {

uuid_t::uuid_t() = default;

uuid_t::uuid_t(std::array<uint8_t, 16> s) : bytes_(s) {
  for (const auto& i : bytes_) {
    if (i != 0) {
      empty_ = false;
    }
  }

}

uuid_t::uuid_t(const uuid_t& that) {
  this->bytes_ = that.bytes_;
  this->empty_ = that.empty_;
}

uuid_t& uuid_t::operator=(const uuid_t& that) {
  this->bytes_ = that.bytes_;
  this->empty_ = that.empty_;
  return *this;
}

std::ostream& operator<<(std::ostream& os, const uuid_t& u) {
  os << u.to_string();
  return os;
}


std::string uuid_t::to_string() const {
  std::ostringstream ss;
  ss << std::hex << std::setfill('0')
     << std::setw(2) << static_cast<int>(bytes_[0])
     << std::setw(2) << static_cast<int>(bytes_[1])
     << std::setw(2) << static_cast<int>(bytes_[2])
     << std::setw(2) << static_cast<int>(bytes_[3])
     << '-'
     << std::setw(2) << static_cast<int>(bytes_[4])
     << std::setw(2) << static_cast<int>(bytes_[5])
     << '-'
     << std::setw(2) << static_cast<int>(bytes_[6])
     << std::setw(2) << static_cast<int>(bytes_[7])
     << '-'
     << std::setw(2) << static_cast<int>(bytes_[8])
     << std::setw(2) << static_cast<int>(bytes_[9])
     << '-'
     << std::setw(2) << static_cast<int>(bytes_[10])
     << std::setw(2) << static_cast<int>(bytes_[11])
     << std::setw(2) << static_cast<int>(bytes_[12])
     << std::setw(2) << static_cast<int>(bytes_[13])
     << std::setw(2) << static_cast<int>(bytes_[14])
     << std::setw(2) << static_cast<int>(bytes_[15]);
   return ss.str();
}

int from_single_hex_byte(const char r) {
  const auto b = std::tolower(r);
  if (b >= 'a' && b <= 'f') {
    return b - 'a' + 10;
  }
  if (b >= '0' && b <= '9') {
    return b - '0';
  }
  return 0;
}
uint8_t from_hex_bytes(char l, char r) {
  return (from_single_hex_byte(l) << 4 & 0xf0) | (from_single_hex_byte(r) & 0x0f);
}

std::optional<uuid_t> uuid_t::from_string(const std::string& s) {
  if (s.empty()) {
    return std::nullopt;
  }
  const auto has_braces = s.front() == '{';
  const auto expected_size = has_braces ? 38 : 36;
  if (wwiv::stl::ssize(s) != expected_size) {
    return std::nullopt;
  }

  auto sit = std::begin(s);
  if (has_braces) {
    ++sit;
  }
  // fde5e4ff-8053-43ff-9696-5bffc2cdf3ff
  std::array<uint8_t, 16> out{};
  for (auto outit = std::begin(out); outit != std::end(out); ++outit) {
    while (sit != std::end(s) && *sit == '-') { ++sit; }
    if (sit == std::end(s)) {
      break;
    }
    const auto l = *sit++;
    while (sit != std::end(s) && *sit == '-') { ++sit; }
    if (sit == std::end(s)) {
      break;
    }
    const auto r = *sit++;
    *outit = from_hex_bytes(l, r);
  }
  return uuid_t(out);
}

int uuid_t::version() const {
  return bytes_[6] >> 4;
}

int uuid_t::variant() const {
  const int v = bytes_[8] >> 6;
  return v;
}

uuid_t uuid_generator::generate() {
  std::mt19937 gen(rd_());
  std::array<uint8_t, 16> data{};
  for (auto i = 0; i < 4; i++) {
    const auto u = distribution_(gen);
    data[(i * 4) + 0] = (u >> 24) & 0xff;
    data[(i * 4) + 1] = (u >> 16) & 0xff;
    data[(i * 4) + 2] = (u >> 8) & 0xff;
    data[(i * 4) + 3] = u & 0xff;
  }

  // From https://tools.ietf.org/html/rfc4122#page-6
  // A Universally Unique IDentifier (UUID) URN Namespace

  // 4.1.1: The variant field determines the layout of the UUID.
  // Variant (10?xxxxx) is "The variant specified in this document."
  data[8] &= 0xBF;
  data[8] |= 0x80;
  // 4.1.3 Version: 4 (0100xxxx) is "The randomly or pseudo-randomly
  // generated version specified in this document.
  data[6] &= 0x4F;
  data[6] |= 0x40;
  
  return uuid_t(data);
}

} 
