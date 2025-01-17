/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License Version 2.0 with LLVM Exceptions
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *   https://llvm.org/LICENSE.txt
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <unifex/tag_invoke.hpp>
#include <unifex/type_traits.hpp>

#include <unifex/detail/prologue.hpp>

namespace unifex {

namespace _block {
enum class _enum : unsigned char {
  // Caller guarantees that the receiver will be called inline on the
  // current thread that called .start() before .start() returns.
  always_inline = 0,

  // Guarantees that the receiver will be called strongly-happens-before
  // .start() returns. Does not guarantee the call to the receiver happens
  // on the same thread that called .start(), however.
  always,

  // No guarantees about the timing and context on which the receiver will
  // be called.
  maybe,

  // Always completes asynchronously.
  // Guarantees that the receiver will not be called on the current thread
  // before .start() returns. The receiver may be called on another thread
  // before .start() returns, however, or may be called on the current thread
  // some time after .start() returns.
  never,
};

constexpr bool operator<(_enum lhs, _enum rhs) noexcept {
  return static_cast<unsigned char>(lhs) < static_cast<unsigned char>(rhs);
}

constexpr bool operator>(_enum lhs, _enum rhs) noexcept {
  return rhs < lhs;
}

constexpr bool operator<=(_enum lhs, _enum rhs) noexcept {
  return !(lhs > rhs);
}

constexpr bool operator>=(_enum lhs, _enum rhs) noexcept {
  return !(lhs < rhs);
}

struct blocking_kind {
  template <_enum Kind>
  using constant = std::integral_constant<_enum, Kind>;

  blocking_kind() = default;

  constexpr blocking_kind(_enum kind) noexcept
    : value(kind)
  {}

  template <_enum Kind>
  constexpr blocking_kind(constant<Kind>) noexcept
    : value(Kind)
  {}

  constexpr operator _enum() const noexcept {
    return value;
  }

  constexpr _enum operator()() const noexcept {
    return value;
  }

  static constexpr constant<_enum::maybe> maybe {};
  static constexpr constant<_enum::never> never {};
  static constexpr constant<_enum::always> always {};
  static constexpr constant<_enum::always_inline> always_inline {};

  _enum value{_enum::maybe};
};

template <typename Sender, typename = void>
struct _has_blocking : std::false_type {};

template <typename Sender>
struct _has_blocking<
    Sender,
    std::void_t<decltype(Sender::blocking)>>
  : std::true_type {};

struct _fn {
  template(typename Sender)
    (requires tag_invocable<_fn, const Sender&>)
  constexpr auto operator()(const Sender& s) const
      noexcept(is_nothrow_tag_invocable_v<_fn, const Sender&>)
      -> tag_invoke_result_t<_fn, const Sender&> {
    return tag_invoke(_fn{}, s);
  }
  template(typename Sender)
    (requires (!tag_invocable<_fn, const Sender&>))
  constexpr auto operator()(const Sender&) const noexcept {
    if constexpr (_has_blocking<Sender>::value) {
      return blocking_kind::constant<Sender::blocking>{};
    }
    else {
      return blocking_kind::maybe;
    }
  }
};

} // namespace _block

inline constexpr _block::_fn blocking {};
using _block::blocking_kind;

} // namespace unifex

#include <unifex/detail/epilogue.hpp>
