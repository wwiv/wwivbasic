
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

#include <iostream>

#include "antlr4-runtime.h"
#include "BasicLexer.h"
#include "BasicParser.h"
#include "BasicParserBaseVisitor.h"
#include "executor.h"
#include "execution_context.h"
#include "function_def_visitor.h"

using namespace wwivbasic;
using namespace antlr4;

class MyVisitor : public wwivbasic::BasicParserBaseVisitor {
  std::any visitProcedureCall(BasicParser::ProcedureCallContext* ctx) override {
    auto result = visitChildren(ctx);
    if (ctx->procedureName()) {
      std::cout << "Procedure Call: " << ctx->procedureName()->getText() << std::endl;
      for (const auto param : ctx->parameterList()->expr()) {
        std::cout << "param: " << param->getText() << std::endl;
      }
    }
    return result;
  }

  std::any visitRelationalExpression(BasicParser::RelationalExpressionContext* ctx) override {
    return visitChildren(ctx);
  }

  std::any visitIfThenStatement(BasicParser::IfThenStatementContext* context) { return {}; }

  std::any visitIfThenElseStatement(BasicParser::IfThenElseStatementContext* ctx) override {
    auto re = ctx->relationalExpression();

    auto s = ctx->statements();
    std::cout << "visitIfThenElseStatement" << s.size() << std::endl;
    return visitChildren(ctx);
  }

  std::any visitChildren(antlr4::tree::ParseTree* node) override {
    auto result = BasicParserBaseVisitor::visitChildren(node);

    std::cout << "visitChildren: " << node->toStringTree() << " | "
      << node->getText() << std::endl;

    return result;
  }



};

int main(int argc, const char* argv[]) {

  ANTLRInputStream input(R"(

def foo(a, b) 
  return a + b
enddef

a = 20
f = foo(a, 123 + 17)
a = a + 12

IF a = 1 THEN 
  print(1) 
ELSE
  print(0) 
ENDIF
)");
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

  for (const auto& [k, v] : fd.functions_) {
    std::cout << "Found Function: " << k << "(";
    bool comma = false;
    for (const auto& p : v.params) {
      if (comma) { std::cout << ",";}
      std::cout << p;
      comma = true;
    }
    std::cout << ")" << std::endl;
    std::cout << v.fn->toStringTree();
  }

  ExecutionVisitor v(execution_context);
  v.visit(tree);

  return 0;
}