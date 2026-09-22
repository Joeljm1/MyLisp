#include <iostream>
#include <print>
#include <stdexcept>
#include <string>

#include "types.h"
#include "reader.h"

//extern AST::malValuePtr readStr(const std::string& input);

auto READ(const std::string& param) -> AST::malValuePtr {
  return Parser::readStr(param);
}

auto EVAL(const AST::malValuePtr param) -> AST::malValuePtr { return param; }

auto prStr(AST::malValuePtr ptr) -> std::string { return ptr->print(); }

auto rep(const std::string& param) -> std::string {
  return prStr(EVAL(READ(param)));
}

auto main() -> int {
  while (true) {
    std::print("user> ");
    std::string inp;
    if (!std::getline(std::cin, inp)) {  // EOF /ctrl-d
      break;
    }
    std::string res;
    try {
      res = rep(inp);
    } catch (std::runtime_error& err) {
      res = err.what();
    }
    catch (std::string& s) {
      res = s;
    };
    std::print("{}\n", res);
  }
}
