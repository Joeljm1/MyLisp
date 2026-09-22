#pragma once

#include <concepts>

// Base class
class RefCounted {
 public:
  RefCounted() = default;
  virtual ~RefCounted() = default;
  void acquire() const { refCount++; }
  int release() const {
    refCount--;
    return refCount;
  }
  int getRefCount() const { return refCount; }
  RefCounted(const RefCounted&) = delete;
  RefCounted& operator=(const RefCounted&) = delete;

 private:
  mutable int refCount = 0;
};

//RefCountedPtr requird the template param  to be base class of RefCounted 
template <typename T>
//TODO: find some way to put this concept for T8 being baseclass of RefCounted
// requires std::derived_from<T, RefCounted>
// this concept casues me to not use like a class with a refCount of that type
// not . I dont know how to do that.

class RefCountedPtr {

 public:
  RefCountedPtr() = default;

  RefCountedPtr(T* param) { acquire(param); }
  RefCountedPtr(const RefCountedPtr& rhs) { acquire(rhs.ptr); }
  RefCountedPtr& operator=(const RefCountedPtr& rhs) {
    acquire(rhs.ptr);
    return *this;
  }
  // RefCountedPtr& operator=(RefCountedPtr rhs) { may be this and a swap is more exception safe???

  operator bool () const { return ptr != nullptr; }
  bool operator==(const RefCountedPtr& rhs) const { return rhs.ptr == ptr; }
  bool operator!=(const RefCountedPtr& rhs) const { return rhs.ptr != ptr; }
  T* operator->() const { return ptr; }
  T* cptr() const { return ptr; }

  ~RefCountedPtr() { release(); }

 private:
  T* ptr = nullptr;
  void acquire(T* param) {
    if (param) {
      param->acquire();
    }
    release();
    ptr = param;
  }

  void release() {
    if (ptr != nullptr && (ptr->release() == 0)) {
      delete ptr;
    }
    ptr = nullptr;
  }
};
