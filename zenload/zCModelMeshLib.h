#pragma once
#include <vector>
#include "zCMeshSoftSkin.h"
#include "zTypes.h"
#include "utils/mathlib.h"

namespace VDFS
{
    class FileIndex;
}

namespace ZenLoad
{
    struct ModelNode
    {
        /**
         * @return Whether the parent is valid or not (Invalid on root-nodes)
         */
        bool parentValid() const { return parentIndex != 0xFFFF; }

        uint16_t parentIndex;
        std::string name;
        ZMath::Matrix transformLocal;
        ZMath::Matrix transformBindPoseDebug;  // "Global"

        //std::vector<uint16_t> childIndices;
    };

    class ZenParser;
    class zCModelMeshLib
    {
    public:
        zCModelMeshLib() = default;

        /**
         * @brief Loads the mesh from the given VDF-Archive
         */
        zCModelMeshLib(const std::string& fileName, const VDFS::FileIndex& fileIndex);

        /**
         * @brief Reads the mesh-object from the given binary stream
         * @param parser ZenParser object
         */
        void loadMDM(ZenParser& parser);

        /**
         * @brief Reads the model hierachy from a file (MDH-File)
         */
        void loadMDH(ZenParser& parser);

        /**
         * @brief reads this lib as MDL
         */
        void loadMDL(ZenParser& parser);

        /**
         * @brief Creates packed submesh-data
         */
        void packMesh(PackedSkeletalMesh& mesh) const;

        /**
         * @return List of meshes registered in this library
         */
        const std::vector<zCMeshSoftSkin>& getMeshes() const { return m_Meshes; }

        /**
         * @return List of nodes in the hierachy
         */
        const std::vector<ModelNode>& getNodes() const { return m_Nodes; }

        /**
         * @return List of attached meshes
         */
        const std::vector<std::pair<std::string, zCProgMeshProto>>& getAttachments() const { return m_NodeAttachments; }

        /**
         * @return Checksum for this node hierachy
         */
        uint32_t getNodeChecksum() const { return m_NodeChecksum; }

        /**
         * @return The index of the node with the given name
         */
        size_t findNodeIndex(const std::string& nodeName) const;

        /**
         * @return Whether this was loaded correctly
         */
        bool isValid()
        {
            return !m_Meshes.empty() || !m_NodeAttachments.empty() || !m_Nodes.empty();
        }

        /**
         * @return BBox min/max
         */
        ZMath::float3 getBBoxMin() const { return m_BBox[0]; }
        ZMath::float3 getBBoxMax() const { return m_BBox[1]; }
        ZMath::float3 getBBoxCollisionMin() const { return m_BBoxCollision[0]; }
        ZMath::float3 getBBoxCollisionMax() const { return m_BBoxCollision[1]; }
        ZMath::float3 getRootNodeTranslation() const { return m_RootNodeTranslation; }

    private:
        /**
         * @brief List of meshes registered in this library
         */
        std::vector<zCMeshSoftSkin> m_Meshes;

        /**
         * @brief Node-Attachments
         */
        std::vector<std::pair<std::string, zCProgMeshProto>> m_NodeAttachments;

        /**
         * @brief Model hierachy
         */
        std::vector<ModelNode> m_Nodes;
        std::vector<uint16_t> m_RootNodes;

        /**
         * @brief Checksum for this node hierachy
         */
        uint32_t m_NodeChecksum{};

        ZMath::float3 m_BBox[2] = {{0.f,0.f,0.f}, {0.f,0.f,0.f}};
        ZMath::float3 m_BBoxCollision[2] = {{0.f,0.f,0.f}, {0.f,0.f,0.f}};
        ZMath::float3 m_RootNodeTranslation = {0.f,0.f,0.f};

        /*
         *  source file information
         */
        struct ModelSource
        {
          uint32_t    version;
          ZMath::float4 c[4];
          std::string source;
        };

        ModelSource mdm;
        ModelSource mdh;
    };
}  // namespace ZenLoad
