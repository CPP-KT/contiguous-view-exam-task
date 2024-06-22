#pragma once

#include <string>
#include <utility>

struct AssertionError : std::exception {
  explicit AssertionError(std::string error);

  std::string error_;
};

void runtime_assert(bool condition, std::string message);
