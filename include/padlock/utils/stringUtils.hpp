#ifndef PADLOCK_UTILS_STRINGUTILS_HPP
#define PADLOCK_UTILS_STRINGUTILS_HPP

#include <sstream>
#include <string>

namespace padlock::utils {

template <typename T, typename... TArgs>
[[maybe_unused]] std::ostream& serialize(std::ostream& stream, T&& first, TArgs&&... args) {
    stream << std::forward<T>(first);
    ((stream << std::forward<TArgs>(args)), ...);
    return stream;
}

template <typename T, typename... TArgs>
[[nodiscard]] std::string concatenate(T&& first, TArgs&&... args) {
    std::stringstream ss;
    serialize(ss, std::forward<T>(first), std::forward<TArgs>(args)...);
    return ss.str();
}

} // namespace padlock

#endif // PADLOCK_UTILS_STRINGUTILS_HPP
