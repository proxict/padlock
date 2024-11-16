#ifndef PADLOCK_EXCEPTION_HPP
#define PADLOCK_EXCEPTION_HPP

#include "padlock/utils/stringUtils.hpp"

#include <stdexcept>

namespace padlock {

class Exception : public std::runtime_error {
public:
    template <typename... TArgs>
    explicit Exception(const std::string& message, TArgs&&... args)
        : std::runtime_error(utils::concatenate(message, std::forward<TArgs>(args)...)) {}

    virtual ~Exception() noexcept = default;

    Exception(Exception&&) noexcept = default;

    Exception(const Exception&) = delete;
    Exception& operator=(const Exception&) = delete;
    Exception& operator=(Exception&&) = delete;
};

} // namespace padlock

#endif // PADLOCK_EXCEPTION_HPP
