#pragma once

#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

#include "zTypes.h"
#include "utils/mathlib.h"
#include "utils/split.h"
#include "zCMesh.h"

namespace VDFS
{
class FileIndex;
}

namespace ZenLoad
{
class ParserImpl;
class zCMesh;

class ZenParser
  {
  friend class ParserImpl;
  friend class ParserImplBinSafe;
  friend class ParserImplASCII;
  friend class ParserImplBinary;

public:
  /**
     * @brief Possible filetypes this zen can have
     */
  enum EFileType
    {
    FT_UNKNOWN,
    FT_ASCII,
    FT_BINARY,
    FT_BINSAFE
    };

  enum class FileVersion
    {
    Gothic1,
    Gothic2
    };

  enum ZenClass
    {
    zUnknown = 0,
    zReference,
    zCCSLib,
    zCCSBlock,
    zCCSAtomicBlock,
    oCCSTrigger,
    zCAICamera,
    zCMoverController,
    zCVobScreenFX,
    zCDecal,
    zCProgMeshProto,
    zCParticleFX,
    zCMesh,
    zCModel,
    zCMorphMesh,
    zCWorld,
    zCWayNet,
    zCWaypoint,
    zCVob,
    zCVobLevelCompo,
    zCVobStair,
    oCItem,
    zCTrigger,
    oCMOB,
    oCMobInter,
    oCMobBed,
    oCMobFire,
    oCMobLadder,
    oCMobSwitch,
    oCMobWheel,
    oCMobContainer,
    oCMobDoor,
    zCPFXController,
    zCVobAnimate,
    zCVobLensFlare,
    zCVobLight,
    zCVobLightPreset,
    zCVobSpot,
    zCVobStartpoint,
    zCVobSound,
    zCVobSoundDaytime,
    oCZoneMusic,
    oCZoneMusicDefault,
    zCZoneReverb,
    zCZoneReverbDefault,
    zCZoneZFog,
    zCZoneZFogDefault,
    zCZoneVobFarPlane,
    zCZoneVobFarPlaneDefault,
    zCMessageFilter,
    zCCodeMaster,
    zCTriggerList,
    oCTriggerScript,
    zCMover,
    oCTriggerChangeLevel,
    zCTriggerWorldStart,
    zCCSCamera,
    zCCamTrj_KeyFrame,
    oCTouchDamage,
    zCTriggerUntouch,
    zCEarthquake,
    zCTouchAnimate,
    zCTouchAnimateSound,
    zCMusicControler,
    ocVisualFX,
    ocVisFX_MultiTarget,
    zCLensFlareFX,
    zCLensFlare,
    zCEventManager,
    zCCutscene,
    zCCSSyncBlock,
    zCCSRole,
    oCCSProps,
    oCNpcMessage,
    zCEventMessage,
    zCEventScreenFX,
    oCMsgAttack,
    oCMsgConversation,
    ocMsgCutscene,
    oCMsgDamage,
    oCMsgMagic,
    oCMsgManipulate,
    oCMsgMovement,
    oCMsgState,
    oCMsgUseItem,
    oCMsgWeapon,
    ocMobMsg,
    zCEventCommon,
    zCEventCore,
    zCEventMover,
    zCEventMusicControler,
    zCEvMsgCutscene,
    zCCSCamera_EventMsg,
    zCCSCamera_EventMsgActivate,
    };

  /**
     * @brief Information about one of the chunks in a zen-file
     */
  struct ChunkHeader
    {
    uint32_t    size;
    uint16_t    version;
    uint32_t    objectID;
    std::string name;
    ZenClass    classId = ZenClass::zUnknown;
    };

  /**
     * @brief File-Header for ZEN-Files
     */
  struct ZenHeader
    {
    int version;
    EFileType fileType;
    bool saveGame;
    std::string date;
    std::string user;
    int objectCount;

    struct
      {
      uint32_t bsVersion;
      uint32_t bsHashTableOffset;
      } binSafeHeader;
    };

  /**
    * @brief reads a zen from a file
    */
  ZenParser(const std::string& file, const VDFS::FileIndex& vdfs);

  /**
    * @brief reads a zen from memory
    */
  ZenParser(const uint8_t* data, size_t size);
  ZenParser() = default;
  ZenParser(ZenParser&) = delete;
  ZenParser(ZenParser&&) = delete;
  ~ZenParser()=default;
  ZenParser& operator=(ZenParser&) = delete;
  ZenParser& operator=(ZenParser&&) = delete;

  /**
   * @brief Reads the main ZEN-Header
   */
  void readHeader();

  /**
   * @brief reads the main oCWorld-Object, found in the level-zens
   */
  void readWorld(oCWorldData& info, FileVersion version);

  void readLightPresets(std::vector<zCVobData>& vobs, FileVersion version);
  void readLensFlares  (std::vector<zCVobData>& vobs, FileVersion version);
  void readCameras     (std::vector<zCVobData>& vobs, FileVersion version);

  /**
   * @brief Returns the file-header
   */
  const ZenHeader& getZenHeader() { return m_Header; }
  void setZenHeader(const ZenHeader& header) { m_Header = header; }

  /**
   * @brief Returns the parsed world-mesh
   */
  ZenLoad::zCMesh* getWorldMesh() { return m_pWorldMesh.get(); }

  /**
   * @brief returns the total size of the loaded file
   */
  size_t getFileSize() { return m_DataSize; }

  /**
   * @brief Reads the main ZEN-Header
   */
  void readHeader(ZenHeader& header, ParserImpl*& impl);

  /****************************************************************************************************************/
  /* ############### Note: Rest of the methods are to be used only if you know what you are doing ############### */
  /****************************************************************************************************************/

  /**
    * @brief Reads the given type as binary data and returns it
    */
  uint32_t readBinaryDWord();
  uint16_t readBinaryWord();
  uint8_t  readBinaryByte();
  float    readBinaryFloat();
  void     readBinaryRaw(void* target, size_t numBytes);

  char     readChar();
  char     peekChar() const;

  /**
     * @brief Reads a line to \\r or \\n
     */
  std::string readLine(bool skipSpaces = true);
  bool        readLine(char* buf, size_t size, bool skipSpaces = true);

  /**
     * @brief skips a string and checks if it matches the given expected one
     */
  bool skipString(const std::string& pattern = "");

  /**
     * @brief Skips all whitespace-characters until it hits a non-whitespace one
     */
  void skipSpaces();

  /**
     * @brief Skips all newline-characters until it hits a non-newline one
     */
  void skipNewLines();

  /**
     * @brief Reads a string until \\r, \\n or a space is found
     */
  std::string readString(bool skipSpaces = true);

  /**
     * @brief reads an ASCII datatype from the loaded file
     */
  int32_t readIntASCII();
  bool readBoolASCII();

  /**
     * @brief returns whether the given string is a number
     */
  bool isNumber(const std::string& expr);

  /**
     * @brief returns the current implementatio
     */
  ParserImpl* getImpl() { return m_pParserImpl; }
  void setImpl(ParserImpl* impl) { m_pParserImpl = impl; }

  /**
     * @brief reads a full chunk (TESTING ONLY)
     */
  void readChunkTest();

  /**
    * @brief Returns the current position in our loaded file
    */
  size_t getSeek() const { return m_Seek; }

  /**
    * @brief Sets the current position in the loaded file
    */
  void setSeek(size_t seek) { m_Seek = seek; }

  /**
     * @brief Returns the data-array
     */
  const uint8_t* getDataPtr()     const { return m_Data+m_Seek; }
  size_t         getRemainBytes() const { return m_DataSize-m_Seek; }

  /**
    * @brief Reads one structure of type T. Watch for alignment!
    */
  template <typename T>
  void readStructure(T& s)
    {
    if (m_Seek + sizeof(T) <= m_DataSize)
      {
      void* _s = (void*)&s;
      memcpy(_s, &m_Data[m_Seek], sizeof(T));
      m_Seek += sizeof(T);
      }
    }

  /**
     * @brief Reads a chunk-header. Returns true if there actually was a start.
     *      Otherwise it will leave m_Seek untouched and return false.
     */
  bool readChunkStart(ChunkHeader& header);

  /**
     * @brief Reads the end of a chunk. Returns true if there actually was an end.
     *      Otherwise it will leave m_Seek untouched and return false.
     */
  bool readChunkEnd();

  /**
     * @brief Skips an already started chunk
     */
  void skipChunk();

  /**
     * @brief Skips an entry
     */
  void skipEntry();

private:
  /**
   * @brief Skips the main header
   */
  void skipHeader();

  /**
    * @brief reads the worldmesh-chunk
    */
  void           readWorldMesh (oCWorldData& info);
  size_t         readVobTree   (zCVobData& vob, FileVersion version);
  void           readWayNetData(zCWayNetData& info);
  zCWaypointData readWaypoint  ();

  /**
   * @brief Implementation this archive usese
   */
  ParserImpl* m_pParserImpl{};

  /**
   * @brief Data currently loaded and the current stream position
   */
  std::vector<uint8_t>     m_DataStorage;
  const uint8_t*           m_Data=nullptr;
  size_t                   m_DataSize=0;
  size_t                   m_Seek=0;

  /**
   * @brief ZEN-Header of the loaded file
   */
  ZenHeader                m_Header;

  /**
    * @brief The world mesh. Only non-null if the ZEN had one. (BinSave don't have a worldmesh)
    */
  std::unique_ptr<ZenLoad::zCMesh>  m_pWorldMesh;
  };

}  // namespace ZenLoad
