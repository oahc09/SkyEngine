//
// Created by Zach Lee on 2021/11/7.
//
#pragma once
#include "vulkan/DevObject.h"
#include "vulkan/vulkan.h"
#include "vulkan/ImageView.h"
#include "vk_mem_alloc.h"

namespace sky::drv {

    class Device;

    class Image : public DevObject {
    public:
        ~Image();

        struct Descriptor {
            VkImageType           imageType   = VK_IMAGE_TYPE_2D;
            VkFormat              format      = VK_FORMAT_UNDEFINED;
            VkExtent3D            extent      = {1, 1, 1};
            uint32_t              mipLevels   = 1;
            uint32_t              arrayLayers = 1;
            VkImageUsageFlags     usage       = 0;
            VkSampleCountFlagBits samples     = VK_SAMPLE_COUNT_1_BIT;
            VkImageTiling         tiling      = VK_IMAGE_TILING_OPTIMAL;
            VmaMemoryUsage        memory      = VMA_MEMORY_USAGE_UNKNOWN;
        };

        ImageView* CreateImageView(const ImageView::Descriptor& des);

    private:
        friend class Device;
        Image(Device&);
        bool Init(const Descriptor&);

        VkImage image;
        VmaAllocation allocation;
    };

}