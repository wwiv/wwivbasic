#pragma once
#include "antlr4-runtime.h"
#include "BasicParser.h"
#include "BasicParserBaseVisitor.h"
#include "execution_context.h"

#include <map>
#include <string>
#include <vector>

namespace wwivbasic {

  /**
   * This class defines an abstract visitor for a parse tree
   * produced by BasicParser.
   */
  class  FunctionDefVisitor : public BasicParserBaseVisitor {
  public:
    FunctionDefVisitor(ExecutionContext& ec) : ec_(ec) {}

    std::any visitProcedureDefinition(BasicParser::ProcedureDefinitionContext* context) override;

    std::any visitParameterDefinitionList(BasicParser::ParameterDefinitionListContext* context) override;

    std::map<std::string, basic_function_t> functions_;

private:
  ExecutionContext& ec_;
  };

}  // namespace wwivbasic
