#include "abys/liberty/parser.h"

#include <cctype>
#include <utility>

namespace abys::liberty {
namespace {

enum class TokenKind {
  kEnd,
  kWord,
  kString,
  kLeftParen,
  kRightParen,
  kLeftBrace,
  kRightBrace,
  kColon,
  kSemicolon,
  kComma,
  kEqual,
  kInvalid,
};

struct Token {
  TokenKind kind = TokenKind::kEnd;
  std::string text;
  size_t line = 0;
};

struct ParseResult {
  Ast ast;
  std::string message;
  size_t line = 0;
};

class Lexer {
public:
  explicit Lexer(std::string_view source) : source_(source) {}

  Token next() {
    skip_space();
    if (unterminated_comment_) {
      return {TokenKind::kInvalid, "unterminated comment", line_};
    }
    if (position_ == source_.size()) {
      return {TokenKind::kEnd, {}, line_};
    }

    const size_t token_line = line_;
    const char character = source_[position_++];
    switch (character) {
    case '(':
      return {TokenKind::kLeftParen, {}, token_line};
    case ')':
      return {TokenKind::kRightParen, {}, token_line};
    case '{':
      return {TokenKind::kLeftBrace, {}, token_line};
    case '}':
      return {TokenKind::kRightBrace, {}, token_line};
    case ':':
      return {TokenKind::kColon, {}, token_line};
    case ';':
      return {TokenKind::kSemicolon, {}, token_line};
    case ',':
      return {TokenKind::kComma, {}, token_line};
    case '=':
      return {TokenKind::kEqual, {}, token_line};
    case '"':
      return string(token_line);
    default:
      break;
    }

    std::string text(1, character);
    while (position_ < source_.size() && !is_delimiter(source_[position_])) {
      text.push_back(source_[position_++]);
    }
    return {TokenKind::kWord, std::move(text), token_line};
  }

private:
  static bool is_delimiter(char character) {
    return std::isspace(static_cast<unsigned char>(character)) || character == '(' ||
           character == ')' || character == '{' || character == '}' || character == ':' ||
           character == ';' || character == ',' || character == '=' || character == '"';
  }

  void skip_space() {
    while (position_ < source_.size()) {
      if (source_[position_] == '\\' && position_ + 1 < source_.size() &&
          source_[position_ + 1] == '\n') {
        position_ += 2;
        ++line_;
        continue;
      }
      if (std::isspace(static_cast<unsigned char>(source_[position_]))) {
        if (source_[position_] == '\n') {
          ++line_;
        }
        ++position_;
        continue;
      }
      if (source_[position_] != '/' || position_ + 1 == source_.size()) {
        return;
      }
      if (source_[position_ + 1] == '/') {
        position_ += 2;
        while (position_ < source_.size() && source_[position_] != '\n') {
          ++position_;
        }
        continue;
      }
      if (source_[position_ + 1] != '*') {
        return;
      }
      position_ += 2;
      while (position_ + 1 < source_.size() &&
             (source_[position_] != '*' || source_[position_ + 1] != '/')) {
        if (source_[position_] == '\n') {
          ++line_;
        }
        ++position_;
      }
      if (position_ + 1 == source_.size()) {
        unterminated_comment_ = true;
        position_ = source_.size();
        return;
      }
      position_ += 2;
    }
  }

  Token string(size_t token_line) {
    std::string text;
    while (position_ < source_.size()) {
      const char character = source_[position_++];
      if (character == '"') {
        return {TokenKind::kString, std::move(text), token_line};
      }
      if (character == '\n') {
        ++line_;
      }
      if (character != '\\') {
        text.push_back(character);
        continue;
      }
      if (position_ == source_.size()) {
        break;
      }
      const char escaped = source_[position_++];
      if (escaped == '\n') {
        ++line_;
      } else if (escaped == '"' || escaped == '\\') {
        text.push_back(escaped);
      } else {
        text.push_back('\\');
        text.push_back(escaped);
      }
    }
    return {TokenKind::kInvalid, "unterminated string", token_line};
  }

  std::string_view source_;
  size_t position_ = 0;
  size_t line_ = 1;
  bool unterminated_comment_ = false;
};

class Parser {
public:
  explicit Parser(std::string_view source) : lexer_(source), token_(lexer_.next()) {}

  ParseResult run() {
    ParseResult result;
    while (token_.kind != TokenKind::kEnd && message_.empty()) {
      Group group;
      if (!parse_group(group)) {
        break;
      }
      result.ast.groups.push_back(std::move(group));
    }
    result.message = std::move(message_);
    result.line = error_line_;
    return result;
  }

private:
  void advance() { token_ = lexer_.next(); }

  bool expect(TokenKind kind, std::string message) {
    if (token_.kind != kind) {
      fail(std::move(message));
      return false;
    }
    advance();
    return true;
  }

  void fail(std::string message) {
    if (message_.empty()) {
      message_ = token_.kind == TokenKind::kInvalid ? token_.text : std::move(message);
      error_line_ = token_.line;
    }
  }

  bool parse_value(Value &value) {
    if (token_.kind != TokenKind::kWord && token_.kind != TokenKind::kString) {
      fail("expected attribute value");
      return false;
    }
    value.text = std::move(token_.text);
    value.quoted = token_.kind == TokenKind::kString;
    advance();
    return true;
  }

  bool parse_values(std::vector<Value> &values) {
    if (token_.kind == TokenKind::kRightParen) {
      return true;
    }
    while (true) {
      Value value;
      if (!parse_value(value)) {
        return false;
      }
      values.push_back(std::move(value));
      if (token_.kind != TokenKind::kComma) {
        return true;
      }
      advance();
    }
  }

  bool parse_group(Group &group) {
    if (token_.kind != TokenKind::kWord) {
      fail("expected group name");
      return false;
    }
    group.name = std::move(token_.text);
    group.line = token_.line;
    advance();
    if (!expect(TokenKind::kLeftParen, "expected '(' after group name") ||
        !parse_values(group.arguments) ||
        !expect(TokenKind::kRightParen, "expected ')' after group arguments") ||
        !expect(TokenKind::kLeftBrace, "expected '{' after group arguments")) {
      return false;
    }
    while (token_.kind != TokenKind::kRightBrace) {
      if (token_.kind == TokenKind::kEnd) {
        fail("expected '}' before end of file");
        return false;
      }
      if (!parse_statement(group)) {
        return false;
      }
    }
    advance();
    return true;
  }

  bool parse_statement(Group &group) {
    if (token_.kind != TokenKind::kWord) {
      fail("expected group or attribute name");
      return false;
    }
    const std::string name = std::move(token_.text);
    const size_t line = token_.line;
    advance();

    if (token_.kind == TokenKind::kLeftParen) {
      advance();
      std::vector<Value> values;
      if (!parse_values(values) ||
          !expect(TokenKind::kRightParen, "expected ')' after argument list")) {
        return false;
      }
      if (token_.kind == TokenKind::kLeftBrace) {
        Group child;
        child.name = name;
        child.arguments = std::move(values);
        child.line = line;
        child.order = group.attributes.size() + group.groups.size();
        advance();
        while (token_.kind != TokenKind::kRightBrace) {
          if (token_.kind == TokenKind::kEnd) {
            fail("expected '}' before end of file");
            return false;
          }
          if (!parse_statement(child)) {
            return false;
          }
        }
        advance();
        group.groups.push_back(std::move(child));
        return true;
      }
      if (token_.kind == TokenKind::kSemicolon) {
        advance();
      }
      group.attributes.push_back({name, AttributeKind::kComplex, std::move(values), line,
                                  group.attributes.size() + group.groups.size()});
      return true;
    }

    AttributeKind kind;
    if (token_.kind == TokenKind::kColon) {
      kind = AttributeKind::kSimple;
    } else if (token_.kind == TokenKind::kEqual) {
      kind = AttributeKind::kVariable;
    } else {
      fail("expected '(', ':', or '=' after name");
      return false;
    }
    advance();
    Value value;
    if (!parse_value(value) ||
        !expect(TokenKind::kSemicolon, "expected ';' after attribute value")) {
      return false;
    }
    std::vector<Value> values;
    values.push_back(std::move(value));
    group.attributes.push_back(
        {name, kind, std::move(values), line, group.attributes.size() + group.groups.size()});
    return true;
  }

  Lexer lexer_;
  Token token_;
  std::string message_;
  size_t error_line_ = 0;
};

} // namespace

std::optional<Ast> parse(std::string_view source, Diagnostics &diagnostics,
                         std::string_view source_name) {
  ParseResult result = Parser(source).run();
  if (result.message.empty()) {
    return std::move(result.ast);
  }
  std::string detail;
  if (!source_name.empty()) {
    detail = std::string(source_name) + ':';
  }
  detail += std::to_string(result.line) + ": " + result.message;
  diagnostics.error(DiagnosticId::kLibertySyntaxError, std::move(detail));
  return std::nullopt;
}

} // namespace abys::liberty
