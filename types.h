#pragma once

#include <algorithm>
#include <cstdint>
#include <exception>
#include <format>
#include <print>
#include <string>
#include <unordered_map>
#include <vector>

#include "debug.h"
#include "mal.h"
#include "string.h"

namespace AST {
class EmptyInpExeption : public std::exception {};

class malValue : public RefCounted {
 public:
  malValue() { TRACE_OBJECT("Creating MalValue %p", this); }
  malValue(malValuePtr meta) : m_meta(meta) {
    TRACE_OBJECT("Creating MalValue %p\n", this);
  }
  virtual ~malValue() { TRACE_OBJECT("Destroying MalValue %p\n", this); }
  virtual std::string print(bool readonly = true) const = 0;
  virtual malValuePtr eval(malEnvPtr env) {
    return malValuePtr(this);  // default case
  }
  bool isTrue();
  virtual bool isEqualTo(const malValue* rhs) const { return this == rhs; }

 private:
  malValuePtr m_meta;
};

class Constant : public malValue {
 public:
  Constant(std::string val) : val(val) {}
  Constant(const Constant& that, malValuePtr meta)
      : malValue(meta), val(that.val) {}

  std::string print(bool readonly) const { return val; }
  bool isEqualTo(const malValue* rhs) const override {
    if (auto o = dynamic_cast<const Constant*>(rhs)) return val == o->val;
    return false;
  }

  const std::string val;
};
class Integer : public malValue {
 public:
  Integer(int64_t val) : val(val) {}
  Integer(const Integer& val, malValuePtr ptr) : val(val.val), malValue(ptr) {}
  int64_t value() const { return val; }
  std::string print(bool readonly = true) const { return std::to_string(val); }
  bool isEqualTo(const malValue* rhs) const override {
    if (auto o = dynamic_cast<const Integer*>(rhs)) return val == o->val;
    return false;
  }

 private:
  int64_t val;
};

// TODO: makebe use custom StringRef class or string_view like clang does to
// avoid unecessary string copying
class String : public malValue {
 public:
  String(const std::string& str) : val(str) {}
  String(std::string&& str) : val(std::move(str)) {}
  String(const String& str, malValuePtr ptr) : val(str.val), malValue(ptr) {}

  std::string value() const { return val; }
  std::string print(bool readonly = true) const {
    if (readonly) {
      return escape(val);
    }
    return val;
  }
  bool isEqualTo(const malValue* rhs) const override {
    if (auto o = dynamic_cast<const String*>(rhs)) return val == o->val;
    return false;
  }

 private:
  std::string val;
};

class Symbol : public malValue {
 public:
  Symbol(const std::string& str) : val(str) {}
  Symbol(std::string&& str) : val(std::move(str)) {}
  Symbol(const Symbol& str, malValuePtr ptr) : val(str.val), malValue(ptr) {}

  std::string value() const { return val; }
  const char* cValue() const { return val.c_str(); }
  std::string print(bool readonly = true) const { return val; }
  bool isEqualTo(const malValue* rhs) const override {
    if (auto o = dynamic_cast<const Symbol*>(rhs)) return val == o->val;
    return false;
  }

  virtual malValuePtr eval(malEnvPtr env);

 private:
  std::string val;
};

class Keywords : public malValue {
 public:
  Keywords(const std::string& str) : val(str) {}
  Keywords(std::string&& str) : val(std::move(str)) {}
  Keywords(const Keywords& str, malValuePtr ptr)
      : val(str.val), malValue(ptr) {}

  std::string value() const { return val; }

  std::string print(bool readonly = true) const { return val; }
  bool isEqualTo(const malValue* rhs) const override {
    if (auto o = dynamic_cast<const Keywords*>(rhs)) return val == o->val;
    return false;
  }

 private:
  std::string val;
};

class Sequence : public malValue {
 public:
  Sequence(malValueVec&& vec) : m_vec(std::move(vec)) {}
  malValueVec evalItems(malEnvPtr env) const;
  malValueConstIter begin() const { return m_vec.begin(); }
  malValueIter begin() { return m_vec.begin(); }
  malValuePtr item(int index) const { return (m_vec)[index]; }
  malValueConstIter end() const { return m_vec.end(); }
  malValueIter end() { return m_vec.end(); }
  size_t size() const { return m_vec.size(); }
  bool empty() const { return m_vec.empty(); }
  bool isEqualTo(const malValue* rhs) const override;

  std::string printSeq(char start, char end, bool readonly = true) const {
    std::string s;
    s += start;
    bool first = true;
    for (auto& v : m_vec) {
      if (!first) {
        s += " ";
      }
      first = false;
      s += v->print(readonly);
    }
    s += end;
    return s;
  }

  // removing private cause it annoying and im not making a lib
  malValueVec m_vec;
};

// TODO: may be consider using a linked list here like it use done in lisps
class List : public Sequence {
 public:
  List(malValueVec&& vec) : Sequence(std::move(vec)) {};

  // // NOTE: not sure if needed as it does copy may be more efficent way there
  // List(const List& list, malValuePtr ptr) : vec(list.vec), malValue(ptr) {}
  // virtual ~List() = default;

  // std::string print(bool readonly = true) const {
  //   std::string s = "(";
  //   for (auto& v : vec) {
  //     s += v->print(readonly);
  //     s += " ";
  //   }
  //   // s += ")";
  //   s[s.size()-1]=')';
  //   return s;
  // }
  std::string print(bool readonly = true) const { return printSeq('(', ')', readonly); }
  virtual malValuePtr eval(malEnvPtr env);
};

class Vector : public Sequence {
 public:
  Vector(malValueVec&& vec) : Sequence(std::move(vec)) {};

  // NOTE: not sure if needed as it does copy may be more efficent way there
  // Vector(const Vector& list, malValuePtr ptr) : vec(list.vec), malValue(ptr)
  // {}

  // virtual ~Vector() = default;

  // std::string print(bool readonly = true) const {
  //   std::string s = "[";
  //   for (auto& v : vec) {
  //     s += v->print(readonly);
  //     s += " ";
  //   }
  //   s += "]";
  //   return s;
  // }
  std::string print(bool readonly = true) const { return printSeq('[', ']', readonly); }

  virtual malValuePtr eval(malEnvPtr env);
};

class HashMap : public malValue {
 public:
  HashMap(malValueMap&& map) : map(std::move(map)) {}

  // TODO: lots of str concat rn prolly awful lot of allocations fix later may
  // be
  std::string print(bool readonly = true) const {
    std::string s = "{ ";

    for (auto& [k, v] : map) {
      s += k + " : ";
      s += v->print(readonly);
      s += ",";
    }
    s += " }";
    return s;
  }

  virtual malValuePtr eval(malEnvPtr env);
  bool isEqualTo(const malValue* rhs) const override;

 private:
  bool isEvalueated = false;
  malValueMap map;
};

class Applicable : public malValue {
 public:
  Applicable() = default;
  // Applicable(malValuePtr meta) : malValue(meta) {}
  virtual malValuePtr apply(malValueIter argBegin,
                            malValueIter argEnd) const = 0;
};

class BuiltInFn : public Applicable {
 public:
  using ApplyFn = malValuePtr(const std::string& name, malValueIter argsBegin,
                              malValueIter argsEnd);
  BuiltInFn(const std::string& name, ApplyFn* fn)
      : m_name(name), m_handler(fn) {}

  virtual malValuePtr apply(malValueIter argBegin, malValueIter argEnd) const;

  virtual std::string print(bool readonly = true) const {
    return std::format("builtin function - {}", m_name);
  }

 private:
  const std::string m_name;
  // NOTE: use std::function instead of fn pointer if i want to put lambdas to
  ApplyFn* m_handler;
};

class Lambda : public Applicable {
 public:
  Lambda(std::vector<std::string>&& params, malValuePtr body, malEnvPtr env);
  virtual malValuePtr apply(malValueIter argBegin, malValueIter argEnd) const;
  virtual std::string print(bool readonly = true) const {
    return "lambda function";
  }

 private:
  const std::vector<std::string> m_params;
  const malValuePtr m_body;
  const malEnvPtr m_env;
};

// API for createing objects
malValuePtr atom(malValuePtr value);
malValuePtr boolean(bool value);
malValuePtr falseValue();
// malValuePtr hash(AST::malValueIter argsBegin, AST::malValueIter argsEnd,
//                     bool isEvaluated);
// malValuePtr hash(const malHash::Map& map);
malValuePtr integer(int64_t value);
malValuePtr integer(const std::string& token);
malValuePtr keyword(const std::string& token);
// malValuePtr lambda(const StringVec&, malValuePtr, malEnvPtr);
malValuePtr list(malValueVec&& items);
// malValuePtr list(malValueIter begin, malValueIter end);
// malValuePtr list(malValuePtr a);
malValuePtr list(malValuePtr a, malValuePtr b);
malValuePtr list(malValuePtr a, malValuePtr b, malValuePtr c);
// malValuePtr macro(const malLambda& lambda);
malValuePtr nilValue();
malValuePtr string(const std::string& token);
malValuePtr symbol(const std::string& token);
malValuePtr trueValue();
malValuePtr vector(malValueVec&& items);
malValuePtr builtin(const std::string& name, BuiltInFn::ApplyFn* handler);

malValuePtr hash(malValueMap&& map);
malValuePtr lambda(std::vector<std::string>&& params, malValuePtr body,
                   malEnvPtr env);
// AST::malValuePtr vector(malValueIter begin, malValueIter end);
};  // namespace AST
