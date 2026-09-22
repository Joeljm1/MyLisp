#pragma once

#include "mal.h"

class malEnv : public RefCounted {
 public:
  malEnv(malEnvPtr outer = nullptr);
  malEnv(malEnvPtr outer, const std::vector<std::string>& bindings,
         malValueIter argsBegin, malValueIter argsEnd);
  ~malEnv();

  malValuePtr get(const std::string& symbol);
  malEnvPtr find(const std::string& symbol);
  malValuePtr set(const std::string& symbol, malValuePtr value);
  malEnvPtr getRoot();
  std::vector<std::string> getKeys() const;

 private:
  std::unordered_map<std::string, malValuePtr> m_map;
  malEnvPtr m_outer;
};

