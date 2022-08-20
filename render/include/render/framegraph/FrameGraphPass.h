//
// Created by Zach Lee on 2022/6/2.
//

#pragma once

#include <render/framegraph/FrameGraphNode.h>
#include <render/framegraph/FrameGraphAttachment.h>
#include <render/RenderEncoder.h>
#include <render/RenderMeshPrimtive.h>

namespace sky {
    class FrameGraphBuilder;

    class FrameGraphPass : public FrameGraphNode {
    public:
        FrameGraphPass(const std::string& str) : FrameGraphNode(str) {}
        ~FrameGraphPass() = default;

        virtual void Compile() = 0;

        virtual void UseImageAttachment(FrameGraphImageAttachment* attachment) = 0;
    };

    class FrameGraphEmptyPass : public FrameGraphPass {
    public:
        FrameGraphEmptyPass(const std::string& str) : FrameGraphPass(str) {}
        ~FrameGraphEmptyPass() = default;

        void Compile() override {}

        void Execute(drv::CommandBufferPtr commandBuffer) override {}

        void UseImageAttachment(FrameGraphImageAttachment* attachment) override {}
    };

    class FrameGraphGraphicPass : public FrameGraphPass {
    public:
        FrameGraphGraphicPass(const std::string& str) : FrameGraphPass(str) {}
        ~FrameGraphGraphicPass();

        void UseImageAttachment(FrameGraphImageAttachment* attachment) override;

        void Compile() override;

        void Execute(drv::CommandBufferPtr commandBuffer) override;

        void SetEncoder(RenderEncoder* encoder);

        drv::RenderPassPtr GetPass() const;

    private:
        void AddClearValue(FrameGraphImageAttachment* attachment);

        drv::PassBeginInfo passInfo = {};
        std::vector<VkClearValue> clearValues;
        std::vector<FrameGraphImageAttachment*> colors;
        std::vector<FrameGraphImageAttachment*> resolves;
        std::vector<FrameGraphImageAttachment*> inputs;
        FrameGraphImageAttachment* depthStencil;
        RenderEncoder* encoder;
    };

}
