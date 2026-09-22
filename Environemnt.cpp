#include <cassert>
#include <iterator>
#include <stdexcept>

#include "Environment.h"
#include "debug.h"
#include "mal.h"
#include "types.h"

malEnv::malEnv(malEnvPtr outer) : m_outer(outer) {
  TRACE_ENV("Creating malEnv %p, outer=%p\n", this, m_outer.ptr());
}

malEnv::malEnv(malEnvPtr outer, const std::vector<std::string>& bindings,
               malValueIter argsBegin, malValueIter argsEnd)
    : m_outer(outer) {
  TRACE_ENV("Creating malEnv %p, outer=%p\n", this, m_outer.ptr());
  auto currIt = argsBegin;
  for (size_t i = 0; i < bindings.size(); ++i) {
    if (bindings[i] == "&") {
      if (i + 1 < bindings.size()) {
        malValueVec rest(currIt, argsEnd);
        this->set(bindings[i + 1], AST::list(std::move(rest)));
      }
      return;
    }
    if (currIt == argsEnd) {
      throw std::runtime_error("bindings and args are not of same length");
    }
    this->set(bindings[i], *(currIt++));
  }
  if (currIt != argsEnd) {
    throw std::runtime_error("bindings and args are not of same length");
  }
}

malEnv::~malEnv() {
  TRACE_ENV("Destroying malEnv %p, outer=%p\n", this, m_outer.ptr());
}

malValuePtr malEnv::get(const std::string& symbol) {
  for (malEnvPtr env = this; env != nullptr; env = env->m_outer) {
    auto it = env->m_map.find(symbol);
    if (it != env->m_map.end()) {
      return it->second;
    }
  }
  throw std::runtime_error("'" + symbol + "' not found");
}

malEnvPtr malEnv::find(const std::string& symbol) {
  for (malEnvPtr env = this; env != nullptr; env = env->m_outer) {
    if (auto it = env->m_map.find(symbol); it != env->m_map.end()) {
      return env;
    }
  }
  return nullptr;
}

malValuePtr malEnv::set(const std::string& symbol, malValuePtr value) {
  m_map[symbol] = value;
  return value;
}
malEnvPtr malEnv::getRoot() {
  malEnvPtr env = this;
  while (env->m_outer != nullptr) {
    env = env->m_outer;
  }
  return env;
}

std::vector<std::string> malEnv::getKeys() const {
  std::vector<std::string> keys;
  for (const auto& [k, _] : m_map) {
    keys.push_back(k);
  }
  if (m_outer != nullptr) {
    auto outerKeys = m_outer->getKeys();
    keys.insert(keys.end(), outerKeys.begin(), outerKeys.end());
  }
  return keys;
}
