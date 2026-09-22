//NOTE: this is prolly not needed could delte this file
#include <print>
#include "mal.h"
#include "types.h"

void print(const malValuePtr node) {
  std::println("{}",node->print());
}
