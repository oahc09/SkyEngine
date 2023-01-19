//
// Created by Zach Lee on 2022/3/12.
//

#pragma once

#include <core/type/Any.h>

namespace sky {

    bool SetAny(Any &source, const std::string &str, const Any &any);
    bool SetAny(void *ptr, uint32_t id, const std::string &str, const Any &any);

    Any GetAny(Any &source, const std::string &str);
    Any GetAny(const Any &source, const std::string &str);

    Any GetAny(const void *ptr, uint32_t id, const std::string &str);

} // namespace sky