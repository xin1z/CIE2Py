#include "lexer/Lexer.h"
#include "parser/Parser.h"

#include <iostream>
#include <string>
#include <utility>

const char *tokenTypeToString(TokenType type) {
  switch (type) {
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

  // subprograms
  case TokenType::KEYWORD_PROCEDURE:
    return "KEYWORD_PROCEDURE";
  case TokenType::KEYWORD_ENDPROCEDURE:
    return "KEYWORD_ENDPROCEDURE";
  case TokenType::KEYWORD_FUNCTION:
    return "KEYWORD_FUNCTION";
  case TokenType::KEYWORD_ENDFUNCTION:
    return "KEYWORD_ENDFUNCTION";
  case TokenType::KEYWORD_RETURNS:
    return "KEYWORD_RETURNS";
  case TokenType::KEYWORD_CALL:
    return "KEYWORD_CALL";
  case TokenType::KEYWORD_RETURN:
    return "KEYWORD_RETURN";

  // file handling
  case TokenType::KEYWORD_OPENFILE:
    return "KEYWORD_OPENFILE";
  case TokenType::KEYWORD_READFILE:
    return "KEYWORD_READFILE";
  case TokenType::KEYWORD_WRITEFILE:
    return "KEYWORD_WRITEFILE";
  case TokenType::KEYWORD_CLOSEFILE:
    return "KEYWORD_CLOSEFILE";
  case TokenType::KEYWORD_READ:
    return "KEYWORD_READ";
  case TokenType::KEYWORD_WRITE:
    return "KEYWORD_WRITE";

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

const char *nodeKindToString(NodeKind kind) {
  switch (kind) {
  case NodeKind::PROGRAM:
    return "PROGRAM";
  case NodeKind::VARIABLE_DECLARATION:
    return "VARIABLE_DECLARATION";
  case NodeKind::CONSTANT_DECLARATION:
    return "CONSTANT_DECLARATION";
  case NodeKind::ARRAY_DECLARATION:
    return "ARRAY_DECLARATION";
  case NodeKind::ASSIGNMENT:
    return "ASSIGNMENT";
  case NodeKind::INPUT:
    return "INPUT";
  case NodeKind::OUTPUT:
    return "OUTPUT";
  case NodeKind::IF:
    return "IF";
  case NodeKind::CASE:
    return "CASE";
  case NodeKind::FOR:
    return "FOR";
  case NodeKind::WHILE:
    return "WHILE";
  case NodeKind::REPEAT:
    return "REPEAT";
  case NodeKind::PROCEDURE_DECLARATION:
    return "PROCEDURE_DECLARATION";
  case NodeKind::FUNCTION_DECLARATION:
    return "FUNCTION_DECLARATION";
  case NodeKind::CALL:
    return "CALL";
  case NodeKind::RETURN:
    return "RETURN";
  case NodeKind::OPEN_FILE:
    return "OPEN_FILE";
  case NodeKind::READ_FILE:
    return "READ_FILE";
  case NodeKind::WRITE_FILE:
    return "WRITE_FILE";
  case NodeKind::CLOSE_FILE:
    return "CLOSE_FILE";
  case NodeKind::INTEGER_LITERAL:
    return "INTEGER_LITERAL";
  case NodeKind::REAL_LITERAL:
    return "REAL_LITERAL";
  case NodeKind::BOOLEAN_LITERAL:
    return "BOOLEAN_LITERAL";
  case NodeKind::CHAR_LITERAL:
    return "CHAR_LITERAL";
  case NodeKind::STRING_LITERAL:
    return "STRING_LITERAL";
  case NodeKind::VARIABLE:
    return "VARIABLE";
  case NodeKind::ARRAY_ACCESS:
    return "ARRAY_ACCESS";
  case NodeKind::UNARY:
    return "UNARY";
  case NodeKind::BINARY:
    return "BINARY";
  case NodeKind::FUNCTION_CALL:
    return "FUNCTION_CALL";
  }

  return "UNKNOWN";
}

int main() {
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

// Subprogram Test
DECLARE Line : STRING

PROCEDURE Greet(Name : STRING)
  OUTPUT "Hello ", Name
ENDPROCEDURE

FUNCTION Square(N : INTEGER) RETURNS INTEGER
  RETURN N * N
ENDFUNCTION

CALL Greet("CIE")

// File Handling Test
OPENFILE "data.txt" FOR READ
READFILE "data.txt", Line
CLOSEFILE "data.txt"

OPENFILE "out.txt" FOR WRITE
WRITEFILE "out.txt", Line
CLOSEFILE "out.txt"
)";

  Lexer lexer(source);
  std::vector<Token> tokens = lexer.tokenize();

  for (const Token &token : tokens) {
    if (token.type == TokenType::NEWLINE)
      std::cout << token.line << ":" << token.column << "\t"
                << tokenTypeToString(token.type) << "\t\"" << "\\n"
                << "\"\n";
    else
      std::cout << token.line << ":" << token.column << "\t"
                << tokenTypeToString(token.type) << "\t\"" << token.value
                << "\"\n";
  }

  Parser parser(std::move(tokens));
  std::unique_ptr<Program> program = parser.parse();

  if (!parser.errors().empty()) {
    for (const ParseError &error : parser.errors())
      std::cerr << error.loc.line << ":" << error.loc.column << ": "
                << error.message << "\n";
    return 1;
  }

  std::cout << "\nParsed " << program->statements.size()
            << " top-level statement(s)\n";
  for (const StmtPtr &statement : program->statements)
    std::cout << "  " << nodeKindToString(statement->kind) << "\n";

  return 0;
}
