//
// Created by Zach Lee on 2022/1/9.
//

#include <vulkan/Shader.h>
#include <vulkan/Device.h>
#include <vulkan/Basic.h>
#include <core/logger/Logger.h>
#include <core/hash/Crc32.h>
static const char* TAG = "Driver";

namespace sky::drv {

    Shader::Shader(Device& dev) : DevObject(dev), shaderModule(VK_NULL_HANDLE), hash(0)
    {
    }

    Shader::~Shader()
    {

    }

    bool Shader::Init(const Descriptor& des)
    {
        VkShaderModuleCreateInfo shaderInfo = {};
        shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderInfo.codeSize = des.spv.size() * sizeof(uint32_t);
        shaderInfo.pCode = des.spv.data();

        auto rst = vkCreateShaderModule(device.GetNativeHandle(), &shaderInfo, VKL_ALLOC, &shaderModule);
        if (rst != VK_SUCCESS) {
            LOG_E(TAG, "create shader module failed %d", rst);
            return false;
        }
        hash = Crc32::Cal(reinterpret_cast<const uint8_t*>(shaderInfo.pCode), shaderInfo.codeSize);
        return true;
    }

    VkShaderModule Shader::GetNativeHandle() const
    {
        return shaderModule;
    }


}