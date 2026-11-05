#include "Parser.h"

#include <exception>
#include <utility>

namespace
{
class ParseFailure
{
public:
  explicit ParseFailure(ParseError error) : error_(std::move(error)) {}

  const ParseError& error() const
  {
    return error_;
  }

private:
  ParseError error_;
};

SourceLocation locationOf(const Token& token)
{
  return SourceLocation{token.line, token.column};
}

bool equalsIgnoreCase(const std::string& left, const std::string& right)
{
  if (left.size() != right.size())
    return false;

  for (std::size_t i = 0; i < left.size(); ++i)
  {
    char leftChar = left[i];
    char rightChar = right[i];
    if (leftChar >= 'A' && leftChar <= 'Z')
      leftChar = static_cast<char>(leftChar - 'A' + 'a');
    if (rightChar >= 'A' && rightChar <= 'Z')
      rightChar = static_cast<char>(rightChar - 'A' + 'a');
    if (leftChar != rightChar)
      return false;
  }

  return true;
}

std::string tokenDescription(const Token& token)
{
  switch (token.type)
  {
  case TokenType::END_OF_FILE:
    return "end of input";
  case TokenType::NEWLINE:
    return "newline";
  default:
    return "'" + token.value + "'";
  }
}
} // namespace

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens))
{
  if (tokens_.empty() || tokens_.back().type != TokenType::END_OF_FILE)
  {
    const int line = tokens_.empty() ? 1 : tokens_.back().line;
    const int column = tokens_.empty() ? 1 : tokens_.back().column;
    tokens_.emplace_back(TokenType::END_OF_FILE, "", line, column);
  }
}

std::unique_ptr<Program> Parser::parse()
{
  current_ = 0;
  errors_.clear();

  try
  {
    return parseProgram();
  }
  catch (const ParseFailure& failure)
  {
    errors_.push_back(failure.error());
    return nullptr;
  }
}

const std::vector<ParseError>& Parser::errors() const
{
  return errors_;
}

// ---------------------------------------------------------------------------
// Token helpers
// ---------------------------------------------------------------------------

const Token& Parser::peek() const
{
  return peekAt(0);
}

const Token& Parser::peekAt(std::size_t offset) const
{
  const std::size_t index = current_ + offset;
  if (index >= tokens_.size())
    return tokens_.back();
  return tokens_[index];
}

const Token& Parser::previous() const
{
  return tokens_[current_ - 1];
}

const Token& Parser::advance()
{
  const std::size_t index = current_;
  if (!isAtEnd())
    ++current_;
  return tokens_[index];
}

bool Parser::check(TokenType type) const
{
  return peek().type == type;
}

bool Parser::checkAny(std::initializer_list<TokenType> types) const
{
  for (TokenType type : types)
  {
    if (check(type))
      return true;
  }
  return false;
}

bool Parser::match(TokenType type)
{
  if (!check(type))
    return false;
  advance();
  return true;
}

bool Parser::isAtEnd() const
{
  return check(TokenType::END_OF_FILE);
}

void Parser::skipNewlines()
{
  while (match(TokenType::NEWLINE))
  {
  }
}

const Token& Parser::expect(TokenType type, const std::string& message)
{
  if (!check(type))
    error(peek(), message);
  return advance();
}

[[noreturn]] void Parser::error(const Token& token, const std::string& message)
{
  ParseError parseError;
  parseError.message = message + " (found " + tokenDescription(token) + ")";
  parseError.loc = locationOf(token);
  throw ParseFailure(std::move(parseError));
}

// ---------------------------------------------------------------------------
// Statements
// ---------------------------------------------------------------------------

std::unique_ptr<Program> Parser::parseProgram()
{
  skipNewlines();

  auto program = std::make_unique<Program>(locationOf(peek()));

  while (!isAtEnd())
  {
    program->statements.push_back(parseStatement());
    skipNewlines();
  }

  return program;
}

StmtPtr Parser::parseStatement()
{
  if (match(TokenType::KEYWORD_DECLARE))
    return parseDeclaration();
  if (match(TokenType::KEYWORD_CONSTANT))
    return parseConstantDeclaration();
  if (match(TokenType::KEYWORD_INPUT))
    return parseInput();
  if (match(TokenType::KEYWORD_OUTPUT))
    return parseOutput();
  if (match(TokenType::KEYWORD_IF))
    return parseIf();
  if (match(TokenType::KEYWORD_CASE))
    return parseCase();
  if (match(TokenType::KEYWORD_FOR))
    return parseFor();
  if (match(TokenType::KEYWORD_WHILE))
    return parseWhile();
  if (match(TokenType::KEYWORD_REPEAT))
    return parseRepeat();
  if (match(TokenType::KEYWORD_PROCEDURE))
    return parseProcedure();
  if (match(TokenType::KEYWORD_FUNCTION))
    return parseFunction();
  if (match(TokenType::KEYWORD_CALL))
    return parseCall();
  if (match(TokenType::KEYWORD_RETURN))
    return parseReturn();
  if (match(TokenType::KEYWORD_OPENFILE))
    return parseOpenFile();
  if (match(TokenType::KEYWORD_READFILE))
    return parseReadFile();
  if (match(TokenType::KEYWORD_WRITEFILE))
    return parseWriteFile();
  if (match(TokenType::KEYWORD_CLOSEFILE))
    return parseCloseFile();
  if (check(TokenType::IDENTIFIER))
    return parseAssignment();

  error(peek(), "expected statement");
}

void Parser::parseBlock(std::vector<StmtPtr>& body, std::initializer_list<TokenType> terminators)
{
  skipNewlines();

  while (!isAtEnd() && !checkAny(terminators))
  {
    body.push_back(parseStatement());
    skipNewlines();
  }
}

StmtPtr Parser::parseDeclaration()
{
  const SourceLocation loc = locationOf(previous());
  const Token& nameToken = expect(TokenType::IDENTIFIER, "expected variable name after DECLARE");
  expect(TokenType::COLON, "expected ':' after variable name");

  if (match(TokenType::KEYWORD_ARRAY))
  {
    expect(TokenType::LBRACKET, "expected '[' after ARRAY");

    std::vector<ArrayDimension> dimensions;
    do
    {
      skipNewlines();
      ExprPtr lower = parseExpression();
      const SourceLocation dimensionLoc = lower->loc;
      skipNewlines();
      expect(TokenType::COLON, "expected ':' between array bounds");
      skipNewlines();
      ExprPtr upper = parseExpression();
      dimensions.emplace_back(dimensionLoc, std::move(lower), std::move(upper));
      skipNewlines();
    } while (match(TokenType::COMMA));

    expect(TokenType::RBRACKET, "expected ']' after array bounds");
    expect(TokenType::KEYWORD_OF, "expected 'OF' after array declaration");

    DataType elementType = parseDataType();
    return std::make_unique<ArrayDeclarationStmt>(loc, nameToken.value, elementType,
                                                  std::move(dimensions));
  }

  DataType type = parseDataType();
  return std::make_unique<VariableDeclarationStmt>(loc, nameToken.value, type);
}

StmtPtr Parser::parseConstantDeclaration()
{
  const SourceLocation loc = locationOf(previous());
  const Token& nameToken = expect(TokenType::IDENTIFIER, "expected constant name after CONSTANT");
  expect(TokenType::ASSIGN, "expected '<-' after constant name");
  skipNewlines();

  ExprPtr value = parseExpression();
  return std::make_unique<ConstantDeclarationStmt>(loc, nameToken.value, std::move(value));
}

StmtPtr Parser::parseInput()
{
  const SourceLocation loc = locationOf(previous());
  std::vector<LValuePtr> targets;

  do
  {
    skipNewlines();
    targets.push_back(parseLValue());
    skipNewlines();
  } while (match(TokenType::COMMA));

  return std::make_unique<InputStmt>(loc, std::move(targets));
}

StmtPtr Parser::parseOutput()
{
  const SourceLocation loc = locationOf(previous());
  std::vector<ExprPtr> values;

  do
  {
    skipNewlines();
    values.push_back(parseExpression());
    skipNewlines();
  } while (match(TokenType::COMMA));

  return std::make_unique<OutputStmt>(loc, std::move(values));
}

StmtPtr Parser::parseAssignment()
{
  const SourceLocation loc = locationOf(peek());
  LValuePtr target = parseLValue();
  expect(TokenType::ASSIGN, "expected '<-' in assignment");
  skipNewlines();

  ExprPtr value = parseExpression();
  return std::make_unique<AssignmentStmt>(loc, std::move(target), std::move(value));
}

StmtPtr Parser::parseIf()
{
  const SourceLocation loc = locationOf(previous());
  skipNewlines();

  ExprPtr condition = parseExpression();
  skipNewlines();
  expect(TokenType::KEYWORD_THEN, "expected 'THEN' after IF condition");
  skipNewlines();

  std::vector<StmtPtr> thenBody;
  parseBlock(thenBody, {TokenType::KEYWORD_ELSE, TokenType::KEYWORD_ENDIF});

  std::vector<StmtPtr> elseBody;
  if (match(TokenType::KEYWORD_ELSE))
  {
    skipNewlines();
    parseBlock(elseBody, {TokenType::KEYWORD_ENDIF});
  }

  expect(TokenType::KEYWORD_ENDIF, "expected 'ENDIF' to close IF");
  return std::make_unique<IfStmt>(loc, std::move(condition), std::move(thenBody),
                                  std::move(elseBody));
}

StmtPtr Parser::parseCase()
{
  const SourceLocation loc = locationOf(previous());
  expect(TokenType::KEYWORD_OF, "expected 'OF' after CASE");
  skipNewlines();

  ExprPtr selector = parseExpression();
  skipNewlines();

  std::vector<CaseClause> clauses;
  std::vector<StmtPtr> otherwiseBody;

  while (!isAtEnd() && !check(TokenType::KEYWORD_ENDCASE) && !check(TokenType::KEYWORD_OTHERWISE))
  {
    if (!isCaseLabelStart())
      error(peek(), "expected case value");

    const SourceLocation clauseLoc = locationOf(peek());
    ExprPtr value = parseExpression();
    expect(TokenType::COLON, "expected ':' after case value");
    skipNewlines();

    std::vector<StmtPtr> body;
    while (!isAtEnd() && !check(TokenType::KEYWORD_ENDCASE) &&
           !check(TokenType::KEYWORD_OTHERWISE) && !isCaseLabelStart())
    {
      body.push_back(parseStatement());
      skipNewlines();
    }

    clauses.emplace_back(clauseLoc, std::move(value), std::move(body));
  }

  if (match(TokenType::KEYWORD_OTHERWISE))
  {
    match(TokenType::COLON); // tolerate an optional colon
    skipNewlines();

    while (!isAtEnd() && !check(TokenType::KEYWORD_ENDCASE))
    {
      otherwiseBody.push_back(parseStatement());
      skipNewlines();
    }
  }

  expect(TokenType::KEYWORD_ENDCASE, "expected 'ENDCASE' to close CASE");
  return std::make_unique<CaseStmt>(loc, std::move(selector), std::move(clauses),
                                    std::move(otherwiseBody));
}

StmtPtr Parser::parseFor()
{
  const SourceLocation loc = locationOf(previous());
  skipNewlines();

  const Token& variableToken = expect(TokenType::IDENTIFIER, "expected loop variable after FOR");
  expect(TokenType::ASSIGN, "expected '<-' after loop variable");
  skipNewlines();

  ExprPtr start = parseExpression();
  skipNewlines();
  expect(TokenType::KEYWORD_TO, "expected 'TO' in FOR loop");
  skipNewlines();
  ExprPtr end = parseExpression();
  skipNewlines();

  ExprPtr step;
  if (match(TokenType::KEYWORD_STEP))
  {
    skipNewlines();
    step = parseExpression();
    skipNewlines();
  }

  std::vector<StmtPtr> body;
  parseBlock(body, {TokenType::KEYWORD_NEXT});
  expect(TokenType::KEYWORD_NEXT, "expected 'NEXT' to close FOR loop");

  if (check(TokenType::IDENTIFIER))
  {
    const Token& nextVariable = advance();
    if (!equalsIgnoreCase(nextVariable.value, variableToken.value))
      error(nextVariable, "NEXT variable must match the FOR loop variable");
  }

  return std::make_unique<ForStmt>(loc, variableToken.value, std::move(start), std::move(end),
                                   std::move(step), std::move(body));
}

StmtPtr Parser::parseWhile()
{
  const SourceLocation loc = locationOf(previous());
  skipNewlines();

  ExprPtr condition = parseExpression();
  skipNewlines();
  expect(TokenType::KEYWORD_DO, "expected 'DO' after WHILE condition");
  skipNewlines();

  std::vector<StmtPtr> body;
  parseBlock(body, {TokenType::KEYWORD_ENDWHILE});
  expect(TokenType::KEYWORD_ENDWHILE, "expected 'ENDWHILE' to close WHILE loop");

  return std::make_unique<WhileStmt>(loc, std::move(condition), std::move(body));
}

StmtPtr Parser::parseRepeat()
{
  const SourceLocation loc = locationOf(previous());

  std::vector<StmtPtr> body;
  parseBlock(body, {TokenType::KEYWORD_UNTIL});
  expect(TokenType::KEYWORD_UNTIL, "expected 'UNTIL' to close REPEAT loop");
  skipNewlines();

  ExprPtr condition = parseExpression();
  return std::make_unique<RepeatStmt>(loc, std::move(body), std::move(condition));
}

StmtPtr Parser::parseProcedure()
{
  const SourceLocation loc = locationOf(previous());
  const Token& nameToken = expect(TokenType::IDENTIFIER, "expected procedure name");

  std::vector<Parameter> parameters = parseParameters();
  skipNewlines();

  std::vector<StmtPtr> body;
  parseBlock(body, {TokenType::KEYWORD_ENDPROCEDURE});
  expect(TokenType::KEYWORD_ENDPROCEDURE, "expected 'ENDPROCEDURE' to close procedure");

  return std::make_unique<ProcedureDeclarationStmt>(loc, nameToken.value, std::move(parameters),
                                                    std::move(body));
}

StmtPtr Parser::parseFunction()
{
  const SourceLocation loc = locationOf(previous());
  const Token& nameToken = expect(TokenType::IDENTIFIER, "expected function name");

  std::vector<Parameter> parameters = parseParameters();
  skipNewlines();
  expect(TokenType::KEYWORD_RETURNS, "expected 'RETURNS' in function declaration");
  skipNewlines();

  DataType returnType = parseDataType();
  skipNewlines();

  std::vector<StmtPtr> body;
  parseBlock(body, {TokenType::KEYWORD_ENDFUNCTION});
  expect(TokenType::KEYWORD_ENDFUNCTION, "expected 'ENDFUNCTION' to close function");

  return std::make_unique<FunctionDeclarationStmt>(loc, nameToken.value, std::move(parameters),
                                                   returnType, std::move(body));
}

StmtPtr Parser::parseCall()
{
  const SourceLocation loc = locationOf(previous());
  const Token& nameToken = expect(TokenType::IDENTIFIER, "expected procedure name after CALL");

  std::vector<ExprPtr> arguments;
  if (check(TokenType::LPAREN))
    arguments = parseArguments();

  return std::make_unique<CallStmt>(loc, nameToken.value, std::move(arguments));
}

StmtPtr Parser::parseReturn()
{
  const SourceLocation loc = locationOf(previous());
  skipNewlines();

  ExprPtr value = parseExpression();
  return std::make_unique<ReturnStmt>(loc, std::move(value));
}

StmtPtr Parser::parseOpenFile()
{
  const SourceLocation loc = locationOf(previous());
  ExprPtr fileName = parseExpression();
  expect(TokenType::KEYWORD_FOR, "expected 'FOR' in OPENFILE");

  FileMode mode = FileMode::READ;
  if (match(TokenType::KEYWORD_READ))
    mode = FileMode::READ;
  else if (match(TokenType::KEYWORD_WRITE))
    mode = FileMode::WRITE;
  else
    error(peek(), "expected READ or WRITE in OPENFILE");

  return std::make_unique<OpenFileStmt>(loc, std::move(fileName), mode);
}

StmtPtr Parser::parseReadFile()
{
  const SourceLocation loc = locationOf(previous());
  ExprPtr fileName = parseExpression();
  expect(TokenType::COMMA, "expected ',' between file name and variable");
  skipNewlines();

  LValuePtr target = parseLValue();
  return std::make_unique<ReadFileStmt>(loc, std::move(fileName), std::move(target));
}

StmtPtr Parser::parseWriteFile()
{
  const SourceLocation loc = locationOf(previous());
  ExprPtr fileName = parseExpression();
  expect(TokenType::COMMA, "expected ',' between file name and value");
  skipNewlines();

  ExprPtr value = parseExpression();
  return std::make_unique<WriteFileStmt>(loc, std::move(fileName), std::move(value));
}

StmtPtr Parser::parseCloseFile()
{
  const SourceLocation loc = locationOf(previous());
  ExprPtr fileName = parseExpression();
  return std::make_unique<CloseFileStmt>(loc, std::move(fileName));
}

// ---------------------------------------------------------------------------
// Expressions
// ---------------------------------------------------------------------------

// Precedence, from lowest to highest:
//   OR, AND, NOT, comparisons, + -, * / MOD DIV, unary + -, ^, primary.
// Parentheses only affect the shape of the tree and do not need a node of
// their own.
ExprPtr Parser::parseExpression()
{
  return parseOr();
}

ExprPtr Parser::parseOr()
{
  ExprPtr left = parseAnd();

  while (match(TokenType::KEYWORD_OR))
  {
    const SourceLocation opLoc = locationOf(previous());
    skipNewlines();
    ExprPtr right = parseAnd();
    left = std::make_unique<BinaryExpr>(opLoc, BinaryOp::OR, std::move(left), std::move(right));
  }

  return left;
}

ExprPtr Parser::parseAnd()
{
  ExprPtr left = parseNot();

  while (match(TokenType::KEYWORD_AND))
  {
    const SourceLocation opLoc = locationOf(previous());
    skipNewlines();
    ExprPtr right = parseNot();
    left = std::make_unique<BinaryExpr>(opLoc, BinaryOp::AND, std::move(left), std::move(right));
  }

  return left;
}

ExprPtr Parser::parseNot()
{
  if (match(TokenType::KEYWORD_NOT))
  {
    const SourceLocation opLoc = locationOf(previous());
    skipNewlines();
    return std::make_unique<UnaryExpr>(opLoc, UnaryOp::NOT, parseNot());
  }

  return parseComparison();
}

ExprPtr Parser::parseComparison()
{
  ExprPtr left = parseAdditive();

  BinaryOp op;
  SourceLocation opLoc;

  if (match(TokenType::EQUAL))
  {
    op = BinaryOp::EQUAL;
    opLoc = locationOf(previous());
  }
  else if (match(TokenType::NOT_EQUAL))
  {
    op = BinaryOp::NOT_EQUAL;
    opLoc = locationOf(previous());
  }
  else if (match(TokenType::LESS_THAN))
  {
    op = BinaryOp::LESS;
    opLoc = locationOf(previous());
  }
  else if (match(TokenType::LESS_EQUAL))
  {
    op = BinaryOp::LESS_EQUAL;
    opLoc = locationOf(previous());
  }
  else if (match(TokenType::GREATER_THAN))
  {
    op = BinaryOp::GREATER;
    opLoc = locationOf(previous());
  }
  else if (match(TokenType::GREATER_EQUAL))
  {
    op = BinaryOp::GREATER_EQUAL;
    opLoc = locationOf(previous());
  }
  else
  {
    return left;
  }

  skipNewlines();
  ExprPtr right = parseAdditive();
  return std::make_unique<BinaryExpr>(opLoc, op, std::move(left), std::move(right));
}

ExprPtr Parser::parseAdditive()
{
  ExprPtr left = parseMultiplicative();

  while (true)
  {
    BinaryOp op;

    if (match(TokenType::PLUS))
      op = BinaryOp::ADD;
    else if (match(TokenType::MINUS))
      op = BinaryOp::SUBTRACT;
    else
      break;

    const SourceLocation opLoc = locationOf(previous());
    skipNewlines();
    ExprPtr right = parseMultiplicative();
    left = std::make_unique<BinaryExpr>(opLoc, op, std::move(left), std::move(right));
  }

  return left;
}

ExprPtr Parser::parseMultiplicative()
{
  ExprPtr left = parseUnary();

  while (true)
  {
    BinaryOp op;

    if (match(TokenType::MULTIPLY))
      op = BinaryOp::MULTIPLY;
    else if (match(TokenType::DIVIDE))
      op = BinaryOp::DIVIDE;
    else if (match(TokenType::KEYWORD_MOD))
      op = BinaryOp::MOD;
    else if (match(TokenType::KEYWORD_DIV))
      op = BinaryOp::DIV;
    else
      break;

    const SourceLocation opLoc = locationOf(previous());
    skipNewlines();
    ExprPtr right = parseUnary();
    left = std::make_unique<BinaryExpr>(opLoc, op, std::move(left), std::move(right));
  }

  return left;
}

ExprPtr Parser::parseUnary()
{
  if (match(TokenType::MINUS))
  {
    const SourceLocation opLoc = locationOf(previous());
    skipNewlines();
    return std::make_unique<UnaryExpr>(opLoc, UnaryOp::NEGATE, parseUnary());
  }

  if (match(TokenType::PLUS))
  {
    skipNewlines();
    return parseUnary();
  }

  return parsePower();
}

ExprPtr Parser::parsePower()
{
  ExprPtr left = parsePrimary();

  if (match(TokenType::POWER))
  {
    const SourceLocation opLoc = locationOf(previous());
    skipNewlines();
    ExprPtr right = parseUnary();
    return std::make_unique<BinaryExpr>(opLoc, BinaryOp::POWER, std::move(left), std::move(right));
  }

  return left;
}

ExprPtr Parser::parsePrimary()
{
  if (match(TokenType::INTEGER))
  {
    const Token& token = previous();
    return std::make_unique<IntegerLiteralExpr>(locationOf(token), parseIntegerLiteral(token));
  }

  if (match(TokenType::REAL))
  {
    const Token& token = previous();
    return std::make_unique<RealLiteralExpr>(locationOf(token), parseRealLiteral(token));
  }

  if (match(TokenType::STRING))
  {
    const Token& token = previous();
    return std::make_unique<StringLiteralExpr>(locationOf(token), parseStringLiteral(token));
  }

  if (match(TokenType::CHAR))
  {
    const Token& token = previous();
    return std::make_unique<CharLiteralExpr>(locationOf(token), parseCharLiteral(token));
  }

  if (match(TokenType::BOOLEAN))
  {
    const Token& token = previous();
    return std::make_unique<BooleanLiteralExpr>(locationOf(token), parseBooleanLiteral(token));
  }

  if (match(TokenType::IDENTIFIER))
  {
    const Token& token = previous();
    return parseIdentifierExpression(token);
  }

  if (match(TokenType::LPAREN))
  {
    skipNewlines();
    ExprPtr expression = parseExpression();
    skipNewlines();
    expect(TokenType::RPAREN, "expected ')' after expression");
    return expression;
  }

  if (checkAny({TokenType::KEYWORD_LENGTH, TokenType::KEYWORD_SUBSTRING, TokenType::KEYWORD_LCASE,
                TokenType::KEYWORD_UCASE, TokenType::KEYWORD_ROUND, TokenType::KEYWORD_RANDOM}))
  {
    return parseBuiltinCall();
  }

  if (check(TokenType::KEYWORD_MOD) || check(TokenType::KEYWORD_DIV))
    return parseIntegerFunctionCall();

  error(peek(), "expected expression");
}

ExprPtr Parser::parseIdentifierExpression(const Token& name)
{
  if (check(TokenType::LBRACKET))
  {
    std::vector<ExprPtr> indices = parseArrayIndices();
    return std::make_unique<ArrayAccessExpr>(locationOf(name), name.value, std::move(indices));
  }

  if (check(TokenType::LPAREN))
  {
    std::vector<ExprPtr> arguments = parseArguments();
    return std::make_unique<FunctionCallExpr>(locationOf(name), name.value, std::move(arguments));
  }

  return std::make_unique<VariableExpr>(locationOf(name), name.value);
}

ExprPtr Parser::parseBuiltinCall()
{
  const Token& nameToken = advance();
  std::vector<ExprPtr> arguments = parseArguments();
  return std::make_unique<FunctionCallExpr>(locationOf(nameToken), nameToken.value,
                                            std::move(arguments));
}

ExprPtr Parser::parseIntegerFunctionCall()
{
  const Token& opToken = advance();
  expect(TokenType::LPAREN, "expected '(' after " + opToken.value);
  skipNewlines();

  ExprPtr left = parseExpression();
  skipNewlines();
  expect(TokenType::COMMA, "expected ',' in " + opToken.value + " call");
  skipNewlines();
  ExprPtr right = parseExpression();
  skipNewlines();
  expect(TokenType::RPAREN, "expected ')' after " + opToken.value + " arguments");

  const BinaryOp op = opToken.type == TokenType::KEYWORD_MOD ? BinaryOp::MOD : BinaryOp::DIV;
  return std::make_unique<BinaryExpr>(locationOf(opToken), op, std::move(left), std::move(right));
}

std::vector<ExprPtr> Parser::parseArguments()
{
  expect(TokenType::LPAREN, "expected '('");
  skipNewlines();

  std::vector<ExprPtr> arguments;
  if (!check(TokenType::RPAREN))
  {
    do
    {
      skipNewlines();
      arguments.push_back(parseExpression());
      skipNewlines();
    } while (match(TokenType::COMMA));
  }

  expect(TokenType::RPAREN, "expected ')' after arguments");
  return arguments;
}

std::vector<ExprPtr> Parser::parseArrayIndices()
{
  expect(TokenType::LBRACKET, "expected '['");
  skipNewlines();

  if (check(TokenType::RBRACKET))
    error(peek(), "expected array index");

  std::vector<ExprPtr> indices;
  do
  {
    skipNewlines();
    indices.push_back(parseExpression());
    skipNewlines();
  } while (match(TokenType::COMMA));

  expect(TokenType::RBRACKET, "expected ']' after array index");
  return indices;
}

LValuePtr Parser::parseLValue()
{
  const Token& nameToken = expect(TokenType::IDENTIFIER, "expected variable name");

  if (check(TokenType::LBRACKET))
  {
    std::vector<ExprPtr> indices = parseArrayIndices();
    return std::make_unique<ArrayAccessExpr>(locationOf(nameToken), nameToken.value,
                                             std::move(indices));
  }

  return std::make_unique<VariableExpr>(locationOf(nameToken), nameToken.value);
}

// ---------------------------------------------------------------------------
// Shared helpers
// ---------------------------------------------------------------------------

DataType Parser::parseDataType()
{
  if (match(TokenType::KEYWORD_INTEGER))
    return DataType::INTEGER;
  if (match(TokenType::KEYWORD_REAL))
    return DataType::REAL;
  if (match(TokenType::KEYWORD_CHAR))
    return DataType::CHAR;
  if (match(TokenType::KEYWORD_STRING))
    return DataType::STRING;
  if (match(TokenType::KEYWORD_BOOLEAN))
    return DataType::BOOLEAN;

  error(peek(), "expected data type");
}

std::vector<Parameter> Parser::parseParameters()
{
  std::vector<Parameter> parameters;

  if (!check(TokenType::LPAREN))
    return parameters;

  advance(); // consume '('
  skipNewlines();

  if (!check(TokenType::RPAREN))
  {
    do
    {
      skipNewlines();
      const Token& nameToken = expect(TokenType::IDENTIFIER, "expected parameter name");
      expect(TokenType::COLON, "expected ':' after parameter name");
      skipNewlines();

      DataType type = parseDataType();
      parameters.emplace_back(locationOf(nameToken), nameToken.value, type);
      skipNewlines();
    } while (match(TokenType::COMMA));
  }

  expect(TokenType::RPAREN, "expected ')' after parameters");
  return parameters;
}

bool Parser::isCaseLabelStart() const
{
  std::size_t offset = 0;
  if (peek().type == TokenType::MINUS || peek().type == TokenType::PLUS)
    offset = 1;

  const TokenType valueType = peekAt(offset).type;
  if (!isLiteralToken(valueType) && valueType != TokenType::IDENTIFIER)
    return false;

  return peekAt(offset + 1).type == TokenType::COLON;
}

bool Parser::isLiteralToken(TokenType type) const
{
  switch (type)
  {
  case TokenType::INTEGER:
  case TokenType::REAL:
  case TokenType::STRING:
  case TokenType::CHAR:
  case TokenType::BOOLEAN:
    return true;
  default:
    return false;
  }
}

std::int64_t Parser::parseIntegerLiteral(const Token& token)
{
  try
  {
    return std::stoll(token.value);
  }
  catch (const std::exception&)
  {
    error(token, "integer literal out of range");
  }
}

double Parser::parseRealLiteral(const Token& token)
{
  try
  {
    return std::stod(token.value);
  }
  catch (const std::exception&)
  {
    error(token, "real literal out of range");
  }
}

std::string Parser::parseStringLiteral(const Token& token)
{
  if (token.value.size() < 2 || token.value.front() != '"' || token.value.back() != '"')
    error(token, "invalid string literal");

  return token.value.substr(1, token.value.size() - 2);
}

char Parser::parseCharLiteral(const Token& token)
{
  if (token.value.size() != 3 || token.value.front() != '\'' || token.value.back() != '\'')
    error(token, "invalid character literal");

  return token.value[1];
}

bool Parser::parseBooleanLiteral(const Token& token)
{
  return token.value == "TRUE";
}
