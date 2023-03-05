#pragma once
#include "BasicParserBaseVisitor.h"
#include "antlr4-runtime.h"
#include "execution_context.h"

namespace wwivbasic {

class ExecutionVisitor : public BasicParserBaseVisitor {
public:
  ExecutionVisitor(ExecutionContext& ec) : ec_(ec) {}


  std::any visitMain(BasicParser::MainContext* context) override;

  std::any visitProcedureCall(BasicParser::ProcedureCallContext* context) override;

  std::any visitParameterList(BasicParser::ParameterListContext* context) override;

  std::any visitProcedureDefinition(BasicParser::ProcedureDefinitionContext* context) override;

  // std::any
  // visitParameterDefinitionList(BasicParser::ParameterDefinitionListContext*
  // context) override;

  std::any visitImportModule(BasicParser::ImportModuleContext* context) override;

  std::any visitStatements(BasicParser::StatementsContext* context) override;

  std::any visitStatement(BasicParser::StatementContext* context) override;

  // std::any visitEmptyStatement(BasicParser::EmptyStatementContext* context)
  // override;

  std::any visitRelation(BasicParser::RelationContext* context) override;

  std::any visitIdent(BasicParser::IdentContext* context) override;

  //std::any visitProcCall(BasicParser::ProcCallContext* context) override;

  std::any visitMulDiv(BasicParser::MulDivContext* context) override;

  std::any visitAddSub(BasicParser::AddSubContext* context) override;

  std::any visitParens(BasicParser::ParensContext* context) override;

  std::any visitString(BasicParser::StringContext* context) override;

  std::any visitBooleanExpr(BasicParser::BooleanExprContext* context) override;

  std::any visitInt(BasicParser::IntContext* context) override;

  // std::any visitAdditiveoperator(BasicParser::AdditiveoperatorContext*
  // context) override;

  // std::any
  // visitMultiplicativeoperator(BasicParser::MultiplicativeoperatorContext*
  // context) override;

  // std::any visitRelationaloperator(BasicParser::RelationaloperatorContext*
  // context) override;

  std::any visitAssignmentStatement(BasicParser::AssignmentStatementContext* context) override;

  // std::any visitIfStatement(BasicParser::IfStatementContext* context)
  // override;

  std::any visitIfThenStatement(BasicParser::IfThenStatementContext* context) override;

  std::any visitIfThenElseStatement(BasicParser::IfThenElseStatementContext* context) override;

  std::any
  visitIfThenElseIfElseStatement(BasicParser::IfThenElseIfElseStatementContext* context) override;

  std::any visitForStatement(BasicParser::ForStatementContext* ctx) override;

  std::any visitReturnStatement(BasicParser::ReturnStatementContext* context) override;

  // std::any visitId(BasicParser::IdContext* context) override;

  std::any visitVariable(BasicParser::VariableContext* context) override;

  // std::any visitProcedureName(BasicParser::ProcedureNameContext* context)
  // override;

  // std::any visitPackageName(BasicParser::PackageNameContext* context)
  // override;

private:
  ExecutionContext& ec_;
  bool return_{ false };
  bool break_{ false };
};

} // namespace wwivbasic
