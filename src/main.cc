#include "lexer/Lexer.h"

#include <iostream>
#include <string>

const char* tokenTypeToString(TokenType type)
{
  switch (type)
  {
  // conditional
  case TokenType::KEYWORD_IF:
    return "KEYWORD_IF";
  case TokenType::KEYWORD_THEN:
    return "KEYWORD_THEN";
  case TokenType::KEYWORD_ELSE:
    return "KEYWORD_ELSE";
  case TokenType::KEYWORD_ENDIF:
    return "KEYWORD_ENDIF";

  // loops
  case TokenType::KEYWORD_WHILE:
    return "KEYWORD_WHILE";
  case TokenType::KEYWORD_DO:
    return "KEYWORD_DO";
  case TokenType::KEYWORD_ENDWHILE:
    return "KEYWORD_ENDWHILE";
  case TokenType::KEYWORD_FOR:
    return "KEYWORD_FOR";
  case TokenType::KEYWORD_TO:
    return "KEYWORD_TO";
  case TokenType::KEYWORD_STEP:
    return "KEYWORD_STEP";
  case TokenType::KEYWORD_NEXT:
    return "KEYWORD_NEXT";
  case TokenType::KEYWORD_REPEAT:
    return "KEYWORD_REPEAT";
  case TokenType::KEYWORD_UNTIL:
    return "KEYWORD_UNTIL";

  // IO
  case TokenType::KEYWORD_INPUT:
    return "KEYWORD_INPUT";
  case TokenType::KEYWORD_OUTPUT:
    return "KEYWORD_OUTPUT";

  // identifiers and literals
  case TokenType::IDENTIFIER:
    return "IDENTIFIER";
  case TokenType::INTEGER:
    return "INTEGER";
  case TokenType::REAL:
    return "REAL";
  case TokenType::STRING:
    return "STRING";

  // operators
  case TokenType::ASSIGN:
    return "ASSIGN";
  case TokenType::PLUS:
    return "PLUS";
  case TokenType::MINUS:
    return "MINUS";
  case TokenType::MULTIPLY:
    return "MULTIPLY";
  case TokenType::DIVIDE:
    return "DIVIDE";
  case TokenType::EQUAL:
    return "EQUAL";
  case TokenType::NOT_EQUAL:
    return "NOT_EQUAL";
  case TokenType::LESS_THAN:
    return "LESS_THAN";
  case TokenType::LESS_EQUAL:
    return "LESS_EQUAL";
  case TokenType::GREATER_THAN:
    return "GREATER_THAN";
  case TokenType::GREATER_EQUAL:
    return "GREATER_EQUAL";

  // logical
  case TokenType::KEYWORD_AND:
    return "KEYWORD_AND";
  case TokenType::KEYWORD_OR:
    return "KEYWORD_OR";
  case TokenType::KEYWORD_NOT:
    return "KEYWORD_NOT";

  // structural
  case TokenType::NEWLINE:
    return "NEWLINE";
  case TokenType::END_OF_FILE:
    return "END_OF_FILE";

  case TokenType::UNKNOWN:
    return "UNKNOWN";
  }

  return "UNKNOWN";
}

int main()
{
  const std::string source = R"(IF score >= 90 THEN
  OUTPUT "Excellent"
ELSE
  OUTPUT "Try again"
ENDIF

FOR i ← 1 TO 10 STEP 2
  OUTPUT i
NEXT

x ← 123
y ← 3.14
z ← 123.45.67

IF x <> y AND x < 200 THEN
  OUTPUT "Test"
ENDIF
)";

  Lexer lexer(source);
  std::vector<Token> tokens = lexer.tokenize();

  for (const Token& token : tokens)
  {
    std::cout << token.line << ":" << token.column << "  " << tokenTypeToString(token.type)
              << "  \"" << token.value << "\"\n";
  }

  return 0;
}
