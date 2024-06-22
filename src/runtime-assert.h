#pragma once

#include <string>
#include <utility>

struct assertion_error : std::exception {
  explicit assertion_error(std::string error);

  std::string error_;
};

void runtime_assert(bool condition, std::string message);
