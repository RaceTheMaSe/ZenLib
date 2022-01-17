#include "zCModelMeshLib.h"
#include <algorithm>
#include <cfloat>
#include <string>
#include "parserImpl.h"
#include "zCMeshSoftSkin.h"
#include "zTypes.h"
#include "zenParser.h"
#include "utils/logger.h"
#include "utils/mathlib.h"
#include "vdfs/fileIndex.h"

using namespace ZenLoad;

// Types of chunks we will find in a zCModelMeshLib-Section

//const uint32_t MDM_VERSION = 67699974;  // TODO: Calculate this!

const uint16_t MLID_MODELMESH = 0xD000;
const uint16_t MLID_MDM_SOURCE = 0xD010;
const uint16_t MLID_MDM_NODEMESHES = 0xD020;
const uint16_t MLID_MDM_SOFSKINLIST = 0xD030;
const uint16_t MLID_MDM_END = 0xD040;

//const uint32_t zMDH_VERS = 3;

const uint16_t MLID_MODELHIERARCHY = 0xD100;
const uint16_t MLID_MDH_SOURCE = 0xD110;
const uint16_t MLID_MDH_END = 0xD120;

/**
* @brief Loads the lib from the given VDF-Archive
*/
zCModelMeshLib::zCModelMeshLib(const std::string& fileName, const VDFS::FileIndex& fileIndex) {
  std::vector<uint8_t> data;
  fileIndex.getFileData(fileName, data);

  if(data.empty())
    return;  // TODO: Throw an exception or something

  // Create parser from memory
  ZenLoad::ZenParser parser(data.data(), data.size());

  if (fileName.find(".MDM") != std::string::npos)
      loadMDM(parser);
  else if (fileName.find(".MDH") != std::string::npos)
      loadMDH(parser);
  else if (fileName.find(".MDL") != std::string::npos)
      loadMDL(parser);
  }

/**
* @brief Reads the mesh-object from the given binary stream
*/
void zCModelMeshLib::loadMDM(ZenParser& parser) {
  if(parser.getRemainBytes()==0)
    return;

  // Information about a single chunk
  BinaryChunkInfo chunkInfo{};

  // Read chunks until we left the virtual binary file or got to the end-chunk
  // Each chunk starts with a header (BinaryChunkInfo) which gives information
  // about what to do and how long the chunk is
  bool doneReadingChunks = false;
  while (!doneReadingChunks)
  {
    // Read chunk header and calculate position of next chunk
    parser.readStructure(chunkInfo);

    size_t chunkEnd = parser.getSeek() + chunkInfo.length;

    switch (chunkInfo.id) {
      case MLID_MODELMESH: {
        mdm.version = parser.readBinaryDWord();
        parser.setSeek(chunkEnd);  // Skip chunk
        }
      break;

      case MLID_MDM_SOURCE: {
        mdm.c[0].fromABGR8(parser.readBinaryDWord());
        mdm.c[1].fromABGR8(parser.readBinaryDWord());
        mdm.c[1].fromABGR8(parser.readBinaryDWord());
        mdm.c[1].fromABGR8(parser.readBinaryDWord());
        mdm.source = parser.readString();
        // TODO: Implement MDM-Conversion - data is read but unused
        parser.setSeek(chunkEnd);  // Skip chunk
        }
      break;

      case MLID_MDM_NODEMESHES: {
        uint16_t numNodes = parser.readBinaryWord();

        //std::vector<zCModelNodeInst> nodeList(numNodes);
        m_NodeAttachments.resize(numNodes);

        for (uint16_t i = 0; i < numNodes; i++)
          m_NodeAttachments[i].first = parser.readLine(true);

        for (uint16_t i = 0; i < numNodes; i++)
          m_NodeAttachments[i].second.readObjectData(parser);
        }
      break;

      case MLID_MDM_SOFSKINLIST: {
        uint32_t checksum = parser.readBinaryDWord(); (void)checksum;
        uint16_t numSoftSkins = parser.readBinaryWord();

        for (uint16_t i = 0; i < numSoftSkins; i++) {
          m_Meshes.emplace_back();
          m_Meshes.back().readObjectData(parser);
          }
        }
      break;

      case MLID_MDM_END:
        doneReadingChunks = true;
        break;
      default:
        parser.setSeek(chunkEnd);  // Skip chunk
      }
    }
  }

/**
* @brief Reads the model hierachy from a file (MDH-File)
*/
void zCModelMeshLib::loadMDH(ZenParser& parser) {
  if(parser.getRemainBytes()==0)
    return;

  // Information about a single chunk
  BinaryChunkInfo chunkInfo{};

  // Read chunks until we left the virtual binary file or got to the end-chunk
  // Each chunk starts with a header (BinaryChunkInfo) which gives information
  // about what to do and how long the chunk is
  bool doneReadingChunks = false;
  while(!doneReadingChunks) {
    // Read chunk header and calculate position of next chunk
    parser.readStructure(chunkInfo);

    size_t chunkEnd = parser.getSeek() + chunkInfo.length;

    switch (chunkInfo.id) {
      case MLID_MODELHIERARCHY: {
        mdh.version = parser.readBinaryDWord();
        //if(mdh_version != MDM_VERSION)
        //  LogWarn() << "MDM Version mismatch!";

        uint16_t numNodes = parser.readBinaryWord();
        m_Nodes.resize(numNodes);

        for (uint16_t i = 0; i < numNodes; i++) {
          m_Nodes[i].name = parser.readLine(false);
          m_Nodes[i].parentIndex = parser.readBinaryWord();

          if (m_Nodes[i].parentIndex != 0xFFFF) {
            if (m_Nodes[i].parentIndex >= numNodes)
                LogWarn() << "MSH-Parse fail: parentIndex >= numNodes";
            }
          else {
            m_Nodes[i].parentIndex = 0xFFFF;

            // Found root node
            m_RootNodes.push_back(i);
            }

          parser.readBinaryRaw(&m_Nodes[i].transformLocal, sizeof(ZMath::Matrix));
          m_Nodes[i].transformLocal.Transpose();

          ZMath::float3 t = m_Nodes[i].transformLocal.Translation();
          m_Nodes[i].transformLocal.Translation(t);
          }

        parser.readBinaryRaw((ZMath::float3*)m_BBox, sizeof(m_BBox));
        parser.readBinaryRaw((ZMath::float3*)m_BBoxCollision, sizeof(m_BBoxCollision));

        parser.readBinaryRaw(&m_RootNodeTranslation, sizeof(m_RootNodeTranslation));

        m_NodeChecksum = parser.readBinaryDWord();

        parser.setSeek(chunkEnd);  // Skip chunk
        }
      break;

      case MLID_MDH_SOURCE: {
        mdh.c[0].fromABGR8(parser.readBinaryDWord());
        mdh.c[1].fromABGR8(parser.readBinaryDWord());
        mdh.c[1].fromABGR8(parser.readBinaryDWord());
        mdh.c[1].fromABGR8(parser.readBinaryDWord());
        mdh.source = parser.readString();
        // TODO: Implement MDH-Conversion - data is read but unused
        parser.setSeek(chunkEnd);  // Skip chunk
        }
      break;

      case MLID_MDH_END:
        doneReadingChunks = true;
        break;
      default:
        parser.setSeek(chunkEnd);  // Skip chunk
      }
    }
  }

/**
* @brief reads this lib as MDL
*/
void zCModelMeshLib::loadMDL(ZenParser& parser) {
  loadMDH(parser);
  loadMDM(parser);
  }

/**
* @brief Creates packed submesh-data
*/
void zCModelMeshLib::packMesh(PackedSkeletalMesh& mesh) const {
  for (const auto& m : m_Meshes)
    m.packMesh(mesh);

  mesh.bbox[0] = {FLT_MAX, FLT_MAX, FLT_MAX};
  mesh.bbox[1] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

  // Choose the biggest BBox possible (From model hierarchy or soft-meshes)
  for (const auto& m : m_Meshes) {
    ZMath::float3 min, max;
    m.getAABBTotal(min, max);

    mesh.bbox[0].x = std::min(mesh.bbox[0].x, min.x);
    mesh.bbox[0].y = std::min(mesh.bbox[0].y, min.y);
    mesh.bbox[0].z = std::min(mesh.bbox[0].z, min.z);

    mesh.bbox[1].x = std::max(mesh.bbox[1].x, max.x);
    mesh.bbox[1].y = std::max(mesh.bbox[1].y, max.y);
    mesh.bbox[1].z = std::max(mesh.bbox[1].z, max.z);
    }

  mesh.bbox[0].x = std::min(mesh.bbox[0].x, m_BBox[0].x);
  mesh.bbox[0].y = std::min(mesh.bbox[0].y, m_BBox[0].y);
  mesh.bbox[0].z = std::min(mesh.bbox[0].z, m_BBox[0].z);

  mesh.bbox[1].x = std::max(mesh.bbox[1].x, m_BBox[1].x);
  mesh.bbox[1].y = std::max(mesh.bbox[1].y, m_BBox[1].y);
  mesh.bbox[1].z = std::max(mesh.bbox[1].z, m_BBox[1].z);
  }

size_t zCModelMeshLib::findNodeIndex(const std::string& nodeName) const {
  for (size_t i = 0; i < m_Nodes.size(); i++) {
    if (m_Nodes[i].name == nodeName)
      return i;
    }

  return static_cast<size_t>(-1);
  }
