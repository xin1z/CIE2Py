#include "ast/ASTPrinter.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"

#include <iostream>
#include <string>

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
  Parser parser(lexer.tokenize());
  auto program = parser.parse();

  if (!parser.errors().empty())
  {
    for (const ParseError& error : parser.errors())
      std::cerr << error.loc.line << ":" << error.loc.column << ": " << error.message << "\n";
    return 1;
  }

  ASTPrinter printer(std::cout);
  printer.print(*program);

  return 0;
}
