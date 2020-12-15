#pragma once

#include <hex.hpp>

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#ifdef __MINGW32__
#include <winsock.h>

#else
#include <arpa/inet.h>
#endif

#define TOKEN_CONCAT_IMPL(x, y) x ## y
#define TOKEN_CONCAT(x, y) TOKEN_CONCAT_IMPL(x, y)

namespace hex {

    template<typename ... Args>
    inline std::string format(const std::string &format, Args ... args) {
        ssize_t size = snprintf( nullptr, 0, format.c_str(), args ... );

        if( size <= 0 )
            return "";

        std::vector<char> buffer(size + 1, 0x00);
        snprintf(buffer.data(), size + 1, format.c_str(), args ...);

        return std::string(buffer.data(), buffer.data() + size);
    }

    [[nodiscard]] constexpr inline u64 extract(u8 from, u8 to, const std::unsigned_integral auto &value) {
        std::remove_cvref_t<decltype(value)> mask = (std::numeric_limits<std::remove_cvref_t<decltype(value)>>::max() >> (((sizeof(value) * 8) - 1) - (from - to))) << to;
        return (value & mask) >> to;
    }

    [[nodiscard]] constexpr inline u64 signExtend(u64 value, u8 currWidth, u8 targetWidth) {
        u64 mask = 1LLU << (currWidth - 1);
        return (((value ^ mask) - mask) << (64 - targetWidth)) >> (64 - targetWidth);
    }

    std::string toByteString(u64 bytes);
    std::string makePrintable(char c);

    template<typename T>
    struct always_false : std::false_type {};

    template<typename T>
    constexpr T changeEndianess(T value, std::endian endian) {
        if (endian == std::endian::native)
            return value;

        if constexpr (sizeof(T) == 1)
            return value;
        else if constexpr (sizeof(T) == 2)
            return __builtin_bswap16(value);
        else if constexpr (sizeof(T) == 4)
            return __builtin_bswap32(value);
        else if constexpr (sizeof(T) == 8)
            return __builtin_bswap64(value);
        else
            static_assert(always_false<T>::value, "Invalid type provided!");
    }

    template<typename T>
    constexpr T changeEndianess(T value, size_t size, std::endian endian) {
        if (endian == std::endian::native)
            return value;

        if (size == 1)
            return value;
        else if (size == 2)
            return __builtin_bswap16(value);
        else if (size == 4)
            return __builtin_bswap32(value);
        else if (size == 8)
            return __builtin_bswap64(value);
        else
            throw std::invalid_argument("Invalid value size!");
    }

    std::vector<u8> readFile(std::string_view path);

    #define SCOPE_EXIT(func) ScopeExit TOKEN_CONCAT(scopeGuard, __COUNTER__)([&] { func })
    class ScopeExit {
    public:
        ScopeExit(std::function<void()> func) : m_func(std::move(func)) {}
        ~ScopeExit() { if (!this->m_released) this->m_func(); }

        void release() {
            this->m_released = true;
        }

    private:
        bool m_released = false;
        std::function<void()> m_func;
    };

    struct Region {
        u64 address;
        size_t size;
    };

    struct Bookmark {
        Region region;

        std::vector<char> name;
        std::vector<char> comment;
    };
}