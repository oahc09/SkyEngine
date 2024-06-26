//
// Created by Zach Lee on 2021/11/7.
//

#pragma once

#include "vulkan/CommandPool.h"
#include "vulkan/vulkan.h"
#include "rhi/Queue.h"
#include "rhi/BufferPool.h"
#include <thread>
#include <array>
#include <unordered_map>

namespace sky::vk {

    class Device;

    class Queue : public rhi::Queue, public DevObject {
    public:
        ~Queue() override = default;

        void WaitIdle();

        uint32_t GetQueueFamilyIndex() const { return queueFamilyIndex; }
        VkQueue GetNativeHandle() const { return queue; }

        CommandBufferPtr AllocateCommandBuffer(const CommandBuffer::VkDescriptor &desc);
        CommandBufferPtr AllocateTlsCommandBuffer(const CommandBuffer::VkDescriptor &desc);

        rhi::TransferTaskHandle UploadImage(const rhi::ImagePtr &image, const std::vector<rhi::ImageUploadRequest> &requests) override;
        rhi::TransferTaskHandle UploadBuffer(const rhi::BufferPtr &image, const std::vector<rhi::BufferUploadRequest> &requests) override;
    private:
        friend class Device;
        const CommandPoolPtr &GetOrCreatePool();

        void SetupInternal() override;
        void PostShutdown() override;

        void BeginFrame();
        void EndFrame();

        static constexpr uint64_t BUFFER_PER_FRAME_SIZE = 8 * 1024 * 1024;
        static constexpr uint32_t INFLIGHT_NUM = 3;

        friend class Device;
        Queue(Device &dev, VkQueue q, uint32_t family) : DevObject(dev), queueFamilyIndex(family), currentFrameId(0), queue(q)
        {
        }

        uint32_t       queueFamilyIndex;
        uint32_t       currentFrameId;
        VkQueue        queue;
        CommandPoolPtr pool;

        std::array<std::unique_ptr<rhi::StagingBufferPool>, INFLIGHT_NUM> stagingBuffers;
        std::array<FencePtr, INFLIGHT_NUM> fences;
        std::array<CommandBufferPtr, INFLIGHT_NUM> inflightCommands;

        mutable std::mutex                                  mutex;
        std::unordered_map<std::thread::id, CommandPoolPtr> tlsPools;
    };

    using QueuePtr = std::unique_ptr<Queue>;
} // namespace sky::vk
