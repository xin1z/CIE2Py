#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

enum class TokenType
{
  // conditional
  KEYWORD_IF,
  KEYWORD_THEN,
  KEYWORD_ELSE,
  KEYWORD_ENDIF,

  KEYWORD_CASE,
  KEYWORD_OTHERWISE,
  KEYWORD_ENDCASE,

  // loops
  KEYWORD_WHILE,
  KEYWORD_DO,
  KEYWORD_ENDWHILE,

  KEYWORD_FOR,
  KEYWORD_TO,
  KEYWORD_STEP,
  KEYWORD_NEXT,

  KEYWORD_REPEAT,
  KEYWORD_UNTIL,

  // IO
  KEYWORD_INPUT,
  KEYWORD_OUTPUT,

  // subprograms
  KEYWORD_PROCEDURE,
  KEYWORD_ENDPROCEDURE,
  KEYWORD_FUNCTION,
  KEYWORD_ENDFUNCTION,
  KEYWORD_RETURNS,
  KEYWORD_CALL,
  KEYWORD_RETURN,

  // file handling
  KEYWORD_OPENFILE,
  KEYWORD_READFILE,
  KEYWORD_WRITEFILE,
  KEYWORD_CLOSEFILE,
  KEYWORD_READ,
  KEYWORD_WRITE,

  // identifiers and literals
  IDENTIFIER,
  INTEGER,
  REAL,
  STRING,
  CHAR,
  BOOLEAN,

  KEYWORD_INTEGER,
  KEYWORD_REAL,
  KEYWORD_STRING,
  KEYWORD_CHAR,
  KEYWORD_BOOLEAN,

  // properties of literals
  KEYWORD_DECLARE,
  KEYWORD_CONSTANT,
  KEYWORD_ARRAY,
  KEYWORD_OF,

  // operators
  ASSIGN,

  PLUS,
  MINUS,
  MULTIPLY,
  DIVIDE,

  EQUAL,
  NOT_EQUAL,
  LESS_THAN,
  LESS_EQUAL,
  GREATER_THAN,
  GREATER_EQUAL,

  // Special operators
  KEYWORD_MOD,
  KEYWORD_DIV,
  POWER,
  KEYWORD_ROUND,
  KEYWORD_RANDOM,

  // string
  KEYWORD_LENGTH,
  KEYWORD_SUBSTRING,
  KEYWORD_LCASE,
  KEYWORD_UCASE,

  // logical
  KEYWORD_AND,
  KEYWORD_OR,
  KEYWORD_NOT,

  // structural things
  NEWLINE,
  END_OF_FILE,

  // delimiters
  COLON,
  COMMA,
  LPAREN,
  RPAREN,
  LBRACKET,
  RBRACKET,

  // yeah I don't know
  UNKNOWN
};

struct Token
{
  TokenType type;
  std::string value;
  int line;
  int column;

  Token(TokenType t, const std::string& v, int l, int c) : type(t), value(v), line(l), column(c) {}
};

class Lexer
{
public:
  explicit Lexer(const std::string& source);
  std::vector<Token> tokenize();

private:
  std::string source;
  size_t cursor = 0;
  int line = 1;
  int column = 1;
  int start_line = 1;
  int start_column = 1;

  char peek() const;
  char advance();
  void advanceLeftArrow();
  bool isAtEnd() const;
  void skipWhitespaceAndComments();

  void refreshTokenStart();
  Token makeToken(TokenType type, const std::string& value);
  Token lexNumber();
  Token lexIdentifierOrKeyword();
  Token lexChar();
  Token lexString();

  bool tryUTF8LeftArrow();
};

#endif
