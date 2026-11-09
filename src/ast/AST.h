#ifndef AST_H
#define AST_H

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Abstract syntax tree for Cambridge IGCSE pseudocode (0478 / 0984).
//
// The grammar covered here follows the pseudocode appendix of the syllabus:
// declarations, assignment, INPUT/OUTPUT, selection, iteration, arrays,
// procedures/functions and file handling. Each node is owned through
// std::unique_ptr. Nodes are built once by the parser; the public fields keep
// construction and traversal straightforward.
//
// Every node records a representative source location (usually the first token
// of the construct) so that later stages can report errors against the
// original program.
//
// Identifiers are stored exactly as written. IGCSE identifiers are
// case-insensitive; the code generator maps them to Python names.
// Keywords and built-in routine names are also stored as written.

struct SourceLocation
{
  int line = 1;
  int column = 1;
};

enum class DataType
{
  INTEGER,
  REAL,
  CHAR,
  STRING,
  BOOLEAN
};

enum class UnaryOp
{
  NEGATE, // -
  NOT     // NOT
};

enum class BinaryOp
{
  ADD,           // +
  SUBTRACT,      // -
  MULTIPLY,      // *
  DIVIDE,        // /
  POWER,         // ^
  DIV,           // DIV(a, b) or a DIV b, integer quotient
  MOD,           // MOD(a, b) or a MOD b, remainder
  EQUAL,         // =
  NOT_EQUAL,     // <>
  LESS,          // <
  LESS_EQUAL,    // <=
  GREATER,       // >
  GREATER_EQUAL, // >=
  AND,           // AND
  OR             // OR
};

// Mode used by OPENFILE ... FOR <mode>.
enum class FileMode
{
  READ,
  WRITE
};

struct Expr;
struct Stmt;
struct LValueExpr;

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;
using LValuePtr = std::unique_ptr<LValueExpr>;

enum class NodeKind
{
  PROGRAM,

  // statements
  VARIABLE_DECLARATION,
  CONSTANT_DECLARATION,
  ARRAY_DECLARATION,
  ASSIGNMENT,
  INPUT,
  OUTPUT,
  IF,
  CASE,
  FOR,
  WHILE,
  REPEAT,
  PROCEDURE_DECLARATION,
  FUNCTION_DECLARATION,
  CALL,
  RETURN,
  OPEN_FILE,
  READ_FILE,
  WRITE_FILE,
  CLOSE_FILE,

  // expressions
  INTEGER_LITERAL,
  REAL_LITERAL,
  BOOLEAN_LITERAL,
  CHAR_LITERAL,
  STRING_LITERAL,
  VARIABLE,
  ARRAY_ACCESS,
  UNARY,
  BINARY,
  FUNCTION_CALL
};

struct Node
{
  const NodeKind kind;
  SourceLocation loc;

  virtual ~Node() = default;

protected:
  Node(NodeKind kind, SourceLocation loc) : kind(kind), loc(loc) {}
};

struct Stmt : Node
{
protected:
  Stmt(NodeKind kind, SourceLocation loc) : Node(kind, loc) {}
};

struct Expr : Node
{
protected:
  Expr(NodeKind kind, SourceLocation loc) : Node(kind, loc) {}
};

// An expression that may appear on the left-hand side of an assignment, in
// INPUT, or as the destination of READFILE. In IGCSE pseudocode this is either
// a scalar variable or one element of an array.
struct LValueExpr : Expr
{
protected:
  LValueExpr(NodeKind kind, SourceLocation loc) : Expr(kind, loc) {}
};

// ---------------------------------------------------------------------------
// Shared sub-structures
// ---------------------------------------------------------------------------

struct Parameter
{
  std::string name;
  DataType type;
  SourceLocation loc;

  Parameter(SourceLocation loc, std::string name, DataType type)
      : name(std::move(name)), type(type), loc(loc)
  {
  }
};

// One bound pair of an array declaration, i.e. [lower:upper].
struct ArrayDimension
{
  ExprPtr lower;
  ExprPtr upper;
  SourceLocation loc;

  ArrayDimension(SourceLocation loc, ExprPtr lower, ExprPtr upper)
      : lower(std::move(lower)), upper(std::move(upper)), loc(loc)
  {
  }
};

// One "<value> : <statement>" arm of a CASE statement. The body is a vector so
// that a clause containing more than one statement can still be represented.
struct CaseClause
{
  ExprPtr value;
  std::vector<StmtPtr> body;
  SourceLocation loc;

  CaseClause(SourceLocation loc, ExprPtr value, std::vector<StmtPtr> body)
      : value(std::move(value)), body(std::move(body)), loc(loc)
  {
  }
};

// ---------------------------------------------------------------------------
// Expressions
// ---------------------------------------------------------------------------
//
// Literal values are stored in decoded form: CHAR and STRING values do not
// include their surrounding quotes.

struct IntegerLiteralExpr : Expr
{
  std::int64_t value;

  IntegerLiteralExpr(SourceLocation loc, std::int64_t value)
      : Expr(NodeKind::INTEGER_LITERAL, loc), value(value)
  {
  }
};

struct RealLiteralExpr : Expr
{
  double value;

  RealLiteralExpr(SourceLocation loc, double value)
      : Expr(NodeKind::REAL_LITERAL, loc), value(value)
  {
  }
};

struct BooleanLiteralExpr : Expr
{
  bool value;

  BooleanLiteralExpr(SourceLocation loc, bool value)
      : Expr(NodeKind::BOOLEAN_LITERAL, loc), value(value)
  {
  }
};

struct CharLiteralExpr : Expr
{
  char value;

  CharLiteralExpr(SourceLocation loc, char value) : Expr(NodeKind::CHAR_LITERAL, loc), value(value)
  {
  }
};

struct StringLiteralExpr : Expr
{
  std::string value;

  StringLiteralExpr(SourceLocation loc, std::string value)
      : Expr(NodeKind::STRING_LITERAL, loc), value(std::move(value))
  {
  }
};

struct VariableExpr : LValueExpr
{
  std::string name;

  VariableExpr(SourceLocation loc, std::string name)
      : LValueExpr(NodeKind::VARIABLE, loc), name(std::move(name))
  {
  }
};

// arr[index] or grid[row, column]. The indices are kept in source order.
struct ArrayAccessExpr : LValueExpr
{
  std::string name;
  std::vector<ExprPtr> indices;

  ArrayAccessExpr(SourceLocation loc, std::string name, std::vector<ExprPtr> indices)
      : LValueExpr(NodeKind::ARRAY_ACCESS, loc), name(std::move(name)), indices(std::move(indices))
  {
  }
};

struct UnaryExpr : Expr
{
  UnaryOp op;
  ExprPtr operand;

  UnaryExpr(SourceLocation loc, UnaryOp op, ExprPtr operand)
      : Expr(NodeKind::UNARY, loc), op(op), operand(std::move(operand))
  {
  }
};

struct BinaryExpr : Expr
{
  BinaryOp op;
  ExprPtr left;
  ExprPtr right;

  BinaryExpr(SourceLocation loc, BinaryOp op, ExprPtr left, ExprPtr right)
      : Expr(NodeKind::BINARY, loc), op(op), left(std::move(left)), right(std::move(right))
  {
  }
};

// A call used inside an expression. This covers both the library routines
// (LENGTH, LCASE, UCASE, SUBSTRING, ROUND, RANDOM) and user-defined functions.
// MOD and DIV are represented as BinaryExpr when they appear in either the
// MOD(a, b) or the infix a MOD b form.
struct FunctionCallExpr : Expr
{
  std::string name;
  std::vector<ExprPtr> arguments;

  FunctionCallExpr(SourceLocation loc, std::string name, std::vector<ExprPtr> arguments)
      : Expr(NodeKind::FUNCTION_CALL, loc), name(std::move(name)), arguments(std::move(arguments))
  {
  }
};

// ---------------------------------------------------------------------------
// Statements
// ---------------------------------------------------------------------------

struct VariableDeclarationStmt : Stmt
{
  std::string name;
  DataType type;

  VariableDeclarationStmt(SourceLocation loc, std::string name, DataType type)
      : Stmt(NodeKind::VARIABLE_DECLARATION, loc), name(std::move(name)), type(type)
  {
  }
};

// The value is kept as an expression; the converter does not validate it.
struct ConstantDeclarationStmt : Stmt
{
  std::string name;
  ExprPtr value;

  ConstantDeclarationStmt(SourceLocation loc, std::string name, ExprPtr value)
      : Stmt(NodeKind::CONSTANT_DECLARATION, loc), name(std::move(name)), value(std::move(value))
  {
  }
};

struct ArrayDeclarationStmt : Stmt
{
  std::string name;
  DataType elementType;
  std::vector<ArrayDimension> dimensions;

  ArrayDeclarationStmt(SourceLocation loc, std::string name, DataType elementType,
                       std::vector<ArrayDimension> dimensions)
      : Stmt(NodeKind::ARRAY_DECLARATION, loc), name(std::move(name)), elementType(elementType),
        dimensions(std::move(dimensions))
  {
  }
};

struct AssignmentStmt : Stmt
{
  LValuePtr target;
  ExprPtr value;

  AssignmentStmt(SourceLocation loc, LValuePtr target, ExprPtr value)
      : Stmt(NodeKind::ASSIGNMENT, loc), target(std::move(target)), value(std::move(value))
  {
  }
};

// INPUT officially takes a single variable, but a vector allows a comma-
// separated extension without changing the tree.
struct InputStmt : Stmt
{
  std::vector<LValuePtr> targets;

  InputStmt(SourceLocation loc, std::vector<LValuePtr> targets)
      : Stmt(NodeKind::INPUT, loc), targets(std::move(targets))
  {
  }
};

struct OutputStmt : Stmt
{
  std::vector<ExprPtr> values;

  OutputStmt(SourceLocation loc, std::vector<ExprPtr> values)
      : Stmt(NodeKind::OUTPUT, loc), values(std::move(values))
  {
  }
};

// An empty elseBody means that the IF statement has no ELSE clause.
struct IfStmt : Stmt
{
  ExprPtr condition;
  std::vector<StmtPtr> thenBody;
  std::vector<StmtPtr> elseBody;

  IfStmt(SourceLocation loc, ExprPtr condition, std::vector<StmtPtr> thenBody,
         std::vector<StmtPtr> elseBody = {})
      : Stmt(NodeKind::IF, loc), condition(std::move(condition)), thenBody(std::move(thenBody)),
        elseBody(std::move(elseBody))
  {
  }
};

// An empty otherwiseBody means that the CASE statement has no OTHERWISE
// clause. The clauses are kept in source order; they are tested in that order.
struct CaseStmt : Stmt
{
  ExprPtr selector;
  std::vector<CaseClause> clauses;
  std::vector<StmtPtr> otherwiseBody;

  CaseStmt(SourceLocation loc, ExprPtr selector, std::vector<CaseClause> clauses,
           std::vector<StmtPtr> otherwiseBody = {})
      : Stmt(NodeKind::CASE, loc), selector(std::move(selector)), clauses(std::move(clauses)),
        otherwiseBody(std::move(otherwiseBody))
  {
  }
};

// The identifier after NEXT is redundant with ForStmt::variable and is checked
// by the parser rather than stored separately.
struct ForStmt : Stmt
{
  std::string variable;
  ExprPtr start;
  ExprPtr end;
  ExprPtr step; // nullptr when STEP is omitted (implicit step of 1)
  std::vector<StmtPtr> body;

  ForStmt(SourceLocation loc, std::string variable, ExprPtr start, ExprPtr end, ExprPtr step,
          std::vector<StmtPtr> body)
      : Stmt(NodeKind::FOR, loc), variable(std::move(variable)), start(std::move(start)),
        end(std::move(end)), step(std::move(step)), body(std::move(body))
  {
  }
};

struct WhileStmt : Stmt
{
  ExprPtr condition;
  std::vector<StmtPtr> body;

  WhileStmt(SourceLocation loc, ExprPtr condition, std::vector<StmtPtr> body)
      : Stmt(NodeKind::WHILE, loc), condition(std::move(condition)), body(std::move(body))
  {
  }
};

struct RepeatStmt : Stmt
{
  std::vector<StmtPtr> body;
  ExprPtr condition;

  RepeatStmt(SourceLocation loc, std::vector<StmtPtr> body, ExprPtr condition)
      : Stmt(NodeKind::REPEAT, loc), body(std::move(body)), condition(std::move(condition))
  {
  }
};

struct ProcedureDeclarationStmt : Stmt
{
  std::string name;
  std::vector<Parameter> parameters;
  std::vector<StmtPtr> body;

  ProcedureDeclarationStmt(SourceLocation loc, std::string name, std::vector<Parameter> parameters,
                           std::vector<StmtPtr> body)
      : Stmt(NodeKind::PROCEDURE_DECLARATION, loc), name(std::move(name)),
        parameters(std::move(parameters)), body(std::move(body))
  {
  }
};

struct FunctionDeclarationStmt : Stmt
{
  std::string name;
  std::vector<Parameter> parameters;
  DataType returnType;
  std::vector<StmtPtr> body;

  FunctionDeclarationStmt(SourceLocation loc, std::string name, std::vector<Parameter> parameters,
                          DataType returnType, std::vector<StmtPtr> body)
      : Stmt(NodeKind::FUNCTION_DECLARATION, loc), name(std::move(name)),
        parameters(std::move(parameters)), returnType(returnType), body(std::move(body))
  {
  }
};

struct CallStmt : Stmt
{
  std::string name;
  std::vector<ExprPtr> arguments;

  CallStmt(SourceLocation loc, std::string name, std::vector<ExprPtr> arguments)
      : Stmt(NodeKind::CALL, loc), name(std::move(name)), arguments(std::move(arguments))
  {
  }
};

struct ReturnStmt : Stmt
{
  ExprPtr value;

  ReturnStmt(SourceLocation loc, ExprPtr value)
      : Stmt(NodeKind::RETURN, loc), value(std::move(value))
  {
  }
};

struct OpenFileStmt : Stmt
{
  ExprPtr fileName;
  FileMode mode;

  OpenFileStmt(SourceLocation loc, ExprPtr fileName, FileMode mode)
      : Stmt(NodeKind::OPEN_FILE, loc), fileName(std::move(fileName)), mode(mode)
  {
  }
};

struct ReadFileStmt : Stmt
{
  ExprPtr fileName;
  LValuePtr target;

  ReadFileStmt(SourceLocation loc, ExprPtr fileName, LValuePtr target)
      : Stmt(NodeKind::READ_FILE, loc), fileName(std::move(fileName)), target(std::move(target))
  {
  }
};

struct WriteFileStmt : Stmt
{
  ExprPtr fileName;
  ExprPtr value;

  WriteFileStmt(SourceLocation loc, ExprPtr fileName, ExprPtr value)
      : Stmt(NodeKind::WRITE_FILE, loc), fileName(std::move(fileName)), value(std::move(value))
  {
  }
};

struct CloseFileStmt : Stmt
{
  ExprPtr fileName;

  CloseFileStmt(SourceLocation loc, ExprPtr fileName)
      : Stmt(NodeKind::CLOSE_FILE, loc), fileName(std::move(fileName))
  {
  }
};

// ---------------------------------------------------------------------------
// Root
// ---------------------------------------------------------------------------

// Procedure and function declarations are ordinary statements at the top of
// the program body.
struct Program : Node
{
  std::vector<StmtPtr> statements;

  explicit Program(SourceLocation loc = SourceLocation{}) : Node(NodeKind::PROGRAM, loc) {}
};

#endif
