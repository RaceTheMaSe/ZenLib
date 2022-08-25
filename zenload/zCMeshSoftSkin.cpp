#include "zCMeshSoftSkin.h"

#include <cmath>
#include <algorithm>
#include <cfloat>
#include <string>
#include "zCProgMeshProto.h"
#include "zTypes.h"
#include "zenParser.h"
#include "utils/alignment.h"
#include "utils/logger.h"
#include "vdfs/fileIndex.h"

using namespace ZenLoad;

enum MSID_CHUNK : uint16_t {
  MSID_NONE             = 0,
  MSID_MESHSOFTSKIN     = 0xE100,
  MSID_MESHSOFTSKIN_END = 0xE110,

  MSID_PROGMESH         = 0xB100,
  MSID_PROGMESH_END     = 0xB1FF,
  };

void oBBox3d::load(ZenParser& parser) {
  parser.readBinaryRaw(&center, sizeof(center));

  parser.readBinaryRaw(&axis, sizeof(axis));
  parser.readBinaryRaw(&extends, sizeof(extends));

  uint16_t numChildren = parser.readBinaryWord();
  children.resize(numChildren);
  for(auto& i:children)
    i.load(parser);
  }

void oBBox3d::getAABB(ZMath::float3& min, ZMath::float3& max) const
{
  const float sign[8][3] = {
    {-1, -1, -1},
    {-1, -1, +1},
    {-1, +1, -1},
    {-1, +1, +1},
    {+1, -1, -1},
    {+1, -1, +1},
    {+1, +1, -1},
    {+1, +1, +1}};

  min = {FLT_MAX, FLT_MAX, FLT_MAX};
  max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

  for (auto i:sign) {
    ZMath::float3 point = center;
    const ZMath::float3 axis0 = axis[0] * extends.x * i[0];
    const ZMath::float3 axis1 = axis[1] * extends.y * i[1];
    const ZMath::float3 axis2 = axis[2] * extends.z * i[2];

    point.x += axis0.x + axis1.x + axis2.x;
    point.y += axis0.y + axis1.y + axis2.y;
    point.z += axis0.z + axis1.z + axis2.z;

    min.x = std::min(min.x, point.x);
    min.y = std::min(min.y, point.y);
    min.z = std::min(min.z, point.z);

    max.x = std::max(max.x, point.x);
    max.y = std::max(max.y, point.y);
    max.z = std::max(max.z, point.z);
    }
  }

/**
* @brief Reads the mesh-object from the given binary stream
*/
void zCMeshSoftSkin::readObjectData(ZenParser& parser) {
  // Information about a single chunk
  BinaryChunkInfo chunkInfo{};

  // Read chunks until we left the virtual binary file or got to the end-chunk
  // Each chunk starts with a header (BinaryChunkInfo) which gives information
  // about what to do and how long the chunk is
  bool doneReadingChunks = false;
  while (!doneReadingChunks) {
    // Read chunk header and calculate position of next chunk
    parser.readStructure(chunkInfo);

    size_t chunkEnd = parser.getSeek() + chunkInfo.length;

    switch(MSID_CHUNK(chunkInfo.id)) {
      case MSID_MESHSOFTSKIN: {
        version = parser.readBinaryDWord();

        m_Mesh.readObjectData(parser);

        uint32_t vertexWeightStreamSize = parser.readBinaryDWord();

        m_VertexWeightStream.resize(vertexWeightStreamSize);
        parser.readBinaryRaw(m_VertexWeightStream.data(), vertexWeightStreamSize);

        uint32_t numNodeWedgeNormals = parser.readBinaryDWord();
        std::vector<zTNodeWedgeNormal> nodeWedgeNormals(numNodeWedgeNormals);
        parser.readBinaryRaw(nodeWedgeNormals.data(), numNodeWedgeNormals * sizeof(zTNodeWedgeNormal));

        uint16_t numNodes = parser.readBinaryWord();
        std::vector<int32_t> nodeList(numNodes);
        parser.readBinaryRaw(nodeList.data(), numNodes * sizeof(int32_t));

        m_BBoxesByNodes.resize(numNodes);
        for(auto& i:m_BBoxesByNodes)
          i.load(parser);

        updateBboxTotal();
        parser.setSeek(chunkEnd); // Skip chunk
        break;
        }

      case MSID_MESHSOFTSKIN_END:
        doneReadingChunks = true;
        break;

      case MSID_PROGMESH:
      case MSID_PROGMESH_END:
        parser.setSeek(chunkEnd);    // Not supported: skip chunk
        break;

      default:
        parser.setSeek(chunkEnd);  // Skip chunk
      }
    }
  }

/**
* @brief Creates packed submesh-data
*/
void zCMeshSoftSkin::packMesh(PackedSkeletalMesh& mesh) const {
  std::vector<SkeletalVertex> vertices(m_Mesh.getVertices().size());
  mesh.bbox[0] = m_BBoxTotal[0];
  mesh.bbox[1] = m_BBoxTotal[1];

  // Extract weights and local positions
  const uint8_t* stream = m_VertexWeightStream.data();
  //LogInfo() << "Stream size: " << m_VertexWeightStream.size();

  for(auto & vertex:vertices) {
    // Layout:
    //  uint32_t: numWeights
    //  numWeights* zTWeightEntry: weights
    uint32_t numWeights = 0;
    std::memcpy(&numWeights,stream,sizeof(numWeights)); stream+=sizeof(numWeights);

    for(size_t j=0; j<numWeights; j++) {
      float weight = 0;
      std::memcpy(&weight,stream,sizeof(weight)); stream+=sizeof(weight);

      ZMath::float3 localVertexPosition;
      std::memcpy(&localVertexPosition,stream,sizeof(localVertexPosition)); stream+=sizeof(localVertexPosition);

      uint8_t nodeIndex = 0;
      std::memcpy(&nodeIndex,stream,sizeof(nodeIndex)); stream+=sizeof(nodeIndex);

      vertex.BoneIndices[j]    = nodeIndex;
      vertex.LocalPositions[j] = localVertexPosition;
      vertex.Weights[j]        = weight;
      }
    }

  size_t vboSize = 0;
  size_t iboSize = 0;
  for(size_t s=0; s<m_Mesh.getNumSubmeshes(); s++) {
    const zCProgMeshProto::SubMesh& sm = m_Mesh.getSubmesh(s);
    vboSize += sm.m_WedgeList.size();
    iboSize += sm.m_TriangleList.size()*3;
    }

  mesh.vertices.resize(vboSize);
  mesh.indices .resize(iboSize);
  mesh.subMeshes.resize(m_Mesh.getNumSubmeshes());
  auto* vbo = mesh.vertices.data();
  auto* ibo = mesh.indices.data();

  uint32_t meshVxStart = 0, iboStart = 0;
  for(size_t s=0; s<m_Mesh.getNumSubmeshes(); s++) {
    const zCProgMeshProto::SubMesh& sm = m_Mesh.getSubmesh(s);
    // Get data
    for(const auto & wedge:sm.m_WedgeList) {
        SkeletalVertex v     = vertices[wedge.m_VertexIndex];

      v.Normal   = wedge.m_Normal;
      v.TexCoord = wedge.m_Texcoord;
      v.Color    = sm.m_Material.color;
      *vbo = v;
      ++vbo;
      }

    // And get the indices
    for(auto i:sm.m_TriangleList) {
      for(auto m_Wedge:i.m_Wedges) {
        *ibo = m_Wedge + meshVxStart;
        ++ibo;
        }
      }

    auto& pack = mesh.subMeshes[s];
    pack.indexOffset = iboStart;
    pack.indexSize   = sm.m_TriangleList.size()*3;
    pack.material    = sm.m_Material;
    meshVxStart += uint32_t(sm.m_WedgeList.size());
    iboStart    += uint32_t(sm.m_TriangleList.size()*3);
    }
  }

void zCMeshSoftSkin::updateBboxTotal() {
  m_BBoxTotal[0] = { FLT_MAX,  FLT_MAX,  FLT_MAX};
  m_BBoxTotal[1] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

  {
    ZMath::float3 min, max;
    m_Mesh.getBoundingBox(min,max);

    m_BBoxTotal[0].x = std::min(m_BBoxTotal[0].x, min.x);
    m_BBoxTotal[0].y = std::min(m_BBoxTotal[0].y, min.y);
    m_BBoxTotal[0].z = std::min(m_BBoxTotal[0].z, min.z);

    m_BBoxTotal[1].x = std::max(m_BBoxTotal[1].x, max.x);
    m_BBoxTotal[1].y = std::max(m_BBoxTotal[1].y, max.y);
    m_BBoxTotal[1].z = std::max(m_BBoxTotal[1].z, max.z);
  }

  for (const oBBox3d& bb : m_BBoxesByNodes) {
    ZMath::float3 min, max;
    bb.getAABB(min, max);

    m_BBoxTotal[0].x = std::min(m_BBoxTotal[0].x, min.x);
    m_BBoxTotal[0].y = std::min(m_BBoxTotal[0].y, min.y);
    m_BBoxTotal[0].z = std::min(m_BBoxTotal[0].z, min.z);

    m_BBoxTotal[1].x = std::max(m_BBoxTotal[1].x, max.x);
    m_BBoxTotal[1].y = std::max(m_BBoxTotal[1].y, max.y);
    m_BBoxTotal[1].z = std::max(m_BBoxTotal[1].z, max.z);
    }
  }

void zCMeshSoftSkin::getAABBTotal(ZMath::float3& min, ZMath::float3& max) const {
  min = m_BBoxTotal[0];
  max = m_BBoxTotal[1];
  }
