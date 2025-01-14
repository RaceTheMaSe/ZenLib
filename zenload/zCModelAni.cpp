// FIXME: COMPATIBILITY FOR MASTER - REMOVE LATER! (Complete file)

#include "zCModelAni.h"
#include <cmath>
#include <string>
#include "zCProgMeshProto.h"
#include "zTypes.h"
#include "zenParser.h"
#include <cmath>
#include "utils/logger.h"
#include "vdfs/fileIndex.h"

using namespace ZenLoad;

//static const uint16_t MSID_MODELANI = 0xA000;
static const uint16_t MSID_MAN_HEADER = 0xA020;
static const uint16_t MSID_MAN_SOURCE = 0xA010;
static const uint16_t MSID_MAN_ANIEVENTS = 0xA030;
static const uint16_t MSID_MAN_RAWDATA = 0xA090;

static const float SAMPLE_ROT_BITS = float(1 << 16) - 1.0f;
//static const float SAMPLE_ROT_SCALER = (float(1.0f) / SAMPLE_ROT_BITS) * 2.0f * ZMath::Pi;
static const float SAMPLE_QUAT_SCALER = (1.0f / SAMPLE_ROT_BITS) * 2.1f;
static const uint16_t SAMPLE_QUAT_MIDDLE = (1 << 15) - 1;

void zCModelAni::zCModelAniEvent::load(ZenParser& parser) {
  aniEventType = static_cast<zTMdl_AniEventType>(parser.readBinaryDWord());
  frameNr = parser.readBinaryDWord();
  tagString = parser.readLine(true);

  for (auto & i : string)
      i = parser.readLine(true);

  for (float & value : values)
      value = parser.readBinaryFloat();

  prob = parser.readBinaryFloat();
  }

void SampleUnpackTrans(const uint16_t* in, ZMath::float3& out, float samplePosScaler, float samplePosRangeMin) {
  out.x = float(in[0]) * samplePosScaler + samplePosRangeMin;
  out.y = float(in[1]) * samplePosScaler + samplePosRangeMin;
  out.z = float(in[2]) * samplePosScaler + samplePosRangeMin;
  };

void SampleUnpackQuat(const uint16_t* in, ZMath::float4& out) {
  out.x = (float)(int(in[0]) - SAMPLE_QUAT_MIDDLE) * SAMPLE_QUAT_SCALER;
  out.y = (float)(int(in[1]) - SAMPLE_QUAT_MIDDLE) * SAMPLE_QUAT_SCALER;
  out.z = (float)(int(in[2]) - SAMPLE_QUAT_MIDDLE) * SAMPLE_QUAT_SCALER;

  float len_q = out.x * out.x + out.y * out.y + out.z * out.z;

  if (len_q > 1.0f) {
    auto l = 1.0f / std::sqrt(len_q);
    out.x *= l;
    out.y *= l;
    out.z *= l;
    out.w = 0;
    }
  else
    out.w = std::sqrt(1.0f - len_q);
  };

/**
* @brief Loads the animation from the given VDF-Archive
*/
zCModelAni::zCModelAni(const std::string& fileName, const VDFS::FileIndex& fileIndex, float scale) {
  m_ModelAniHeader.version = 0;

  std::vector<uint8_t> data;
  fileIndex.getFileData(fileName, data);

  if (data.empty())
      return;  // TODO: Throw an exception or something

  try {
    // Create parser from memory
    ZenLoad::ZenParser parser(data.data(), data.size());

    readObjectData(parser);

    // Apply scale
    if (scale != 1.0f) {
      for (auto & m_AniSample : m_AniSamples)
        m_AniSample.position = m_AniSample.position * scale;
      }
    }
  catch (std::exception& e) {
    LogError() << e.what();
    return;
    }
}

/**
* @brief Reads the mesh-object from the given binary stream
*/
void zCModelAni::readObjectData(ZenParser& parser) {
  // Information about the whole file we are reading here
  // BinaryFileInfo fileInfo;

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
      case MSID_MAN_HEADER:
        m_ModelAniHeader.version = parser.readBinaryWord();

        m_ModelAniHeader.aniName = parser.readLine(true);

        m_ModelAniHeader.layer = parser.readBinaryDWord();
        m_ModelAniHeader.numFrames = parser.readBinaryDWord();
        m_ModelAniHeader.numNodes = parser.readBinaryDWord();
        m_ModelAniHeader.fpsRate = parser.readBinaryFloat();
        m_ModelAniHeader.fpsRateSource = parser.readBinaryFloat();
        m_ModelAniHeader.samplePosRangeMin = parser.readBinaryFloat();
        m_ModelAniHeader.samplePosScaler = parser.readBinaryFloat();

        parser.readBinaryRaw((ZMath::float3*)m_ModelAniHeader.aniBBox, sizeof(m_ModelAniHeader.aniBBox));

        m_ModelAniHeader.nextAniName = parser.readLine(true);
        break;

      case MSID_MAN_SOURCE:
        parser.setSeek(chunkEnd);  // Skip chunk
        break;

      case MSID_MAN_ANIEVENTS: {
        uint32_t numAniEvents = parser.readBinaryDWord();
        m_AniEvents.resize(numAniEvents);

        for (uint32_t i = 0; i < numAniEvents; i++)
            m_AniEvents[i].load(parser);
        }
      break;

      case MSID_MAN_RAWDATA: {
        m_ModelAniHeader.nodeChecksum = parser.readBinaryDWord();

        m_NodeIndexList.resize(m_ModelAniHeader.numNodes);
        parser.readBinaryRaw(m_NodeIndexList.data(), m_NodeIndexList.size() * sizeof(uint32_t));

        uint32_t numSamples = m_ModelAniHeader.numNodes * m_ModelAniHeader.numFrames;
        m_AniSamples.resize(numSamples);

        for (size_t i = 0; i < numSamples; i++) {
          zTMdl_AniSample smp={};
          parser.readBinaryRaw(&smp, sizeof(zTMdl_AniSample));
          SampleUnpackTrans((uint16_t*)smp.position, m_AniSamples[i].position, m_ModelAniHeader.samplePosScaler, m_ModelAniHeader.samplePosRangeMin);
          SampleUnpackQuat((uint16_t*)smp.rotation, m_AniSamples[i].rotation);
          }
        }
      break;
      default:
          parser.setSeek(chunkEnd);  // Skip chunk
      }

    if (parser.getSeek() >= parser.getFileSize())
        doneReadingChunks = true;  // No end-tag in here...
    }
  }
