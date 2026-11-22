// Uses only the public embedding API (include/wwivbasic/), as an embedder
// such as WWIV would.

#include "gtest/gtest.h"
#include "wwivbasic/interpreter.h"
#include "wwivbasic/stdio_console.h"

#include <sstream>
#include <string>

using namespace wwivbasic;

TEST(EmbeddingTest, ModuleAndConsole) {
  std::ostringstream out;
  std::istringstream in("world\nY\n");
  StdioConsole console(out, in);
  Interpreter interp(console);

  auto& m = interp.add_module("my.io");
  m.native_function("HELLO", [](std::string name) { return "Hello, " + name; });
  m.native_function("ASK", [&console]() { return console.read_line(3); });

  const auto result = interp.run_source("embed.bas", R"(
import "@my.io"
print hello(ask())
print my.io.ask()
)");
  ASSERT_TRUE(result.ok()) << result.errors.front();
  // read_line(3) cuts "world" to "wor".
  EXPECT_EQ("Hello, wor\nY\n", out.str());
}

TEST(EmbeddingTest, GlobalsAreCallableWithoutPrefix) {
  std::ostringstream out;
  std::istringstream in;
  StdioConsole console(out, in);
  Interpreter interp(console);
  interp.globals().native_function("TWICE", [](int n) { return n * 2; });

  ASSERT_TRUE(interp.run_source("t.bas", "print twice(21)\n").ok());
  EXPECT_EQ("42\n", out.str());
}

TEST(EmbeddingTest, ErrorsHaveLocations) {
  std::ostringstream out;
  std::istringstream in;
  StdioConsole console(out, in);
  Interpreter interp(console);

  const auto result = interp.run_source("bad.bas", "print 1\nnosuch()\n");
  ASSERT_EQ(1u, result.errors.size());
  EXPECT_EQ("bad.bas:2: error: Unknown function 'nosuch'", result.errors.front());
}

TEST(EmbeddingTest, CheckOnlyDoesNotRun) {
  std::ostringstream out;
  std::istringstream in;
  StdioConsole console(out, in);
  Interpreter interp(console);

  RunOptions options;
  options.execute = false;
  ASSERT_TRUE(interp.run_source("t.bas", "print \"x\"\n", options).ok());
  EXPECT_EQ("", out.str());
  EXPECT_TRUE(interp.parse_tree("t.bas").has_value());
}

TEST(StdioConsoleTest, ReadKeyAndEndOfInput) {
  std::ostringstream out;
  std::istringstream in("yes\r\n");
  StdioConsole console(out, in);
  EXPECT_EQ("y", console.read_key());
  EXPECT_EQ("", console.read_line(0));
  console.clear_screen();
  EXPECT_EQ("[cls]\n", out.str());
}

TEST(EmbeddingTest, SecondRunIsAnError) {
  std::ostringstream out;
  std::istringstream in;
  StdioConsole console(out, in);
  Interpreter interp(console);

  ASSERT_TRUE(interp.run_source("one.bas", "print 1\n").ok());
  const auto result = interp.run_source("two.bas", "print 2\n");
  ASSERT_EQ(1u, result.errors.size());
  EXPECT_EQ("two.bas: error: this Interpreter has already run a script; use a new one",
            result.errors.front());
  // The second script didn't run.
  EXPECT_EQ("1\n", out.str());
}

// Objects: a fake message base bound the way WWIV's SDK bindings will be.

namespace {

struct Header {
  int number{0};
  std::string from;
  std::string subject;
  std::string summary(int width) const { return subject.substr(0, static_cast<size_t>(width)); }
};

struct Area {
  std::string name;
  std::vector<Header> headers;
  int count() const { return static_cast<int>(headers.size()); }
};

class ObjectTest : public ::testing::Test {
protected:
  ObjectTest() {
    area_ = std::make_shared<Area>(Area{
        "general", {{1, "Rushfan", "Hello"}, {2, "Sysop", "Welcome to the BBS"}}});

    auto& msgs = interp.add_module("msgs");
    auto& header = msgs.class_<Header>("header");
    header.property("number", &Header::number)
        .property("from", [](const Header& h) { return h.from; })
        .property("subject", &Header::subject, &Header::subject)
        .method("summary", &Header::summary);

    auto& area = msgs.class_<Area>("area");
    area.property("name", &Area::name)
        .method("count", &Area::count)
        // Each header object points into the area (the shared_ptr aliasing
        // constructor), so changes show up in the area and it stays alive.
        .method("header",
                [this, &header](Area&, int i) {
                  return header.wrap(std::shared_ptr<Header>(
                      area_, &area_->headers.at(static_cast<size_t>(i))));
                })
        .method("headers", [this, &header](Area&) {
          std::vector<wwivbasic::Value> result;
          for (auto& h : area_->headers) {
            result.push_back(header.wrap(std::shared_ptr<Header>(area_, &h)));
          }
          return result;
        });

    msgs.native_function("OPEN_AREA", [this, &area](std::string name) {
      if (name != area_->name) {
        throw BasicError("No such area: " + name);
      }
      return area.wrap(area_);
    });
    // A native function can get the C++ object back from a script value.
    msgs.native_function("SUBJECT_OF", [&header](wwivbasic::Value v) {
      auto h = header.unwrap(v);
      return h ? h->subject : std::string("(not a header)");
    });
  }

  RunResult run(const std::string& script) { return interp.run_source("viewer.bas", script); }

  std::shared_ptr<Area> area_;
  std::ostringstream out;
  std::istringstream in;
  StdioConsole console{out, in};
  Interpreter interp{console};
};

} // namespace

TEST_F(ObjectTest, MessageViewer) {
  const auto result = run(R"bas(
import "@msgs"
area = open_area("general")
print area.name; " ("; area.count(); " messages)"
for m in area.headers()
  print m.number; " "; m.from; ": "; m.subject
next
print area.header(1).summary(7)
)bas");
  ASSERT_TRUE(result.ok()) << result.errors.front();
  EXPECT_EQ("general (2 messages)\n"
            "1 Rushfan: Hello\n"
            "2 Sysop: Welcome to the BBS\n"
            "Welcome\n",
            out.str());
}

TEST_F(ObjectTest, ReadWritePropertyChangesTheObject) {
  const auto result = run(R"bas(
area = msgs.open_area("general")
h = area.header(0)
h.SUBJECT = "Re: Hello"
print area.header(0).subject; " / "; msgs.subject_of(h); " / "; msgs.subject_of(42)
)bas");
  ASSERT_TRUE(result.ok()) << result.errors.front();
  EXPECT_EQ("Re: Hello / Re: Hello / (not a header)\n", out.str());
  EXPECT_EQ("Re: Hello", area_->headers[0].subject);
}

TEST_F(ObjectTest, ObjectsAreReferences) {
  const auto result = run(R"bas(
a = msgs.open_area("general")
b = a
print a = b; " "; a.header(0) = a.header(0); " "; a
)bas");
  ASSERT_TRUE(result.ok()) << result.errors.front();
  // Two wraps of the same header are different values; printing shows the class.
  EXPECT_EQ("TRUE FALSE area\n", out.str());
}

TEST_F(ObjectTest, Errors) {
  auto expect_error = [](const RunResult& r, const std::string& message) {
    ASSERT_EQ(1u, r.errors.size());
    EXPECT_EQ("viewer.bas:2: error: " + message, r.errors.front());
  };
  expect_error(run("a = msgs.open_area(\"general\")\nprint a.nosuch\n"),
               "area has no member 'nosuch'");
}

TEST_F(ObjectTest, ReadOnlyProperty) {
  const auto r = run("a = msgs.open_area(\"general\")\na.name = \"x\"\n");
  ASSERT_EQ(1u, r.errors.size());
  EXPECT_EQ("viewer.bas:2: error: area.name is read-only", r.errors.front());
}

TEST_F(ObjectTest, MethodWithoutParens) {
  const auto r = run("a = msgs.open_area(\"general\")\nprint a.count\n");
  ASSERT_EQ(1u, r.errors.size());
  EXPECT_EQ("viewer.bas:2: error: area.count is a method; call it with ()", r.errors.front());
}

TEST_F(ObjectTest, PropertyCalledAsMethod) {
  const auto r = run("a = msgs.open_area(\"general\")\nprint a.name()\n");
  ASSERT_EQ(1u, r.errors.size());
  EXPECT_EQ("viewer.bas:2: error: area.name is a property, not a method; use it without ()",
            r.errors.front());
}

TEST_F(ObjectTest, MethodArgumentCount) {
  const auto r = run("a = msgs.open_area(\"general\")\nprint a.header()\n");
  ASSERT_EQ(1u, r.errors.size());
  EXPECT_EQ("viewer.bas:2: error: area.header: expected 1 argument(s), got 0", r.errors.front());
}

TEST_F(ObjectTest, ErrorFromNativeCode) {
  const auto r = run("print 1\na = msgs.open_area(\"nosuch\")\n");
  ASSERT_EQ(1u, r.errors.size());
  EXPECT_EQ("viewer.bas:2: error: No such area: nosuch", r.errors.front());
}

TEST_F(ObjectTest, ObjectsCantBeUsedInArithmeticOrSorted) {
  const auto r = run("a = msgs.open_area(\"general\")\nprint a + 1\n");
  ASSERT_EQ(1u, r.errors.size());
  EXPECT_EQ("viewer.bas:2: error: Operator '+' cannot be used with an area", r.errors.front());
}

TEST(EmbeddingTest, NativeFunctionTakingConstStringRef) {
  // Parameters declared as const std::string& used to bind to a destroyed
  // temporary.
  std::ostringstream out;
  std::istringstream in;
  StdioConsole console(out, in);
  Interpreter interp(console);
  interp.globals().native_function(
      "SHOUT", [](const std::string& s) { return s + std::string(3, '!'); });
  ASSERT_TRUE(interp.run_source("t.bas", "print shout(\"a fairly long string, past SSO\")\n").ok());
  EXPECT_EQ("a fairly long string, past SSO!!!\n", out.str());
}
