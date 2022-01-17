#include "zCVob.h"
#include "zenParser.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <memory>

#include "parserImplASCII.h"
#include "parserImplBinSafe.h"
#include "parserImplBinary.h"
#include "zenParserPropRead.h"
#include "zCBspTree.h"
#include "zCMesh.h"
#include "utils/logger.h"
#include <cmath>
#include "zenload/zTypes.h"
#include <vdfs/fileIndex.h>

using namespace ZenLoad;

/**
  * @brief reads a zen from a vdf
  */
ZenParser::ZenParser(const std::string& file, const VDFS::FileIndex& vdfs) {
  vdfs.getFileData(file, m_DataStorage);
  m_Data     = m_DataStorage.data();
  m_DataSize = m_DataStorage.size();
  }

/**
  * @brief reads a zen from memory
  */
ZenParser::ZenParser(const uint8_t* data, size_t size) : m_Data(data), m_DataSize(size) { }

/**
  * @brief Reads the main ZEN-Header
  */
void ZenParser::readHeader()
{
    readHeader(m_Header, m_pParserImpl);
}

/**
  * @brief Skips the main header
  */
void ZenParser::skipHeader()
{
    ZenHeader header;
    ParserImpl* impl = nullptr;
    readHeader(header, impl);

    delete impl;
}

/**
  * @brief Skipts the ZEN-Header
  */
void ZenParser::readHeader(ZenHeader& header, ParserImpl*& impl) {
  if(!skipString("ZenGin Archive"))
    throw std::runtime_error("Not a valid format");

  if(!skipString("ver"))
    throw std::runtime_error("Not a valid header");

  // Version should be always 1...
  header.version = atoi(readLine(true).c_str()); // changed from readIntASCII(); to have proper advancing in ascii zen

  // Skip archiver type
  if(header.version!=0) // lensflare.zen has version==0 and no archiver type
    skipString();

  // Read file-type, to create the right archiver implementation
  std::string fileType = readLine();

  // Read string of the format "savegame b", where b is 0 or 1
  if(!skipString("saveGame"))
    throw std::runtime_error("Unsupported file format");
  header.saveGame = readBoolASCII();

  // Read possible date
  if(skipString("date")) {
    header.date = readString() + " ";
    header.date += readString();
    }

  // Skip possible user
  if(skipString("user"))
    header.user = readLine();

  if(header.version!=0) { // version 0 continues with object count,
    // Reached the end of the main header
    if(!skipString("END"))
      throw("No END in header(1)");

    // Continue with the implementationspecific header
    skipSpaces();
    }

  if(fileType == "ASCII") {
    header.fileType = FT_ASCII;
    impl = new ParserImplASCII(this);
    }
  else if (fileType == "BINARY") {
    header.fileType = FT_BINARY;
    impl = new ParserImplBinary(this);
    }
  else if (fileType == "BIN_SAFE") {
    header.fileType = FT_BINSAFE;
    impl = new ParserImplBinSafe(this);
    }
  else {
    throw std::runtime_error("Unsupported file format");
    }

  impl->readImplHeader();
  }

void ZenParser::readWorldMesh(oCWorldData& info) {
  LogInfo() << "ZEN: Reading mesh...";
  m_pWorldMesh = std::make_unique<ZenLoad::zCMesh>();
  info.bspTree = zCBspTree::readObjectData(*this, m_pWorldMesh.get());
  LogInfo() << "ZEN: Done reading mesh!";
  }

/**
  * @brief Reads a chunk-header
  */
bool ZenParser::readChunkStart(ChunkHeader& header) {
  return m_pParserImpl->readChunkStart(header);
  }

/**
  * @brief Reads the end of a chunk. Returns true if there actually was an end. 
  *      Otherwise it will leave m_Seek untouched and return false.
  */
bool ZenParser::readChunkEnd() {
  return m_pParserImpl->readChunkEnd();
  }

/**
  * @brief  Skips an already started chunk
  */
void ZenParser::skipChunk() {
  size_t level = 1;

  do {
    ChunkHeader header;
    if (readChunkStart(header))
      level++;
    else if (readChunkEnd())
      level--;
    else
      skipEntry();
    } while (level > 0);
  }

/**
  * @brief Skips an entry
  */
void ZenParser::skipEntry() {
  ParserImpl::EZenValueType type = ParserImpl::EZenValueType::ZVT_0;
  size_t size = 0;

  // Read type and size first, so we can allocate the data
  m_pParserImpl->readEntryType(type, size);

  // Skip the entry
  m_Seek += size;
  }

/**
 * @brief reads an ASCII datatype from the loaded file
 */
int32_t ZenParser::readIntASCII() {
  int32_t ret=0;
  skipSpaces();

  const char* i = reinterpret_cast<const char*>(m_Data)+m_Seek;
  while(m_Seek<m_DataSize && '0'<=*i && *i <='9') {
    ret = ret*10+(*i-'0');
    ++m_Seek;
    ++i;
    }
  return ret;
  }

/**
 * @brief reads an ASCII datatype from the loaded file
 */
bool ZenParser::readBoolASCII() {
  skipSpaces();
  bool retVal = false;
  if (m_Data[m_Seek] != '0' && m_Data[m_Seek] != '1')
      throw std::runtime_error("Value is not a bool"); else
      retVal = m_Data[m_Seek] == '0' ? false : true;

  ++m_Seek;
  return retVal;
  }

/**
 * @brief Reads a string until \r, \n or a space is found
 */
std::string ZenParser::readString(bool skip) {
  if(skip)
    skipSpaces();

  const char* begin = reinterpret_cast<const char*>(m_Data+m_Seek);
  size_t      size  = 0;
  while(m_Seek<m_DataSize) {
    if(m_Data[m_Seek]=='\n' || m_Data[m_Seek]=='\r' || m_Data[m_Seek]=='\t' || m_Data[m_Seek]==' ' || m_Data[m_Seek]=='\0') {
      ++m_Seek;
      break;
      }
    ++size;
    ++m_Seek;
    }

  std::string str(begin,size);
  return str;
  }

/**
  * @brief skips a string and checks if it matches the given expected one
  */
bool ZenParser::skipString(const std::string& pattern) {
  skipSpaces();
  bool retVal = true;
  if (pattern.empty()) {
    while(m_Seek<m_DataSize && m_Data[m_Seek]!='\n' && m_Data[m_Seek]!=' ')
      ++m_Seek;
    if(m_Seek<m_DataSize)
      ++m_Seek;
    }
  else {
    size_t lineSeek = 0;
    while (lineSeek < pattern.size()) {
      if(m_Seek>=m_DataSize || m_Data[m_Seek]!=(uint8_t)pattern[lineSeek]) {
        retVal = false;
        break;
        }

      ++m_Seek;
      ++lineSeek;
      }
    }
  return retVal;
  }

/**
  * @brief Skips all whitespace-characters until it hits a non-whitespace one
  */
void ZenParser::skipSpaces() {
  bool search = true;
  while (search && m_Seek < m_DataSize) {
    switch (m_Data[m_Seek]) {
      case ' ':
      case '\r':
      case '\t':
      case '\n':
        ++m_Seek;
        break;
      default:
        search = false;
        break;
      }
    }
  }

/**
  * @brief Skips all newline-characters until it hits a non-newline one
  */
void ZenParser::skipNewLines()
{
    while (m_Seek < m_DataSize && m_Data[m_Seek] == '\n')
        ++m_Seek;
}

/**
  * @brief Reads the given type as binary data and returns it
  */
uint32_t ZenParser::readBinaryDWord() {
  uint32_t retVal = 0;
  readStructure(retVal);

  return retVal;
  }

uint16_t ZenParser::readBinaryWord() {
  uint16_t retVal = 0;
  readStructure(retVal);

  return retVal;
  }

uint8_t ZenParser::readBinaryByte() {
  uint8_t retVal = *reinterpret_cast<const uint8_t*>(m_Data+m_Seek);
  m_Seek += sizeof(uint8_t);
  return retVal;
  }

float ZenParser::readBinaryFloat() {
  float retVal = {};
  readStructure(retVal);

  return retVal;
  }

void ZenParser::readBinaryRaw(void* target, size_t numBytes) {
  if (target != nullptr && numBytes != 0) {
    std::memcpy(target, &m_Data[m_Seek], numBytes);
    m_Seek += numBytes;
    }
  }

char ZenParser::readChar() {
  uint8_t retVal = *reinterpret_cast<const uint8_t*>(m_Data+m_Seek);
  m_Seek++;
  return char(retVal);
  }

char ZenParser::peekChar() const {
  uint8_t retVal = *reinterpret_cast<const uint8_t*>(m_Data+m_Seek);
  return char(retVal);
  }

/**
* @brief Reads a line to \r or \n
*/
std::string ZenParser::readLine(bool skip) {
  const char* at = reinterpret_cast<const char*>(m_Data+m_Seek);
  size_t      sz = 0;
  while(m_Seek<m_DataSize && m_Data[m_Seek]!='\r' && m_Data[m_Seek]!='\t' && m_Data[m_Seek]!='\n' && m_Data[m_Seek]!='\0') {
    sz++;
    m_Seek++;
    }
  std::string retVal(at,sz);

  // Skip trailing \n\r\0
  //if(m_Header.fileType == FT_BINARY)
  m_Seek++;

  if(skip)
    skipSpaces();
  return retVal;
  }

bool ZenParser::readLine(char* buf, size_t size, bool skip) {
  auto seek0 = m_Seek;

  const char* at = reinterpret_cast<const char*>(m_Data+m_Seek);
  size_t      sz = 0;
  while(m_Seek<m_DataSize && m_Data[m_Seek]!='\r' && m_Data[m_Seek]!='\t' && m_Data[m_Seek]!='\n' && m_Data[m_Seek]!='\0') {
    sz++;
    m_Seek++;
    }

  if(sz>=size) {
    m_Seek = seek0;
    return false;
    }
  std::memcpy(buf,at,sz);
  buf[sz] = '\0';

  m_Seek++;

  if(skip)
    skipSpaces();
  return true;
  }

/**
* @brief reads the main oCWorld-Object, found in the level-zens
*/
void ZenParser::readWorld(oCWorldData& info, FileVersion version) {
  LogInfo() << "ZEN: Reading world...";

  ChunkHeader worldHeader;
  readChunkStart(worldHeader);

  bool readSuccess=false;
  if(worldHeader.classId!=ZenParser::zCWorld)
    throw std::runtime_error("Expected oCWorld:zCWorld-Chunk not found!");

  bool isUncompressZen=true;
  while(!readChunkEnd()) {
    ZenParser::ChunkHeader header;
    readChunkStart(header);

    LogInfo() << "oCWorld reading chunk: " << header.name;

    if(header.name == "MeshAndBsp") {
      readWorldMesh(info);
      readChunkEnd();
      isUncompressZen=false;
      }
    else if(header.name == "VobTree") {
      // Read how many vobs this one has as child
      uint32_t numChildren = 0;
      getImpl()->readEntry("", numChildren);
      info.rootVobs.reserve(numChildren);

      // Read children
      info.numVobsTotal = 0;
      info.rootVobs.resize(numChildren);
      for(uint32_t i=0; i<numChildren; i++) {
        info.numVobsTotal += readVobTree(info.rootVobs[i],version);
        }
      readChunkEnd();
      }
    else if (header.name == "WayNet") {
      readWayNetData(info.waynet);
      readChunkEnd();
      }
    else if (header.name == "EndMarker") {
      LogInfo() << "ZEN: Done reading world";
      readSuccess=true;
      }
    else {
      LogInfo() << "ZEN: No clean read of zen file - there could be loading errors. Last chunk: " << header.name << " - classId: " << header.classId;
      skipChunk();
      }
    }
  if(!readSuccess)
    LogInfo() << "ZEN: No clean read of zen file - there could be following issues originating from this!!!";
  if(isUncompressZen && info.rootVobs.size()>1) // NOTE: only emit this message for more than one rootVob, assuming those are level zen's and size==1 are VobBundles
    LogInfo() << "ZEN: No world mesh given - probably an uncompressed zen file. Please provide a compressed zen, either ASCII or BinSafe format. Ignore this message if its a VobBundle zen";
  }

void ZenParser::readLightPresets(std::vector<zCVobData>& vobs, ZenParser::FileVersion version) {
  LogInfo() << "ZEN: Reading light presets...";

  int numVobLightPresets = 0;
  getImpl()->readEntry("numVobLightPresets",numVobLightPresets);
  if(numVobLightPresets<0)
    return;

  for(int i=0; i<numVobLightPresets; ++i) {
    ChunkHeader header;
    if(!readChunkStart(header))
      break;

    if(header.classId==ZenClass::zCVobLightPreset) {
      zCVobData vob;
      zCVob::readObjectData(vob, *this, header, version);
      vobs.emplace_back(std::move(vob));
      readChunkEnd();
      } else {
      skipChunk();
      }
    }
  }

void ZenParser::readLensFlares(std::vector<ZenLoad::zCVobData>& vobs, ZenParser::FileVersion version) {
  LogInfo() << "ZEN: Reading lensflare presets...";

  ChunkHeader header;
  if(!readChunkStart(header) || header.name!="LensFlareFXList")
    return;
  
  while(readChunkStart(header)) {
    zCVobData preset;
    preset.vobType = zCVobData::VT_zCLensFlareFX;
    zCVob::readObjectData(preset, *this, header, version);
    vobs.emplace_back(preset);
    }
  readChunkEnd();
  }

void ZenParser::readCameras(std::vector<ZenLoad::zCVobData>& vobs, ZenParser::FileVersion version) {
  LogInfo() << "ZEN: Reading camera presets...";

  ChunkHeader header;
  while(readChunkStart(header)) {
    zCVobData preset;
    preset.vobType = zCVobData::VT_zCCSCamera;
    zCVob::readObjectData(preset, *this, header, version);
    vobs.emplace_back(preset);
    }
  readChunkEnd();
  if(getRemainBytes()>0)
    LogInfo() << "ZEN: Unclean read of camera presets " << getRemainBytes() << " bytes remained";
  }

size_t ZenParser::readVobTree(zCVobData& vob, FileVersion version) {
  ZenParser::ChunkHeader header = {};
  readChunkStart(header);
  vob.vobName     = std::move(header.name);
  vob.vobType     = zCVobData::VT_Unknown;
  vob.vobObjectID = header.objectID;
  zCVob::readObjectData(vob, *this, header, version);

  // Read how many vobs this one has as child
  uint32_t numChildren = 0;
  getImpl()->readEntry("", numChildren);
  vob.childVobs.resize(numChildren);

  size_t num = 0;
  for(uint32_t i=0; i<numChildren; i++)
    num += readVobTree(vob.childVobs[i],version);
  return numChildren+1;
  }

void ZenParser::readWayNetData(zCWayNetData& info) {
  ZenParser::ChunkHeader waynetHeader;
  readChunkStart(waynetHeader);

  ReadObjectProperties(*this,
                       Prop("waynetVersion", info.waynetVersion));

  if(info.waynetVersion == 0) {
    // TODO: Implement old waynet format
    LogWarn() << "Old waynet-format not yet supported!";
    return;
    }

  // First, read the waypoints array
  uint32_t numWaypoints = 0;
  getImpl()->readEntry("numWaypoints", numWaypoints);

  LogInfo() << "Loading " << numWaypoints << (numWaypoints!=1 ? " freepoints" : " freepoint");

  std::unordered_map<uint32_t, size_t> wpRefMap;
  for(uint32_t i = 0; i < numWaypoints; i++) {
    ZenParser::ChunkHeader wph;
    // These are always new ones
    readChunkStart(wph);
    zCWaypointData w = readWaypoint();
    info.waypoints.push_back(w);
    readChunkEnd();

    // Save for later access
    wpRefMap[wph.objectID] = info.waypoints.size() - 1;
    }

  // Then, the edges (ways)
  uint32_t numWays = 0;
  getImpl()->readEntry("numWays", numWays);

  LogInfo() << "Loading " << numWays << (numWays!=1 ? " edges" : " edge");

  for(uint32_t i = 0; i < numWays; i++) {
    size_t wp1 = 0, wp2 = 0;

    size_t* tgt = &wp1;
    for(int r = 0; r < 2; r++) {
      ZenParser::ChunkHeader wph;
      // References might occur here
      readChunkStart(wph);
      assert(wph.name[0]=='w' && wph.name[1]=='a' && wph.name[2]=='y' && (wph.name[3]=='l' || wph.name[3]=='r'));

      // Loading a reference?
      if(wph.classId==zReference) {
        *tgt = wpRefMap[wph.objectID];
        } 
      else if(wph.classId==zCWaypoint) {
        // Create new waypoint
        zCWaypointData w = readWaypoint();
        info.waypoints.push_back(w);

        // Save for later access
        wpRefMap[wph.objectID] = info.waypoints.size() - 1;
        *tgt = info.waypoints.size() - 1;
        }

      readChunkEnd();
      tgt = &wp2;
      }
    info.edges.emplace_back(wp1, wp2);
    }

  LogInfo() << "Done loading edges!";
  readChunkEnd();
  }

zCWaypointData ZenParser::readWaypoint() {
  zCWaypointData info;
  getImpl()->readEntry("wpName",     info.wpName);
  getImpl()->readEntry("waterDepth", info.waterDepth);
  getImpl()->readEntry("underWater", info.underWater);
  getImpl()->readEntry("position",   info.position);
  getImpl()->readEntry("direction",  info.direction);
  return info;
  }
