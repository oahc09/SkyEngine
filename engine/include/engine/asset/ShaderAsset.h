//
// Created by Zach Lee on 2022/1/16.
//

#pragma once

#include <vulkan/Shader.h>
#include <framework/asset/Asset.h>
#include <framework/asset/Resource.h>

namespace sky {

    class ShaderAsset : public AssetBase {
    public:
        ShaderAsset(const Uuid& id) : AssetBase(id) {}
        ~ShaderAsset() = default;

        static constexpr Uuid TYPE = Uuid::CreateFromString("1338d2ed-5d6d-4324-aba5-1bfb6908fd7a");

        struct ShaderData {
            std::vector<uint32_t> data;
            VkShaderStageFlagBits stage;
            std::string entry = "main";
        };

        struct SourceData {
            std::vector<ShaderData> shaders;
        };

        const SourceData& GetSourceData() const
        {
            return sourceData;
        }

    private:
        friend class Shader;
        friend class ShaderAssetHandler;
        const Uuid& GetType() const override { return TYPE; }
        SourceData sourceData;

        AssetPtr Create(const Uuid &id);
    };

    class ShaderAssetHandler : public AssetHandlerBase {
    public:
        ShaderAssetHandler() = default;
        virtual ~ShaderAssetHandler() = default;

        AssetPtr Create(const Uuid& id) override;

        AssetPtr Load(const std::string&) override;
    };

    class Shader : public ResourceBase {
    public:
        Shader(const Uuid& id) : ResourceBase(id) {}
        ~Shader() = default;

        static CounterPtr<Shader> CreateFromAsset(AssetPtr asset);

    private:
        std::vector<drv::ShaderPtr> shaders;
    };

}