#include "BasicLexer.h"
#include "BasicParser.h"
#include "BasicParserBaseVisitor.h"
#include "antlr4-runtime.h"
#include "core/command_line.h"
#include "core/textfile.h"
#include "execution_context.h"
#include "executor.h"
#include "fmt/format.h"
#include "function_def_visitor.h"
#include <cerrno>
#include <cstdio>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

using namespace wwivbasic;
using namespace wwiv::core;

int main(int argc, char* argv[]) {
  wwiv::core::CommandLine cmdline(argc, argv, {});
  cmdline.add_argument(BooleanCommandLineArgument(
    "show_parsetree", 't', "Display the parse tree before executing", false));
  cmdline.add_argument(BooleanCommandLineArgument(
    "execute", 'e', "Execute the script", true));
  if (!cmdline.Parse()) {
    return 2;
  }

  if (cmdline.remaining().empty()) {
    fmt::print("{}", cmdline.GetHelp());
    return 2;
  }
  const auto& filename = cmdline.remaining().front();

  TextFile f(filename, "rb");
  if (!f) {
    fmt::print("Unable to open file: {}\r\n", filename);
    return 1;
  }

  const auto text = f.ReadFileIntoString();
  antlr4::ANTLRInputStream input(text);
  BasicLexer lexer(&input);
  antlr4::CommonTokenStream tokens(&lexer);

  BasicParser parser(&tokens);
  antlr4::tree::ParseTree* tree = parser.main();

  if (cmdline.barg("show_parsetree")) {
    auto s = tree->toStringTree(&parser, true);
    fmt::print("Parse Tree: {}\r\n\n\n", s);
  }

  wwivbasic::ExecutionContext ec;
  wwivbasic::FunctionDefVisitor fd(ec);
  fd.visit(tree);

  ec.module->native_function("PRINT", [](std::vector<Value> args) -> Value {
    for (const auto& arg : args) {
      std::cout << arg.toString() << " ";
    }
    std::cout << std::endl;
    return {};
  });

  if (cmdline.barg("execute")) {
    ExecutionVisitor v(ec);
    v.visit(tree);
  }

  return 0;
}