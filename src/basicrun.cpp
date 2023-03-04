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

int main(int argc, char* argv[]) {
  wwiv::core::CommandLine cmdline_(argc, argv, {});
  if (argc < 1) {
    fmt::print("Usage: \n\tbasicrun FILENAME.BAS\n\n");
    return 2;
  }

  TextFile f(argv[1], "rb");
  if (!f) {
    fmt::print("Unable to open file: {}\r\n", argv[1]);
    return 1;
  }

  const auto text = f.ReadFileIntoString();
  antlr4::ANTLRInputStream input(text);
  BasicLexer lexer(&input);
  antlr4::CommonTokenStream tokens(&lexer);

  BasicParser parser(&tokens);
  antlr4::tree::ParseTree* tree = parser.main();

  // auto s = tree->toStringTree(&parser, true);
  // std::cout << "Parse Tree: " << s << std::endl;
  // std::cout << std::endl << std::endl;

  wwivbasic::ExecutionContext ec;
  wwivbasic::FunctionDefVisitor fd(ec);
  fd.visit(tree);

  ec.native_function("PRINT", [](std::vector<Value> args) -> Value {
    for (const auto& arg : args) {
      std::cout << arg.toString() << " ";
    }
    std::cout << std::endl;
    return {};
  });

  ExecutionVisitor v(ec);
  v.visit(tree);

  return 0;
}