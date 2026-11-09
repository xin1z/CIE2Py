#include "ASTPrinter.h"
#include "LiteralFormat.h"

#include <ostream>
#include <sstream>

namespace
{
const char* dataTypeName(DataType type)
{
  switch (type)
  {
  case DataType::INTEGER:
    return "INTEGER";
  case DataType::REAL:
    return "REAL";
  case DataType::CHAR:
    return "CHAR";
  case DataType::STRING:
    return "STRING";
  case DataType::BOOLEAN:
    return "BOOLEAN";
  }

  return "UNKNOWN";
}

const char* unaryOpName(UnaryOp op)
{
  switch (op)
  {
  case UnaryOp::NEGATE:
    return "-";
  case UnaryOp::NOT:
    return "NOT";
  }

  return "UNKNOWN";
}

const char* binaryOpName(BinaryOp op)
{
  switch (op)
  {
  case BinaryOp::ADD:
    return "+";
  case BinaryOp::SUBTRACT:
    return "-";
  case BinaryOp::MULTIPLY:
    return "*";
  case BinaryOp::DIVIDE:
    return "/";
  case BinaryOp::POWER:
    return "^";
  case BinaryOp::DIV:
    return "DIV";
  case BinaryOp::MOD:
    return "MOD";
  case BinaryOp::EQUAL:
    return "=";
  case BinaryOp::NOT_EQUAL:
    return "<>";
  case BinaryOp::LESS:
    return "<";
  case BinaryOp::LESS_EQUAL:
    return "<=";
  case BinaryOp::GREATER:
    return ">";
  case BinaryOp::GREATER_EQUAL:
    return ">=";
  case BinaryOp::AND:
    return "AND";
  case BinaryOp::OR:
    return "OR";
  }

  return "UNKNOWN";
}

const char* fileModeName(FileMode mode)
{
  switch (mode)
  {
  case FileMode::READ:
    return "READ";
  case FileMode::WRITE:
    return "WRITE";
  }

  return "UNKNOWN";
}

} // namespace

ASTPrinter::ASTPrinter(std::ostream& out) : out_(out) {}

void ASTPrinter::print(const Program& program)
{
  out_ << "PROGRAM\n";
  printStmtList(program.statements, 1, "statements");
}

std::string ASTPrinter::toString(const Program& program)
{
  std::ostringstream stream;
  ASTPrinter printer(stream);
  printer.print(program);
  return stream.str();
}

void ASTPrinter::indent(int depth)
{
  out_ << std::string(depth * 2, ' ');
}

void ASTPrinter::head(int depth, const std::string& label, const std::string& text)
{
  indent(depth);
  if (!label.empty())
    out_ << label << ": ";
  out_ << text << '\n';
}

void ASTPrinter::printStmtList(const std::vector<StmtPtr>& body, int depth,
                               const std::string& label)
{
  indent(depth);
  out_ << label << ':';
  if (body.empty())
    out_ << " (empty)";
  out_ << '\n';

  for (const StmtPtr& statement : body)
    printStmt(*statement, depth + 1);
}

void ASTPrinter::printExprList(const std::vector<ExprPtr>& values, int depth,
                               const std::string& label)
{
  indent(depth);
  out_ << label << ':';
  if (values.empty())
    out_ << " (empty)";
  out_ << '\n';

  for (const ExprPtr& value : values)
    printExpr(*value, depth + 1, "");
}

void ASTPrinter::printLValueList(const std::vector<LValuePtr>& targets, int depth,
                                 const std::string& label)
{
  indent(depth);
  out_ << label << ':';
  if (targets.empty())
    out_ << " (empty)";
  out_ << '\n';

  for (const LValuePtr& target : targets)
    printExpr(*target, depth + 1, "");
}

void ASTPrinter::printParameters(const std::vector<Parameter>& parameters, int depth)
{
  indent(depth);
  out_ << "parameters:";
  if (parameters.empty())
    out_ << " (empty)";
  out_ << '\n';

  for (const Parameter& parameter : parameters)
  {
    indent(depth + 1);
    out_ << "PARAMETER name=" << parameter.name << " type=" << dataTypeName(parameter.type) << '\n';
  }
}

void ASTPrinter::printDimensions(const std::vector<ArrayDimension>& dimensions, int depth)
{
  indent(depth);
  out_ << "dimensions:";
  if (dimensions.empty())
    out_ << " (empty)";
  out_ << '\n';

  for (const ArrayDimension& dimension : dimensions)
  {
    indent(depth + 1);
    out_ << "DIMENSION\n";
    printExpr(*dimension.lower, depth + 2, "lower");
    printExpr(*dimension.upper, depth + 2, "upper");
  }
}

void ASTPrinter::printClauses(const std::vector<CaseClause>& clauses, int depth)
{
  indent(depth);
  out_ << "clauses:";
  if (clauses.empty())
    out_ << " (empty)";
  out_ << '\n';

  for (const CaseClause& clause : clauses)
  {
    indent(depth + 1);
    out_ << "CASE_CLAUSE\n";
    printExpr(*clause.value, depth + 2, "value");
    printStmtList(clause.body, depth + 2, "body");
  }
}

void ASTPrinter::printStmt(const Stmt& stmt, int depth)
{
  switch (stmt.kind)
  {
  case NodeKind::VARIABLE_DECLARATION:
  {
    const auto& declaration = static_cast<const VariableDeclarationStmt&>(stmt);
    head(depth, "",
         "VARIABLE_DECLARATION name=" + declaration.name +
           " type=" + dataTypeName(declaration.type));
    break;
  }
  case NodeKind::CONSTANT_DECLARATION:
  {
    const auto& declaration = static_cast<const ConstantDeclarationStmt&>(stmt);
    head(depth, "", "CONSTANT_DECLARATION name=" + declaration.name);
    printExpr(*declaration.value, depth + 1, "value");
    break;
  }
  case NodeKind::ARRAY_DECLARATION:
  {
    const auto& declaration = static_cast<const ArrayDeclarationStmt&>(stmt);
    head(depth, "",
         "ARRAY_DECLARATION name=" + declaration.name +
           " elementType=" + dataTypeName(declaration.elementType));
    printDimensions(declaration.dimensions, depth + 1);
    break;
  }
  case NodeKind::ASSIGNMENT:
  {
    const auto& assignment = static_cast<const AssignmentStmt&>(stmt);
    head(depth, "", "ASSIGNMENT");
    printExpr(*assignment.target, depth + 1, "target");
    printExpr(*assignment.value, depth + 1, "value");
    break;
  }
  case NodeKind::INPUT:
  {
    const auto& input = static_cast<const InputStmt&>(stmt);
    head(depth, "", "INPUT");
    printLValueList(input.targets, depth + 1, "targets");
    break;
  }
  case NodeKind::OUTPUT:
  {
    const auto& output = static_cast<const OutputStmt&>(stmt);
    head(depth, "", "OUTPUT");
    printExprList(output.values, depth + 1, "values");
    break;
  }
  case NodeKind::IF:
  {
    const auto& ifStatement = static_cast<const IfStmt&>(stmt);
    head(depth, "", "IF");
    printExpr(*ifStatement.condition, depth + 1, "condition");
    printStmtList(ifStatement.thenBody, depth + 1, "then");
    if (!ifStatement.elseBody.empty())
      printStmtList(ifStatement.elseBody, depth + 1, "else");
    break;
  }
  case NodeKind::CASE:
  {
    const auto& caseStatement = static_cast<const CaseStmt&>(stmt);
    head(depth, "", "CASE");
    printExpr(*caseStatement.selector, depth + 1, "selector");
    printClauses(caseStatement.clauses, depth + 1);
    if (!caseStatement.otherwiseBody.empty())
      printStmtList(caseStatement.otherwiseBody, depth + 1, "otherwise");
    break;
  }
  case NodeKind::FOR:
  {
    const auto& forStatement = static_cast<const ForStmt&>(stmt);
    head(depth, "", "FOR variable=" + forStatement.variable);
    printExpr(*forStatement.start, depth + 1, "start");
    printExpr(*forStatement.end, depth + 1, "end");
    if (forStatement.step)
      printExpr(*forStatement.step, depth + 1, "step");
    printStmtList(forStatement.body, depth + 1, "body");
    break;
  }
  case NodeKind::WHILE:
  {
    const auto& whileStatement = static_cast<const WhileStmt&>(stmt);
    head(depth, "", "WHILE");
    printExpr(*whileStatement.condition, depth + 1, "condition");
    printStmtList(whileStatement.body, depth + 1, "body");
    break;
  }
  case NodeKind::REPEAT:
  {
    const auto& repeatStatement = static_cast<const RepeatStmt&>(stmt);
    head(depth, "", "REPEAT");
    printStmtList(repeatStatement.body, depth + 1, "body");
    printExpr(*repeatStatement.condition, depth + 1, "condition");
    break;
  }
  case NodeKind::PROCEDURE_DECLARATION:
  {
    const auto& procedure = static_cast<const ProcedureDeclarationStmt&>(stmt);
    head(depth, "", "PROCEDURE name=" + procedure.name);
    printParameters(procedure.parameters, depth + 1);
    printStmtList(procedure.body, depth + 1, "body");
    break;
  }
  case NodeKind::FUNCTION_DECLARATION:
  {
    const auto& function = static_cast<const FunctionDeclarationStmt&>(stmt);
    head(depth, "",
         "FUNCTION name=" + function.name + " returnType=" + dataTypeName(function.returnType));
    printParameters(function.parameters, depth + 1);
    printStmtList(function.body, depth + 1, "body");
    break;
  }
  case NodeKind::CALL:
  {
    const auto& call = static_cast<const CallStmt&>(stmt);
    head(depth, "", "CALL name=" + call.name);
    printExprList(call.arguments, depth + 1, "arguments");
    break;
  }
  case NodeKind::RETURN:
  {
    const auto& returnStatement = static_cast<const ReturnStmt&>(stmt);
    head(depth, "", "RETURN");
    printExpr(*returnStatement.value, depth + 1, "value");
    break;
  }
  case NodeKind::OPEN_FILE:
  {
    const auto& openFile = static_cast<const OpenFileStmt&>(stmt);
    head(depth, "", "OPEN_FILE mode=" + std::string(fileModeName(openFile.mode)));
    printExpr(*openFile.fileName, depth + 1, "fileName");
    break;
  }
  case NodeKind::READ_FILE:
  {
    const auto& readFile = static_cast<const ReadFileStmt&>(stmt);
    head(depth, "", "READ_FILE");
    printExpr(*readFile.fileName, depth + 1, "fileName");
    printExpr(*readFile.target, depth + 1, "target");
    break;
  }
  case NodeKind::WRITE_FILE:
  {
    const auto& writeFile = static_cast<const WriteFileStmt&>(stmt);
    head(depth, "", "WRITE_FILE");
    printExpr(*writeFile.fileName, depth + 1, "fileName");
    printExpr(*writeFile.value, depth + 1, "value");
    break;
  }
  case NodeKind::CLOSE_FILE:
  {
    const auto& closeFile = static_cast<const CloseFileStmt&>(stmt);
    head(depth, "", "CLOSE_FILE");
    printExpr(*closeFile.fileName, depth + 1, "fileName");
    break;
  }
  default:
    head(depth, "", "UNKNOWN_STATEMENT");
    break;
  }
}

void ASTPrinter::printExpr(const Expr& expr, int depth, const std::string& label)
{
  switch (expr.kind)
  {
  case NodeKind::INTEGER_LITERAL:
  {
    const auto& literal = static_cast<const IntegerLiteralExpr&>(expr);
    head(depth, label, "INTEGER_LITERAL value=" + std::to_string(literal.value));
    break;
  }
  case NodeKind::REAL_LITERAL:
  {
    const auto& real = static_cast<const RealLiteralExpr&>(expr);
    head(depth, label, "REAL_LITERAL value=" + literal::formatReal(real.value));
    break;
  }
  case NodeKind::BOOLEAN_LITERAL:
  {
    const auto& literal = static_cast<const BooleanLiteralExpr&>(expr);
    head(depth, label, std::string("BOOLEAN_LITERAL value=") + (literal.value ? "TRUE" : "FALSE"));
    break;
  }
  case NodeKind::CHAR_LITERAL:
  {
    const auto& character = static_cast<const CharLiteralExpr&>(expr);
    head(depth, label, "CHAR_LITERAL value=" + literal::quoteChar(character.value));
    break;
  }
  case NodeKind::STRING_LITERAL:
  {
    const auto& string = static_cast<const StringLiteralExpr&>(expr);
    head(depth, label, "STRING_LITERAL value=" + literal::quoteString(string.value));
    break;
  }
  case NodeKind::VARIABLE:
  {
    const auto& variable = static_cast<const VariableExpr&>(expr);
    head(depth, label, "VARIABLE name=" + variable.name);
    break;
  }
  case NodeKind::ARRAY_ACCESS:
  {
    const auto& access = static_cast<const ArrayAccessExpr&>(expr);
    head(depth, label, "ARRAY_ACCESS name=" + access.name);
    printExprList(access.indices, depth + 1, "indices");
    break;
  }
  case NodeKind::UNARY:
  {
    const auto& unary = static_cast<const UnaryExpr&>(expr);
    head(depth, label, std::string("UNARY op='") + unaryOpName(unary.op) + "'");
    printExpr(*unary.operand, depth + 1, "operand");
    break;
  }
  case NodeKind::BINARY:
  {
    const auto& binary = static_cast<const BinaryExpr&>(expr);
    head(depth, label, std::string("BINARY op='") + binaryOpName(binary.op) + "'");
    printExpr(*binary.left, depth + 1, "left");
    printExpr(*binary.right, depth + 1, "right");
    break;
  }
  case NodeKind::FUNCTION_CALL:
  {
    const auto& call = static_cast<const FunctionCallExpr&>(expr);
    head(depth, label, "FUNCTION_CALL name=" + call.name);
    printExprList(call.arguments, depth + 1, "arguments");
    break;
  }
  default:
    head(depth, label, "UNKNOWN_EXPRESSION");
    break;
  }
}
