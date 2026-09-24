#include "Lexer.h"
#include <cctype>
#include <unordered_map>

static const std::unordered_map<std::string, TokenType> KEYWORDS = {
  {"IF", TokenType::KEYWORD_IF},
  {"THEN", TokenType::KEYWORD_THEN},
  {"ELSE", TokenType::KEYWORD_ELSE},
  {"ENDIF", TokenType::KEYWORD_ENDIF},
  {"OUTPUT", TokenType::KEYWORD_OUTPUT},
  {"WHILE", TokenType::KEYWORD_WHILE},
  {"DO", TokenType::KEYWORD_DO},
  {"ENDWHILE", TokenType::KEYWORD_ENDWHILE}
};

Lexer::Lexer(const std::string& source) : source(source) {}

bool Lexer::isAtEnd() const {
  return cursor >= source.length();
}

char Lexer::peek() const {
  if (isAtEnd()) return '\0';
  return source[cursor];
}

char Lexer::advance() {
  return source[cursor++];
}

void Lexer::skipWhitespaceAndComments() {
  while (!isAtEnd()) {
    char c = peek();
    if (c == ' ' || c == '\r' || c == '\t') {
      advance();
    }
    else if (c == '/' && cursor + 1 < source.length() && source[cursor + 1] == '/') {
      while (!isAtEnd() && peek() !='\n') {
        advance();
      }
    }
    else break;
  }
}

Token Lexer::makeToken(TokenType type, const std::string &value) {
  return Token(type, value, line);
}

Token Lexer::lexNumber() {
  std::string numStr;
  while (!isAtEnd() && (std::isdigit(peek()) || peek() == '.')) {
    numStr += advance();
  }
  return makeToken(TokenType::NUMBER, numStr);
}

Token Lexer::lexIdentifierOrKeyword() {
  std::string text;
  while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) {
    text += advance();
  }

  auto it = KEYWORDS.find(text);
  if(it!= KEYWORDS.end()) {
    return makeToken(it -> second, text);
  }
  return makeToken(TokenType::IDENTIFIER, text);
}

Token Lexer::lexString() {
  advance();
  std::string text;
  while (!isAtEnd() && peek() != '"') {
    if(peek() == '\n') line++;
    text += advance();
  }
  if(!isAtEnd()) advance();
  return makeToken(TokenType::STRING, "\"" + text + "\"");
}

std::vector<Token> Lexer::tokenize() {
  std::vector<Token> tokens;

  while (!isAtEnd()) {
    skipWhitespaceAndComments();
    if(isAtEnd()) break;
    char c = peek();

    if (c == '\n') {
      tokens.push_back(makeToken(TokenType::NEWLINE, "\n"));
      line++;
      advance();
    }
    else if (std::isdigit(c)) {
      tokens.push_back(lexNumber());
    }
    else if (std::isalpha(c) || c == '_') {
      tokens.push_back(lexIdentifierOrKeyword());
    }
    else if (c == '\"')
    {
      tokens.push_back(lexString());
    }
    else if (c == '>' || c == '<' || c == '=' || c == '+' || c == '-' || c == '*' || c == '/') {
      std::string op(1, advance());
      if (!isAtEnd() && peek() == '=') op += advance();

      tokens.push_back(makeToken(TokenType::OPERATOR, op));
    }
    else {
      std::string unknown(1, advance());
      tokens.push_back(makeToken(TokenType::UNKNOWN, unknown));
    }
  }

  tokens.push_back(makeToken(TokenType::END_OF_FILE, ""));
  return tokens;
}
