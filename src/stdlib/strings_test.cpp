#include "gtest/gtest.h"
#include "stdlib/strings.h"

using namespace wwivbasic::stdlib;

TEST(StringsTest, ASC) {
  ASSERT_EQ(13, asc("\r"));
  ASSERT_EQ(254, asc("\xFE"));
}

TEST(StringsTest, CHR) {
  ASSERT_STREQ("\r", chr(13).c_str());
  ASSERT_STREQ("A", chr(65).c_str());
  ASSERT_STREQ("\xFE", chr(254).c_str());
  ASSERT_STREQ("", chr(999).c_str());
}