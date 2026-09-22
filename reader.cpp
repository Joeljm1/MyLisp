#include <cassert>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "string.h"
#include "types.h"

static const std::regex intRegex("^[-+]?\\d+$");
static const std::regex closeRegex("[\\)\\]}]");

static const std::regex whitespaceRegex("[\\s,]+|;.*");
static const std::regex tokenRegexes[] = {
    std::regex("~@"),
    std::regex("[\\[\\]{}()'`~^@]"),
    std::regex("\"(?:\\\\.|[^\\\\\"])*\""),
    std::regex("[^\\s\\[\\]{}('\"`,;)]+"),
};

namespace Parser {

using std::string;
using StringIter = std::string::const_iterator;

class Tokenizer {
 public:
  Tokenizer(const string& str) : m_iter(str.begin()), m_end(str.end()) {
    nextToken();
  }

  string next() {
    assert(!eof() && "Tokeniser reading past EOF in next\n");
    string curr = m_token;
    nextToken();
    return curr;
  }

  string peek() const { return m_token; }
  bool eof() const { return m_iter == m_end; }

 private:
  bool matchRegex(const std::regex& regex);
  void nextToken();

  void skipSpace();

  string m_token;
  StringIter m_iter;
  StringIter m_end;
};

bool Tokenizer::matchRegex(const std::regex& regex) {
  if (eof()) {
    return false;
  }

  std::smatch match;
  auto flags = std::regex_constants::match_continuous;
  if (!std::regex_search(m_iter, m_end, match, regex, flags)) {
    return false;
  }
  m_token = match.str();
  assert(match.position() == 0 && "Position not 0");
  assert(match.length() > 0 && " match is zero length");

  return true;
}

void Tokenizer::skipSpace() {
  while (matchRegex(whitespaceRegex)) {
    m_iter += m_token.size();
  }
}

void Tokenizer::nextToken() {
  m_iter += m_token.size();
  skipSpace();
  if (eof()) {
    m_token.clear();
    return;
  }
  for (auto& reg : tokenRegexes) {
    if (matchRegex(reg)) {
      return;
    }
  }

  // NOTE: more alloc does not matter as error i think so why not
  std::string err(m_iter, m_end);
  if (err[0] == '"') {
    err += " -> \" missing to close";
  } else {
    err += " -> not a valid token";
  }
  throw std::runtime_error(err);
}

malValuePtr readFrom(Tokenizer& tok);

static malValuePtr processMacro(std::string symbol, Tokenizer& tok) {
  return AST::list(AST::symbol(symbol), readFrom(tok));
}

malValuePtr readAtom(Tokenizer& tok) {
  string token = tok.next();
  struct Constants {
    const char* token;
    malValuePtr val;
  };

  Constants constTable[] = {{"false", AST::falseValue()},
                            {"true", AST::trueValue()},
                            {"nil", AST::nilValue()}};

  struct ReaderMacro {
    const char* token;
    const char* symbol;
  };
  ReaderMacro macroTable[] = {
      {"@", "deref"},           {"`", "quasiquote"}, {"'", "quote"},
      {"~@", "splice-unquote"}, {"~", "unquote"},
  };
  if (token[0] == '"') {
    return AST::string(unescape(token));
  }
  if (token[0] == ':') {
    return AST::keyword(token);
  }

  for (auto& constVal : constTable) {
    if (token == constVal.token) {
      return constVal.val;
    }
  }
  for (auto& macroVal : macroTable) {
    if (token == macroVal.token) {
      return processMacro(macroVal.symbol, tok);
    }
  }

  if (std::regex_match(token, intRegex)) {
    return AST::integer(token);
  }

  return AST::symbol(token);
}

// TODO: move to header file to be reused
#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)

#define errWithLineNo(err) (err " (line " STRINGIFY(__LINE__) ")")

malValueVec readList(Tokenizer& tok, const char* end) {
  malValueVec vec;
  while (tok.peek() != end) {
    if (tok.eof()) {
      throw std::runtime_error(
          errWithLineNo("end of file unexpected expected ')' in"));//TODO: later remove line number
    }
    vec.push_back(readFrom(tok));
  }
  tok.next();  // consume ')'
  return vec;  // using nrvo skip copying it think
}

std::string hashKey(malValuePtr key) {
  if (const AST::String* s = dynamic_cast<AST::String*>(key.cptr())) {
    return s->print();
  } else if (const AST::Keywords* k =
                 dynamic_cast<AST::Keywords*>(key.cptr())) {
    return k->value();
  } else {
    // TODO: add more keys (make keys type as malValPtr instead of string)
    throw std::runtime_error("unexpected key");
  }
}

malValueMap readMap(Tokenizer& tok) {
  malValueMap map;
  while (tok.peek() != "}") {
    if (tok.eof()) {
      throw std::runtime_error(
          errWithLineNo("end of file unexpected , expected '}' in"));
    }

    malValuePtr keyPtr = readFrom(tok);
    std::string key = hashKey(keyPtr);
    if (tok.eof()) {
      throw std::runtime_error("eexpected value but got eof");
    }
    map[key] = readFrom(tok);
  }
  return map;
}

malValuePtr readFrom(Tokenizer& tok) {
  assert(!tok.eof() && "unexpected eof");
  string token = tok.peek();
  // MAL_CHECK(!std::regex_match(token, closeRegex),
  //         "unexpected '%s'", token.c_str());
  switch (token[0]) {
    case '(': {
      tok.next();
      malValueVec v = readList(tok, ")");
      return AST::list(std::move(v));
    }
    case '[': {
      tok.next();
      malValueVec v = readList(tok, "]");
      return AST::vector(std::move(v));
    }
    case '{': {
      tok.next();
      malValueMap m = readMap(tok);
      return AST::hash(std::move(m));
    }
  }
  return readAtom(tok);
}

malValuePtr readStr(const std::string& input) {
  Tokenizer tokenizer(input);
    if (tokenizer.eof()) {
        throw std::runtime_error("malEmptyInputException");
    }
    return readFrom(tokenizer);
}

}  // namespace Parser

