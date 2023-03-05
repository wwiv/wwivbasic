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
  wwivbasic::ExecutionContext ec(filename);
  antlr4::tree::ParseTree* tree = ec.parseTree(filename);

  if (cmdline.barg("show_parsetree")) {
    auto& su = ec.sources.at(filename);
    const auto s = su->tree()->toStringTree(su->parser(), true);
    fmt::print("Parse Tree: {}\r\n\n\n", s);
  }

  wwivbasic::FunctionDefVisitor fd(ec);
  fd.visit(tree);

  Module io("wwiv.io");
  io.native_function("PRINT", [](std::vector<Value> args) -> Value {
    if (!args.empty()) {
      fmt::print("WWIV.IO: {}\r\n", args.front().toString());
    }
    return {};
    });
  ec.modules.insert_or_assign("wwiv.io", io);

  ec.module->native_function("PRINT", [](std::vector<Value> args) -> Value {
    for (const auto& arg : args) {
      std::cout << arg.toString() << " ";
    }
    std::cout << std::endl;
    return {};
  });
  ec.module->native_function("VERSION", [](std::vector<Value> args) -> Value {
    return Value("1.0.2");
    });

  if (cmdline.barg("execute")) {
    ExecutionVisitor v(ec);
    v.visit(tree);
  }

  return 0;
}