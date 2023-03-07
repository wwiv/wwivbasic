#include "BasicLexer.h"
#include "BasicParser.h"
#include "BasicParserBaseVisitor.h"
#include "antlr4-runtime.h"
#include "context.h"
#include "executor.h"
#include "fmt/format.h"
#include "function_def_visitor.h"
#include <cerrno>
#include <cstdio>
#include <iostream>
#include <optional>
#include <string>

std::optional<std::string> get_file_contents(const char* filename) {
  if (auto* fp = std::fopen(filename, "rb")) {
    std::string contents;
    std::fseek(fp, 0, SEEK_END);
    contents.resize(std::ftell(fp));
    std::rewind(fp);
    std::fread(&contents[0], 1, contents.size(), fp);
    std::fclose(fp);
    return contents;
  }
  return std::nullopt;
}

using namespace wwivbasic;

int main(int argc, const char* argv[]) {
  const auto text = get_file_contents("example.bas");
  if (!text) {
    fmt::print("Unable to open file: example.bas\r\n");
    return 1;
  }
  antlr4::ANTLRInputStream input(text.value());
  BasicLexer lexer(&input);
  antlr4::CommonTokenStream tokens(&lexer);

  BasicParser parser(&tokens);
  antlr4::tree::ParseTree* tree = parser.main();

  auto s = tree->toStringTree(&parser, true);
  std::cout << "Parse Tree: " << s << std::endl;

  std::cout << std::endl << std::endl;

  wwivbasic::Context context;
  wwivbasic::FunctionDefVisitor fd(context);
  fd.visit(tree);

  context.module->native_functionl("PRINT", [](std::vector<Value> args) -> Value {
    for (const auto& arg : args) {
      std::cout << arg.toString() << " ";
    }
    std::cout << std::endl;
    return {};
  });

  ExecutionVisitor v(context);
  v.visit(tree);

  return 0;
}