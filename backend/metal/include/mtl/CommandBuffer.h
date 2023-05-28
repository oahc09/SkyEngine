//
// Created by Zach Lee on 2023/5/28.
//

#pragma once

#include <rhi/CommandBuffer.h>
#include <mtl/DevObject.h>
#include <mtl/RenderPass.h>
#include <mtl/FrameBuffer.h>
#import <Metal/MTLCommandBuffer.h>

namespace sky::mtl {
    class Device;
    class CommandBuffer;

    class GraphicsEncoder : public rhi::GraphicsEncoder {
    public:
        GraphicsEncoder(CommandBuffer &cmd) : commandBuffer(cmd) {}
        ~GraphicsEncoder() = default;

        rhi::GraphicsEncoder &BeginPass(const rhi::PassBeginInfo &beginInfo) override;
        rhi::GraphicsEncoder &BindPipeline(const rhi::GraphicsPipelinePtr &pso) override;
        rhi::GraphicsEncoder &BindAssembly(const rhi::VertexAssemblyPtr &assembly) override;
        rhi::GraphicsEncoder &SetViewport(uint32_t count, const rhi::Viewport *viewport) override;
        rhi::GraphicsEncoder &SetScissor(uint32_t count, const rhi::Rect2D *scissor) override;
        rhi::GraphicsEncoder &DrawIndexed(const rhi::CmdDrawIndexed &indexed) override;
        rhi::GraphicsEncoder &DrawLinear(const rhi::CmdDrawLinear &linear) override;
        rhi::GraphicsEncoder &DrawIndirect(const rhi::BufferPtr &buffer, uint32_t offset, uint32_t size) override;
        rhi::GraphicsEncoder &BindSet(uint32_t id, const rhi::DescriptorSetPtr &set) override;
        rhi::GraphicsEncoder &NextSubPass() override;
        rhi::GraphicsEncoder &EndPass() override;

    private:
        CommandBuffer &commandBuffer;
        RenderPassPtr currentRenderPass;
        FrameBufferPtr currentFramebuffer;
        id<MTLRenderCommandEncoder> encoder;
    };

    class CommandBuffer : public rhi::CommandBuffer, public DevObject {
    public:
        CommandBuffer(Device &dev) : DevObject(dev) {}
        ~CommandBuffer();

        void Begin() override {}
        void End() override {}
        void Submit(rhi::Queue &queue, const rhi::SubmitInfo &submit) override {}

        std::shared_ptr<rhi::GraphicsEncoder> EncodeGraphics() override { return std::make_shared<GraphicsEncoder>(*this); }
        id<MTLCommandBuffer> GetNativeHandle() const { return commandBuffer; }

    private:
        friend class Device;
        bool Init(const Descriptor &desc);

        id<MTLCommandBuffer> commandBuffer = nil;
    };

} // namespace sky::mtl
