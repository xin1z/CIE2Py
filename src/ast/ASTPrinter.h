#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include "ast/AST.h"

#include <iosfwd>
#include <string>
#include <vector>

// Prints a human-readable, indented view of the AST. Every node is shown with
// its NodeKind and the fields that matter for that node, e.g.
//
//   PROGRAM
//     VARIABLE_DECLARATION name=Score type=INTEGER
//     ASSIGNMENT
//       target: VARIABLE name=Score
//       value: BINARY op='+'
//         left: VARIABLE name=Score
//         right: INTEGER_LITERAL value=1
class ASTPrinter
{
public:
  explicit ASTPrinter(std::ostream& out);

  void print(const Program& program);

  static std::string toString(const Program& program);

private:
  std::ostream& out_;

  void printStmt(const Stmt& stmt, int depth);
  void printExpr(const Expr& expr, int depth, const std::string& label);
  void printStmtList(const std::vector<StmtPtr>& body, int depth, const std::string& label);
  void printExprList(const std::vector<ExprPtr>& values, int depth, const std::string& label);
  void printLValueList(const std::vector<LValuePtr>& targets, int depth, const std::string& label);
  void printParameters(const std::vector<Parameter>& parameters, int depth);
  void printDimensions(const std::vector<ArrayDimension>& dimensions, int depth);
  void printClauses(const std::vector<CaseClause>& clauses, int depth);

  void indent(int depth);
  void head(int depth, const std::string& label, const std::string& text);
};

#endif
