#include "executor.h"

namespace wwivbasic {

  std::any ExecutionVisitor::visitProcedureCall(BasicParser::ProcedureCallContext* ctx) {
    if (!ctx->procedureName()) {
      return {};
    }
    const auto fn_name = ctx->procedureName()->getText();
    std::cout << "Procedure Call: " << fn_name << std::endl;
    for (const auto param : ctx->parameterList()->expr()) {
      std::cout << "param: " << param->getText() << std::endl;
    }
    const auto params = visit(ctx->parameterList());
    const auto v = std::any_cast<std::vector<Value>>(params);
    auto val = ec_.call(fn_name, v, this);
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

  std::any ExecutionVisitor::visitProcedureDefinition(BasicParser::ProcedureDefinitionContext* context) {
    // Ignore procedure defintions while executing, we've already grabbed a reference
    // to it earlier in another visitor.
    return {};
  }


  std::any ExecutionVisitor::visitParens(BasicParser::ParensContext* context) {
    return visit(context->children.front());
  }

  std::any ExecutionVisitor::visitString(BasicParser::StringContext* context) {
    auto s = context->getText();
    s.pop_back();
    return (s.size() <= 1) ? "" : s.substr(1);
  }

  std::any ExecutionVisitor::visitInt(BasicParser::IntContext* context) {
    return to_int(context->getText());
  }

  std::any ExecutionVisitor::visitAssignmentStatement(BasicParser::AssignmentStatementContext* context) {
    const auto varname = context->variable()->getText();
    const auto value = Value(visit(context->expr()));
    std::cout << "ASSIGN: " << varname << " = " << value.toString() << std::endl;
    ec_.upsert(varname, value);
    return {};
  }

  std::any ExecutionVisitor::visitRelationalExpression(BasicParser::RelationalExpressionContext * context) {
    return visitChildren(context);
  }

  std::any ExecutionVisitor::visitIdent(BasicParser::IdentContext* context) {
    return ec_.var(context->getText()).toAny();
  }

  std::any ExecutionVisitor::visitProcCall(BasicParser::ProcCallContext* context) {
    return visitChildren(context);
  }

  std::any ExecutionVisitor::visitMulDiv(BasicParser::MulDivContext* context) {
    return visitChildren(context);
  }

  std::any ExecutionVisitor::visitAddSub(BasicParser::AddSubContext* context) {
    const auto op = context->additiveoperator()->getText().front();
    Value left(visit(context->expr(0)));
    Value right(visit(context->expr(1)));
    switch (op) {
    case '+': {
      auto result = left + right;
      std::cout << left.toString() << " + " << right.toString() << " = " << result.toString() << std::endl;
      return result.toAny();
    } break;
    case '-': {
      auto result = left - right;
      return result.toAny();
    } break;
    default:
      std::cerr << "WTF: " << context->getText();
    }
    return visitChildren(context);
  }

  std::any ExecutionVisitor::visitIfThenStatement(BasicParser::IfThenStatementContext* context) { return {}; }

  std::any ExecutionVisitor::visitIfThenElseStatement(BasicParser::IfThenElseStatementContext* ctx) {
    auto re = ctx->relationalExpression();

    auto s = ctx->statements();
    std::cout << "visitIfThenElseStatement" << s.size() << std::endl;
    return visitChildren(ctx);
  }

  std::any ExecutionVisitor::visitReturnStatement(BasicParser::ReturnStatementContext* context) {
    auto result = visit(context->expr());
    std::cout << "RETURN: " << Value(result).toString() << std::endl;
    return result;
  }

}
