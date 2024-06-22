#pragma once

#include "runtime_assert.h"

#include <cstddef>

inline constexpr size_t dynamic_extent = -1;

template <typename T, size_t Extent = dynamic_extent>
class contiguous_view {
public:
  using value_type = void;
  using pointer = void;
  using const_pointer = void;
  using reference = void;
  using const_reference = void;

  using iterator = void;
  using const_iterator = void;

public:
  contiguous_view() noexcept;

  template <typename It>
  explicit(true) contiguous_view(It first, size_t count);

  template <typename It>
  explicit(true) contiguous_view(It first, It last);

  contiguous_view(const contiguous_view& other) noexcept;

  template <typename U, size_t N>
  explicit(true) contiguous_view(const contiguous_view<U, N>& other) noexcept;

  contiguous_view& operator=(const contiguous_view& other) noexcept;

  pointer data() const noexcept;

  size_t size() const noexcept;

  size_t size_bytes() const noexcept;

  size_t empty() const noexcept;

  iterator begin() const noexcept;

  const_iterator cbegin() const noexcept;

  iterator end() const noexcept;

  const_iterator cend() const noexcept;

  reference operator[](size_t idx) const;

  reference front() const;

  reference back() const;

  contiguous_view<T, dynamic_extent> subview(size_t offset, size_t count = dynamic_extent) const;

  template <size_t Offset, size_t Count = dynamic_extent>
  contiguous_view<T, 0> subview() const;

  template <size_t Count>
  contiguous_view<T, Count> first() const;

  contiguous_view<T, dynamic_extent> first(size_t count) const;

  template <size_t Count>
  contiguous_view<T, Count> last() const;

  contiguous_view<T, dynamic_extent> last(size_t count) const;

  contiguous_view</*const?*/ std::byte, 0> as_bytes() const;
};
