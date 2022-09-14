#pragma once

#include <Application/types.h>
#include <string>

namespace utils {

inline std::string strFromHex(const std::string &data) {
    std::string ans;
    u64 size = data.size() - 2;
    for (u64 i = 0; i < size; i += 2) {
        ans += static_cast<i8>(
                std::stoi(data.substr(i, 2), nullptr, 16));
    }
    return ans;
}

} // namespace utils
