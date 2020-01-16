#ifndef PADLOCK_EXCEPTION_H_
#define PADLOCK_EXCEPTION_H_

#include <string>

namespace padlock {

class Exception final {
public:
    Exception(std::string msg)
        : mMsg(std::move(msg)) {}

    const char* what() const noexcept { return mMsg.c_str(); }

private:
    std::string mMsg;
};

} // namespace padlock

#endif // PADLOCK_EXCEPTION_H_
