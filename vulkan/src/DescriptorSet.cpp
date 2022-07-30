//
// Created by Zach Lee on 2022/1/26.
//

#include <vulkan/DescriptorSet.h>
#include <vulkan/DescriptorSetPool.h>
#include <vulkan/Device.h>
#include <vulkan/CacheManager.h>

namespace sky::drv {

    DescriptorSet::~DescriptorSet()
    {
        if (pool) {
            pool->Free(*this);
        }
    }

    VkDescriptorSet DescriptorSet::GetNativeHandle() const
    {
        return handle;
    }

    DescriptorSet::Writer DescriptorSet::CreateWriter()
    {
        return Writer(*this);
    }

    DescriptorSetPtr DescriptorSet::Allocate(DescriptorSetPoolPtr pool, DescriptorSetLayoutPtr layout)
    {
        auto setCreateFn = [&pool, &layout](VkDescriptorSet set) {
            auto setPtr = std::make_shared<DescriptorSet>(pool->device);
            setPtr->handle = set;
            setPtr->layout = layout;
            setPtr->pool = pool;
            return setPtr;
        };

        auto hash = layout->GetHash();
        auto iter = pool->freeList.find(hash);
        if (iter != pool->freeList.end() && !iter->second.empty()) {
            auto back = iter->second.back();
            iter->second.pop_back();
            return setCreateFn(back);
        }

        auto vl = layout->GetNativeHandle();
        VkDescriptorSetAllocateInfo setInfo = {};
        setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        setInfo.pNext = nullptr;
        setInfo.descriptorPool = pool->pool;
        setInfo.descriptorSetCount = 1;
        setInfo.pSetLayouts = &vl;

        VkDescriptorSet set = VK_NULL_HANDLE;
        auto result = vkAllocateDescriptorSets(pool->device.GetNativeHandle(), &setInfo, &set);
        if (result != VK_SUCCESS) {
            return {};
        }

        return setCreateFn(set);
    }

    DescriptorSet::Writer& DescriptorSet::Writer::Write(uint32_t binding, VkDescriptorType type,
        BufferPtr buffer, VkDeviceSize offset, VkDeviceSize size)
    {
        set.buffers.emplace(binding, buffer);

        buffers.emplace_back();
        auto& bufferInfo = buffers.back();
        bufferInfo.buffer = buffer->GetNativeHandle();
        bufferInfo.offset = offset;
        bufferInfo.range = size;

        Write(binding, type, &bufferInfo, nullptr);
        return *this;
    }

    DescriptorSet::Writer& DescriptorSet::Writer::Write(uint32_t binding, VkDescriptorType type,ImageViewPtr view, SamplerPtr sampler)
    {
        set.views.emplace(binding, view);
        set.samplers.emplace(binding, sampler);

        images.emplace_back();
        auto& imageInfo = images.back();
        imageInfo.sampler = sampler->GetNativeHandle();
        imageInfo.imageView = view->GetNativeHandle();

        if (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        } else if (type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        }

        Write(binding, type, nullptr, &imageInfo);
        return *this;
    }

    void DescriptorSet::Writer::Write(uint32_t binding, VkDescriptorType type,
        const VkDescriptorBufferInfo* bufferInfo,
        const VkDescriptorImageInfo* imageInfo)
    {
        VkWriteDescriptorSet bindingInfo = {};
        bindingInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        bindingInfo.pNext = nullptr;
        bindingInfo.dstSet = set.GetNativeHandle();
        bindingInfo.dstBinding = binding;
        bindingInfo.dstArrayElement = 0;
        bindingInfo.descriptorCount = 1;
        bindingInfo.descriptorType = type;
        bindingInfo.pImageInfo = imageInfo;
        bindingInfo.pBufferInfo = bufferInfo;
        bindingInfo.pTexelBufferView = nullptr;
        writes.emplace_back(bindingInfo);
    }

    void DescriptorSet::Writer::Update()
    {
        vkUpdateDescriptorSets(set.device.GetNativeHandle(), static_cast<uint32_t>(writes.size()), writes.data(),
                               0, nullptr);
    }

}