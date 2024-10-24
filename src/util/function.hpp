#pragma once

#include <utility>
namespace mtd {

template <typename Rt, typename... Args> class Function;

template <typename Rt, typename... Args> class Function<Rt(Args...)> {
  struct CallableBase {
    virtual ~CallableBase() = default;
    virtual Rt invoke(Args... args) = 0;
    virtual CallableBase *clone() const = 0;
  };

  template <typename Callable> struct CallableHolder : public CallableBase {
    Callable callable_;

    CallableHolder(Callable &&callable)
        : callable_(std::forward<Callable>(callable)) {}

    CallableHolder(const Callable &callable) // Add a const copy constructor
        : callable_(callable) {}

    Rt invoke(Args... args) override {
      return callable_(std::forward<Args>(args)...);
    }

    CallableBase *clone() const override {
      return new CallableHolder<Callable>{callable_};
    }
  };

public:
  // Default constructor
  Function() = default;

  ~Function() {
    if (callable_ != nullptr) {
      delete callable_;
    }
  }

  // Constructor accepting any callable object
  template <typename Callable>
  Function(Callable &&callable)
      : callable_(new CallableHolder<std::decay_t<Callable>>(
            std::forward<Callable>(callable))) {}

  // Copy constructor
  Function(const Function &other) {
    if (other.callable_) {
      callable_ = other.callable_->clone();
    }
  }

  // Move constructor
  Function(Function &&other) noexcept = default;

  // Copy assignment
  Function &operator=(const Function &other) {
    if (this != &other) {
      callable_ = other.callable_ ? other.callable_->clone() : nullptr;
    }
    return *this;
  }

  // Move assignment
  Function &operator=(Function &&other) noexcept = default;

  // Call operator
  Rt operator()(Args... args) {
    if (!callable_) {
      throw "wtf";
    }
    return callable_->invoke(std::forward<Args>(args)...);
  }

private:
  CallableBase *callable_;
};
} // namespace mtd