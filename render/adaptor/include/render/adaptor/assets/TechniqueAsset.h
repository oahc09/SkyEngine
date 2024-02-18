//
// Created by Zach Lee on 2023/2/23.
//

#pragma once

#include <framework/asset/AssetManager.h>
#include <render/resource/Technique.h>

#include <rhi/Core.h>
#include <string>
#include <vector>

namespace sky {
    class BinaryInputArchive;
    class BinaryOutputArchive;

    enum class TechAssetType : uint32_t {
        GRAPHIC,
        MESH,
        COMPUTE
    };

    struct ShaderRefData {
        std::string path;
        std::string objectOrCSMain;
        std::string vertOrMeshMain;
        std::string fragmentMain;
    };

    struct TechniqueAssetData {
        ShaderRefData shader;
        std::string passTag;
        std::string vertexDesc;
        TechAssetType type;

        rhi::DepthStencil            depthStencil;
        rhi::RasterState             rasterState;
        std::vector<rhi::BlendState> blendStates;

        void Load(BinaryInputArchive &archive);
        void Save(BinaryOutputArchive &archive) const;
    };

    std::shared_ptr<Technique> CreateTechnique(const TechniqueAssetData &data);

    template <>
    struct AssetTraits<Technique> {
        using DataType                                = TechniqueAssetData;
        static constexpr Uuid          ASSET_TYPE     = Uuid::CreateFromString("79F513A7-8BC1-48B4-B086-FB2E78798D60");
        static constexpr SerializeType SERIALIZE_TYPE = SerializeType::BIN;

        static std::shared_ptr<Technique> CreateFromData(const DataType &data)
        {
            return CreateTechnique(data);
        }
    };
    using TechniqueAssetPtr = std::shared_ptr<Asset<Technique>>;
}