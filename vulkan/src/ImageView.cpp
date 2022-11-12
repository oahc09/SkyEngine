//
// Created by Zach Lee on 2021/11/7.
//

#include "vulkan/ImageView.h"
#include "core/logger/Logger.h"
#include "vulkan/Basic.h"
#include "vulkan/Device.h"
#include "vulkan/Image.h"
#include "vulkan/Conversion.h"
static const char *TAG = "Vulkan";

namespace sky::vk {

    ImageView::ImageView(Device &dev) : DevObject(dev), source{}, view{VK_NULL_HANDLE}, viewInfo{}
    {
    }

    ImageView::~ImageView()
    {
        if (view != VK_NULL_HANDLE) {
            vkDestroyImageView(device.GetNativeHandle(), view, VKL_ALLOC);
        }
    }

    bool ImageView::Init(const VkDescriptor &des)
    {
        viewInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image            = source->GetNativeHandle();
        viewInfo.viewType         = des.viewType;
        viewInfo.format           = des.format;
        viewInfo.components       = des.components;
        viewInfo.subresourceRange = des.subResourceRange;
        VkResult rst              = vkCreateImageView(device.GetNativeHandle(), &viewInfo, VKL_ALLOC, &view);
        if (rst != VK_SUCCESS) {
            LOG_E(TAG, "create image view failed, -%d", rst);
        }
        return true;
    }

    bool ImageView::Init(const Descriptor &des)
    {
        viewInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image            = source->GetNativeHandle();
        viewInfo.viewType         = FromRHI(des.viewType);
        viewInfo.format           = source->GetImageInfo().format;

        viewInfo.components       = {VK_COMPONENT_SWIZZLE_IDENTITY,
                                     VK_COMPONENT_SWIZZLE_IDENTITY,
                                     VK_COMPONENT_SWIZZLE_IDENTITY,
                                     VK_COMPONENT_SWIZZLE_IDENTITY};

        viewInfo.subresourceRange = {FromRHI(des.mask),
                                     des.subRange.baseLevel,
                                     des.subRange.levels,
                                     des.subRange.baseLayer,
                                     des.subRange.layers};

        VkResult rst              = vkCreateImageView(device.GetNativeHandle(), &viewInfo, VKL_ALLOC, &view);
        if (rst != VK_SUCCESS) {
            LOG_E(TAG, "create image view failed, -%d", rst);
        }
        return true;
    }

    VkImageView ImageView::GetNativeHandle() const
    {
        return view;
    }

    const VkImageViewCreateInfo &ImageView::GetViewInfo() const
    {
        return viewInfo;
    }

    std::shared_ptr<ImageView> ImageView::CreateImageView(const ImagePtr &image, ImageView::VkDescriptor &des)
    {
        ImageViewPtr ptr = std::make_shared<ImageView>(image->device);
        ptr->source      = image;
        if (ptr->Init(des)) {
            return ptr;
        }
        return {};
    }

    std::shared_ptr<ImageView> ImageView::CreateImageView(const ImagePtr &image, ImageView::Descriptor &des)
    {
        ImageViewPtr ptr = std::make_shared<ImageView>(image->device);
        ptr->source      = image;
        if (ptr->Init(des)) {
            return ptr;
        }
        return {};
    }
} // namespace sky::vk
