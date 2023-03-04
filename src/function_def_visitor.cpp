#include "function_def_visitor.h"

#include <string>
#include <vector>

namespace wwivbasic {

std::any
FunctionDefVisitor::visitProcedureDefinition(BasicParser::ProcedureDefinitionContext* context) {
  const auto params = visitParameterDefinitionList(context->parameterDefinitionList());
  const auto name = context->procedureName()->getText();

  BasicFunction fn(name, context, std::any_cast<std::vector<std::string>>(params));
  ec_.functions.insert_or_assign(name, fn);
  return {};
}

std::any FunctionDefVisitor::visitParameterDefinitionList(
    BasicParser::ParameterDefinitionListContext* context) {
  auto ids = context->id();
  std::vector<std::string> result;
  for (auto* id : ids) {
    // todo: add visitId
    result.push_back(id->ID()->getText());
  }

  return result;
}

} // namespace wwivbasic
