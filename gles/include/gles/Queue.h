//
// Created by Zach on 2023/1/31.
//

#pragma once

#include <rhi/Queue.h>
#include <gles/DevObject.h>
#include <gles/Context.h>

namespace sky::gles {
    class Device;

    class Queue : public rhi::Queue, public DevObject {
    public:
        Queue(Device &dev) : DevObject(dev) {}
        ~Queue() = default;

        bool Init(const Context::Descriptor &cfg, rhi::QueueType type);
        Context *GetContext() const { return context.get(); }

    protected:
        std::unique_ptr<Context> context;
    };

}
