//
// Created by Zach Lee on 2023/2/2.
//

#include <gles/VertexAssembly.h>
#include <gles/BufferView.h>
#include <gles/VertexInput.h>
#include <gles/Core.h>

namespace sky::gles {

#define VA_OFFSET(offset) (static_cast<char *>(0) + (offset))

    VertexAssembly::~VertexAssembly()
    {
        if (vao != 0) {
            CHECK(glDeleteVertexArrays(1, &vao));
        }
    }

    bool VertexAssembly::Init(const Descriptor &desc)
    {
        descriptor = desc;
        return true;
    }

    void VertexAssembly::InitInternal()
    {
        CHECK(glGenVertexArrays(1, &vao));
        CHECK(glBindVertexArray(vao));

        auto vi = std::static_pointer_cast<VertexInput>(descriptor.vertexInput);
        auto &attributes = vi->GetAttributes();
        auto &bindings = vi->GetBindings();

        auto iter = attributes.begin();
        for (auto &binding : bindings) {
            auto bufferView = std::static_pointer_cast<BufferView>(descriptor.vertexBuffers[binding.binding]);
            CHECK(glBindBuffer(GL_ARRAY_BUFFER, bufferView->GetNativeHandle()));
            for (; iter != attributes.end() && (*iter).binding == binding.binding; ++iter) {
                auto &attribute = *iter;
                CHECK(glVertexAttribPointer(attribute.location, attribute.format.size, attribute.format.type, attribute.format.normalized, binding.stride, VA_OFFSET(attribute.offset)));
                CHECK(glVertexAttribDivisor(attribute.location, binding.inputRate == rhi::VertexInputRate::PER_INSTANCE ? 1 : 0));
                CHECK(glEnableVertexAttribArray(attribute.location));
            }
        }

        if (descriptor.indexBuffer) {
            CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, std::static_pointer_cast<BufferView>(descriptor.indexBuffer)->GetNativeHandle()));
        }

        // resume
        CHECK(glBindVertexArray(0));
        CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
        CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

        inited = true;
    }
}