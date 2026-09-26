#include "Lexer.h"
#include <cctype>
#include <unordered_map>

static const std::unordered_map<std::string, TokenType> KEYWORDS = {
  // conditional
  {"IF", TokenType::KEYWORD_IF},
  {"THEN", TokenType::KEYWORD_THEN},
  {"ELSE", TokenType::KEYWORD_ELSE},
  {"ENDIF", TokenType::KEYWORD_ENDIF},

  // loops
  {"WHILE", TokenType::KEYWORD_WHILE},
  {"DO", TokenType::KEYWORD_DO},
  {"ENDWHILE", TokenType::KEYWORD_ENDWHILE},

  {"FOR", TokenType::KEYWORD_FOR},
  {"TO", TokenType::KEYWORD_TO},
  {"STEP", TokenType::KEYWORD_STEP},
  {"NEXT", TokenType::KEYWORD_NEXT},

  {"REPEAT", TokenType::KEYWORD_REPEAT},
  {"UNTIL", TokenType::KEYWORD_UNTIL},

  // IO
  {"INPUT", TokenType::KEYWORD_INPUT},
  {"OUTPUT", TokenType::KEYWORD_OUTPUT},

  // logical
  {"AND", TokenType::KEYWORD_AND},
  {"OR", TokenType::KEYWORD_OR},
  {"NOT", TokenType::KEYWORD_NOT}
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
  char c = source[cursor++];
  column++;
  if (c == '\n')
  {
    line++;
    column = 1;
  }
  return c;
}

void Lexer::advanceLeftArrow() {
  advance();
  advance();
  advance();
  column -= 2;
}

bool Lexer::tryUTF8LeftArrow() {
  if (static_cast<unsigned char>(peek()) != 0xE2)
  {
    return false;
  }
  if (cursor + 2 >= source.size())
  {
    return false;
  }
  if (static_cast<unsigned char>(source[cursor + 1]) != 0x86 || static_cast<unsigned char>(source[cursor + 2]) != 0x90)
  {
    return false;
  }
  return true;
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

void Lexer::refreshTokenStart() {
  start_line = line;
  start_column = column;
}

Token Lexer::makeToken(TokenType type, const std::string &value) {
  return Token(type, value, start_line, start_column);
}

Token Lexer::lexNumber() {
  std::string numStr;

  while (!isAtEnd() && std::isdigit(peek())) {
    numStr += advance();
  }

  if (!isAtEnd() && peek() == '.')
  {
    numStr += advance();

    if (isAtEnd() || !std::isdigit(peek())) {
      return makeToken(TokenType::UNKNOWN, numStr);
    }

    while (!isAtEnd() && std::isdigit(peek())) {
      numStr += advance();
    }

    if (!isAtEnd() && peek() == '.') {
      while (!isAtEnd() && (std::isdigit(peek()) || peek() == '.'))
      {
        numStr += advance();
      }

      return makeToken(TokenType::UNKNOWN, numStr);
    }

    return makeToken(TokenType::REAL, numStr);
  }
  return makeToken(TokenType::INTEGER, numStr);
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
  std::string text = "\"";

  while (!isAtEnd() && peek() != '\"' && peek() != '\n') {
    text += advance();
  }
  if(!isAtEnd())
  {
    char c = advance();
    if (c == '\"')
    {
      text += '\"';

      return makeToken(TokenType::STRING, text);
    }
  }
  return makeToken(TokenType::UNKNOWN, text);
}

std::vector<Token> Lexer::tokenize() {
  std::vector<Token> tokens;

  while (!isAtEnd()) {
    skipWhitespaceAndComments();
    if(isAtEnd()) break;

    refreshTokenStart();

    char c = peek();
    
    if (c == '\n') {
      tokens.push_back(makeToken(TokenType::NEWLINE, "\n"));
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
      if (!isAtEnd() && (peek() == '=' || (c == '<' && peek() == '>'))) op += advance();

      if (op == ">") tokens.push_back(makeToken(TokenType::GREATER_THAN, op));
      else if (op == "<") tokens.push_back(makeToken(TokenType::LESS_THAN, op));
      else if (op == ">=") tokens.push_back(makeToken(TokenType::GREATER_EQUAL, op));
      else if (op == "<=") tokens.push_back(makeToken(TokenType::LESS_EQUAL, op));
      else if (op == "=") tokens.push_back(makeToken(TokenType::EQUAL, op));
      else if (op == "+") tokens.push_back(makeToken(TokenType::PLUS, op));
      else if (op == "-") tokens.push_back(makeToken(TokenType::MINUS, op));
      else if (op == "*") tokens.push_back(makeToken(TokenType::MULTIPLY, op));
      else if (op == "/") tokens.push_back(makeToken(TokenType::DIVIDE, op));
      else if (op == "<>") tokens.push_back(makeToken(TokenType::NOT_EQUAL, op));
      else tokens.push_back(makeToken(TokenType::UNKNOWN, op));
    }
    else if (tryUTF8LeftArrow())
    {
      advanceLeftArrow();
      tokens.push_back(makeToken(TokenType::ASSIGN, "\u2190"));
    }
    else {
      std::string unknown(1, advance());
      tokens.push_back(makeToken(TokenType::UNKNOWN, unknown));
    }
  }

  refreshTokenStart();
  tokens.push_back(makeToken(TokenType::END_OF_FILE, ""));
  return tokens;
}
