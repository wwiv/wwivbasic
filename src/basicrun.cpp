#include <cerrno>
#include <cstdio>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include "fmt/format.h"
#include "antlr4-runtime.h"
#include "BasicLexer.h"
#include "BasicParser.h"
#include "BasicParserBaseVisitor.h"
#include "executor.h"
#include "execution_context.h"
#include "function_def_visitor.h"

std::optional<std::string> get_file_contents(const char *filename) {
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
  if (argc < 1) {
    fmt::print("Usage: \n\tbasicrun FILENAME.BAS\n\n");
    return 2;
  }

  const auto text = get_file_contents(argv[1]);
  if (!text) {
    fmt::print("Unable to open file: {}\r\n", argv[1]);
    return 1;
  }

  antlr4::ANTLRInputStream input(text.value());
  BasicLexer lexer(&input);
  antlr4::CommonTokenStream tokens(&lexer);

  BasicParser parser(&tokens);
  antlr4::tree::ParseTree* tree = parser.main();

  //auto s = tree->toStringTree(&parser, true);
  //std::cout << "Parse Tree: " << s << std::endl;
  //std::cout << std::endl << std::endl;

  wwivbasic::ExecutionContext execution_context;
  wwivbasic::FunctionDefVisitor fd(execution_context);
  fd.visit(tree);

  execution_context.native_function("PRINT", [](std::vector<Value> args) -> Value {
    for (const auto& arg : args) {
      std::cout << arg.toString() << " ";
    }
    std::cout << std::endl;
    return {};
    });

  ExecutionVisitor v(execution_context);
  v.visit(tree);

  return 0;
}