#include "parserImplBinSafe.h"
#include <utils/logger.h>

using namespace ZenLoad;

ZenLoad::ParserImplBinSafe::ParserImplBinSafe(ZenParser* parser)
    : ParserImpl(parser)
{
}

/**
* @brief Read the start of a chunk. [...]
*/
bool ParserImplBinSafe::readChunkStart(ZenParser::ChunkHeader& header)
{
    size_t seek = m_pParser->getSeek();
    EZenValueType type = ZVT_0;
    size_t size = 0;

    // Check if this actually will be a string
    readTypeAndSizeBinSafe(type, size);
    m_pParser->setSeek(seek);

    if(type != ZVT_STRING)
      return false;  // Next property isn't a string or the end

    // Read chunk-header
    char        vobDescStk[256] = {};
    std::string vobDescHeap;
    const char* vobDesc = (char*)vobDescStk;

    if(!readString((char*)vobDescStk,sizeof(vobDescStk))) {
      vobDescHeap = readString();
      vobDesc     = vobDescHeap.c_str();
      }
    if(parseHeader(header,vobDesc,std::strlen(vobDesc)))
      return true;
    m_pParser->setSeek(seek);
    return false;
}

/**
* @brief Read the end of a chunk. []
*/
bool ParserImplBinSafe::readChunkEnd() {
  size_t seek = m_pParser->getSeek();
  EZenValueType type = ZVT_0;
  size_t size = 0;

  // Check if this actually will be a string
  readTypeAndSizeBinSafe(type, size);
  m_pParser->setSeek(seek);

  if(type != ZVT_STRING)
    return false;  // Next property isn't a string or the end

  char l[3] = {};
  if(!readString((char*)l,3) || l[0]!='[' || l[1]!=']') {
    m_pParser->setSeek(seek);  // Next property isn't a string or the end
    return false;
    }
  return true;
  }

/**
* @brief Read the implementation specific header and stores it in the parsers main-header struct
*/
void ParserImplBinSafe::readImplHeader()
{
    // Bin-Safe archiver has its own version stored again for some reason
    m_pParser->m_Header.binSafeHeader.bsVersion = m_pParser->readBinaryDWord();

    // Read object count
    m_pParser->m_Header.objectCount = (int)m_pParser->readBinaryDWord();

    // Read offset of where to find the global hash-table of all objects
    m_pParser->m_Header.binSafeHeader.bsHashTableOffset = m_pParser->readBinaryDWord();

    // Read hashtable
    size_t s = m_pParser->m_Seek;
    m_pParser->m_Seek = m_pParser->m_Header.binSafeHeader.bsHashTableOffset;

    struct BinSafeEntry
    {
        std::string key;
        uint16_t insertionIndex;
    };

    std::unordered_map<uint32_t, std::vector<BinSafeEntry>> map;
    uint32_t htSize = m_pParser->readBinaryDWord();
    for (uint32_t i = 0; i < htSize; i++)
    {
        uint16_t keyLen = m_pParser->readBinaryWord();
        uint16_t insIdx = m_pParser->readBinaryWord();
        uint32_t hashValue = m_pParser->readBinaryDWord();

        (void)insIdx;
        (void)hashValue;

        // Read the keys data from the archive
        std::vector<uint8_t> keyData(keyLen);
        m_pParser->readBinaryRaw(keyData.data(), keyLen);

        // Put the source key together with the insertion index into the map
        //std::string key(keyData.begin(), keyData.end());
        //map[hashValue].push_back( {key, insIdx} );
    }

    // Restore old position
    m_pParser->m_Seek = s;
}

/**
* @brief Reads a string
*/
std::string ParserImplBinSafe::readString()
{
    EZenValueType type = ZVT_0;
    size_t size = 0;

    // These values start with a small header
    readTypeAndSizeBinSafe(type, size);

    if (type != ZVT_STRING)
        throw std::runtime_error("Expected string-type");

    std::string str;
    str.resize(size);
    m_pParser->readBinaryRaw(&str[0], size);

    // Skip potential hash-value at the end of the string
    auto t = static_cast<EZenValueType>(m_pParser->readBinaryByte());
    if (t != ZVT_HASH)
        m_pParser->m_Seek -= sizeof(uint8_t);
    else
        m_pParser->m_Seek += sizeof(uint32_t);

    return str;
  }

bool ParserImplBinSafe::readString(char* buf, size_t bufSize)
{
  EZenValueType type = ZVT_0;
  size_t size = 0;

  // These values start with a small header
  readTypeAndSizeBinSafe(type, size);

  if(type!=ZVT_STRING)
    throw std::runtime_error("Expected string-type");

  if(size>=bufSize)
    return false;

  m_pParser->readBinaryRaw(buf, size);
  buf[size] = '\0';

  // Skip potential hash-value at the end of the string
  auto t = static_cast<EZenValueType>(m_pParser->readBinaryByte());
  if (t != ZVT_HASH)
      m_pParser->m_Seek -= sizeof(uint8_t);
  else
      m_pParser->m_Seek += sizeof(uint32_t);
  return true;
}

/**
* @brief reads the small header in front of datatypes
*/
void ParserImplBinSafe::readTypeAndSizeBinSafe(EZenValueType& type, size_t& size)
{
    type = static_cast<EZenValueType>(m_pParser->readBinaryByte());
    if(type==ZVT_RAW || type==ZVT_RAW_FLOAT || type==ZVT_STRING)
      size = m_pParser->readBinaryWord(); else
      size = valueTypeSize(type);
}

/**
* @brief Reads data of the expected type. Throws if the read type is not the same as specified and not 0
*/
void ParserImplBinSafe::readEntryImpl(const char *expectedName, void* target, size_t targetSize, EZenValueType expectedType, bool optional)
{
    EZenValueType realType = ZVT_0;
    size_t        size = 0;

    size_t seek=0;
    if(optional)
      seek=m_pParser->getSeek();

    // Read type and size of the entry
    readTypeAndSizeBinSafe(realType, size);

    if(realType==ZVT_ENUM) {
      switch(targetSize) {
        case 1:
          realType = ZVT_BYTE;
          size     = 1;
          break;
        case 2:
          realType = ZVT_WORD;
          size     = 2;
          break;
        case 4:
          realType = ZVT_INT;
          size     = 4;
          break;
        }
      }

    if (expectedType != realType && realType != ZVT_0) {
      if(optional) {
        m_pParser->setSeek(seek);
        return;
        }
      throw std::runtime_error(std::string("Valuetype name does not match expected type. Value:") + expectedName);
      }

    if(expectedType!=ZVT_RAW && expectedType!=ZVT_RAW_FLOAT && expectedType!=ZVT_STRING && realType!=ZVT_0) {
      if(optional) {
        m_pParser->setSeek(seek);
        return;
        }
      if(size!=targetSize)
        throw std::runtime_error(std::string("Valuetype size does not match expected size. Value:") + expectedName);
      }

    switch(realType) {
      case ZVT_0:
        break;
      case ZVT_STRING: {
        std::string str;
        str.resize(size);
        m_pParser->readBinaryRaw(&str[0], size);
        *reinterpret_cast<std::string*>(target) = str;
        break;
        }
      case ZVT_HASH:
      case ZVT_INT:
        *reinterpret_cast<uint32_t*>(target) = m_pParser->readBinaryDWord();
        break;
      case ZVT_FLOAT:
        *reinterpret_cast<float*>(target) = m_pParser->readBinaryFloat();
        break;
      case ZVT_BYTE:
        *reinterpret_cast<uint8_t*>(target) = m_pParser->readBinaryByte();
        break;
      case ZVT_WORD:
        *reinterpret_cast<int16_t*>(target) = (int16_t)m_pParser->readBinaryWord();
        break;
      case ZVT_BOOL:
        *reinterpret_cast<bool*>(target) = m_pParser->readBinaryDWord() != 0;
        break;
      case ZVT_VEC3:
        reinterpret_cast<ZMath::float3*>(target)->x = m_pParser->readBinaryFloat();
        reinterpret_cast<ZMath::float3*>(target)->y = m_pParser->readBinaryFloat();
        reinterpret_cast<ZMath::float3*>(target)->z = m_pParser->readBinaryFloat();
        break;

      case ZVT_COLOR:
        reinterpret_cast<uint8_t*>(target)[2] = m_pParser->readBinaryByte();
        reinterpret_cast<uint8_t*>(target)[1] = m_pParser->readBinaryByte();
        reinterpret_cast<uint8_t*>(target)[0] = m_pParser->readBinaryByte();
        reinterpret_cast<uint8_t*>(target)[3] = m_pParser->readBinaryByte();
        break;

      case ZVT_RAW_FLOAT:
      case ZVT_RAW:
        if(targetSize<size)
          LogInfo() << "Reading more than expected - might produce an overflow!";
        m_pParser->readBinaryRaw(target, size);
        break;
      case ZVT_10:
      case ZVT_11:
      case ZVT_12:
      case ZVT_13:
      case ZVT_14:
      case ZVT_15:
        break;
      case ZVT_ENUM:
        *reinterpret_cast<uint8_t*>(target) = m_pParser->readBinaryDWord();
        break;
      }

    // Skip potential hash-value at the end of the entry
    auto t = static_cast<EZenValueType>(m_pParser->readBinaryByte());
    if(t != ZVT_HASH)
      m_pParser->m_Seek -= sizeof(uint8_t);
    else
      m_pParser->m_Seek += sizeof(uint32_t);
}

/**
* @brief Reads the type of a single entry
*/
void ParserImplBinSafe::readEntryType(EZenValueType& outtype, size_t& size)
{
    readTypeAndSizeBinSafe(outtype, size);
}
