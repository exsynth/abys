#include "abys/frontend/slang_pragma.h"

#include "slang/driver/Driver.h"
#include "slang/parsing/Token.h"
#include "slang/syntax/AllSyntax.h"
#include "slang/syntax/SyntaxTree.h"
#include "slang/syntax/SyntaxVisitor.h"
#include "slang/text/SourceLocation.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

namespace abys::frontend {

namespace {

std::string normalize_pragma_comment(std::string_view raw) {
  if (raw.starts_with("//")) {
    raw.remove_prefix(2);
  } else if (raw.starts_with("/*")) {
    raw.remove_prefix(2);
    if (raw.ends_with("*/")) {
      raw.remove_suffix(2);
    }
  }
  while (!raw.empty() && std::isspace(static_cast<unsigned char>(raw.front()))) {
    raw.remove_prefix(1);
  }
  while (!raw.empty() && std::isspace(static_cast<unsigned char>(raw.back()))) {
    raw.remove_suffix(1);
  }
  std::string result(raw);
  std::transform(result.begin(), result.end(), result.begin(), [](unsigned char character) {
    return static_cast<char>(std::tolower(character));
  });
  return result;
}

std::vector<std::string_view> split_words(std::string_view text) {
  std::vector<std::string_view> words;
  size_t pos = 0;
  while (pos < text.size()) {
    while (pos < text.size() && std::isspace(static_cast<unsigned char>(text[pos]))) {
      ++pos;
    }
    const size_t start = pos;
    while (pos < text.size() && !std::isspace(static_cast<unsigned char>(text[pos]))) {
      ++pos;
    }
    if (start != pos) {
      words.push_back(text.substr(start, pos - start));
    }
  }
  return words;
}

bool is_synthesis_pragma_prefix(std::string_view word) {
  return std::find(kSynthesisPragmaPrefixes.begin(), kSynthesisPragmaPrefixes.end(), word) !=
         kSynthesisPragmaPrefixes.end();
}

bool is_synthesis_comment(std::string_view text) {
  size_t pos = 0;
  while (pos < text.size() && !std::isspace(static_cast<unsigned char>(text[pos]))) {
    ++pos;
  }
  return is_synthesis_pragma_prefix(text.substr(0, pos));
}

class PragmaCollector final : public slang::syntax::SyntaxVisitor<PragmaCollector> {
public:
  PragmaCollector(const slang::SourceManager &source_manager, PragmaMap &pragmas,
                  Diagnostics &diagnostics)
      : source_manager_(source_manager), pragmas_(pragmas), diagnostics_(diagnostics) {}

  void handle(const slang::syntax::CaseStatementSyntax &syntax) {
    PragmaInfo info;
    for (const auto *attr : syntax.attributes) {
      for (const auto *spec : attr->specs) {
        const std::string_view name = spec->name.valueText();
        bool handled = false;
        if (name == "full_case") {
          info.full_case = true;
          handled = true;
        } else if (name == "parallel_case") {
          info.parallel_case = true;
          handled = true;
        }
        if (handled) {
          handled_attributes_.insert(spec);
        }
      }
    }
    if (!syntax.items.empty()) {
      const slang::parsing::Token first_token = syntax.items[0]->getFirstToken();
      for (const auto &trivia : first_token.trivia()) {
        if (trivia.kind == slang::parsing::TriviaKind::EndOfLine) {
          break;
        }
        if (trivia.kind != slang::parsing::TriviaKind::LineComment &&
            trivia.kind != slang::parsing::TriviaKind::BlockComment) {
          continue;
        }
        const std::string_view raw = trivia.getRawText();
        const std::string text = normalize_pragma_comment(raw);
        const auto words = split_words(text);
        if (words.empty() || !is_synthesis_pragma_prefix(words[0])) {
          continue;
        }
        bool handled = false;
        for (size_t i = 1; i < words.size(); ++i) {
          if (words[i] == "full_case") {
            info.full_case = true;
            handled = true;
          } else if (words[i] == "parallel_case") {
            info.parallel_case = true;
            handled = true;
          }
        }
        if (handled) {
          handled_comments_.insert(raw.data());
        }
      }
    }
    if (info.full_case || info.parallel_case) {
      pragmas_.by_node[&syntax] = info;
    }
    visitDefault(syntax);
  }

  void handle(const slang::syntax::AttributeSpecSyntax &syntax) {
    if (handled_attributes_.find(&syntax) == handled_attributes_.end()) {
      const slang::SourceLocation loc = syntax.name.location();
      std::string detail;
      if (loc.valid()) {
        detail = "at " + std::string(source_manager_.getFileName(loc)) + ":" +
                 std::to_string(source_manager_.getLineNumber(loc)) + ":" +
                 std::to_string(source_manager_.getColumnNumber(loc)) + ": ";
      }
      detail += syntax.name.valueText();
      diagnostics_.warning(DiagnosticId::kFrontendUnhandledSynthesisAttribute, std::move(detail));
    }
    visitDefault(syntax);
  }

  void visitToken(slang::parsing::Token token) {
    for (const auto &trivia : token.trivia()) {
      if (trivia.kind != slang::parsing::TriviaKind::LineComment &&
          trivia.kind != slang::parsing::TriviaKind::BlockComment) {
        continue;
      }
      const std::string_view raw = trivia.getRawText();
      if (handled_comments_.find(raw.data()) != handled_comments_.end()) {
        continue;
      }
      const std::string text = normalize_pragma_comment(raw);
      if (!is_synthesis_comment(text)) {
        continue;
      }
      const slang::SourceLocation loc = token.location();
      std::string detail;
      if (loc.valid()) {
        detail = "at " + std::string(source_manager_.getFileName(loc)) + ":" +
                 std::to_string(source_manager_.getLineNumber(loc)) + ":" +
                 std::to_string(source_manager_.getColumnNumber(loc)) + ": ";
      }
      detail += raw;
      diagnostics_.warning(DiagnosticId::kFrontendUnhandledSynthesisComment, std::move(detail));
    }
  }

private:
  const slang::SourceManager &source_manager_;
  PragmaMap &pragmas_;
  Diagnostics &diagnostics_;
  std::unordered_set<const char *> handled_comments_;
  std::unordered_set<const slang::syntax::AttributeSpecSyntax *> handled_attributes_;
};

} // namespace

PragmaMap collect_pragmas(slang::driver::Driver &driver, Diagnostics &diagnostics) {
  PragmaMap pragmas;
  for (const auto &tree : driver.syntaxTrees) {
    PragmaCollector collector(tree->sourceManager(), pragmas, diagnostics);
    tree->root().visit(collector);
  }
  return pragmas;
}

} // namespace abys::frontend
