//
// Created by Zach on 2023/2/1.
//

#include <gles/RenderPass.h>
#include <gles/Conversion.h>

namespace sky::gles {

    bool RenderPass::Init(const Descriptor &desc)
    {
        attachments = desc.attachments;
        subPasses = desc.subPasses;

        drawBuffers.resize(subPasses.size());
        attachmentGLInfos.resize(attachments.size());

        auto colorIndex = GL_COLOR_ATTACHMENT0;
        for (uint32_t i = 0; i < subPasses.size(); ++i) {
            auto &sub = subPasses[i];
            auto &drawBuffer = drawBuffers[i];

            for (auto &color : sub.colors) {
                auto &index = attachmentGLInfos[color.index].index;
                if (index == INVALID_INDEX) {
                    index = static_cast<uint32_t>(colors.size());
                    colors.emplace_back(color.index);
                }
                drawBuffer.emplace_back(colorIndex + index);
            }

            for (auto &resolve : sub.resolves) {
                auto &index = attachmentGLInfos[resolve.index].index;
                if (index == INVALID_INDEX) {
                    index = static_cast<uint32_t>(resolves.size());
                    resolves.emplace_back(resolve.index);
                }
            }

            for (auto &input : sub.inputs) {
                auto &index = attachmentGLInfos[input.index].index;
                if (index == INVALID_INDEX) {
                    index = static_cast<uint32_t>(inputs.size());
                    inputs.emplace_back(input.index);
                }
            }

            if (sub.depthStencil.index != INVALID_INDEX) {
                auto &attachment = attachments[sub.depthStencil.index];
                auto &attachmentInfo = attachmentGLInfos[sub.depthStencil.index];
                attachmentInfo.index = 0;
                const auto &feature = GetFormatFeature(attachment.format);
                attachmentInfo.hasDepth = HasDepth(attachment.format);
                attachmentInfo.hasStencil = HasStencil(attachment.format);
                depthStencil = sub.depthStencil.index;
            }
        }
        return true;
    }

}
