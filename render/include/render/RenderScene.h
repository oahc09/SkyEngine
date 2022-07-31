
//
// Created by Zach Lee on 2021/11/14.
//


#pragma once

#include <render/RenderView.h>
#include <render/RenderPipeline.h>
#include <render/RenderFeature.h>
#include <render/RenderMesh.h>
#include <render/resources/DescirptorGroup.h>
#include <render/resources/DescriptorPool.h>
#include <core/type/Rtti.h>

namespace sky {

    struct SceneInfo {
        uint32_t lightCount;
    };

    class RenderScene {
    public:
        RenderScene() = default;
        ~RenderScene() = default;

        void OnPreRender();

        void OnPostRender();

        void OnRender();

        void AddView(RDViewPtr view);

        const std::vector<RDViewPtr>& GetViews() const;

        void Setup(RenderViewport& viewport);

        void ViewportChange(RenderViewport& viewport);

        template <typename T, typename ...Args>
        T* RegisterFeature(Args&& ...args)
        {
            auto iter = features.emplace(TypeInfo<T>::Hash(), std::make_unique<T>(std::forward<Args>(args)...));
            return static_cast<T*>(iter.first->second.get());
        }

        template <typename T>
        T* GetFeature()
        {
            auto iter = features.find(TypeInfo<T>::Hash());
            if (iter == features.end()) {
                return nullptr;
            }
            return static_cast<T*>(iter->second.get());
        }

        RDBufferViewPtr GetSceneBuffer() const;

        RDBufferViewPtr GetMainViewBuffer() const;

        RDDescriptorPoolPtr GetObjectSetPool() const;

    private:
        void InitSceneResource();

        RDPipeline pipeline;
        RDDescriptorPoolPtr objectPool;
        std::unordered_map<uint32_t, std::unique_ptr<RenderFeature>> features;

        // dynamic data
        std::vector<RDViewPtr> views;
        RDDesGroupPtr sceneSet;
        RDBufferViewPtr sceneInfo;
        RDBufferViewPtr mainViewInfo;

    };
    using RDScenePtr = std::shared_ptr<RenderScene>;
}