#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "ast/AST.h"

#include <cstdint>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Translates a Cambridge IGCSE pseudocode AST into executable Python 3 source.
//
// The mapping follows the meaning of the IGCSE constructs rather than their
// spelling:
//
//   DECLARE / CONSTANT     -> initial assignment (Python has no declarations)
//   arrays                 -> nested Python lists, with the declared lower
//                             bound subtracted from every index
//   IF / CASE / loops      -> if / if-elif / for / while
//   procedures, functions  -> def
//   OPENFILE etc.          -> a dictionary of open file objects
//   library routines       -> len / lower / upper / helper functions
//
// DIV and MOD keep the integer-division behaviour described by the syllabus
// (the fractional part is discarded, so negative operands truncate towards
// zero). ROUND and RANDOM accept both the current forms, ROUND(value, places)
// and RANDOM(), and the older one-argument forms.
//
// Identifiers are case-insensitive in IGCSE pseudocode, so each spelling is
// mapped to one Python name (the first spelling seen). A name that would clash
// with a Python keyword or with a name used by the generated code gets a
// trailing underscore.
class CodeGenerator
{
public:
  CodeGenerator() = default;

  std::string generate(const Program& program);

  static std::string toString(const Program& program);

private:
  struct ArrayBound
  {
    const Expr* expression = nullptr;
    // Set when the bound is not a compile-time integer. The generated program
    // stores the declared value here so that index translation never evaluates
    // the bound expression a second time.
    std::string capture;
  };

  struct Symbol
  {
    DataType type = DataType::INTEGER;
    bool isArray = false;
    std::vector<ArrayBound> lowerBounds;
  };

  using Scope = std::unordered_map<std::string, Symbol>;

  std::ostringstream body_;

  int indent_ = 0;
  int tempCounter_ = 0;
  bool usesRandom_ = false;
  bool usesRound_ = false;
  bool usesSubstring_ = false;
  bool usesDiv_ = false;
  bool usesMod_ = false;
  bool usesText_ = false;
  bool usesChar_ = false;
  bool usesDynamicRange_ = false;
  bool usesFiles_ = false;

  std::unordered_map<std::string, std::string> names_;
  std::unordered_set<std::string> usedNames_;
  std::vector<Scope> scopes_;

  // identifiers and scopes
  std::string pythonName(const std::string& name);
  std::string freshName(const std::string& prefix);
  void pushScope();
  void popScope();
  void declare(const std::string& name, Symbol symbol);
  const Symbol* lookup(const std::string& name) const;

  // statements
  void generateStatements(const std::vector<StmtPtr>& statements);
  void generateBlock(const std::vector<StmtPtr>& statements);
  void generateStatement(const Stmt& statement);
  void prescanGlobals(const std::vector<StmtPtr>& statements);
  void generateVariableDeclaration(const VariableDeclarationStmt& statement);
  void generateConstantDeclaration(const ConstantDeclarationStmt& statement);
  void generateArrayDeclaration(const ArrayDeclarationStmt& statement);
  void generateAssignment(const AssignmentStmt& statement);
  void generateInput(const InputStmt& statement);
  void generateOutput(const OutputStmt& statement);
  void generateIf(const IfStmt& statement);
  void generateCase(const CaseStmt& statement);
  void generateFor(const ForStmt& statement);
  void generateWhile(const WhileStmt& statement);
  void generateRepeat(const RepeatStmt& statement);
  void generateProcedure(const ProcedureDeclarationStmt& statement);
  void generateFunction(const FunctionDeclarationStmt& statement);
  void generateSubprogram(const std::string& name, const std::vector<Parameter>& parameters,
                          const std::vector<StmtPtr>& body);
  void collectAssignments(const std::vector<StmtPtr>& statements,
                          std::unordered_set<std::string>& assigned,
                          std::unordered_set<std::string>& locals) const;
  void emitGlobalDeclaration(const std::unordered_set<std::string>& assigned,
                             const std::unordered_set<std::string>& locals);
  void generateCall(const CallStmt& statement);
  void generateReturn(const ReturnStmt& statement);
  void generateOpenFile(const OpenFileStmt& statement);
  void generateReadFile(const ReadFileStmt& statement);
  void generateWriteFile(const WriteFileStmt& statement);
  void generateCloseFile(const CloseFileStmt& statement);
  void generateAssignmentTo(const LValueExpr& target, const std::string& value);

  // expressions
  std::string generateExpression(const Expr& expression, int minimumPrecedence = 0);
  int precedenceOf(const Expr& expression) const;
  std::string generateArguments(const std::vector<ExprPtr>& arguments);
  std::string generateArrayAccess(const ArrayAccessExpr& access);
  std::string generateBuiltinCall(const FunctionCallExpr& call);

  // helpers
  void line(const std::string& text);
  void blankLine();
  std::string defaultValue(DataType type) const;
  DataType targetType(const LValueExpr& target) const;
  std::string inputExpression(DataType type);
  std::string readExpression(const std::string& stream, DataType type);
  std::string arrayIndex(const Expr& index, const ArrayBound& lowerBound);
  std::string arrayInitializer(const ArrayDeclarationStmt& statement,
                               const std::vector<std::string>& lowerBounds,
                               const std::vector<std::string>& upperBounds);
  std::string rangeExpression(const ForStmt& statement);
  bool foldInteger(const Expr& expression, std::int64_t& value) const;
  void emitPreamble(std::ostringstream& document) const;
};

#endif
