#include <iostream>
#include <print>
#include <string>

auto READ(const std::string& param) -> std::string { return param; }

auto EVAL(const std::string& param) -> std::string { return param; }

auto PRINT(const std::string& param) -> std::string { return param; }

auto rep(const std::string& param) -> std::string {
  const std::string read = READ(param);
  const std::string eval = EVAL(read);
  const std::string print = PRINT(eval);
  return print;
}

auto main() -> int {
  while (true) {
    std::print("user> ");
    std::string inp;
    if (!std::getline(std::cin, inp)) {  // EOF /ctrl-d
      break;
    }
    std::string res = rep(inp);
    std::print("{}\n", res);
  }
}
