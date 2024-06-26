//
// Created by Zach Lee on 2023/9/5.
//

#include <render/RHI.h>
#include <render/RenderDefaultResource.h>
#include <rhi/Queue.h>
#include <rhi/Stream.h>

namespace sky {

    const uint8_t DEFAULT_TEX_DATA[] = {
        255, 255, 255, 255, 127, 127, 127, 255,
        127, 127, 127, 255, 255, 255, 255, 255
    };

    void RenderDefaultResource::Init()
    {
        auto *device = RHI::Get()->GetDevice();
        defaultPool = device->CreateDescriptorSetPool({1});
        auto emptyRHIDesLayout = device->CreateDescriptorSetLayout({});
        emptyDesLayout = std::make_shared<ResourceGroupLayout>();
        emptyDesLayout->SetRHILayout(emptyRHIDesLayout);

        emptySet = std::make_shared<ResourceGroup>();
        emptySet->Init(emptyDesLayout, *defaultPool);

        rhi::Sampler::Descriptor samplerDesc = {};
        samplerDesc.minFilter = rhi::Filter::LINEAR;
        samplerDesc.magFilter = rhi::Filter::LINEAR;
        samplerDesc.addressModeU = rhi::WrapMode::REPEAT;
        samplerDesc.addressModeV = rhi::WrapMode::REPEAT;
        samplerDesc.addressModeW = rhi::WrapMode::REPEAT;
        samplerDesc.maxLod = 13.f;
        defaultSampler = device->CreateSampler(samplerDesc);

        rhi::Image::Descriptor imageDesc = {};
        imageDesc.imageType = rhi::ImageType::IMAGE_2D;
        imageDesc.format = rhi::PixelFormat::RGBA8_UNORM;
        imageDesc.extent = {2, 2, 1};
        imageDesc.memory = rhi::MemoryType::GPU_ONLY;
        imageDesc.usage = rhi::ImageUsageFlagBit::SAMPLED | rhi::ImageUsageFlagBit::TRANSFER_DST;

        auto *queue = device->GetQueue(rhi::QueueType::TRANSFER);
        rhi::TransferTaskHandle taskHandle = 0;

        rhi::ImageUploadRequest request = {};
        request.source = std::make_shared<rhi::RawPtrStream>(DEFAULT_TEX_DATA);
        request.size = sizeof(DEFAULT_TEX_DATA);
        request.imageOffset = {0, 0, 0};
        request.imageExtent = imageDesc.extent;

        {
            texture2D = std::make_shared<Texture2D>();
            texture2D->Init(rhi::PixelFormat::RGBA8_UNORM, 2, 2, 1);
            taskHandle = texture2D->Upload(DEFAULT_TEX_DATA, sizeof(DEFAULT_TEX_DATA), *queue);
        }

//        {
//            rhi::ImageViewDesc viewDesc = {};
//            imageDesc.arrayLayers = 6;
//            viewDesc.subRange.layers = 6;
//            viewDesc.viewType = rhi::ImageViewType::VIEW_CUBE;
//
//            auto image = device->CreateImage(imageDesc);
//            textureCube = image->CreateView(viewDesc);
//            std::vector<rhi::ImageUploadRequest> requests;
//            for (uint32_t i = 0; i < 6; ++i) {
//                request.layer = i;
//                requests.emplace_back(request);
//            }
//            taskHandle = queue->UploadImage(image, requests);
//        }

        queue->Wait(taskHandle);
    }

    void RenderDefaultResource::Reset()
    {
        defaultPool = nullptr;
        emptyDesLayout = nullptr;
        emptySet = nullptr;
        defaultSampler = nullptr;

        texture2D = nullptr;
    }
} // namespace sky
