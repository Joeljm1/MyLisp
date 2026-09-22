#include <iostream>
#include <print>
#include <stdexcept>
#include <string>

#include "Environment.h"
#include "mal.h"
#include "reader.h"
#include "types.h"

// extern AST::malValuePtr readStr(const std::string& input);

auto EVAL(const malValuePtr ast, malEnvPtr env) -> malValuePtr {
  return ast->eval(env);
}

auto READ(const std::string& param) -> malValuePtr {
  return Parser::readStr(param);
}

auto prStr(malValuePtr ptr) -> std::string { return ptr->print(); }

auto rep(const std::string& param, malEnvPtr env) -> std::string {
  return prStr(EVAL(READ(param), env));
}

static AST::BuiltInFn::ApplyFn builtIn_add, builtIn_sub, builtIn_mul,
    builtIn_div;

auto main() -> int {
  malEnv globalEnv;
  malEnvPtr globalEnvPtr(&globalEnv);
  globalEnv.set("+", AST::builtin("+", &builtIn_add));
  globalEnv.set("-", AST::builtin("-", &builtIn_sub));
  globalEnv.set("*", AST::builtin("*", &builtIn_mul));
  globalEnv.set("/", AST::builtin("/", &builtIn_div));

  while (true) {
    std::print("user> ");
    std::string inp;
    if (!std::getline(std::cin, inp)) {  // EOF /ctrl-d
      break;
    }
    std::string res;
    try {
      res = rep(inp, globalEnvPtr);
    } catch (std::runtime_error err) {
      res = err.what();
    } catch (std::string& s) {
      res = s;
    };
    std::print("{}\n", res);
  }
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
