#ifndef PARSER_H
#define PARSER_H

#include "ast/AST.h"
#include "lexer/Lexer.h"

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

struct ParseError
{
  std::string message;
  SourceLocation loc;
};

// Recursive-descent parser for the IGCSE pseudocode accepted by the lexer:
// declarations, assignment, INPUT/OUTPUT, selection, iteration, arrays,
// procedures/functions and file handling.
//
// Newlines are treated as soft separators: they separate statements but are
// skipped wherever a construct may continue on the next line (between IF and
// THEN, after binary operators, inside argument lists, and so on).
//
// parse() returns a complete tree on success. On a syntax error it records the
// first error and returns nullptr; error recovery can be added later without
// changing the AST.
class Parser
{
public:
  explicit Parser(std::vector<Token> tokens);

  std::unique_ptr<Program> parse();

  const std::vector<ParseError>& errors() const;

private:
  std::vector<Token> tokens_;
  std::size_t current_ = 0;
  std::vector<ParseError> errors_;

  // token helpers
  const Token& peek() const;
  const Token& peekAt(std::size_t offset) const;
  const Token& previous() const;
  const Token& advance();
  bool check(TokenType type) const;
  bool checkAny(std::initializer_list<TokenType> types) const;
  bool match(TokenType type);
  bool isAtEnd() const;
  void skipNewlines();
  const Token& expect(TokenType type, const std::string& message);
  [[noreturn]] void error(const Token& token, const std::string& message);

  // statements
  std::unique_ptr<Program> parseProgram();
  StmtPtr parseStatement();
  void parseBlock(std::vector<StmtPtr>& body, std::initializer_list<TokenType> terminators);
  StmtPtr parseDeclaration();
  StmtPtr parseConstantDeclaration();
  StmtPtr parseInput();
  StmtPtr parseOutput();
  StmtPtr parseAssignment();
  StmtPtr parseIf();
  StmtPtr parseCase();
  StmtPtr parseFor();
  StmtPtr parseWhile();
  StmtPtr parseRepeat();
  StmtPtr parseProcedure();
  StmtPtr parseFunction();
  StmtPtr parseCall();
  StmtPtr parseReturn();
  StmtPtr parseOpenFile();
  StmtPtr parseReadFile();
  StmtPtr parseWriteFile();
  StmtPtr parseCloseFile();

  // expressions
  ExprPtr parseExpression();
  ExprPtr parseOr();
  ExprPtr parseAnd();
  ExprPtr parseNot();
  ExprPtr parseComparison();
  ExprPtr parseAdditive();
  ExprPtr parseMultiplicative();
  ExprPtr parseUnary();
  ExprPtr parsePower();
  ExprPtr parsePrimary();
  ExprPtr parseIdentifierExpression(const Token& name);
  ExprPtr parseBuiltinCall();
  ExprPtr parseIntegerFunctionCall();
  std::vector<ExprPtr> parseArguments();
  std::vector<ExprPtr> parseArrayIndices();
  LValuePtr parseLValue();

  // shared helpers
  DataType parseDataType();
  std::vector<Parameter> parseParameters();
  bool isCaseLabelStart() const;
  bool isLiteralToken(TokenType type) const;
  std::int64_t parseIntegerLiteral(const Token& token);
  double parseRealLiteral(const Token& token);
  std::string parseStringLiteral(const Token& token);
  char parseCharLiteral(const Token& token);
  bool parseBooleanLiteral(const Token& token);
};

#endif
