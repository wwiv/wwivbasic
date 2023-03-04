#include "executor.h"
#include "BasicLexer.h"
#include "fmt/format.h"

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
    const auto op = context->relationaloperator()->getStart();
    Value left(visit(context->expr(0)));
    Value right(visit(context->expr(1)));
    bool result{ false };
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
    std::cout << left.toString() << op->getText() << right.toString() << " = " << std::boolalpha << result << std::endl;
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
    if (const auto result = std::any_cast<bool>(visit(context->relationalExpression())); !result) {
      return {};
    }
    return visit(context->statements());
  }

  std::any ExecutionVisitor::visitIfThenElseStatement(BasicParser::IfThenElseStatementContext* context) {
    if (const auto result = std::any_cast<bool>(visit(context->relationalExpression()))) {
      return visit(context->statements(0));
    }
    else {
      return visit(context->statements(1));
    }
  }

  std::any ExecutionVisitor::visitIfThenElseIfElseStatement(BasicParser::IfThenElseIfElseStatementContext* context) {
    // Visit the IF and any ELSEIF clauses (that have relation expressions)
    for (int i=0; i < context->relationalExpression().size(); i++) {
      if (const auto result = std::any_cast<bool>(visit(context->relationalExpression(i)))) {
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
}
