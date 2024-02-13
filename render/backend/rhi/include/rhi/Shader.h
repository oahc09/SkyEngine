//
// Created by Zach Lee on 2022/11/10.
//

#pragma once

#include <rhi/Core.h>
#include <vector>

namespace sky::rhi {

    class Shader {
    public:
        Shader() = default;
        virtual ~Shader() = default;

        struct Descriptor {
            ShaderStageFlagBit stage;
            const uint8_t *data = nullptr;
            uint32_t size = 0;
        };

        ShaderStageFlagBit GetStage() const { return stage; }

    protected:
        ShaderStageFlagBit stage;
    };
    using ShaderPtr = std::shared_ptr<Shader>;
}