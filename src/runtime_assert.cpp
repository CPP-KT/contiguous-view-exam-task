#include "runtime_assert.h"

AssertionError::AssertionError(std::string error)
    : error_(std::move(error)) {}

void runtime_assert(bool condition, std::string message) {
  if (!condition) {
    throw AssertionError(std::move(message));
  }
}
