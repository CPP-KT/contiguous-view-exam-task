#include "contiguous-view.h"

#include "counting-iterator.h"
#include "test-utils.h"

#include <gtest/gtest.h>

#include <array>
#include <iterator>
#include <type_traits>
#include <utility>

namespace {

using tested_extents = ::testing::Types<std::false_type, std::true_type>;

class extent_name_generator {
public:
  template <typename IsStatic>
  static std::string GetName(int) {
    if (IsStatic::value) {
      return "static";
    } else {
      return "dynamic";
    }
  }
};

TYPED_TEST_SUITE(common_tests, tested_extents, extent_name_generator);

} // namespace

template class contiguous_view<element>;
template class contiguous_view<element, 3>;

TYPED_TEST(common_tests, two_iterators_ctor) {
  auto c = make_array(10, 20, 30);
  auto [first, last] = obfuscate_iterators(c.begin(), c.end());

  typename TestFixture::template view<element, 3> v(first, last);

  EXPECT_EQ(v.data(), c.data());
  EXPECT_EQ(v.size(), 3);
  EXPECT_EQ(v.size_bytes(), 3 * sizeof(element));
  EXPECT_FALSE(v.empty());

  expect_eq(v, {10, 20, 30});
}

TYPED_TEST(common_tests, two_iterators_ctor_empty) {
  auto c = make_array();
  auto [first, last] = obfuscate_iterators(c.begin(), c.end());
  typename TestFixture::template view<element, 0> v(first, last);

  EXPECT_EQ(v.data(), c.data());
  EXPECT_EQ(v.size(), 0);
  EXPECT_EQ(v.size_bytes(), 0);
  EXPECT_TRUE(v.empty());

  expect_eq(v, {});
}

TYPED_TEST(common_tests, iterator_and_count_ctor) {
  auto c = make_array(10, 20, 30);
  auto [first, last] = obfuscate_iterators(c.begin(), c.end());

  typename TestFixture::template view<element, 3> v(first, last);

  EXPECT_EQ(v.data(), c.data());
  EXPECT_EQ(v.size(), 3);
  EXPECT_EQ(v.size_bytes(), 3 * sizeof(element));
  EXPECT_FALSE(v.empty());

  expect_eq(v, {10, 20, 30});
}

TYPED_TEST(common_tests, iterator_and_count_ctor_empty) {
  auto c = make_array();
  auto [first, last] = obfuscate_iterators(c.begin(), c.end());

  typename TestFixture::template view<element, 0> v(first, 0);

  EXPECT_EQ(v.data(), c.data());
  EXPECT_EQ(v.size(), 0);
  EXPECT_EQ(v.size_bytes(), 0);
  EXPECT_TRUE(v.empty());

  expect_eq(v, {});
}

TYPED_TEST(common_tests, copy_ctor) {
  auto c = make_array(10, 20, 30);
  typename TestFixture::template view<element, 3> v(c.begin(), c.end());

  typename TestFixture::template view<element, 3> copy = std::as_const(v);

  EXPECT_EQ(v.data(), copy.data());
  EXPECT_EQ(v.size(), copy.size());

  expect_eq(copy, {10, 20, 30});
}

TYPED_TEST(common_tests, copy_assignment) {
  auto c1 = make_array(10, 20, 30);
  auto c2 = make_array(40, 50, 60);

  typename TestFixture::template view<element, 3> v1(c1.begin(), c1.end());
  typename TestFixture::template view<element, 3> v2(c2.begin(), c2.end());

  v2 = std::as_const(v1);

  EXPECT_EQ(v1.data(), v2.data());
  EXPECT_EQ(v1.size(), v2.size());

  expect_eq(v2, {10, 20, 30});
}

TYPED_TEST(common_tests, subscript) {
  auto c = make_array(10, 20, 30);
  const typename TestFixture::template view<element, 3> v(c.begin(), c.end());

  v[1].update_if_non_const(42);

  EXPECT_EQ(v[0], 10);
  EXPECT_EQ(v[1], 42);
  EXPECT_EQ(v[2], 30);

  EXPECT_EQ(&v[0], &c[0]);
  EXPECT_EQ(&v[1], &c[1]);
  EXPECT_EQ(&v[2], &c[2]);
}

TYPED_TEST(common_tests, subscript_const) {
  auto c = make_array(10, 20, 30);
  typename TestFixture::template view<const element, 3> v(c.begin(), c.end());

  v[1].update_if_non_const(42);

  EXPECT_EQ(v[0], 10);
  EXPECT_EQ(v[1], 20);
  EXPECT_EQ(v[2], 30);

  EXPECT_EQ(&v[0], &c[0]);
  EXPECT_EQ(&v[1], &c[1]);
  EXPECT_EQ(&v[2], &c[2]);
}

TYPED_TEST(common_tests, front_back) {
  auto c = make_array(10, 20, 30);
  const typename TestFixture::template view<element, 3> v(c.begin(), c.end());

  EXPECT_EQ(v.front(), 10);
  EXPECT_EQ(v.back(), 30);

  EXPECT_EQ(&v.front(), &c.front());
  EXPECT_EQ(&v.back(), &c.back());

  v.front().update_if_non_const(42);
  v.back().update_if_non_const(43);

  EXPECT_EQ(v.front(), 42);
  EXPECT_EQ(v.back(), 43);
}

TYPED_TEST(common_tests, front_back_const) {
  auto c = make_array(10, 20, 30);
  typename TestFixture::template view<const element, 3> v(c.begin(), c.end());

  EXPECT_EQ(v.front(), 10);
  EXPECT_EQ(v.back(), 30);

  EXPECT_EQ(&v.front(), &c.front());
  EXPECT_EQ(&v.back(), &c.back());

  v.front().update_if_non_const(42);
  v.back().update_if_non_const(43);

  EXPECT_EQ(v.front(), 10);
  EXPECT_EQ(v.back(), 30);
}

TYPED_TEST(common_tests, subview) {
  auto c = make_array(10, 20, 30, 40, 50);
  typename TestFixture::template view<element, 5> v(c.begin(), c.end());

  {
    contiguous_view<element, 3> static_slice = v.template subview<2, 3>();
    contiguous_view<element> dynamic_slice = v.subview(2, 3);

    expect_eq(static_slice, {30, 40, 50});
    expect_eq(dynamic_slice, {30, 40, 50});
  }

  {
    contiguous_view<element, 2> static_slice = v.template subview<1, 2>();
    contiguous_view<element> dynamic_slice = v.subview(1, 2);

    expect_eq(static_slice, {20, 30});
    expect_eq(dynamic_slice, {20, 30});
  }

  {
    contiguous_view<element, 0> static_slice = v.template subview<5, 0>();
    contiguous_view<element> dynamic_slice = v.subview(5, 0);

    expect_eq(static_slice, {});
    expect_eq(dynamic_slice, {});
  }
}

TYPED_TEST(common_tests, subview_dynamic_extent) {
  auto c = make_array(10, 20, 30, 40, 50);
  typename TestFixture::template view<element, 5> v(c.begin(), c.end());

  {
    typename TestFixture::template view<element, 5> static_slice = v.template subview<0, dynamic_extent>();
    contiguous_view<element> dynamic_slice = v.subview(0, dynamic_extent);

    expect_eq(static_slice, {10, 20, 30, 40, 50});
    expect_eq(dynamic_slice, {10, 20, 30, 40, 50});
  }

  {
    typename TestFixture::template view<element, 3> static_slice = v.template subview<2, dynamic_extent>();
    contiguous_view<element> dynamic_slice = v.subview(2, dynamic_extent);

    expect_eq(static_slice, {30, 40, 50});
    expect_eq(dynamic_slice, {30, 40, 50});
  }

  {
    typename TestFixture::template view<element, 0> static_slice = v.template subview<5, dynamic_extent>();
    contiguous_view<element> dynamic_slice = v.subview(5, dynamic_extent);

    expect_eq(static_slice, {});
    expect_eq(dynamic_slice, {});
  }
}

TYPED_TEST(common_tests, first) {
  auto c = make_array(10, 20, 30);
  typename TestFixture::template view<element, 3> v(c.begin(), c.end());

  contiguous_view<element, 2> static_slice = v.template first<2>();
  contiguous_view<element> dynamic_slice = v.first(2);

  expect_eq(static_slice, {10, 20});
  expect_eq(dynamic_slice, {10, 20});
}

TYPED_TEST(common_tests, last) {
  auto c = make_array(10, 20, 30);
  typename TestFixture::template view<element, 3> v(c.begin(), c.end());

  contiguous_view<element, 2> static_slice = v.template last<2>();
  contiguous_view<element> dynamic_slice = v.last(2);

  expect_eq(static_slice, {20, 30});
  expect_eq(dynamic_slice, {20, 30});
}

TYPED_TEST(common_tests, as_bytes) {
  if constexpr (std::endian::native != std::endian::little) {
    GTEST_SKIP();
  } else {
    auto ints = make_array<std::uint32_t>(0x11223344, 0xABABCDEF);
    auto bytes = make_array<std::byte>(0x44, 0x33, 0x22, 0x11, 0xEF, 0xCD, 0xAB, 0xAB);

    typename TestFixture::template view<std::uint32_t, 2> ints_view(ints.begin(), ints.end());
    typename TestFixture::template view<std::byte, 8> bytes_view(bytes.begin(), bytes.end());

    typename TestFixture::template view<std::byte, 8> as_bytes = std::as_const(ints_view).as_bytes();

    EXPECT_EQ(ints_view.size_bytes(), 8);
    EXPECT_EQ(as_bytes.size(), 8);
    EXPECT_EQ(as_bytes.size_bytes(), 8);

    expect_eq(as_bytes, bytes_view);

    as_bytes[3] = std::byte(0x80);
    as_bytes[4] = std::byte(0x42);

    expect_eq<std::uint32_t>(ints_view, {0x80223344, 0xABABCD42});
  }
}

TYPED_TEST(common_tests, as_bytes_const) {
  if constexpr (std::endian::native != std::endian::little) {
    GTEST_SKIP();
  } else {
    auto ints = make_array<std::uint32_t>(0x11223344, 0xABABCDEF);
    auto bytes = make_array<std::byte>(0x44, 0x33, 0x22, 0x11, 0xEF, 0xCD, 0xAB, 0xAB);

    typename TestFixture::template view<const std::uint32_t, 2> ints_view(ints.begin(), ints.end());
    typename TestFixture::template view<std::byte, 8> bytes_view(bytes.begin(), bytes.end());

    typename TestFixture::template view<const std::byte, 8> as_bytes = ints_view.as_bytes();

    EXPECT_EQ(ints_view.size_bytes(), 8);
    EXPECT_EQ(as_bytes.size(), 8);
    EXPECT_EQ(as_bytes.size_bytes(), 8);

    expect_eq(as_bytes, bytes_view);
  }
}

TYPED_TEST(common_tests, traits) {
  using writable_contiguous_view = typename TestFixture::template view<element, 3>;
  using const_contiguous_view = typename TestFixture::template view<const element, 3>;

  EXPECT_TRUE(std::is_trivially_copyable_v<writable_contiguous_view>);
  EXPECT_TRUE(std::is_trivially_copyable_v<const_contiguous_view>);

  EXPECT_TRUE((std::is_same_v<typename writable_contiguous_view::value_type, element>) );
  EXPECT_TRUE((std::is_same_v<typename const_contiguous_view::value_type, element>) );
}

TEST(dynamic_extent_tests, default_ctor) {
  contiguous_view<element> v;

  EXPECT_EQ(v.data(), nullptr);
  EXPECT_EQ(v.size(), 0);
  EXPECT_EQ(v.size_bytes(), 0);
  EXPECT_TRUE(v.empty());

  expect_eq(v, {});
}

TEST(dynamic_extent_tests, copy_assignment) {
  auto c1 = make_array(10, 20, 30);
  auto c2 = make_array(42);

  contiguous_view<element> v1(c1.begin(), c1.end());
  contiguous_view<element> v2(c2.begin(), c2.end());

  v2 = std::as_const(v1);

  EXPECT_EQ(v1.data(), v2.data());
  EXPECT_EQ(v1.size(), v2.size());

  expect_eq(v2, {10, 20, 30});
}

TEST(static_extent_tests, traits) {
  using contiguous_view = contiguous_view<element, 10>;

  EXPECT_GE(sizeof(contiguous_view::pointer), sizeof(contiguous_view));
}

TEST(conversion_tests, dynamic_add_const) {
  auto c = make_array(10, 20, 30);
  contiguous_view<element> v(c.begin(), c.end());

  contiguous_view<const element> cv = v;

  EXPECT_EQ(v.data(), cv.data());
  EXPECT_EQ(v.size(), cv.size());

  expect_eq(cv, {10, 20, 30});
}

TEST(conversion_tests, static_add_const) {
  auto c = make_array(10, 20, 30);
  contiguous_view<element, 3> v(c.begin(), c.end());

  contiguous_view<const element, 3> cv = v;

  EXPECT_EQ(v.data(), cv.data());
  EXPECT_EQ(v.size(), cv.size());

  expect_eq(cv, {10, 20, 30});
}

TEST(conversion_tests, dynamic_to_static) {
  auto c = make_array(10, 20, 30);
  contiguous_view<element> v1(c.begin(), c.end());

  contiguous_view<element, 3> v2(v1);

  EXPECT_EQ(v1.data(), v2.data());
  EXPECT_EQ(v1.size(), v2.size());

  expect_eq(v2, {10, 20, 30});
}

TEST(conversion_tests, static_to_dynamic) {
  auto c = make_array(10, 20, 30);
  contiguous_view<element, 3> v1(c.begin(), c.end());

  contiguous_view<element> v2 = v1;

  EXPECT_EQ(v1.data(), v2.data());
  EXPECT_EQ(v1.size(), v2.size());

  expect_eq(v2, {10, 20, 30});
}

TEST(conversion_tests, illegal) {
  EXPECT_FALSE((std::is_convertible_v<contiguous_view<element>, contiguous_view<element, 3>>) );
  EXPECT_FALSE((std::is_constructible_v<contiguous_view<element, 3>, contiguous_view<element, 2>>) );
}
