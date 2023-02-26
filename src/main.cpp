
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
#include "TLexer.h"
#include "TParser.h"

using namespace antlrcpptest;
using namespace antlr4;

int main(int argc, const char* argv[]) {

  ANTLRInputStream input(R"(
IF a = 1 THEN 
  print(1) 
ELSE
  print(0) 
ENDIF
f = foo(a, 123 + 17)
a = b + "c"
)");
  TLexer lexer(&input);
  CommonTokenStream tokens(&lexer);

  TParser parser(&tokens);
  tree::ParseTree* tree = parser.main();

  auto s = tree->toStringTree(&parser, true);
  std::cout << "Parse Tree: " << s << std::endl;

  return 0;
}