#include "executor.h"
#include "utils.h"
#include "BasicLexer.h"
#include "core/stl.h"
#include "core/strings.h"
#include "fmt/format.h"

namespace wwivbasic {

using namespace wwiv::stl;
using namespace wwiv::strings;

std::any ExecutionVisitor::visitMain(BasicParser::MainContext* context) {
  // When starting, reset the module to the root.
  ec_.module = ec_.root;

  return visitChildren(context);
}

std::any ExecutionVisitor::visitProcedureCall(BasicParser::ProcedureCallContext* ctx) {
  if (!ctx->procedureName()) {
    return {};
  }
  const auto fn_name = ctx->procedureName()->getText();
  std::cout << "Procedure Call: " << fn_name << std::endl;
  std::vector<Value> params;
  if (ctx->parameterList()) {
    const auto ctxparams = visit(ctx->parameterList());
    params = std::any_cast<std::vector<Value>>(ctxparams);
  }
  auto val = ec_.call(fn_name, params, this);
  return_ = false;
  return val.toAny();
}

std::any ExecutionVisitor::visitParameterList(BasicParser::ParameterListContext* context) {
  std::vector<Value> result;
  for (auto* expr : context->expr()) {
    result.push_back(Value(visit(expr)));
  }
  return result;
}

std::any
ExecutionVisitor::visitProcedureDefinition(BasicParser::ProcedureDefinitionContext* context) {
  // Ignore procedure defintions while executing, we've already grabbed a
  // reference to it earlier in another visitor.
  return {};
}

std::any ExecutionVisitor::visitImportModule(BasicParser::ImportModuleContext* context) {
  if (context->ID()) {
    // import package
    auto modulename = context->ID()->getText();
    fmt::print("Import module: '{}'\n", modulename);
    ec_.module->imported_modules.emplace(modulename);
  }
  else if (context->STRING()) {
    auto fn = remove_quotes(context->STRING()->getText());
    fmt::print("Import file: '{}'\n", fn);
  }
  else {
    fmt::print("Malformed import statement: '{}'\n", context->getText());

  }

  return {};
}

std::any ExecutionVisitor::visitStatements(BasicParser::StatementsContext* context) {
  std::any result;
  for (auto* stmt : context->statement()) {
    result = visitStatement(stmt);
    if (return_) {
      return result;
    }
  }
  return result;
}

std::any ExecutionVisitor::visitStatement(BasicParser::StatementContext* context) {
  return visitChildren(context);
}

std::any ExecutionVisitor::visitParens(BasicParser::ParensContext* context) {
  return visit(context->children.front());
}

std::any ExecutionVisitor::visitString(BasicParser::StringContext* context) {
  return remove_quotes(context->getText());
}

std::any ExecutionVisitor::visitBooleanExpr(BasicParser::BooleanExprContext* context) {
  return context->TRUE() != nullptr;
}

std::any ExecutionVisitor::visitInt(BasicParser::IntContext* context) {
  return to_number<int>(context->getText());
}

std::any
ExecutionVisitor::visitAssignmentStatement(BasicParser::AssignmentStatementContext* context) {
  // TODO(rushfan): make this return the lvalue if we want to support chaining
  // of assignments like:
  // a = b = 10
  const auto lvalue_name = context->lvalue()->getText();
  if (context->expr()) {
    const auto value = Value(visit(context->expr()));
    std::cout << "ASSIGN: " << lvalue_name << " = " << value << std::endl;
    ec_.upsert(lvalue_name, value);
  }
  else if (context->rvalue()) {
    const auto rvalueName = context->rvalue()->getText();
    if (auto rvalue = ec_.var(rvalueName)) {
      std::cout << "ASSIGN LVALUE=RVALUE: " << lvalue_name << " = " << rvalue.value().value() << std::endl;
      ec_.upsert(lvalue_name, rvalue.value().value());
    }
  }
  else {
    fmt::print("Need expr or rvalue for assignment.");
  }
  return {};
}
 
std::any ExecutionVisitor::visitRelation(BasicParser::RelationContext* context) {

  const auto op = context->relationaloperator()->getStart();
  Value left(visit(context->expr(0)));
  Value right(visit(context->expr(1)));
  bool result{false};
  switch (op->getType()) {
  case BasicLexer::GT: {
    result = left > right;
  } break;
  case BasicLexer::GE: {
    result = left == right || left > right;
  } break;
  case BasicLexer::LT: {
    result = left < right;
  } break;
  case BasicLexer::LE: {
    result = left == right || left < right;
  } break;
  case BasicLexer::NE: {
    result = left != right;
  } break;
  case BasicLexer::EQ: {
    result = left == right;
  } break;
  default:
    std::cerr << "WTF: " << context->getText();
    return {};
  }
  std::cout << left << op->getText() << right << " = " << std::boolalpha
            << result << std::endl;
  return result;
}

std::any ExecutionVisitor::visitIdent(BasicParser::IdentContext* context) {
  LOG(INFO) << fmt::format("visitIdent: '{}'", context->getText());
  return visitRvalue(context->rvalue());
}


std::any ExecutionVisitor::visitMulDiv(BasicParser::MulDivContext* context) {
  const auto op = context->multiplicativeoperator()->getStart();
  Value left(visit(context->expr(0)));
  Value right(visit(context->expr(1)));
  Value result;
  switch (op->getType()) {
  case BasicLexer::STAR: {
    result = left * right;
  } break;
  case BasicLexer::SLASH: {
    result = left / right;
  } break;
  case BasicLexer::MOD: {
    result = left % right;
  } break;
  case BasicLexer::AND: {
    result = left && right;
  } break;
  default:
    std::cerr << "WTF: " << context->getText();
    return {};
  }
  std::cout << left << op << right << " = " << result << std::endl;
  return result.toAny();
}

std::any ExecutionVisitor::visitAddSub(BasicParser::AddSubContext* context) {
  const auto op = context->additiveoperator()->getStart();
  Value left(visit(context->expr(0)));
  Value right(visit(context->expr(1)));
  Value result;
  switch (op->getType()) {
  case BasicLexer::PLUS: {
    result = left + right;
  } break;
  case BasicLexer::MINUS: {
    result = left - right;
  } break;
  case BasicLexer::OR: {
    result = left || right;
  } break;
  default:
    std::cerr << "WTF: " << context->getText();
    return {};
  }
  std::cout << left << op << right << " = " << result << std::endl;
  return result.toAny();
}

std::any ExecutionVisitor::visitIfThenStatement(BasicParser::IfThenStatementContext* context) {
  if (const auto result = std::any_cast<bool>(visit(context->expr())); !result) {
    return {};
  }
  return visit(context->statements());
}

std::any
ExecutionVisitor::visitIfThenElseStatement(BasicParser::IfThenElseStatementContext* context) {
  if (const auto result = std::any_cast<bool>(visit(context->expr()))) {
    return visit(context->statements(0));
  } 
  return visit(context->statements(1));
}

std::any ExecutionVisitor::visitIfThenElseIfElseStatement(
    BasicParser::IfThenElseIfElseStatementContext* context) {
  // Visit the IF and any ELSEIF clauses (that have relation expressions)
  for (int i = 0; i < context->expr().size(); i++) {
    if (const auto result = std::any_cast<bool>(visit(context->expr(i)))) {
      return visit(context->statements(i));
    }
  }
  if (context->ELSE() != nullptr) {
    // We have an else
    return visit(context->statements().back());
  }
  return {};
}

std::any ExecutionVisitor::visitForStatement(BasicParser::ForStatementContext* ctx) {
  auto steptoken = ctx->STEP();
  const int step = steptoken ? to_number<int>(ctx->INT()->getText()) : 1;
  const auto start = Value(visit(ctx->expr(0)));
  const auto end = Value(visit(ctx->expr(1)));

  const auto varname = ctx->ID()->getText();
  const auto scope_name = fmt::format("FOR {}", varname);

  {
    Scope fnscope(scope_name);
    fnscope.local_vars.insert_or_assign(varname, Var(varname, start));
    // Put the scope on top fo the stack
    ec_.module->scopes.push_back(fnscope);
  }
  auto& var = ec_.module->scopes.back().local_vars.at(varname);
  // TODO(rushfan): May need to change to a while loop.
  // TODO(rushfan): Need to figure out how to add RETURN and BREAK support here.
  for (int current = start.toInt(); current != end.toInt(); current += step) {
    var.value().set(current);
    visit(ctx->statements());
    current = var.value().toInt();
  }
  // Handle last loop where current == end;
  var.value().set(end.toInt());
  visit(ctx->statements());

  // remove latest scope.
  ec_.module->scopes.pop_back();
  return {};
}

std::any ExecutionVisitor::visitReturnStatement(BasicParser::ReturnStatementContext* context) {
  auto result = visit(context->expr());
  std::cout << "RETURN: " << Value(result) << std::endl;
  return_ = true;
  return result;
}

std::any ExecutionVisitor::visitVariable(BasicParser::VariableContext* context) {
  if (auto v = ec_.var(context->getText())) {
    return v->value().toAny();
  }
  return {};
}

// RValue variable, unless we support RValue references, always treat it as
// a value
std::any ExecutionVisitor::visitRvalue(BasicParser::RvalueContext* context) {
  if (auto v = ec_.var(context->getText())) {
    return v->value().toAny();
  }
  return {};
}


} // namespace wwivbasic
