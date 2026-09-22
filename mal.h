#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "RefCountedPtr.h"
#include "debug.h"

namespace AST {
class malValue;
}
using malValuePtr = RefCountedPtr<AST::malValue>;
using malValueVec = std::vector<malValuePtr>;
using malValueIter = malValueVec::iterator;
using malValueConstIter = malValueVec::const_iterator;
using malValueMap = std::unordered_map<std::string, malValuePtr>;

class malEnv;
typedef RefCountedPtr<malEnv> malEnvPtr;
