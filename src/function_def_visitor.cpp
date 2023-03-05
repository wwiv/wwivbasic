#include "function_def_visitor.h"
#include "utils.h"
#include "core/stl.h"

#include <string>
#include <vector>

namespace wwivbasic {

using namespace wwiv::stl;

std::any
FunctionDefVisitor::visitProcedureDefinition(BasicParser::ProcedureDefinitionContext* context) {
  const auto params = visitParameterDefinitionList(context->parameterDefinitionList());
  const auto name = context->procedureName()->getText();

  BasicFunction fn(name, context, std::any_cast<std::vector<std::string>>(params));
  //TOOD(rushfan): Once we had "MODULE modulename" support, need to load these
  // into the rigth module.
  ec_.module->functions.insert_or_assign(name, fn);
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

std::any FunctionDefVisitor::visitModuleDefinition(BasicParser::ModuleDefinitionContext* context) {
  const auto s = context->STRING()->getText();
  module = remove_quotes(s);
  if (!contains(ec_.modules, module)) {
    ec_.modules.emplace(module, Module(module));
    ec_.module = &ec_.modules.at(module);
  }

  return {};
}

} // namespace wwivbasic
