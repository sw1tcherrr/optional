#pragma once

#include <type_traits>
#include <utility>
#include "enablers.h"

struct in_place_t {};
inline constexpr in_place_t in_place {};

struct nullopt_t {};
inline constexpr nullopt_t nullopt {};

namespace detail {
struct dummy_t {};

template<typename T, bool trivial>
struct storage_base {
  constexpr storage_base() noexcept : dummy() {}

  constexpr storage_base(T value_) : active{true}, value(std::move(value_)) {}

  template <typename... Args>
  explicit constexpr storage_base(in_place_t, Args&&... args)
  : storage_base(T(std::forward<Args>(args)...)) {}

  // implicit default copy
  // user-defined destructor => no implicit default move, so would be deleted in derived
  constexpr storage_base(storage_base&& other)  = default;
  constexpr storage_base& operator=(storage_base&& other) = default;

  void reset() {
    if (active) {
      value.~T();
      active = false;
    }
  }

  ~storage_base() {
    reset();
  }

protected:
  bool active{false};
  union {
    dummy_t dummy;
    T value;
  };
};

template<typename T>
struct storage_base<T, true> {
  constexpr storage_base() : dummy() {}

  constexpr storage_base(T value_) : active{true}, value(std::move(value_)) {}

  template <typename... Args>
  explicit constexpr storage_base(in_place_t, Args&&... args)
      : storage_base(T(std::forward<Args>(args)...)) {}

  // implicit default copy
  // implicit default destructor => implicit default move

  constexpr void reset() {
    if (active) {
      value.~T();
      active = false;
    }
  }

protected:
  bool active{false};
  union {
    dummy_t dummy;
    T value;
  };
};

template<typename T, bool trivial>
struct copy_base : storage_base<T, std::is_trivially_destructible_v<T>> {
  using base = storage_base<T, std::is_trivially_destructible_v<T>>;
  using base::base;

  constexpr copy_base(copy_base const& other) {
    this->active = other.active;
    if (other.active) {
      new(&this->value) T(other.value);
    }
  }
  
  constexpr copy_base& operator=(copy_base const& other) {
    if (this != &other) {
      if (other.active) {
        if (this->active) {
          this->value = other.value;
        } else {
          new (&this->value) T(other.value);
          this->active = true;
        }
      } else {
        this->reset();
      }
    }
    return *this;
  }

  constexpr copy_base(copy_base&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
    this->active = other.active;
    if (other.active) {
      new(&this->value) T(std::move(other.value));
    }
  }

  constexpr copy_base& operator=(copy_base&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
    if (this != &other) {
      if (other.active) {
        if (this->active) {
          this->value = std::move(other.value);
        } else {
          new (&this->value) T(std::move(other.value));
          this->active = true;
        }
      } else {
        this->reset();
      }
    }
    return *this;
  }
};

template<typename T>
struct copy_base<T, true> : storage_base<T, std::is_trivially_destructible_v<T>> {
  using base = storage_base<T, std::is_trivially_destructible_v<T>>;
  using base::base;
  // inherited default copy and move
};
} // namespace detail

template <typename T>
class optional
    : detail::enable_copy<std::is_copy_constructible_v<T>>,
      detail::enable_move<std::is_move_constructible_v<T>>,
      detail::enable_copy_assign<std::is_copy_assignable_v<T>>,
      detail::enable_move_assign<std::is_move_assignable_v<T>>,
      public detail::copy_base<T, std::is_trivially_copyable_v<T>>
{
  using base = detail::copy_base<T, std::is_trivially_copyable_v<T>>;
  using base::base;
public:
  constexpr optional(nullopt_t) noexcept : optional() {}

  optional& operator=(nullopt_t) noexcept {
    this->reset();
    return *this;
  }

  constexpr explicit operator bool() const noexcept {
    return this->active;
  }

  constexpr T& operator*() noexcept {
    return this->value;
  }
  constexpr T const& operator*() const noexcept {
    return this->value;
  }

  constexpr T* operator->() noexcept {
    return &this->value;
  }
  constexpr T const* operator->() const noexcept {
    return &this->value;
  }

  template <typename... Args>
  void emplace(Args&&... args) {
    this->reset();
    new (&this->value) T(std::forward<Args>(args)...);
    this->active = true;
  }
};

template <typename T>
constexpr bool operator==(optional<T> const& a, optional<T> const& b) {
  if (a && b) {
    return *a == *b;
  } else {
    return !a && !b;
  }
}

template <typename T>
constexpr bool operator!=(optional<T> const& a, optional<T> const& b) {
  return !(a == b);
}

template <typename T>
constexpr bool operator<(optional<T> const& a, optional<T> const& b) {
  if (a && b) {
    return *a < *b;
  } else {
    return !a && b;
  }
}

template <typename T>
constexpr bool operator<=(optional<T> const& a, optional<T> const& b) {
  return !(b < a);
}

template <typename T>
constexpr bool operator>(optional<T> const& a, optional<T> const& b) {
  return b < a;
}

template <typename T>
constexpr bool operator>=(optional<T> const& a, optional<T> const& b) {
  return !(a < b);
}
