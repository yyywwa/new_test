#pragma once
#include "memory.hpp"
#include <utility>

namespace mtd {
class Any {
public:
  Any() = default;

  template <typename T> Any(T &&t) {
    content_.reset(new Holder<T>(std::forward<T>(t)));
  }

  Any(const Any &other) {
    if (other.content_) {
      content_.reset(other.content_->clone());
    }
  }

  Any(Any &&other) noexcept : content_(std::move(other.content_)) {}

  Any &operator=(const Any &other) {
    if (this != &other) {
      Any temp(other);
      // std::swap(content_, temp.content_);
      content_.swap(temp.content_);
    }
    return *this;
  }

  Any &operator=(Any &&other) noexcept {
    if (this != &other) {
      content_ = std::move(other.content_);
    }
    return *this;
  }

  ~Any() = default;

  bool has_value() const { return content_ != nullptr; }

  template <typename T> T &cast() {
    if (auto holder = content_.get()) {
      if (auto derived = dynamic_cast<Holder<T> *>(holder)) {
        return derived->held;
      }
    }
    throw "wtfff";
  }

  template <typename T> const T &cast() const {
    return const_cast<Any *>(this)->cast<T>();
  }

private:
  struct Placeholder {
    virtual Placeholder *clone() const = 0;
    virtual ~Placeholder() = default;
  };

  template <typename T> struct Holder : public Placeholder {
    T held;

    Holder(T &&value) : held(std::move(value)) {}
    Holder(T &value) : held(value) {}

    Placeholder *clone() const override { return new Holder(T{}); }
  };

  UniquePtr<Placeholder> content_;
};
} // namespace mtd