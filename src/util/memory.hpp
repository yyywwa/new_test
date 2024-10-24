#pragma once
#include "function.hpp"
#include <atomic>

namespace mtd {

template <typename T> class SharedPtr {
public:
  class Deleter {
  public:
    void operator()(T *ptr) { delete ptr; }
  };
  using Dt = Function<void(T *)>;
  // 默认构造函数
  SharedPtr() : ptr_(nullptr), ref_count_(nullptr), deleter_(Deleter()) {}

  // 带参构造函数
  explicit SharedPtr(T *ptr, Dt deleter = Deleter())
      : ptr_(ptr), ref_count_(new std::atomic<int>(1)), deleter_(deleter) {}

  // 拷贝构造函数
  SharedPtr(const SharedPtr<T> &other) {
    ptr_ = other.ptr_;
    ref_count_ = other.ref_count_;
    deleter_ = other.deleter_;
    if (ref_count_) {
      ref_count_->fetch_add(1, std::memory_order_relaxed); // 引用计数增加
    }
  }

  // 移动构造函数
  SharedPtr(SharedPtr<T> &&other) noexcept {
    ptr_ = other.ptr_;
    ref_count_ = other.ref_count_;
    deleter_ = std::move(other.deleter_);
    other.ptr_ = nullptr;
    other.ref_count_ = nullptr;
  }

  // 拷贝赋值运算符
  SharedPtr<T> &operator=(const SharedPtr<T> &other) {
    if (this != &other) {
      release(); // 释放当前指针和计数

      ptr_ = other.ptr_;
      ref_count_ = other.ref_count_;
      deleter_ = other.deleter_;
      if (ref_count_) {
        ref_count_->fetch_add(1, std::memory_order_relaxed); // 引用计数增加
      }
    }
    return *this;
  }

  // 移动赋值运算符
  SharedPtr<T> &operator=(SharedPtr<T> &&other) noexcept {
    if (this != &other) {
      release(); // 释放当前指针和计数

      ptr_ = other.ptr_;
      ref_count_ = other.ref_count_;
      deleter_ = std::move(other.deleter_);
      other.ptr_ = nullptr;
      other.ref_count_ = nullptr;
    }
    return *this;
  }

  void swap(SharedPtr<T> &other) noexcept {
    std::swap(ptr_, other.ptr_);
    std::swap(ref_count_, other.ref_count_);
    std::swap(deleter_, other.deleter_);
  }

  bool operator==(const SharedPtr<T> &other) const {
    return ptr_ == other.ptr_;
  }

  // 添加 != 运算符
  bool operator!=(const SharedPtr<T> &other) const {
    return ptr_ != other.ptr_;
  }

  bool operator==(const T *other) const { return ptr_ == other; }

  // 添加 != 运算符
  bool operator!=(const T *other) const { return ptr_ != other; }

  operator bool() const { return ptr_ != nullptr; }

  void set_deleter(Dt deleter) { deleter_ = deleter; }

  // 获取对象
  T *get() const { return ptr_; }

  // 重载 * 运算符
  T &operator*() const { return *ptr_; }

  // 重载 -> 运算符
  T *operator->() const { return ptr_; }

  // 获取引用计数
  int use_count() const {
    return ref_count_ ? ref_count_->load(std::memory_order_relaxed) : 0;
  }

  // 释放资源
  void reset(T *ptr = nullptr, Dt deleter = Deleter()) {
    release(); // 释放当前资源
    if (ptr) {
      ptr_ = ptr;
      ref_count_ = new std::atomic<int>(1); // 新指针，引用计数重置为 1
      deleter_ = deleter;
    } else {
      ptr_ = nullptr;
      ref_count_ = nullptr;
    }
  }

  // 析构函数
  ~SharedPtr() { release(); }

private:
  T *ptr_;                      // 所管理的指针
  std::atomic<int> *ref_count_; // 引用计数
  Dt deleter_;                  // 自定义删除器

  // 释放当前资源
  void release() {
    if (ref_count_) {
      if (ref_count_->fetch_sub(1, std::memory_order_acq_rel) == 1) {
        deleter_(ptr_); // 当引用计数为 0 时，使用自定义删除器释放对象
        delete ref_count_; // 删除引用计数
      }
    }
  }
};

template <typename T, typename... Args>
SharedPtr<T> make_shared(Args &&...args) {
  // 使用new在堆上分配对象并完美转发构造函数参数
  T *ptr = new T(std::forward<Args>(args)...);
  return SharedPtr<T>(ptr);
}

template <typename T> class UniquePtr {
public:
  using Dt = Function<void(T *)>;

  explicit UniquePtr(T *ptr = nullptr, Dt deleter = DefaultDeleter())
      : ptr_(ptr), deleter_(deleter) {}

  // 禁用拷贝构造和拷贝赋值
  UniquePtr(const UniquePtr &) = delete;
  UniquePtr &operator=(const UniquePtr &) = delete;

  // 移动构造
  UniquePtr(UniquePtr &&other) noexcept
      : ptr_(other.ptr_), deleter_(std::move(other.deleter_)) {
    other.ptr_ = nullptr;
  }

  // 移动赋值
  UniquePtr &operator=(UniquePtr &&other) noexcept {
    if (this != &other) {
      reset();
      ptr_ = other.ptr_;
      deleter_ = std::move(other.deleter_);
      other.ptr_ = nullptr;
    }
    return *this;
  }

  ~UniquePtr() { deleter_(ptr_); }

  bool operator==(const UniquePtr<T> &other) const {
    return ptr_ == other.ptr_;
  }

  // 添加 != 运算符
  bool operator!=(const UniquePtr<T> &other) const {
    return ptr_ != other.ptr_;
  }

  bool operator==(const T *other) const { return ptr_ == other; }

  // 添加 != 运算符
  bool operator!=(const T *other) const { return ptr_ != other; }

  void swap(UniquePtr<T> &other) noexcept {
    std::swap(ptr_, other.ptr_);
    std::swap(deleter_, other.deleter_);
  }

  operator bool() const { return ptr_ != nullptr; }

  T &operator*() const { return *ptr_; }
  T *operator->() const { return ptr_; }
  T *get() const { return ptr_; }

  // 设置自定义删除器
  void set_deleter(Dt deleter) { deleter_ = deleter; }

  // 重置指针并应用自定义删除器
  void reset(T *ptr = nullptr, Dt deleter = DefaultDeleter()) {
    deleter_(ptr_);
    ptr_ = ptr;
    deleter_ = deleter;
  }

private:
  T *ptr_;
  Dt deleter_;

  struct DefaultDeleter {
    void operator()(T *ptr) { delete ptr; }
  };
};

// make_unique 函数
template <typename T, typename... Args>
UniquePtr<T> make_unique(Args &&...args) {
  return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

template <typename T> class WeakPtr {
public:
  WeakPtr() : ptr_(nullptr), ref_count_(nullptr) {}

  WeakPtr(const SharedPtr<T> &sharedPtr)
      : ptr_(sharedPtr.get()), ref_count_(sharedPtr.ref_count_) {}

  WeakPtr(const WeakPtr &other)
      : ptr_(other.ptr_), ref_count_(other.ref_count_) {}

  WeakPtr(WeakPtr &&other) noexcept
      : ptr_(other.ptr_), ref_count_(other.ref_count_) {
    other.ptr_ = nullptr;
    other.ref_count_ = nullptr;
  }

  WeakPtr &operator=(const WeakPtr &other) {
    if (this != &other) {
      ptr_ = other.ptr_;
      ref_count_ = other.ref_count_;
    }
    return *this;
  }

  WeakPtr &operator=(WeakPtr &&other) noexcept {
    if (this != &other) {
      ptr_ = other.ptr_;
      ref_count_ = other.ref_count_;
      other.ptr_ = nullptr;
      other.ref_count_ = nullptr;
    }
    return *this;
  }

  // 获取 SharedPtr
  SharedPtr<T> lock() const {
    if (ref_count_ && ref_count_->load(std::memory_order_relaxed) > 0) {
      return SharedPtr<T>(*this);
    }
    return SharedPtr<T>();
  }

  void swap(WeakPtr<T> &other) noexcept {
    std::swap(ptr_, other.ptr_);
    std::swap(ref_count_, other.ref_count_);
  }

  bool operator==(const WeakPtr<T> &other) const { return ptr_ == other.ptr_; }

  // 添加 != 运算符
  bool operator!=(const WeakPtr<T> &other) const { return ptr_ != other.ptr_; }

  bool operator==(const T *other) const { return ptr_ == other; }

  // 添加 != 运算符
  bool operator!=(const T *other) const { return ptr_ != other; }

  // 检查是否过期
  bool expired() const {
    return !ref_count_ || ref_count_->load(std::memory_order_relaxed) == 0;
  }

  operator bool() const { return !expired(); }

private:
  T *ptr_;
  std::atomic<int> *ref_count_;
};

// make_weak 函数
template <typename T> WeakPtr<T> make_weak(const SharedPtr<T> &sharedPtr) {
  return WeakPtr<T>(sharedPtr);
}

} // namespace mtd