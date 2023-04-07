//
// Created by Zach Lee on 2023/2/5.
//

#include <gles/CommandContext.h>
#include <gles/Core.h>
#include <gles/Queue.h>
#include <gles/GraphicsPipeline.h>

namespace sky::gles {

    static bool IsDynamicBuffer(rhi::DescriptorType type)
    {
        return type == rhi::DescriptorType::UNIFORM_BUFFER_DYNAMIC || type == rhi::DescriptorType::STORAGE_BUFFER_DYNAMIC;
    }

    static bool IsBufferType(rhi::DescriptorType type)
    {
        return type == rhi::DescriptorType::STORAGE_BUFFER ||
            type == rhi::DescriptorType::STORAGE_BUFFER_DYNAMIC ||
            type == rhi::DescriptorType::UNIFORM_BUFFER ||
            type == rhi::DescriptorType::UNIFORM_BUFFER_DYNAMIC;
    }

    static bool IsCombinedSampler(rhi::DescriptorType type)
    {
        return type == rhi::DescriptorType::COMBINED_IMAGE_SAMPLER;
    }

    static bool CheckBuffer(const SetDescriptor &desc) {
        return desc.buffer.view && desc.buffer.view && desc.buffer.view->GetNativeHandle() != 0 && desc.buffer.view->GetViewDesc().range != 0;
    }

    static bool CheckTexture(const SetDescriptor& desc) {
        return desc.texture.view && desc.texture.view->GetImage() && desc.texture.view->GetImage()->GetNativeHandle() != 0;
    }

    static GLenum GetBufferTarget(rhi::DescriptorType type) {
        switch (type) {
        case rhi::DescriptorType::STORAGE_BUFFER:
        case rhi::DescriptorType::STORAGE_BUFFER_DYNAMIC:
            return GL_SHADER_STORAGE_BUFFER;
        case rhi::DescriptorType::UNIFORM_BUFFER:
        case rhi::DescriptorType::UNIFORM_BUFFER_DYNAMIC:
            return GL_UNIFORM_BUFFER;
        }
        return 0;
    }

    static GLenum GetTextureTarget(const ImageViewPtr &imageView)
    {
        auto &viewInfo = imageView->GetViewDesc();
        switch (viewInfo.viewType) {
        case rhi::ImageViewType::VIEW_2D: return GL_TEXTURE_2D;
        case rhi::ImageViewType::VIEW_2D_ARRAY: return GL_TEXTURE_2D_ARRAY;
        case rhi::ImageViewType::VIEW_CUBE: return GL_TEXTURE_CUBE_MAP;
        case rhi::ImageViewType::VIEW_CUBE_ARRAY: return GL_TEXTURE_CUBE_MAP_ARRAY;
        case rhi::ImageViewType::VIEW_3D: return GL_TEXTURE_3D;
        }
        return 0;
    }

    static bool HasDepth(rhi::PixelFormat format)
    {
        return format == rhi::PixelFormat::D24_S8 ||
            format == rhi::PixelFormat::D32 ||
            format == rhi::PixelFormat::D32_S8;
    }

    static bool HasStencil(rhi::PixelFormat format)
    {
        return format == rhi::PixelFormat::D32_S8 ||
            format == rhi::PixelFormat::D24_S8;
    }

    void CommandContext::CmdBeginPass(const FrameBufferPtr &frameBuffer, const RenderPassPtr &renderPass, uint32_t count, rhi::ClearValue *values)
    {
        currentFramebuffer = frameBuffer;
        currentRenderPass  = renderPass;
        clearValues = values;
        clearCount = count;

        auto &ext = currentFramebuffer->GetExtent();
        CHECK(glViewport(0, 0, ext.width, ext.height));
        CHECK(glScissor(0, 0, ext.width, ext.height));

        BeginPassInternal();
    }

    void CommandContext::CmdNextPass()
    {
        EndPassInternal();
        ++currentSubPassId;
        BeginPassInternal();
    }

    void CommandContext::BeginPassInternal()
    {
        auto &fbs    = currentFramebuffer->GetNativeHandles();
        auto &subPasses = currentRenderPass->GetSubPasses();
        auto &attachments = currentRenderPass->GetAttachments();

        SKY_ASSERT(currentSubPassId < fbs.size());
        SKY_ASSERT(currentSubPassId < subPasses.size());

        auto &subPass = subPasses[currentSubPassId];
        auto &fb   = fbs[currentSubPassId];
        if (fb.surface) {
            if (fb.surface->GetSurface() != context->GetCurrentSurface()) {
                context->MakeCurrent(*fb.surface);
            }
        }

        if (cache->drawBuffer != fb.fbo) {
            CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb.fbo));
            cache->drawBuffer = fb.fbo;
        }

        GLbitfield clearBit = 0;
        auto attachmentFunc = [this, &clearBit](const rhi::RenderPass::Attachment &attachment, uint32_t index) {
            auto &feature = GetFormatFeature(attachment.format);
            bool hasDepth = HasDepth(attachment.format);
            bool hasStencil = HasStencil(attachment.format);
            bool isColor = !(hasDepth || hasStencil);
            bool isDefaultFb = cache->drawBuffer == 0;

            if (attachment.load == rhi::LoadOp::DONT_CARE) {
                if (hasDepth) {
                    invalidAttachments.emplace_back(isDefaultFb ? GL_DEPTH : GL_DEPTH_ATTACHMENT);
                }
                if (isColor) {
                    invalidAttachments.emplace_back(isDefaultFb ? GL_COLOR : GL_COLOR_ATTACHMENT0 + index);
                }
            } else if (attachment.load == rhi::LoadOp::CLEAR) {
                auto &clearColor = clearValues[index];
                if (isColor) {
                    CHECK(glClearBufferfv(GL_COLOR, index, clearColor.color.float32));
                }
                if (hasDepth) {
                    if (cache->ds.depth.depthWrite != true) {
                        CHECK(glDepthMask(true));
                        cache->ds.depth.depthWrite = true;
                    }
                    CHECK(glClearDepthf(clearColor.depthStencil.depth));
                    clearBit |= GL_DEPTH_BUFFER_BIT;
                }
            }

            if (hasStencil) {
                if (attachment.stencilLoad == rhi::LoadOp::DONT_CARE) {
                    invalidAttachments.emplace_back(isDefaultFb ? GL_STENCIL : GL_STENCIL_ATTACHMENT);
                } else if (attachment.stencilLoad == rhi::LoadOp::CLEAR) {
                    auto &clearColor = clearValues[index];
                    if (cache->ds.front.writemask != 0xffffffff) {
                        CHECK(glStencilMaskSeparate(GL_FRONT, 0xffffffff));
                        cache->ds.front.writemask = 0xffffffff;
                    }
                    if (cache->ds.back.writemask != 0xffffffff) {
                        CHECK(glStencilMaskSeparate(GL_BACK, 0xffffffff));
                        cache->ds.back.writemask = 0xffffffff;
                    }
                    CHECK(glClearStencil(clearColor.depthStencil.stencil));
                    clearBit |= GL_STENCIL_BUFFER_BIT;
                }
            }
        };

        for (uint32_t i = 0; i < subPass.colors.size(); ++i) {
            auto color = subPass.colors[i];
            auto &attachment = attachments[color];
            attachmentFunc(attachment, color);
        }
        if (subPass.depthStencil != ~(0U)) {
            attachmentFunc(attachments[subPass.depthStencil], 0);
        }
        if (clearBit != 0) {
            CHECK(glClear(clearBit));
        }

        if (!invalidAttachments.empty()) {
            CHECK(glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<uint32_t>(invalidAttachments.size()), invalidAttachments.data()));
            invalidAttachments.clear();
        }

        cache->drawBuffer = fb.fbo;
    }

    void CommandContext::EndPassInternal()
    {
        auto &fbs    = currentFramebuffer->GetNativeHandles();
        auto &subPasses = currentRenderPass->GetSubPasses();
        auto &attachments = currentRenderPass->GetAttachments();

        SKY_ASSERT(currentSubPassId < fbs.size());
        SKY_ASSERT(currentSubPassId < subPasses.size());

        auto &subPass = subPasses[currentSubPassId];
        auto &fb   = fbs[currentSubPassId];

        auto attachmentFunc = [this](const rhi::RenderPass::Attachment &attachment, uint32_t index) {
            auto &feature = GetFormatFeature(attachment.format);
            bool hasDepth = HasDepth(attachment.format);
            bool hasStencil = HasStencil(attachment.format);
            bool isColor = !(hasDepth || hasStencil);
            bool isDefaultFb = cache->drawBuffer == 0;

            if (attachment.store == rhi::StoreOp::DONT_CARE) {
                if (hasDepth) {
                    invalidAttachments.emplace_back(isDefaultFb ? GL_DEPTH : GL_DEPTH_ATTACHMENT);
                }
                if (isColor) {
                    invalidAttachments.emplace_back(isDefaultFb ? GL_COLOR : GL_COLOR_ATTACHMENT0 + index);
                }
            }

            if (hasStencil && attachment.stencilStore == rhi::StoreOp::DONT_CARE) {
                invalidAttachments.emplace_back(isDefaultFb ? GL_STENCIL : GL_STENCIL_ATTACHMENT);
            }
        };

        // perform attachment store op
        for (uint32_t i = 0; i < subPass.colors.size(); ++i) {
            auto color = subPass.colors[i];
            auto &attachment = attachments[color];
            attachmentFunc(attachment, color);
        }
        if (subPass.depthStencil != ~(0U)) {
            attachmentFunc(attachments[subPass.depthStencil], 0);
        }

        if (!invalidAttachments.empty()) {
            CHECK(glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<uint32_t>(invalidAttachments.size()), invalidAttachments.data()));
            invalidAttachments.clear();
        }
    }

    void CommandContext::CmdBindDescriptorSet(uint32_t setId, const DescriptorSetPtr &set, uint32_t dynamicCount, uint32_t *dynamicOffsets)
    {
        SKY_ASSERT(setId < MAX_SET_ID);
        if (sets.empty()) {
            sets.resize(MAX_SET_ID);
        }
        sets[setId] = set;

        auto &descriptorOffsets = currentPso->GetDescriptorOffsets();
        auto &descriptorIndices = currentPso->GetDescriptorIndices();

        auto pipelineLayout = currentPso->GetPipelineLayout();
        auto &setLayouts = pipelineLayout->GetLayouts();

        auto &descriptorSetLayout = setLayouts[setId];
        auto &bindings = descriptorSetLayout->GetBindings();
        auto &bindingMap = descriptorSetLayout->GetBindingMap();

        uint32_t dynamicIndex = 0;
        for (auto &binding : bindings) {
            auto offsetToSet            = bindingMap.at(binding.binding);
            auto offsetToPipelineLayout = descriptorOffsets[setId];

            uint32_t *dynamicOffsetPtr = nullptr;
            if (IsDynamicBuffer(binding.type) && dynamicIndex < dynamicCount) {
                dynamicOffsetPtr = &dynamicOffsets[dynamicIndex];
                dynamicIndex += binding.count;
            }

            auto *descriptorIndexBase = &descriptorIndices[offsetToSet + offsetToPipelineLayout];
            auto *descriptorBase      = &set->GetDescriptors()[offsetToSet];

            for (uint32_t i = 0; i < binding.count; ++i) {
                auto &descriptor      = descriptorBase[i];
                auto &descriptorIndex = descriptorIndexBase[i];
                if (IsBufferType(binding.type) && CheckBuffer(descriptor)) {
                    uint32_t dynamicOffset = dynamicOffsetPtr != nullptr ? dynamicOffsetPtr[i] : 0;
                    auto &bufferView = descriptor.buffer.view;
                    auto &viewInfo = bufferView->GetViewDesc();
                    CHECK(glBindBufferRange(GetBufferTarget(binding.type), descriptorIndex.binding, bufferView->GetNativeHandle(),
                        viewInfo.offset + dynamicOffset, viewInfo.range));
                } else if (IsCombinedSampler(binding.type) && CheckTexture(descriptor)) {
                    CHECK(glActiveTexture(GL_TEXTURE0 + descriptorIndex.unit));
                    CHECK(glBindTexture(GetTextureTarget(descriptor.texture.view), descriptor.texture.view->GetNativeHandle()));
                    CHECK(glBindSampler(descriptorIndex.unit, descriptor.texture.sampler->GetNativeHandle()));
                }
            }
        }
    }

    void CommandContext::CmdBindPipeline(const GraphicsPipelinePtr &pso)
    {
        currentPso = pso;

        auto program = pso->GetProgram();
        if (cache->program != program) {
            cache->program = program;
            CHECK(glUseProgram(program));
        }

        auto &state = pso->GetGLState();
        cache->primitive = state.primitive;


    }

    void CommandContext::CmdBindAssembly(const VertexAssemblyPtr &assembly)
    {
        if (!assembly->IsInited()) {
            assembly->InitInternal();
        }

        auto vao = assembly->GetNativeHandle();
        if (cache->vao != vao) {
            cache->vao = vao;
            CHECK(glBindVertexArray(vao));
        }
    }

    void CommandContext::CmdSetViewport(uint32_t count, const rhi::Viewport *viewport)
    {
        CHECK(glViewport(static_cast<GLint>(viewport->x), static_cast<GLint>(viewport->y),
                   static_cast<GLsizei>(viewport->width), static_cast<GLsizei>(viewport->height)));

        CHECK(glDepthRangef(viewport->minDepth, viewport->maxDepth));
    }

    void CommandContext::CmdSetScissor(uint32_t count, const rhi::Rect2D *scissor)
    {
        CHECK(glScissor(scissor->offset.x, scissor->offset.y, scissor->extent.width, scissor->extent.height));
    }

    void CommandContext::CmdDrawIndexed(const rhi::CmdDrawIndexed &indexed)
    {
        // TODO(index type)
        CHECK(glDrawElementsInstanced(cache->primitive, indexed.indexCount, GL_UNSIGNED_SHORT, 0, indexed.instanceCount));
    }

    void CommandContext::CmdDrawLinear(const rhi::CmdDrawLinear &linear)
    {
        CHECK(glDrawArraysInstanced(cache->primitive, linear.firstVertex, linear.vertexCount, linear.instanceCount));
    }

    void CommandContext::CmdDrawIndirect(const BufferPtr &buffer, uint32_t offset, uint32_t size)
    {
    }

    void CommandContext::CmdEndPass()
    {
        EndPassInternal();

        currentFramebuffer = nullptr;
        currentRenderPass  = nullptr;
        currentPso         = nullptr;
        currentSubPassId = 0;
    }

    void CommandContext::Attach(Queue &queue)
    {
        context = queue.GetContext();
        cache = queue.GetCacheState();
        sets.clear();
    }
}