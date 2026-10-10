#include <format>
#include <iostream>
#include <iterator>
#include <print>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstring>
#include <readline/readline.h>
#include <readline/history.h>

#include "Environment.h"
#include "mal.h"
#include "reader.h"
#include "types.h"

// extern AST::malValuePtr readStr(const std::string& input);

static malEnvPtr replEnv(new malEnv);

void assertArgNum(int expected, int actual) {
  if (expected != actual) {
    throw std::runtime_error("invalid no of args");
  }
}

void assertArgNumBetween(std::string_view name, int min, int max, int got) {
  if (got < min or got > max) {
    throw std::runtime_error(std::format(
        "{} required args between {} and {}, got {}", name, min, max, got));
  }
}

// TODO: validation stuff for it
auto EVAL(const malValuePtr ast, malEnvPtr env) -> malValuePtr {
  if (env == nullptr) {
    throw std::runtime_error("EVAL env is nullptr");
  }
  auto list = dynamic_cast<AST::List*>(ast.cptr());
  if (!list || list->size() == 0) {
    return ast->eval(env);
  }
  if (auto s = dynamic_cast<AST::Symbol*>(list->item(0).cptr()); s) {
    std::string_view val = s->cValue();
    if (val == "def!") {
      assertArgNum(2, list->size() - 1);
      auto keyPtr = dynamic_cast<AST::Symbol*>(list->item(1).cptr());
      std::string_view key = keyPtr->cValue();
      return env->set(key.data(), EVAL(list->item(2), env));
    } else if (val == "let*") {
      assertArgNum(2, list->size() - 1);
      auto bindings = dynamic_cast<AST::Sequence*>(list->item(1).cptr());
      if (!bindings) {
        throw std::runtime_error("expected list or vector after let*");
      }
      malEnvPtr nextEnv(new malEnv(env));
      for (size_t i = 0; i < bindings->size(); i += 2) {
        auto key = dynamic_cast<AST::Symbol*>(bindings->item(i).cptr());
        auto val = EVAL(bindings->item(i + 1), nextEnv);
        nextEnv->set(key->cValue(), val);
      }
      return EVAL(list->item(2), nextEnv);
    } else if (val == "do") {
      if (list->size() == 1) {
        return AST::nilValue();
      }
      for (size_t i = 1; i < list->size(); i++) {
        malValuePtr ans = EVAL(list->item(i),env);
        if (i == list->size() - 1) {
          return ans;
        }
      }

    } else if (val == "if") {
      assertArgNumBetween("if", 2, 3, list->size() - 1);
      auto cond = EVAL(list->item(1), env)->isTrue();
      if (!cond and list->size() - 1 == 2) {
        return AST::nilValue();
      }
      return EVAL(list->item(cond ? 2 : 3), env);

    } else if (val == "fn*") {
      if (list->size() != 3) {
        throw std::runtime_error(
            std::format("fn* requires 3 arg got {}", list->size()));
      }
      // malEnvPtr nextEnv(new malEnv(env));
      auto args = dynamic_cast<AST::Sequence*>(list->item(1).cptr());
      if (!args) {
        throw std::runtime_error("require param for fn");
      }
      std::vector<std::string> params;
      for (auto& arg : args->m_vec) {
        auto sym = dynamic_cast<AST::Symbol*>(arg.cptr());
        if (!sym) {
          throw std::runtime_error("need param of fn to be a symbol");
        }
        params.push_back(sym->value());
      }
      return AST::lambda(std::move(params), list->item(2), env);
    }
  }
  return ast->eval(env);
}

auto READ(const std::string& param) -> malValuePtr {
  return Parser::readStr(param);
}

auto prStr(const malValuePtr ptr) -> std::string { return ptr->print(); }

auto rep(const std::string& param, malEnvPtr env) -> std::string {
  return prStr(EVAL(READ(param), env));
}

static AST::BuiltInFn::ApplyFn builtIn_add, builtIn_sub, builtIn_mul,
    builtIn_div, builtIn_less, builtIn_lessEqual, builtIn_greater,
    builtIn_greaterEqual, builtIn_equal, builtIn_list, builtIn_isList,
    builtIn_isEmpty, builtIn_count, builtIn_prStr, builtIn_str, builtIn_prn,
    builtIn_println;


int insertNewLine(int count, int key) {
  (void)count;
  (void)key;
  rl_insert_text("\n");
  rl_redisplay();
  return 0;
}

static malEnvPtr activeEnv = nullptr;

static const std::vector<std::string> specialForms = {
    "def!", "let*", "do", "if", "fn*", "quote", "quasiquote", "splice-unquote"
};

static char* env_variable_generator(const char* text, int state) {
  static std::vector<std::string> matches;
  static size_t match_index = 0;

  if (state == 0) {
    matches.clear();
    match_index = 0;
    std::string prefix(text);

    if (activeEnv != nullptr) {
      for (const auto& sym : activeEnv->getKeys()) {
        if (sym.rfind(prefix, 0) == 0) {
          matches.push_back(sym);
        }
      }
    }

    for (const auto& form : specialForms) {
      if (form.rfind(prefix, 0) == 0) {
        matches.push_back(form);
      }
    }
  }

  if (match_index < matches.size()) {
    return strdup(matches[match_index++].c_str());
  }
  return nullptr;
}

static char** lisp_completion(const char* text, int start, int end) {
  (void)start;
  (void)end;
  rl_attempted_completion_over = 1;
  return rl_completion_matches(text, env_variable_generator);
}

constexpr const char* HISTORY_FILE = ".echo_history";
auto main() -> int {
  malEnvPtr globalEnv = new malEnv;
  activeEnv = globalEnv;
  globalEnv->set("+", AST::builtin("+", &builtIn_add));
  globalEnv->set("-", AST::builtin("-", &builtIn_sub));
  globalEnv->set("*", AST::builtin("*", &builtIn_mul));
  globalEnv->set("/", AST::builtin("/", &builtIn_div));
  globalEnv->set("<", AST::builtin("<", &builtIn_less));
  globalEnv->set("<=", AST::builtin("<=", &builtIn_lessEqual));
  globalEnv->set(">", AST::builtin(">", &builtIn_greater));
  globalEnv->set(">=", AST::builtin(">=", &builtIn_greaterEqual));
  globalEnv->set("=", AST::builtin("=", &builtIn_equal));
  globalEnv->set("list", AST::builtin("list", &builtIn_list));
  globalEnv->set("list?", AST::builtin("list?", &builtIn_isList));
  globalEnv->set("empty?", AST::builtin("empty?", &builtIn_isEmpty));
  globalEnv->set("count", AST::builtin("count", &builtIn_count));
  globalEnv->set("pr-str", AST::builtin("pr-str", &builtIn_prStr));
  globalEnv->set("str", AST::builtin("str", &builtIn_str));
  globalEnv->set("prn", AST::builtin("prn", &builtIn_prn));
  globalEnv->set("println", AST::builtin("println", &builtIn_println));

  rep("(def! not (fn* (a) (if a false true)))", globalEnv);

  char *line = NULL;
  rl_bind_keyseq("\033[13;2u", insertNewLine);
  rl_bind_keyseq("\033[27;2;13~", insertNewLine);
  rl_bind_keyseq("\033\r", insertNewLine);
  rl_attempted_completion_function = lisp_completion;
  rl_basic_word_break_characters = const_cast<char*>(" \t\n\"\\'`@~()[]{};,");
  using_history();
  read_history(HISTORY_FILE);
  while ((line = readline("user> ")) != NULL) {
    if (*line == '\0') {
      free(line);
      continue;
    }
    add_history(line);
    std::string lineStr = line;
    free(line);
    if (lineStr == "exit") {
      break;
    }
    std::string res;
    try {
      res = rep(lineStr, globalEnv);
      std::print("{}\n", res);
    } catch (const std::exception& err) {
      if (std::string_view(err.what()) != "malEmptyInputException") {
        std::print("{}\n", err.what());
      }
    } catch (const std::string& s) {
      std::print("{}\n", s);
    }
  }
  write_history(HISTORY_FILE);
}
malValuePtr builtIn_add(const std::string& name, malValueIter argsBegin,
                        malValueIter argsEnd) {
  int64_t val = 0;
  for (auto& arg : std::ranges::subrange(argsBegin, argsEnd)) {
    AST::Integer* castArg = dynamic_cast<AST::Integer*>(arg.cptr());
    if (castArg) {
      val += castArg->value();
    }
  }
  return AST::integer(val);
}

malValuePtr builtIn_sub(const std::string& name, malValueIter argsBegin,
                        malValueIter argsEnd) {
  long dis = std::distance(argsBegin, argsEnd);
  if (dis > 0) {
    auto& init = *argsBegin;
    AST::Integer* castInit = dynamic_cast<AST::Integer*>(init.cptr());
    int64_t val;
    if (castInit) {
      val = castInit->value();
    } else {
      throw std::runtime_error("in sub not integer");
    }
    if (dis == 1) {
      return AST::integer(-val);
    }
    for (auto& arg : std::ranges::subrange(argsBegin + 1, argsEnd)) {
      AST::Integer* castArg = dynamic_cast<AST::Integer*>(arg.cptr());
      if (castArg) {
        val -= castArg->value();
      }
    }
    return AST::integer(val);
  }
  throw std::runtime_error("sub expects at least 1 argument");
}

malValuePtr builtIn_mul(const std::string& name, malValueIter argsBegin,
                        malValueIter argsEnd) {
  int64_t val = 1;
  for (auto& arg : std::ranges::subrange(argsBegin, argsEnd)) {
    AST::Integer* castArg = dynamic_cast<AST::Integer*>(arg.cptr());
    if (castArg) {
      val *= castArg->value();
    }
  }
  return AST::integer(val);
}

malValuePtr builtIn_div(const std::string& name, malValueIter argsBegin,
                        malValueIter argsEnd) {
  long dis = std::distance(argsBegin, argsEnd);
  if (dis > 0) {
    auto& init = *argsBegin;
    AST::Integer* castInit = dynamic_cast<AST::Integer*>(init.cptr());
    int64_t val;
    if (castInit) {
      val = castInit->value();
    } else {
      throw std::runtime_error("in div not integer");
    }
    if (dis == 1) {
      return AST::integer(1 / val);
    }
    for (auto& arg : std::ranges::subrange(argsBegin + 1, argsEnd)) {
      AST::Integer* castArg = dynamic_cast<AST::Integer*>(arg.cptr());
      if (castArg) {
        int64_t divisor = castArg->value();
        if (divisor == 0) {
          throw std::runtime_error("Division by zero");
        }
        val /= divisor;
      }
    }
    return AST::integer(val);
  }
  throw std::runtime_error("div expects at least 1 argument");
}

template <typename Op>
static malValuePtr numericCompare(const std::string& name, malValueIter argsBegin,
                                  malValueIter argsEnd, Op op) {
  assertArgNum(2, std::distance(argsBegin, argsEnd));
  AST::Integer* arg1 = dynamic_cast<AST::Integer*>(argsBegin->cptr());
  AST::Integer* arg2 = dynamic_cast<AST::Integer*>((argsBegin + 1)->cptr());
  if (!arg1 || !arg2) {
    throw std::runtime_error(std::format("{} needs integer args only", name));
  }
  return AST::boolean(op(arg1->value(), arg2->value()));
}

malValuePtr builtIn_less(const std::string& name, malValueIter argsBegin,
                         malValueIter argsEnd) {
  return numericCompare(name, argsBegin, argsEnd,
                        [](int64_t a, int64_t b) { return a < b; });
}

malValuePtr builtIn_lessEqual(const std::string& name, malValueIter argsBegin,
                              malValueIter argsEnd) {
  return numericCompare(name, argsBegin, argsEnd,
                        [](int64_t a, int64_t b) { return a <= b; });
}

malValuePtr builtIn_greater(const std::string& name, malValueIter argsBegin,
                            malValueIter argsEnd) {
  return numericCompare(name, argsBegin, argsEnd,
                        [](int64_t a, int64_t b) { return a > b; });
}

malValuePtr builtIn_greaterEqual(const std::string& name, malValueIter argsBegin,
                                 malValueIter argsEnd) {
  return numericCompare(name, argsBegin, argsEnd,
                        [](int64_t a, int64_t b) { return a >= b; });
}

malValuePtr builtIn_equal(const std::string& name, malValueIter argsBegin,
                          malValueIter argsEnd) {
  assertArgNum(2, std::distance(argsBegin, argsEnd));
  return AST::boolean((*argsBegin)->isEqualTo((*(argsBegin + 1)).cptr()));
}

malValuePtr builtIn_list(const std::string& name, malValueIter argsBegin,
                         malValueIter argsEnd) {
  malValueVec vec(argsBegin, argsEnd);
  return AST::list(std::move(vec));
}

malValuePtr builtIn_isList(const std::string& name, malValueIter argsBegin,
                           malValueIter argsEnd) {
  assertArgNum(1, std::distance(argsBegin, argsEnd));
  return AST::boolean(dynamic_cast<AST::List*>(argsBegin->cptr()) != nullptr);
}

malValuePtr builtIn_isEmpty(const std::string& name, malValueIter argsBegin,
                            malValueIter argsEnd) {
  assertArgNum(1, std::distance(argsBegin, argsEnd));
  if (auto seq = dynamic_cast<AST::Sequence*>(argsBegin->cptr())) {
    return AST::boolean(seq->empty());
  }
  return AST::boolean(false);
}

malValuePtr builtIn_count(const std::string& name, malValueIter argsBegin,
                          malValueIter argsEnd) {
  assertArgNum(1, std::distance(argsBegin, argsEnd));
  if (argsBegin->cptr() == AST::nilValue().cptr()) {
    return AST::integer(0);
  }
  if (auto seq = dynamic_cast<AST::Sequence*>(argsBegin->cptr())) {
    return AST::integer(seq->size());
  }
  return AST::integer(0);
}

static std::string printValues(malValueIter begin, malValueIter end,
                               const std::string& sep, bool readably) {
  std::string out;
  bool first = true;
  for (auto it = begin; it != end; ++it) {
    if (!first) {
      out += sep;
    }
    first = false;
    out += (*it)->print(readably);
  }
  return out;
}

malValuePtr builtIn_prStr(const std::string& name, malValueIter argsBegin,
                          malValueIter argsEnd) {
  return AST::string(printValues(argsBegin, argsEnd, " ", true));
}

malValuePtr builtIn_str(const std::string& name, malValueIter argsBegin,
                        malValueIter argsEnd) {
  return AST::string(printValues(argsBegin, argsEnd, "", false));
}

malValuePtr builtIn_prn(const std::string& name, malValueIter argsBegin,
                        malValueIter argsEnd) {
  std::println("{}", printValues(argsBegin, argsEnd, " ", true));
  return AST::nilValue();
}

malValuePtr builtIn_println(const std::string& name, malValueIter argsBegin,
                            malValueIter argsEnd) {
  std::println("{}", printValues(argsBegin, argsEnd, " ", false));
  return AST::nilValue();
}
