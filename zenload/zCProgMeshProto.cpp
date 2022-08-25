#include "zCProgMeshProto.h"
#include <algorithm>
#include <string>
#include "zCMaterial.h"
#include "zTypes.h"
#include "zenParser.h"
#include "utils/logger.h"
#include "vdfs/fileIndex.h"
#include "utils/fixWindingOrder.h"

using namespace ZenLoad;

static const uint16_t zCPROGMESH_FILE_VERS_G2 = 0x0905;
static const uint16_t MSID_PROGMESH = 0xB100;
static const uint16_t MSID_PROGMESH_END = 0xB1FF;

struct MeshDataEntry
{
  uint32_t offset;
  uint32_t size;
};

struct MeshOffsetsMain
{
  MeshDataEntry position;
  MeshDataEntry normal;
};

struct MeshOffsetsSubMesh
{
  MeshDataEntry triangleList;
  MeshDataEntry wedgeList;
  MeshDataEntry colorList;
  MeshDataEntry trianglePlaneIndexList;
  MeshDataEntry trianglePlaneList;
  MeshDataEntry wedgeMap;
  MeshDataEntry vertexUpdates;
  MeshDataEntry triangleEdgeList;
  MeshDataEntry edgeList;
  MeshDataEntry edgeScoreList;
};

/**
* @brief Loads the mesh from the given VDF-Archive
*/
zCProgMeshProto::zCProgMeshProto(const std::string& fileName, const VDFS::FileIndex& fileIndex) {
  std::vector<uint8_t> data;
  fileIndex.getFileData(fileName, data);

  if (data.empty()) {
    LogInfo() << "Failed to find progMesh " << fileName;
    return;  // TODO: Throw an exception or something
    }

  try {
    // Create parser from memory
    ZenLoad::ZenParser parser(data.data(), data.size());

    readObjectData(parser);
    }
  catch (std::exception& e) {
    LogError() << e.what();
    return;
    }
  }

/**
* @brief Reads the mesh-object from the given binary stream
*/
void zCProgMeshProto::readObjectData(ZenParser& parser) {
  // Information about a single chunk
  BinaryChunkInfo chunkInfo{};

  bool doneReadingChunks = false;
  while (!doneReadingChunks && parser.getSeek() <= parser.getFileSize()) {
    // Read chunk header and calculate position of next chunk
    parser.readStructure(chunkInfo);

    size_t chunkEnd = parser.getSeek() + chunkInfo.length;
    switch (chunkInfo.id) {
      case MSID_PROGMESH: {
        uint16_t version = parser.readBinaryWord();
        /*if(version != zCPROGMESH_FILE_VERS_G2)
          LogWarn() << "Unsupported zCProgMeshProto-Version: " << version; */

        // Read data-pool
        uint32_t dataSize = parser.readBinaryDWord();
        std::vector<uint8_t> dataPool;
        dataPool.resize(dataSize);
        parser.readBinaryRaw(dataPool.data(), dataSize);

        // Read how many submeshes we got
        uint8_t numSubmeshes = parser.readBinaryByte();

        // Read data offsets for the main position/normal-list
        MeshOffsetsMain mainOffsets{};
        parser.readStructure(mainOffsets);

        // Read offsets to indices data
        std::vector<MeshOffsetsSubMesh> subMeshOffsets;
        subMeshOffsets.resize(numSubmeshes);
        parser.readBinaryRaw(subMeshOffsets.data(), numSubmeshes * sizeof(MeshOffsetsSubMesh));

        // Read materials

        // ZenArchive - Header
        // uint32_t - Num materials
        // For each material:
        //  - String - Name
        //  - zCMaterial-Chunk

        ZenParser p2(parser.getDataPtr(), parser.getRemainBytes());
        p2.readHeader();

        // Read every stored material
        for (uint32_t i = 0; i < numSubmeshes; i++) {
          std::string mname = p2.readLine(false);  // Read unused material name (Stored a second time later)

          // Skip chunk headers - we know these are zCMaterial
          uint32_t chunksize   = p2.readBinaryDWord();
          uint16_t version     = p2.readBinaryWord();
          uint32_t objectIndex = p2.readBinaryDWord();
          (void)chunksize;
          (void)objectIndex;

          p2.skipSpaces();

          // Skip chunk-header
          std::string name = p2.readLine();
          std::string classname = p2.readLine();

          // Save into vector
          m_Materials.emplace_back(zCMaterial::readObjectData(p2, version));
          }

        parser.setSeek(p2.getSeek() + parser.getSeek());

        if (version == zCPROGMESH_FILE_VERS_G2) {
          // Read whether we want to have alphatesting
          m_IsUsingAlphaTest = parser.readBinaryByte() != 0;
          }
        else
          m_IsUsingAlphaTest = true;

        // Read boundingbox
        ZMath::float3 min, max;
        parser.readStructure(min);
        parser.readStructure(max);

        m_BBMin = ZMath::float3(min.x, min.y, min.z);
        m_BBMax = ZMath::float3(max.x, max.y, max.z);

        // Extract data
        m_Vertices.resize(mainOffsets.position.size);
        m_Normals.resize(mainOffsets.normal.size);

        // Copy vertex-data
        memcpy(m_Vertices.data(), &dataPool[mainOffsets.position.offset], sizeof(float) * 3 * mainOffsets.position.size);
        memcpy(m_Normals.data(), &dataPool[mainOffsets.normal.offset], sizeof(float) * 3 * mainOffsets.normal.size);

        // Copy submesh-data
        m_SubMeshes.resize(numSubmeshes);
        for (uint32_t i = 0; i < numSubmeshes; i++) {
          auto& d = subMeshOffsets[i];

          m_SubMeshes[i].m_Material = m_Materials[i];
          m_SubMeshes[i].m_TriangleList.resize(d.triangleList.size);
          m_SubMeshes[i].m_WedgeList.resize(d.wedgeList.size);
          m_SubMeshes[i].m_ColorList.resize(d.colorList.size);
          m_SubMeshes[i].m_TrianglePlaneIndexList.resize(d.trianglePlaneIndexList.size);
          m_SubMeshes[i].m_TrianglePlaneList.resize(d.trianglePlaneList.size);
          m_SubMeshes[i].m_TriEdgeList.resize(d.triangleEdgeList.size);
          m_SubMeshes[i].m_EdgeList.resize(d.edgeList.size);
          m_SubMeshes[i].m_EdgeScoreList.resize(d.edgeScoreList.size);
          m_SubMeshes[i].m_WedgeMap.resize(d.wedgeMap.size);

          if (!m_SubMeshes[i].m_TriangleList.empty())
            memcpy(m_SubMeshes[i].m_TriangleList.data(), &dataPool[d.triangleList.offset], sizeof(zTriangle) * d.triangleList.size);

          if (!m_SubMeshes[i].m_WedgeList.empty())
            memcpy(m_SubMeshes[i].m_WedgeList.data(), &dataPool[d.wedgeList.offset], sizeof(zWedge) * d.wedgeList.size);

          if (!m_SubMeshes[i].m_ColorList.empty())
            memcpy(m_SubMeshes[i].m_ColorList.data(), &dataPool[d.colorList.offset], sizeof(float) * d.colorList.size);

          if (!m_SubMeshes[i].m_TrianglePlaneIndexList.empty())
            memcpy(m_SubMeshes[i].m_TrianglePlaneIndexList.data(), &dataPool[d.trianglePlaneIndexList.offset], sizeof(uint16_t) * d.trianglePlaneIndexList.size);

          if (!m_SubMeshes[i].m_TrianglePlaneList.empty())
            memcpy(m_SubMeshes[i].m_TrianglePlaneList.data(), &dataPool[d.trianglePlaneList.offset], sizeof(zTPlane) * d.trianglePlaneList.size);

          if (!m_SubMeshes[i].m_TriEdgeList.empty())
            memcpy(m_SubMeshes[i].m_TriEdgeList.data(), &dataPool[d.triangleEdgeList.offset], sizeof(zTriangleEdges) * d.triangleEdgeList.size);

          if (!m_SubMeshes[i].m_EdgeList.empty())
            memcpy(m_SubMeshes[i].m_EdgeList.data(), &dataPool[d.edgeList.offset], sizeof(zEdge) * d.edgeList.size);

          if (!m_SubMeshes[i].m_EdgeScoreList.empty())
            memcpy(m_SubMeshes[i].m_EdgeScoreList.data(), &dataPool[d.edgeScoreList.offset], sizeof(float) * d.edgeScoreList.size);

          if (!m_SubMeshes[i].m_WedgeMap.empty())
            memcpy(m_SubMeshes[i].m_WedgeMap.data(), &dataPool[d.wedgeMap.offset], sizeof(uint16_t) * d.wedgeMap.size);
          }
        }
        parser.setSeek(chunkEnd);
        break;

      case MSID_PROGMESH_END:
        doneReadingChunks = true;
        break;

      default:
        parser.setSeek(chunkEnd);
      }
    }
  }

/**
 * @brief Packs vertices only
 */
void ZenLoad::zCProgMeshProto::packVertices(std::vector<WorldVertex>& vxs, std::vector<uint32_t>& ixs, uint32_t indexStart, std::vector<uint32_t>& submeshIndexStarts, float scale) const {
  for (const auto & sm : m_SubMeshes) {
    auto meshVxStart = uint32_t(vxs.size());

    // Get data
    for (const auto & wedge : sm.m_WedgeList) {
      WorldVertex v{};
      v.Position = m_Vertices[wedge.m_VertexIndex] * scale;
      v.Normal   = wedge.m_Normal;
      v.TexCoord = wedge.m_Texcoord;
      v.Color = sm.m_Material.color;
      vxs.push_back(v);
      }

    // Mark when the submesh starts
    submeshIndexStarts.push_back(uint32_t(ixs.size()));

    // And get the indices
    for (auto i : sm.m_TriangleList) {
      for (unsigned short & m_Wedge : i.m_Wedges) {
        ixs.push_back(m_Wedge  // Take wedge-index of submesh
                      + indexStart                        // Add our custom offset
                      + meshVxStart);                     // And add the starting location of the vertices for this submesh
        }
      }
    }
  }

/**
* @brief Creates packed submesh-data
*/
void zCProgMeshProto::packMesh(PackedMesh& mesh, bool noVertexId) const {
  // Put in all materials. There could be more than there are submeshes for animated textures or headmeshes
  mesh.subMeshes.resize(std::max(m_Materials.size(), m_SubMeshes.size()));
  mesh.bbox[0]          = m_BBMin;
  mesh.bbox[1]          = m_BBMax;
  mesh.isUsingAlphaTest = m_IsUsingAlphaTest;

  size_t vboSize = 0;
  size_t iboSize = 0;
  for(auto& sm:m_SubMeshes) {
    vboSize += sm.m_WedgeList.size();
    iboSize += sm.m_TriangleList.size()*3;
    }
  mesh.triangles.clear();
  mesh.vertices.resize(vboSize);
  mesh.indices .resize(iboSize);
  if(!noVertexId)
    mesh.verticesId.resize(vboSize);

  auto* vbo = mesh.vertices.data();
  auto* ibo = mesh.indices.data();
  auto* vId = mesh.verticesId.data();

  uint32_t meshVxStart = 0;
  uint32_t meshIxStart = 0;
  for(size_t smI=0; smI<m_SubMeshes.size(); ++smI) {
    const auto& sm   = m_SubMeshes[smI];
    auto&       pack = mesh.subMeshes[smI];

    pack.material = sm.m_Material;

    for(const auto & wedge : sm.m_WedgeList) {
      vbo->Position = m_Vertices[wedge.m_VertexIndex];
      vbo->Normal   = wedge.m_Normal;
      vbo->TexCoord = wedge.m_Texcoord;
      vbo->Color    = sm.m_Material.color;
      ++vbo;

      if(!noVertexId) {
        *vId = wedge.m_VertexIndex;
        ++vId;
        }
      }

    pack.indexOffset = meshIxStart;
    pack.indexSize   = sm.m_TriangleList.size()*3;
    meshIxStart += (uint32_t)sm.m_TriangleList.size()*3;

    // And get the indices
    // pack.indices.resize(3*sm.m_TriangleList.size());
    for(auto i : sm.m_TriangleList) {
      for(unsigned short m_Wedge : i.m_Wedges) {
        uint32_t id = m_Wedge // Take wedge-index of submesh
                      + meshVxStart; // And add the starting location of the vertices for this submesh
        *ibo = id;
        ++ibo;
        //pack.indices[i*3+j] = id;
        }
      }

    meshVxStart += uint32_t(sm.m_WedgeList.size());
    }
  
  // for(size_t i=0;i<mesh.indices.size();i+=3)
  // {
  //   WorldTriangle tri;
  //   tri.vertices[0] = mesh.vertices[mesh.indices[i]];
  //   tri.vertices[1] = mesh.vertices[mesh.indices[i+1]];
  //   tri.vertices[2] = mesh.vertices[mesh.indices[i+2]];
  //   // NOTE: not all data copied to triangle structure. only as much as needed to do the winding order check and potential fix
  //   mesh.triangles.push_back(tri);
  // }

  // // make sure the winding order of all triangles is the same. otherwise for example the world mesh collision can be inconsistent
  // const auto& fixedTris = fixWindingOrder<WorldTriangle,uint32_t>(mesh.triangles,mesh.indices);
  // for(size_t i=0;i<fixedTris.size();i++)
  // {
  //   mesh.vertices[i*3+0]=fixedTris[i].vertices[0];
  //   mesh.vertices[i*3+1]=fixedTris[i].vertices[1];
  //   mesh.vertices[i*3+2]=fixedTris[i].vertices[2];
  // }
  }
