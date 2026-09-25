#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

enum class TokenType {
  // conditional
  KEYWORD_IF,
  KEYWORD_THEN,
  KEYWORD_ELSE,
  KEYWORD_ENDIF,

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

  // identifiers and literals
  IDENTIFIER,
  INTEGER,
  REAL,
  STRING,

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

  // logical
  KEYWORD_AND,
  KEYWORD_OR,
  KEYWORD_NOT,
  
  // structural things
  NEWLINE,
  END_OF_FILE,

  // yeah I don't know
  UNKNOWN
};

struct Token {
  TokenType type;
  std::string value;
  int line;
  int column;

  Token(TokenType t, const std::string& v, int l, int c) : type(t), value(v), line(l), column(c) {}
};

class Lexer {
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
    bool isAtEnd() const;
    void skipWhitespaceAndComments();

    void refreshTokenStart();
    Token makeToken(TokenType type, const std::string& value);
    Token lexNumber();
    Token lexIdentifierOrKeyword();
    Token lexString();
};

#endif
