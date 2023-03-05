#include "executor.h"
#include "utils.h"
#include "BasicLexer.h"
#include "core/stl.h"
#include "core/strings.h"
#include "fmt/format.h"

namespace wwivbasic {

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
    for (const auto param : ctx->parameterList()->expr()) {
      std::cout << "param: " << param->getText() << std::endl;
    }
    const auto ctxparams = visit(ctx->parameterList());
    params = std::any_cast<std::vector<Value>>(ctxparams);
  }
  auto val = ec_.call(fn_name, params, this);
  return val.toAny();
}

std::any ExecutionVisitor::visitParameterList(BasicParser::ParameterListContext* context) {
  std::vector<Value> result;
  for (auto* expr : context->expr()) {
    // todo: add visitId
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


std::any ExecutionVisitor::visitParens(BasicParser::ParensContext* context) {
  return visit(context->children.front());
}

std::any ExecutionVisitor::visitString(BasicParser::StringContext* context) {
  return remove_quotes(context->getText());
}

std::any ExecutionVisitor::visitInt(BasicParser::IntContext* context) {
  return wwiv::strings::to_number<int>(context->getText());
}

std::any
ExecutionVisitor::visitAssignmentStatement(BasicParser::AssignmentStatementContext* context) {
  const auto varname = context->variable()->getText();
  const auto value = Value(visit(context->expr()));
  std::cout << "ASSIGN: " << varname << " = " << value.toString() << std::endl;
  //TOOD(rushfan): Once we had "MODULE modulename" support, need to load these
  // into the RIGHT module.
  ec_.upsert(varname, value);
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
  std::cout << left.toString() << op->getText() << right.toString() << " = " << std::boolalpha
            << result << std::endl;
  return result;
}

std::any ExecutionVisitor::visitIdent(BasicParser::IdentContext* context) {
  return ec_.var(context->getText()).toAny();
}

std::any ExecutionVisitor::visitProcCall(BasicParser::ProcCallContext* context) {
  return visitChildren(context);
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
  std::cout << left.toString() << op << right.toString() << " = " << result.toString() << std::endl;
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
  std::cout << left.toString() << op << right.toString() << " = " << result.toString() << std::endl;
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
  } else {
    return visit(context->statements(1));
  }
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

std::any ExecutionVisitor::visitReturnStatement(BasicParser::ReturnStatementContext* context) {
  auto result = visit(context->expr());
  std::cout << "RETURN: " << Value(result).toString() << std::endl;
  return result;
}

std::any ExecutionVisitor::visitVariable(BasicParser::VariableContext* context) {
  return ec_.var(context->getText()).toAny();
}
} // namespace wwivbasic
