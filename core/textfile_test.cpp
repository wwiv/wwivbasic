/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*               Copyright (C)2014-2022, WWIV Software Services           */
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
#include "core/file.h"
#include "core/strings.h"
#include "core/test/file_helper.h"
#include "core/textfile.h"
#include "gtest/gtest.h"
#include <filesystem>
#include <memory>
#include <string>

using namespace wwiv::core;
using namespace wwiv::strings;

class TextFileTest : public ::testing::Test {
protected:
  void SetUp() override {
    const auto* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
    test_name_ = test_info->name();
    hello_world_path_ = helper_.CreateTempFile(test_name_, kHelloWorld);
  }

  [[nodiscard]] const std::string& test_name() const { return test_name_; }

  wwiv::core::test::FileHelper helper_;
  std::filesystem::path hello_world_path_;
  std::string test_name_;
  static const std::string kHelloWorld;
};

const std::string TextFileTest::kHelloWorld = "Hello World\n";

TEST_F(TextFileTest, Constructor_SunnyCase) {
  TextFile file(hello_world_path_, "rt");
  std::string s;
  EXPECT_TRUE(file.ReadLine(&s));
  EXPECT_EQ("Hello World", s);
}

TEST_F(TextFileTest, Constructor_Path_And_Name) {
  TextFile file(FilePath(helper_.TempDir(), this->test_name()), "rt");
  std::string s;
  EXPECT_TRUE(file.ReadLine(&s));
  EXPECT_EQ("Hello World", s);
}

TEST_F(TextFileTest, Append) {
  std::unique_ptr<TextFile> file(
      new TextFile(FilePath(helper_.TempDir(), this->test_name()), "a+t"));
  EXPECT_EQ(3, file->Write("abc"));
  const auto filename = file->full_pathname();
  file.reset();

  const auto actual = helper_.ReadFile(filename);
  EXPECT_EQ("Hello World\nabc", actual);
}

TEST_F(TextFileTest, ReadLine_CA) {
  const auto path = helper_.CreateTempFile(this->test_name(), "a\nb\nc\n");
  TextFile file(path, "rt");
  char s[255];
  EXPECT_TRUE(file.ReadLine(s, sizeof(s)));
  EXPECT_STREQ("a\n", s);
  EXPECT_TRUE(file.ReadLine(s, sizeof(s)));
  EXPECT_STREQ("b\n", s);
  EXPECT_TRUE(file.ReadLine(s, sizeof(s)));
  EXPECT_STREQ("c\n", s);
  EXPECT_FALSE(file.ReadLine(s, sizeof(s)));
}

TEST_F(TextFileTest, ReadLine_String) {
  const auto path = helper_.CreateTempFile(this->test_name(), "a\nb\nc\n");
  TextFile file(path, "rt");
  std::string s;
  EXPECT_TRUE(file.ReadLine(&s));
  EXPECT_EQ("a", s);
  EXPECT_TRUE(file.ReadLine(&s));
  EXPECT_EQ("b", s);
  EXPECT_TRUE(file.ReadLine(&s));
  EXPECT_EQ("c", s);
  EXPECT_FALSE(file.ReadLine(&s));
}

TEST_F(TextFileTest, Write) {
  std::string filename;
  {
    TextFile file(FilePath(helper_.TempDir(), this->test_name()), "wt");
    file.Write("Hello");
    filename = file.full_pathname();
  }
  const auto actual = helper_.ReadFile(filename);
  EXPECT_EQ("Hello", actual);
}

TEST_F(TextFileTest, Insertion_Basic) {
  std::string filename;
  {
    TextFile file(FilePath(helper_.TempDir(), this->test_name()), "wt");
    file << "Hello" << " " << "World";
    filename = file.full_pathname();
  }
  const auto actual = helper_.ReadFile(filename);
  EXPECT_EQ("Hello World", actual);
}

TEST_F(TextFileTest, Insertion_TwoLines) {
  std::string filename;
  {
    TextFile file(FilePath(helper_.TempDir(), this->test_name()), "wt");
    file << "Hello" << std::endl;
    file << "World" << std::endl;
    filename = file.full_pathname();
  }
  const auto actual = helper_.ReadFile(filename);
  auto lines = wwiv::strings::SplitString(actual, "\r\n", true);
  EXPECT_EQ(2u, lines.size());
  EXPECT_EQ("Hello", lines.at(0));
  EXPECT_EQ("World", lines.at(1));
}

TEST_F(TextFileTest, WriteChar) {
  std::string filename;
  {
    TextFile file(FilePath(helper_.TempDir(), this->test_name()), "wt");
    file.WriteChar('H');
    filename = file.full_pathname();
  }
  const auto actual = helper_.ReadFile(filename);
  EXPECT_EQ("H", actual);
}

TEST_F(TextFileTest, WriteBinary) {
  std::string filename;
  {
    TextFile file(FilePath(helper_.TempDir(), this->test_name()), "wt");
    file.WriteBinary(kHelloWorld.c_str(), kHelloWorld.size() - 1); // trim off \n
    filename = file.full_pathname();
    // Let the file close.
  }
  const auto actual = helper_.ReadFile(filename);
  EXPECT_EQ("Hello World", actual);
}

TEST_F(TextFileTest, Close) {
  TextFile file(hello_world_path_, "rt");
  ASSERT_TRUE(file.IsOpen());
  EXPECT_TRUE(file.Close());
  EXPECT_FALSE(file.IsOpen());
}

TEST_F(TextFileTest, Close_SecondCloseReturnsFalse) {
  TextFile file(hello_world_path_, "rt");
  ASSERT_TRUE(file.IsOpen());
  EXPECT_TRUE(file.Close());
  EXPECT_FALSE(file.Close());
  EXPECT_FALSE(file.IsOpen());
}

TEST_F(TextFileTest, IsEOF) {
  TextFile file(hello_world_path_, "rt");
  std::string s;
  EXPECT_TRUE(file.ReadLine(&s));
  EXPECT_EQ("Hello World", s);

  EXPECT_FALSE(file.ReadLine(&s));
  EXPECT_TRUE(file.IsEndOfFile());
}

TEST_F(TextFileTest, GetPosition) {
  const auto path = helper_.CreateTempFile(test_name_, "a\nb\nc\n");
  TextFile file(path, "rt");
  ASSERT_EQ(0, file.position());
  std::string s;
  EXPECT_TRUE(file.ReadLine(&s));
  EXPECT_EQ("a", s);
#if defined (_WIN32) || defined (__OS2__)
  EXPECT_EQ(3, file.position());
#else  // _WIN32
  EXPECT_EQ(2, file.position());
#endif // _WIN32
}

TEST_F(TextFileTest, ReadFileIntoString) {
  const auto path = helper_.CreateTempFile(this->test_name(), "a\nb\nc\n");
  TextFile file(path, "rt");
  const auto s = file.ReadFileIntoString();
  EXPECT_EQ("a\nb\nc\n", s);
}

TEST_F(TextFileTest, ReadFileIntoString_EmptyFile) {
  const auto path = helper_.CreateTempFile(this->test_name(), "");
  TextFile file(path, "rt");
  const auto s = file.ReadFileIntoString();
  EXPECT_EQ("", s);
}

TEST_F(TextFileTest, ReadFileNLinesIntoString) {
  const auto path = helper_.CreateTempFile(this->test_name(), "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n");
  TextFile file(path, "rt");
  const auto s = file.ReadLastLinesIntoVector(4);
  EXPECT_EQ(4u, s.size());
}


TEST_F(TextFileTest, ReadFileNLinesIntoString_NoCRLF) {
  const auto path = helper_.CreateTempFile(this->test_name(), "1\n2\n3\n4\n5\n6\n7\n8\n9\n10");
  TextFile file(path, "rt");
  const auto s = file.ReadLastLinesIntoVector(4);
  EXPECT_EQ(4u, s.size());
}

TEST_F(TextFileTest, ReadFileNLinesIntoString_HugeFile) {
  std::string contents(1000, 'x');
  contents += "1\n2\n3\n4\n5\n6\n7\n8\n9\n10";
  const auto path = helper_.CreateTempFile(this->test_name(), contents);
  TextFile file(path, "rt");
  const auto s = file.ReadLastLinesIntoVector(4);
  EXPECT_EQ(4u, s.size());
}

TEST_F(TextFileTest, ReadFileNLinesIntoString_SmallerFileThanN) {
  const auto path = helper_.CreateTempFile(this->test_name(), "1\n2\n");
  TextFile file(path, "rt");
  const auto s = file.ReadLastLinesIntoVector(4);
  EXPECT_EQ(2u, s.size());
}

