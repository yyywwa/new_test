#pragma once
#include <iostream>

namespace mtd {

template <typename... Args> void print(const Args... args) {
  (std::cout << ... << args);
}

template <typename... Args> void println(const Args... args) {
  (std::cout << ... << args);
  std::cout << '\n';
}

} // namespace mtd