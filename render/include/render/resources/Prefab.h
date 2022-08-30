//
// Created by Zach Lee on 2022/8/25.
//

#pragma once

#include <render/resources/Mesh.h>
#include <framework/serialization/BasicSerialization.h>
#include <core/util/Uuid.h>
#include <core/math/Matrix.h>
#include <unordered_map>
#include <string>

namespace sky {

    struct PrefabAssetNode {
        Matrix4 transform;
        uint32_t parentIndex = ~(0u);

        template<class Archive>
        void serialize(Archive &ar)
        {
            ar(transform, parentIndex);
        }
    };

    struct PrefabData {
        std::vector<PrefabAssetNode> nodes;
        std::unordered_map<Uuid, std::string> assetPathMap;

        template<class Archive>
        void serialize(Archive &ar)
        {
            ar(nodes, assetPathMap);
        }
    };

    class Prefab {
    public:
        Prefab() = default;
        ~Prefab() = default;
    };
    using PrefabPtr = std::shared_ptr<Prefab>;


    template <>
    struct AssetTraits <Prefab> {
        using DataType = PrefabData;
        static constexpr Uuid ASSET_TYPE = Uuid::CreateFromString("B3F0BC22-EAF6-47BA-9A8F-3D5F11D06777");
        static constexpr SerializeType SERIALIZE_TYPE = SerializeType::JSON;
    };
}