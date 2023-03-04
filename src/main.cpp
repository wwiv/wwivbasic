
/* Copyright (c) 2012-2017 The ANTLR Project. All rights reserved.
 * Use of this file is governed by the BSD 3-clause license that
 * can be found in the LICENSE.txt file in the project root.
 */

 //
 //  main.cpp
 //  antlr4-cpp-demo
 //
 //  Created by Mike Lischke on 13.03.16.
 //

#include <cerrno>
#include <cstdio>
#include <iostream>
#include <optional>
#include <string>
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


using namespace antlr4;
using namespace wwivbasic;

int main(int argc, const char* argv[]) {
  const auto text = get_file_contents("example.bas");
  if (!text) {
    fmt::print("Unable to open file: example.bas\r\n");
    return 1;
  }
  ANTLRInputStream input(text.value());
  BasicLexer lexer(&input);
  CommonTokenStream tokens(&lexer);

  BasicParser parser(&tokens);
  tree::ParseTree* tree = parser.main();

  auto s = tree->toStringTree(&parser, true);
  std::cout << "Parse Tree: " << s << std::endl;

  std::cout << std::endl << std::endl;

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