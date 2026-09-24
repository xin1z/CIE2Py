#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

enum class TokenType {
  // keywords
  KEYWORD_IF,
  KEYWORD_THEN,
  KEYWORD_ELSE,
  KEYWORD_ENDIF,
  KEYWORD_OUTPUT,
  KEYWORD_WHILE,
  KEYWORD_DO,
  KEYWORD_ENDWHILE,

  // identifiers and types
  IDENTIFIER,
  NUMBER,
  STRING,

  // operators
  OPERATOR,
  ASSIGN,

  // those that stay special
  NEWLINE,
  END_OF_FILE,
  UNKNOWN
};

struct Token {
  TokenType type;
  std::string value;
  int line;

  Token(TokenType t, const std::string& v, int l) : type(t), value(v), line(l) {}
};

class Lexer {
  public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();

  private:
    std::string source;
    size_t cursor = 0;
    int line = 1;
    char peek() const;
    char advance();
    bool isAtEnd() const;
    void skipWhitespaceAndComments();

    Token makeToken(TokenType type, const std::string& value);
    Token lexNumber();
    Token lexIdentifierOrKeyword();
    Token lexString();
};

#endif
