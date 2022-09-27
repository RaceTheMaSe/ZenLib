#pragma once
#include <vector>
#include "zTypes.h"
#include "utils/alignment.h"
#include "utils/mathlib.h"

namespace VDFS
{
class FileIndex;
}

namespace ZenLoad
{
  /**
   * Helper structs for version independend loading of polygon data
   */
  template <typename FT>
  struct polyData1 {
    int16_t materialIndex{};  // -1 if none
    int16_t lightmapIndex{};  // -1 if none
    zTPlane polyPlane;
    FT flags;
    uint8_t polyNumVertices{};
    };

  #pragma pack(push, 1)
  template <typename FT>
  struct polyData1Packed {
    int16_t materialIndex{};  // -1 if none
    int16_t lightmapIndex{};  // -1 if none
    zTPlane polyPlane;
    FT flags;
    uint8_t polyNumVertices{};
    };
  #pragma pack(pop)

  template <typename IT, typename FT>
  struct polyData2 : public polyData1<FT>
  {
    /**
    * @brief ugly helper-constructor to get from the specific G1/G2-format to a generic one
    */
    template <typename IT2, typename FT2>
    void from(const polyData2<IT2, FT2>& src) {
      for (size_t i = 0; i < src.polyNumVertices; i++) {
        indices[i].VertexIndex = static_cast<IT>(src.indices[i].VertexIndex);
        indices[i].FeatIndex   = src.indices[i].FeatIndex;
        }
      polyData1<FT>::materialIndex   = src.materialIndex;
      polyData1<FT>::lightmapIndex   = src.lightmapIndex;
      polyData1<FT>::polyPlane       = src.polyPlane;
      polyData1<FT>::polyNumVertices = src.polyNumVertices;
      polyData1<FT>::flags           = src.flags.generify();
      }

    polyData2() {
      memset(&indices, 0, sizeof(indices));
      }

    struct Index {
      IT       VertexIndex;
      uint32_t FeatIndex;
      };

#pragma pack(push, 1)
    struct IndexPacked {
      IT       VertexIndex;
      uint32_t FeatIndex;
      };
#pragma pack(pop)

    void read(const uint8_t* _data) {
      const void*& data = (const void*&)_data;
      Utils::unalignedRead(polyData1<FT>::materialIndex  , data);
      Utils::unalignedRead(polyData1<FT>::lightmapIndex  , data);
      Utils::unalignedRead(polyData1<FT>::polyPlane      , data);
      Utils::unalignedRead(polyData1<FT>::flags          , data);
      Utils::unalignedRead(polyData1<FT>::polyNumVertices, data);
      for (int i = 0; i < polyData1<FT>::polyNumVertices; i++) {
        Utils::unalignedRead(indices[i].VertexIndex      , data);
        Utils::unalignedRead(indices[i].FeatIndex        , data);
        }
      }

    Index indices[255];
  };

class ZenParser;

class zCMesh  {
  public:
    zCMesh() = default;

    /**
       * @brief Loads the mesh from the given VDF-Archive
       */
    zCMesh(const std::string& fileName, VDFS::FileIndex& fileIndex);

    /**
       * @brief Reads the mesh-object from the given binary stream
       * @param parser Parser reference
       * @param skipPolys These polygons will be skipped while loading. These need to be ordered! (Used to load world witout LOD in G1)
       * @param forceG132bitIndices Loads all indices as 32-bit, even though the ZEN may come from Gothic 1
       */
    void readObjectData(ZenParser& parser, const std::vector<size_t>& skipPolys = std::vector<size_t>(),
                        bool forceG132bitIndices = false);

    /**
       * @brief Reads the oriented bounding boxes
       * @param parser Parser reference
       * @param box Box object
       */
    void readObb(ZenParser& parser, zCOBBox3D& box);

    /**
       * Simply skips all data found here
       * @param parser Parser reference
       */
    static void skip(ZenParser& parser);

    /**
       * @brief Creates packed submesh-data
       */
    void packMesh(PackedMesh& mesh, float scale, bool removeDoubles);

    /**
      @ brief returns the vector of vertex-positions
      */
    const std::vector<ZMath::float3>& getVertices() const { return m_Vertices; }

    /**
      @ brief returns the vector of features
      */
    const std::vector<zTMSH_FeatureChunk>& getFeatures() const { return m_Features; }

    /**
       * @brief returns the vector of triangle-indices
       */
    const std::vector<uint32_t>& getIndices() const { return m_Indices; }

    /**
       * @brief returns the vector of triangle-indices
       */
    const std::vector<uint32_t>& getFeatureIndices() const { return m_FeatureIndices; }

    /**
       * @brief returns the vector of triangle-material-indices
       */
    const std::vector<int16_t>& getTriangleMaterialIndices() const { return m_TriangleMaterialIndices; }

    /**
      * @brief returns the vector of triangle-material-indices
      */
    const std::vector<int16_t>& getTriangleLightmapIndices() const { return m_TriangleLightmapIndices; }

    /**
       * @brief returns the vector of the materials used by this mesh
       */
    const std::vector<zCMaterialData>& getMaterials() const { return m_Materials; }

    /**
       * @brief getter for the boudingboxes
       */
    void getBoundingBox(ZMath::float3& min, ZMath::float3& max) const
    {
      min = m_BBMin;
      max = m_BBMax;
    }

    /**
       * @brief getter for the boudingboxes
       */
    const zCOBBox3D& getOrientedBoundingBox() const
    {
      return m_obb;
    }

    /**
       * @brief getter for the lightmap texture data
       */
    const std::vector<uint8_t>& getLightmapTexture(size_t index) const
    {
      return m_lightMapTextures[index];
    }

    /**
       * @brief getter for the lightmap entries
       */
    const zCLightMap& getLightmap(size_t index) const
    {
      return m_lightMaps[index];
    }

  private:
    /**
       * @brief vector of vertex-positions for this mesh
       */
    std::vector<ZMath::float3> m_Vertices;

    /**
       * @brief Featues for the vertices with the corresponding index
       */
    std::vector<zTMSH_FeatureChunk> m_Features;

    /**
       * @brief indices for the triangles of the mesh
       */
    std::vector<uint32_t> m_Indices;

    /**
      * @brief indices for the triangles of the mesh
      */
    std::vector<uint32_t> m_FeatureIndices;

    /**
       * @brief Triangles of the current mesh, containing flags
       */
    std::vector<WorldTriangle> m_Triangles;

    /**
       * @brief Textures for the triangles. Divide index-position by 3 to get the
       * corresponding material-info
       */
    std::vector<int16_t> m_TriangleMaterialIndices;

    /**
       * @brief Index of the lightmap used by the triangles in order. -1 of no lightmap registered.
       */
    std::vector<int16_t> m_TriangleLightmapIndices;

    /**
       * @brief All materials used by this mesh
       */
    std::vector<zCMaterialData> m_Materials;

    /**
       * @brief Bounding-box of this mesh
       */
    ZMath::float3 m_BBMin;
    ZMath::float3 m_BBMax;

    /**
       * @brief Oriented bounding box of this mesh
       */
    zCOBBox3D m_obb;

    /**
       * @brief Lightmap textures - dds format
       */
    std::vector<std::vector<uint8_t>> m_lightMapTextures;

    /**
       * @brief Lightmap entries
       */
    std::vector<zCLightMap> m_lightMaps;

    /**
		  * @brief Whether this mesh is using alphatest
		  */
    uint8_t m_IsUsingAlphaTest;
  };
}  // namespace ZenLoad
