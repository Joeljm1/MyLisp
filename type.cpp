#include <cassert>
#include <stdexcept>
#include <string>

#include "Environment.h"
#include "mal.h"
#include "types.h"

auto EVAL(const malValuePtr ast, malEnvPtr env) -> malValuePtr;

namespace AST {

malValuePtr BuiltInFn::apply(malValueIter argBegin, malValueIter argEnd) const {
  return m_handler(m_name, argBegin, argEnd);
}

// TODO:
malValuePtr Lambda::apply(malValueIter argBegin, malValueIter argEnd) const {
  return EVAL(m_body, malEnvPtr(new malEnv(m_env, m_params, argBegin, argEnd)));
}

malValuePtr Symbol::eval(malEnvPtr env) { return env->get(val); }

malValuePtr List::eval(malEnvPtr env) {
  if (empty()) {
    return malValuePtr(this);
  }
  malValueVec evaluated = evalItems(env);
  malValuePtr op = evaluated[0];

  if (Applicable* fn = dynamic_cast<Applicable*>(op.cptr()); fn != nullptr) {
    return fn->apply(evaluated.begin() + 1, evaluated.end());
  }
  throw std::runtime_error("first element of list should be a function");
}

malValuePtr Vector::eval(malEnvPtr env) {
  malValueVec v = evalItems(env);
  return vector(std::move(v));
}

// NOTE: currently using nrvo to return the vector
//  Not sure if i need to do a refcounted ptr instead

malValueVec Sequence::evalItems(malEnvPtr env) const {
  malValueVec items;
  items.reserve(size());

  for (auto it = begin(), last = end(); it != last; ++it) {
    items.push_back(EVAL(*it, env));
  }
  return items;
}

bool Sequence::isEqualTo(const malValue* rhs) const {
  if (auto o = dynamic_cast<const Sequence*>(rhs)) {
    if (size() != o->size()) {
      return false;
    }
    for (size_t i = 0; i < size(); ++i) {
      if (!m_vec[i]->isEqualTo(o->m_vec[i].cptr())) {
        return false;
      }
    }
    return true;
  }
  return false;
}

malValuePtr HashMap::eval(malEnvPtr env) {
  if (isEvalueated) {
    return this;
  }
  for (auto& [key, val] : map) {
    map[key] = EVAL(val, env);
  }
  isEvalueated = true;
  return this;
}

bool HashMap::isEqualTo(const malValue* rhs) const {
  if (auto o = dynamic_cast<const HashMap*>(rhs)) {
    if (map.size() != o->map.size()) {
      return false;
    }
    for (const auto& [k, v] : map) {
      auto it = o->map.find(k);
      if (it == o->map.end()) {
        return false;
      }
      if (!v->isEqualTo(it->second.cptr())) {
        return false;
      }
    }
    return true;
  }
  return false;
}

malValuePtr atom(malValuePtr value);

malValuePtr boolean(bool value) {
  if (value) {
    return trueValue();
  }
  return falseValue();
}

malValuePtr falseValue() {
  static malValuePtr val(new Constant("false"));
  return val;
}

// malValuePtr hash(AST::malValueIter argsBegin, AST::malValueIter argsEnd,
//                     bool isEvaluated);
malValuePtr hash(malValueMap&& map) {
  return malValuePtr(new HashMap(std::move(map)));
}

malValuePtr integer(int64_t value) { return malValuePtr(new Integer(value)); }

malValuePtr integer(const std::string& token) {
  return integer(std::stoi(token));
}

malValuePtr keyword(const std::string& token) {
  return malValuePtr(new Keywords(token));
}
// malValuePtr lambda(const StringVec&, malValuePtr, malEnvPtr);
malValuePtr list(malValueVec&& items) {
  return malValuePtr(new List(std::move(items)));
}
// malValuePtr list(malValueIter begin, malValueIter end);
// malValuePtr list(malValuePtr a);
malValuePtr list(malValuePtr a, malValuePtr b) {
  malValueVec v{a, b};
  return malValuePtr(new List(std::move(v)));
}
malValuePtr list(malValuePtr a, malValuePtr b, malValuePtr c) {
  malValueVec v{a, b, c};
  return malValuePtr(new List(std::move(v)));
}

malValuePtr vector(malValueVec&& items) {
  return malValuePtr(new Vector(std::move(items)));
}
// malValuePtr vector(malValueIter begin, malValueIter end);
// malValuePtr macro(const malLambda& lambda);
malValuePtr nilValue() {
  static malValuePtr val(new Constant("nil"));
  return val;
}

bool malValue::isTrue() {
  return this != falseValue().cptr() and this != nilValue().cptr();
}
malValuePtr string(const std::string& token) {
  return malValuePtr(new String(token));
}

malValuePtr symbol(const std::string& token) {
  return malValuePtr(new Symbol(token));
}
malValuePtr trueValue() {
  static malValuePtr val(new Constant("true"));
  return val;
}

malValuePtr builtin(const std::string& name, BuiltInFn::ApplyFn* handler) {
  return malValuePtr(new BuiltInFn(name, handler));
}

malValuePtr lambda(std::vector<std::string>&& params, malValuePtr body,
                   malEnvPtr env) {
  return malValuePtr(new Lambda(std::move(params), body, env));
}
Lambda::Lambda(std::vector<std::string>&& params, malValuePtr body,
               malEnvPtr env)
    : m_params(std::move(params)), m_body(body), m_env(env) {}

}  // namespace AST
