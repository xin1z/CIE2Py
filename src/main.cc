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

  case TokenType::KEYWORD_CASE:
    return "KEYWORD_CASE";
  case TokenType::KEYWORD_OTHERWISE:
    return "KEYWORD_OTHERWISE";
  case TokenType::KEYWORD_ENDCASE:
    return "KEYWORD_ENDCASE";

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
  case TokenType::CHAR:
    return "CHAR";
  case TokenType::BOOLEAN:
    return "BOOLEAN";

  // type keywords
  case TokenType::KEYWORD_INTEGER:
    return "KEYWORD_INTEGER";
  case TokenType::KEYWORD_REAL:
    return "KEYWORD_REAL";
  case TokenType::KEYWORD_STRING:
    return "KEYWORD_STRING";
  case TokenType::KEYWORD_CHAR:
    return "KEYWORD_CHAR";
  case TokenType::KEYWORD_BOOLEAN:
    return "KEYWORD_BOOLEAN";

  // declaration properties
  case TokenType::KEYWORD_DECLARE:
    return "KEYWORD_DECLARE";
  case TokenType::KEYWORD_CONSTANT:
    return "KEYWORD_CONSTANT";
  case TokenType::KEYWORD_ARRAY:
    return "KEYWORD_ARRAY";
  case TokenType::KEYWORD_OF:
    return "KEYWORD_OF";

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

  // Special operators & functions
  case TokenType::KEYWORD_MOD:
    return "KEYWORD_MOD";
  case TokenType::KEYWORD_DIV:
    return "KEYWORD_DIV";
  case TokenType::POWER:
    return "POWER";
  case TokenType::KEYWORD_ROUND:
    return "KEYWORD_ROUND";
  case TokenType::KEYWORD_RANDOM:
    return "KEYWORD_RANDOM";

  // string functions
  case TokenType::KEYWORD_LENGTH:
    return "KEYWORD_LENGTH";
  case TokenType::KEYWORD_SUBSTRING:
    return "KEYWORD_SUBSTRING";
  case TokenType::KEYWORD_LCASE:
    return "KEYWORD_LCASE";
  case TokenType::KEYWORD_UCASE:
    return "KEYWORD_UCASE";

  // logical
  case TokenType::KEYWORD_AND:
    return "KEYWORD_AND";
  case TokenType::KEYWORD_OR:
    return "KEYWORD_OR";
  case TokenType::KEYWORD_NOT:
    return "KEYWORD_NOT";

  // delimiters
  case TokenType::COLON:
    return "COLON";
  case TokenType::COMMA:
    return "COMMA";
  case TokenType::LPAREN:
    return "LPAREN";
  case TokenType::RPAREN:
    return "RPAREN";
  case TokenType::LBRACKET:
    return "LBRACKET";
  case TokenType::RBRACKET:
    return "RBRACKET";

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
  const std::string source = R"(// Declarations & Literals Test
DECLARE score : INTEGER
DECLARE name : STRING
DECLARE grade : CHAR
DECLARE isPassed : BOOLEAN
DECLARE scores : ARRAY[1:10] OF INTEGER

score <- 85
grade <- 'A'
isPassed <- TRUE

// Logic & Built-in functions Test
IF isPassed AND (score MOD 2 = 0) THEN
  OUTPUT UCASE(name), " score is ", score ^ 2
ELSE
  OUTPUT "Keep trying"
ENDIF

// Case Statement Test
CASE OF grade
  'A': OUTPUT "Excellent"
  'B': OUTPUT "Good"
  OTHERWISE OUTPUT "Other"
ENDCASE
)";

  Lexer lexer(source);
  std::vector<Token> tokens = lexer.tokenize();

  for (const Token& token : tokens)
  {
    if (token.type == TokenType::NEWLINE)
      std::cout << token.line << ":" << token.column << "\t" << tokenTypeToString(token.type)
                << "\t\"" << "\\n"
                << "\"\n";
    else
      std::cout << token.line << ":" << token.column << "\t" << tokenTypeToString(token.type)
                << "\t\"" << token.value << "\"\n";
  }

  return 0;
}
