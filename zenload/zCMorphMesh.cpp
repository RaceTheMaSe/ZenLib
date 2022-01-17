#include "zCMorphMesh.h"
#include <string>
#include "zCProgMeshProto.h"
#include "zTypes.h"
#include "zenParser.h"
#include "utils/alignment.h"
#include "utils/logger.h"
#include "vdfs/fileIndex.h"

#include <fstream>

using namespace ZenLoad;

enum MSID_CHUNK : uint16_t {
  MSID_NONE        = 0,
  MMID_MMB_HEADER  = 0xE020,
  MMID_MMB_ANILIST = 0xE030,
  MMID_UNKNOWN0    = 0xE000,
  MMID_SOURCE_INFO = 0xE010
  };

zCMorphMesh::zCMorphMesh(const std::string& fileName, const VDFS::FileIndex& fileIndex) {
  std::vector<uint8_t> data;
  fileIndex.getFileData(fileName, data);

  if(data.empty()) {
    LogInfo() << "Failed to find morph mesh " << fileName;
    return;
    }

  ZenLoad::ZenParser parser(data.data(), data.size());
  readObjectData(parser);
  }

/**
* @brief Reads the mesh-object from the given binary stream
*/
void zCMorphMesh::readObjectData(ZenParser& parser) {
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

    switch (chunkInfo.id) {
      case MMID_MMB_HEADER: {
        uint32_t version = parser.readBinaryDWord(); (void)version;

        std::string morphProtoName = parser.readLine(true);

        // Read source-mesh
        m_Mesh.readObjectData(parser);

        morphPositions.resize(m_Mesh.getVertices().size());
        parser.readBinaryRaw(morphPositions.data(), sizeof(ZMath::float3) * morphPositions.size());
        break;
        }

      case MMID_MMB_ANILIST: {
        doneReadingChunks = true;
        uint16_t aniTotal=0;
        parser.readBinaryRaw(&aniTotal,2);
        aniList.resize(aniTotal);
        for(auto& i:aniList) {
          i.name = parser.readString(false);
          parser.readBinaryRaw(&i.blendIn, 4);
          parser.readBinaryRaw(&i.blendOut,4);
          parser.readBinaryRaw(&i.duration,4);
          parser.readBinaryRaw(&i.layer,   4);

          parser.readBinaryRaw(&i.speed, 4);
          parser.readBinaryRaw(&i.flags, 1);

          uint32_t indexSz=parser.readBinaryDWord();
          i.numFrames = parser.readBinaryDWord();

          i.vertexIndex.resize(indexSz);
          i.samples.resize((size_t)i.numFrames*indexSz);
          parser.readBinaryRaw(i.vertexIndex.data(),(size_t)indexSz*4);
          static_assert(sizeof(i.samples[0])==12,"invalid ani sample size");
          parser.readBinaryRaw(i.samples.data(),(size_t)i.numFrames*indexSz*12);
          }

        parser.setSeek(chunkEnd);
        break;
        }

      case MMID_UNKNOWN0:
        if(chunkInfo.length==0)
          parser.setSeek(chunkEnd);
        else {
          LogInfo() << "Unhandled morph mesh block data of size " << chunkInfo.length << " bytes";
          parser.setSeek(chunkEnd);
        }
        break;
      case MMID_SOURCE_INFO: {
        uint16_t size=0;
        parser.readBinaryRaw(&size,2);
        sourceInfos.resize(size);
        for(size_t i=0;i<size;++i) {
          sourceInfos[i].year   = parser.readBinaryDWord();
          sourceInfos[i].month  = parser.readBinaryWord();
          sourceInfos[i].day    = parser.readBinaryWord();
          sourceInfos[i].hour   = parser.readBinaryWord();
          sourceInfos[i].minute = parser.readBinaryWord();
          sourceInfos[i].second = parser.readBinaryWord();
          sourceInfos[i].data   = parser.readBinaryWord();
          sourceInfos[i].name   = parser.readLine();
          }
        parser.setSeek(chunkEnd);
        break;
        }

      // Morphmeshes don't have a real end-tag. The anilist is the last one, however
      default:
        parser.setSeek(chunkEnd);  // Skip chunk
      }
    }
  if(sourceInfos.size()==aniList.size()) {
    for(size_t i=0;i<aniList.size();++i)
      aniList[i].sourceData=std::move(sourceInfos[i]);
    sourceInfos.clear();
    }

  if(parser.getRemainBytes()>0)
    LogInfo() << parser.getRemainBytes() << " of morph mesh skipped";
  if(sourceInfos.size())
    LogInfo() << "Couldn't match morph animation meta data";
  }
