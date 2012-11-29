#ifndef LEXER_H_
#define LEXER_H_

#include "base/basictypes.h"

#include <string>
#include <map>
#include <vector>

class LexerState;
class Token;

// This module (regex, input, parsed tokens) works entirely in utf8, even on
// Windows, because that's what RE2 processes.

class Lexer {
 public:
  explicit Lexer(const std::string& name);
  LexerState* AddState(const std::string& name);
  void GetTokensUnprocessed(const std::string& text,
                            std::vector<Token>* output_tokens);

  enum TokenType {
    Comment,
    CommentMultiline,
    CommentPreproc,
    CommentSingle,
    Error,
    Keyword,
    KeywordConstant,
    KeywordPseudo,
    KeywordReserved,
    KeywordType,
    LiteralNumberFloat,
    LiteralNumberHex,
    LiteralNumberInteger,
    LiteralNumberOct,
    LiteralString,
    LiteralStringChar,
    LiteralStringEscape,
    Name,
    NameBuiltin,
    NameClass,
    NameLabel,
    Operator,
    Punctuation,
    Text,

    Invalid,
  };

  static LexerState* Push;
  static LexerState* Pop;

 private:
  std::string name_;
  std::map<std::string, LexerState*> states_;

  DISALLOW_COPY_AND_ASSIGN(Lexer);
};

class Token {
 public:
  Token() : index(-1), token(Lexer::Invalid) {
  }

  Token(size_t index, Lexer::TokenType token, const std::string& value) :
      index(index),
      token(token),
      value(value) {
  }

  size_t index;
  Lexer::TokenType token;
  std::string value;
};


#endif  // LEXER_H_
