#pragma once

namespace detail {
template<bool enable>
struct enable_copy {};

template<>
struct enable_copy<false> {
  constexpr enable_copy() = default;
  constexpr enable_copy(enable_copy&&) = default;
  constexpr enable_copy& operator=(enable_copy const&) = default;
  constexpr enable_copy& operator=(enable_copy&&) = default;

  constexpr enable_copy(enable_copy const&) = delete;
};

template<bool enable>
struct enable_move {};

template<>
struct enable_move<false> {
  constexpr enable_move() = default;
  constexpr enable_move(enable_move const&) = default;
  constexpr enable_move& operator=(enable_move const&) = default;
  constexpr enable_move& operator=(enable_move&&) = default;

  constexpr enable_move(enable_move&&) = delete;
};

template<bool enable>
struct enable_copy_assign {};

template<>
struct enable_copy_assign<false> {
  constexpr enable_copy_assign() = default;
  constexpr enable_copy_assign(enable_copy_assign&&) = default;
  constexpr enable_copy_assign(enable_copy_assign const&) = default;
  constexpr enable_copy_assign& operator=(enable_copy_assign&&) = default;

  constexpr enable_copy_assign& operator=(enable_copy_assign const&) = delete;
};

template<bool enable>
struct enable_move_assign {};

template<>
struct enable_move_assign<false> {
  constexpr enable_move_assign() = default;
  constexpr enable_move_assign(enable_move_assign&&) = default;
  constexpr enable_move_assign(enable_move_assign const&) = default;
  constexpr enable_move_assign& operator=(enable_move_assign const&) = default;

  constexpr enable_move_assign& operator=(enable_move_assign&&) = delete;
};
} // namespace detail
