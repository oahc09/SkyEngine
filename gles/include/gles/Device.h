//
// Created by Zach on 2023/1/30.
//

#pragma once

#include <rhi/Device.h>
#include <gles/Context.h>
#include <gles/PBuffer.h>
#include <gles/Swapchain.h>
#include <gles/Image.h>
#include <gles/Queue.h>
#include <gles/RenderPass.h>
#include <gles/FrameBuffer.h>
#include <gles/CommandBuffer.h>
#include <gles/Fence.h>
#include <memory>

namespace sky::gles {

    class Device : public rhi::Device {
    public:
        Device() = default;
        ~Device();

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

        bool Init(const Descriptor &desc);

        Context *GetMainContext() const;
        Queue *GetGraphicsQueue() const;
        Queue *GetTransferQueue() const;
        Queue* GetQueue(rhi::QueueType type) const override;

        // Device Object
        CREATE_DEV_OBJ(SwapChain)
        CREATE_DEV_OBJ(Image)
        CREATE_DEV_OBJ(RenderPass)
        CREATE_DEV_OBJ(FrameBuffer)
        CREATE_DEV_OBJ(CommandBuffer)
        CREATE_DEV_OBJ(Fence)

    private:
        std::unique_ptr<Context> mainContext;
        std::unique_ptr<Queue> graphicsQueue;
        std::unique_ptr<Queue> transferQueue;
    };

}
