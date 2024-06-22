#include "runtime-assert.h"

assertion_error::assertion_error(std::string error)
    : error_(std::move(error)) {}

void runtime_assert(bool condition, std::string message) {
  if (!condition) {
    throw assertion_error(std::move(message));
  }
}
