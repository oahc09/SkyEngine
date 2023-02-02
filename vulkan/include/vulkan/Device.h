//
// Created by Zach Lee on 2021/11/7.
//

#pragma once
#include <core/template/ReferenceObject.h>
#include <vector>
#include <vk_mem_alloc.h>
#include <rhi/Device.h>
#include <vulkan/CacheManager.h>
#include <vulkan/Queue.h>
#include <vulkan/AsyncTransferQueue.h>
#include <vulkan/Swapchain.h>
#include <vulkan/RenderPass.h>
#include <vulkan/FrameBuffer.h>
#include <vulkan/CommandBuffer.h>
#include <vulkan/Fence.h>
#include <vulkan/vulkan.h>

namespace sky::vk {

    class Instance;

    class Device : public rhi::Device {
    public:
        ~Device() override;

        template <typename T, typename Desc>
        inline std::shared_ptr<T> CreateDeviceObject(const Desc &des)
        {
            auto res = new T(*this);
            if (!res->Init(des)) {
                delete res;
                res = nullptr;
            }
            return std::shared_ptr<T>(res);
        }

        void WaitIdle() const;

        VmaAllocator GetAllocator() const;
        VkDevice GetNativeHandle() const;
        VkPhysicalDevice GetGpuHandle() const;
        VkInstance GetInstance() const;

        // Queue
        Queue *GetQueue(VkQueueFlags preferred, VkQueueFlags excluded) const;
        Queue *GetGraphicsQueue() const;
        AsyncTransferQueue *GetAsyncTransferQueue() const;

        // Cache object
        VkSampler GetSampler(uint32_t hash, VkSamplerCreateInfo *samplerInfo = nullptr);
        VkPipelineLayout GetPipelineLayout(uint32_t hash, VkPipelineLayoutCreateInfo * = nullptr);
        VkDescriptorSetLayout GetDescriptorSetLayout(uint32_t hash, VkDescriptorSetLayoutCreateInfo * = nullptr);
        VkRenderPass GetRenderPass(uint32_t hash, VkRenderPassCreateInfo * = nullptr);
        VkPipeline GetPipeline(uint32_t hash, VkGraphicsPipelineCreateInfo * = nullptr);

        // features
        const VkPhysicalDeviceProperties &GetProperties() const;
        bool GetBufferMemoryRequirements(VkBuffer buffer, VkMemoryPropertyFlags flags, MemoryRequirement &requirement) const;
        bool GetImageMemoryRequirements(VkImage image, VkMemoryPropertyFlags flags, MemoryRequirement &requirement) const;
        int32_t FindProperties(uint32_t memoryTypeBits, VkMemoryPropertyFlags requiredProperties) const;

        // rhi
        rhi::Queue* GetQueue(rhi::QueueType type) const override { return graphicsQueue; }

        // Device Object
        CREATE_DEV_OBJ(SwapChain)
        CREATE_DEV_OBJ(Image)
        CREATE_DEV_OBJ(RenderPass)
        CREATE_DEV_OBJ(FrameBuffer)
        CREATE_DEV_OBJ(CommandBuffer)
        CREATE_DEV_OBJ(Fence)
    private:
        bool Init(const Descriptor &, bool enableDebug);

        void ValidateFeature(const DeviceFeature &feature);

        void SetupAsyncTransferQueue();

        bool FillMemoryRequirements(VkMemoryRequirements2               &requirements,
                                    const VkMemoryDedicatedRequirements &dedicated,
                                    VkMemoryPropertyFlags                flags,
                                    MemoryRequirement                   &out) const;

        friend class Instance;
        Device(Instance &);
        Instance        &instance;
        VkPhysicalDevice phyDev;
        VkDevice         device;
        VmaAllocator     allocator;

        VkPhysicalDeviceProperties                 phyProps                   = {};
        VkPhysicalDeviceFeatures2                  phyFeatures                = {};
        VkPhysicalDeviceFeatures                   enabledPhyFeatures         = {};
        VkPhysicalDeviceSynchronization2Features   sync2Feature               = {};
        VkPhysicalDeviceDescriptorIndexingFeatures phyIndexingFeatures        = {};
        VkPhysicalDeviceDescriptorIndexingFeatures enabledPhyIndexingFeatures = {};

        VkPhysicalDeviceMemoryProperties2 memoryProperties = {};

        std::vector<VkQueueFamilyProperties> queueFamilies;
        std::vector<QueuePtr>                queues;
        Queue                               *graphicsQueue;
        AsyncTransferQueuePtr                transferQueue;

        CacheManager<VkSampler>             samplers;
        CacheManager<VkDescriptorSetLayout> setLayouts;
        CacheManager<VkPipelineLayout>      pipelineLayouts;
        CacheManager<VkPipeline>            pipelines;
        CacheManager<VkRenderPass>          renderPasses;
    };

} // namespace sky::vk
