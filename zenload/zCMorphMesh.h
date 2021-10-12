#pragma once
#include <vector>
#include "zCProgMeshProto.h"
#include "zTypes.h"
#include "utils/mathlib.h"

namespace VDFS
{
class FileIndex;
}

namespace ZenLoad
{
class ZenParser;
class zCMorphMesh {
  public:
    struct SourceInfo {
      int32_t  year  =0;
      int16_t  month =0;
      int16_t  day   =0;
      int16_t  hour  =0;
      int16_t  minute=0;
      int16_t  second=0;
      int16_t  data  =0; // FIXME: or padding ... only zero data in files
      std::string name;
      };

    struct Animation {
      std::string                name;
      int32_t                    layer    = 0;
      float                      blendIn  = 0;
      float                      blendOut = 0;
      float                      duration = 0;
      float                      speed    = 0;
      uint8_t                    flags    = 0;

      std::vector<uint32_t>      vertexIndex;
      std::vector<ZMath::float3> samples;
      uint32_t                   numFrames=0;
      SourceInfo                 sourceData;
      };

    zCMorphMesh() = default;

    /**
     * @brief Loads the mesh from the given VDF-Archive
     */
    zCMorphMesh(const std::string& fileName, const VDFS::FileIndex& fileIndex);

    /**
     * @brief Reads the mesh-object from the given binary stream
     * @param parser ZenParser object
     */
    void readObjectData(ZenParser& parser);

    /**
     * @return Internal zCProgMeshProto of this soft skin. The soft-skin only displaces the vertices found in the ProgMesh.
     */
    const zCProgMeshProto& getMesh() const { return m_Mesh; }

    /**
     * @return Animation of this soft skin. The soft-skin only displaces the vertices found in the ProgMesh.
     */
    const std::vector<Animation>& getAnimations() const { return aniList; }

    /**
     * Internal morph positions
     */
    const std::vector<ZMath::float3>& getMorphPositions() const { return morphPositions; }

  private:
    /**
     * @brief Internal zCProgMeshProto of this soft skin. The soft-skin only displaces the vertices found in the ProgMesh.
     */
    zCProgMeshProto m_Mesh;

    /**
     * @brief Internal animation list of this morph mesh
     */
    std::vector<Animation> aniList;

    /**
     * unused morph positions
     */
    std::vector<ZMath::float3> morphPositions;

    /*
     * unknown block data - FIXME: whats this
     */
    struct Unknown00 {
      int32_t     data0=0;
      int16_t     data1=0;
      int32_t     data2=0;
      std::string data3={};
      std::vector<int32_t> data4={};
    } unkown0;

    std::vector<SourceInfo> sourceInfos;
  };
}  // namespace ZenLoad
