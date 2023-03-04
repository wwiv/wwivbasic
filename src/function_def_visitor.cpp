#include "function_def_visitor.h"

#include <string>
#include <vector>

namespace wwivbasic {

std::any FunctionDefVisitor::visitProcedureDefinition(BasicParser::ProcedureDefinitionContext* context) {
  auto params = visitParameterDefinitionList(context->parameterDefinitionList());
  auto name = context->procedureName()->getText();

  basic_function_t f{};
  f.name = name;
  f.type = basic_function_t::Type::BASIC;
  f.params = std::any_cast<std::vector<std::string>>(params);
  f.fn = context;

  ec_.functions[name] = f;
  return {};
}

std::any FunctionDefVisitor::visitParameterDefinitionList(BasicParser::ParameterDefinitionListContext* context) {
  auto ids = context->id();
  std::vector<std::string> result;
  for (auto* id : ids) {
    // todo: add visitId
    result.push_back(id->ID()->getText());
  }

  return result;
}


}
