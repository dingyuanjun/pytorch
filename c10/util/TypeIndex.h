#pragma once

#include <c10/util/ConstexprCrc.h>
#include <c10/util/C++17.h>
#include <c10/util/IdWrapper.h>
#include <cinttypes>
#include <functional>

namespace c10 {
namespace util {

struct type_index final : IdWrapper<type_index, uint64_t> {
  constexpr type_index(uint64_t checksum) : IdWrapper(checksum) {}

  // Allow usage in std::map / std::set
  // TODO Disallow this and rather use std::unordered_map/set everywhere
  friend bool operator<(type_index lhs, type_index rhs) noexcept {
    return lhs.underlyingId() < rhs.underlyingId();
  }

  friend std::ostream& operator<<(std::ostream& stream, type_index typeId) {
    return stream << typeId.underlyingId();
  }
};

namespace detail {

template<typename T>
constexpr type_index type_index_impl() noexcept {
  // Idea: __PRETTY_FUNCTION__ (or __FUNCSIG__ on msvc) contains a qualified name
  // of this function, including its template parameter, i.e. including the
  // type we want an id for. We use this name and run crc64 on it to get a type id.
  // To enforce that this is really computed at compile time, we pass the crc
  // checksum through std::integral_constant.
  #if defined(_MSC_VER)
    return std::integral_constant<uint64_t,
                                  crc64(__FUNCSIG__, sizeof(__FUNCSIG__)).checksum()>::value;
  #else
    return std::integral_constant<uint64_t,
                                  crc64(__PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__)).checksum()>::value;
  #endif
}

} // namespace _detail

template<typename T>
constexpr type_index get_type_index() noexcept {
  return detail::type_index_impl<guts::remove_cv_t<guts::decay_t<T>>>();
}

}
}

C10_DEFINE_HASH_FOR_IDWRAPPER(c10::util::type_index);
