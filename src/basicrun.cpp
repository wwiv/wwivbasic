#include "BasicLexer.h"
#include "BasicParser.h"
#include "BasicParserBaseVisitor.h"
#include "antlr4-runtime.h"
#include "core/command_line.h"
#include "core/log.h"
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

std::string easy(std::string s) {
  fmt::print("easy: {}\n", s);
  return {};
}

std::string easy2(std::string s, std::string s1) {
  fmt::print("easy2: {} {}\n", s, s1);
  return {};
}

int main(int argc, char* argv[]) {
  LoggerConfig config;

  Logger::Init(argc, argv, config);
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
  auto tree = ec.parseTree(filename);

  if (!tree) {
    fmt::print("Unable to parse tree");
    return 1;
  }

  if (cmdline.barg("show_parsetree")) {
    auto& su = ec.sources.at(filename);
    const auto s = su->tree()->toStringTree(su->parser(), true);
    fmt::print("Parse Tree: {}\r\n\n\n", s);
  }

  wwivbasic::FunctionDefVisitor fd(ec);
  fd.visit(tree.value());

  Module io("wwiv.io");
  io.native_functionl("PRINT", [](std::vector<Value> args) -> Value {
    if (!args.empty()) {
      fmt::print("WWIV.IO: {}\r\n", args.front().toString());
    }
    return {};
    });
  ec.modules.insert_or_assign("wwiv.io", io);

  ec.root->native_functionl("PRINT", [](std::vector<Value> args) -> Value {
    for (const auto& arg : args) {
      std::cout << arg.toString() << " ";
    }
    std::cout << std::endl;
    return {};
  });
  REGISTER_NATIVE(ec.root, easy);
  ec.root->native_function("EASY2", easy2);
  ec.root->native_functionl("PRINT", [](std::vector<Value> args) -> Value {
    for (const auto& arg : args) {
      std::cout << arg.toString() << " ";
    }
    std::cout << std::endl;
    return {};
    });

  if (cmdline.barg("execute")) {
    ExecutionVisitor v(ec);
    v.visit(tree.value());
  }

  return 0;
}