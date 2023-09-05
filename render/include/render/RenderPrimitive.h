//
// Created by Zach Lee on 2023/8/19.
//

#pragma once

#include <core/shapes/AABB.h>
#include <rhi/Device.h>
#include <render/resource/Material.h>
#include <render/resource/Technique.h>
#include <render/RenderPackage.h>

namespace sky {

    struct TechniqueInstance {
        uint32_t viewMask = 0xFFFFFFFF;  // mask all
        uint32_t rasterID = ~(0U);       // invalid id
        uint32_t subPassID = 0;
        std::string programKey = "";
        RDGfxTechPtr technique;
        rhi::RenderPassPtr renderPass;
        rhi::VertexInputPtr vertexDesc;
        rhi::GraphicsPipelinePtr pso;
    };

    struct RenderPrimitive {
        std::vector<TechniqueInstance> techniques;

        uint32_t sortKey = 0;
        AABB boundingBox {Vector3(std::numeric_limits<float>::min()), Vector3(std::numeric_limits<float>::max())};

        rhi::DescriptorSetPtr passSet; // custom per pass set.
        rhi::DescriptorSetPtr batchSet;
        rhi::DescriptorSetPtr instanceSet;

        rhi::VertexAssemblyPtr va;
        rhi::BufferPtr indirectBuffer;
        DrawArgs args;
    };

    struct RenderDrawItem {
        RenderPrimitive *primitive = nullptr;
        uint32_t techIndex = 0;
    };

} // namespace sky
