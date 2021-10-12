#pragma once
#include "zenParser.h"

namespace ZenLoad
{
class ParserImpl {
  public:
    /**
      * @brief Type of data in front of the chunks of the binSave-format
      */
    enum EZenValueType
      {
      ZVT_0         = 0,
      ZVT_STRING    = 0x1,
      ZVT_INT       = 0x2,
      ZVT_FLOAT     = 0x3,
      ZVT_BYTE      = 0x4,
      ZVT_WORD      = 0x5,
      ZVT_BOOL      = 0x6,
      ZVT_VEC3      = 0x7,
      ZVT_COLOR     = 0x8,
      ZVT_RAW       = 0x9,
      ZVT_RAW_FLOAT = 0x10,
      ZVT_ENUM      = 0x11,
      ZVT_HASH      = 0x12,
      ZVT_10,
      ZVT_11,
      ZVT_12,
      ZVT_13,
      ZVT_14,
      ZVT_15,
      };

    ParserImpl(ZenParser* parser);
    ParserImpl(ParserImpl&)=delete;
    ParserImpl(ParserImpl&&)=delete;
    virtual ~ParserImpl() = default;
    ParserImpl& operator=(ParserImpl&)=delete;
    ParserImpl& operator=(ParserImpl&&)=delete;

    /**
     * @brief Read the implementation specific header and stores it in the parsers main-header struct
     */
    virtual void readImplHeader() = 0;

    /**
     * @brief Read the start of a chunk. [...] Returns true if there actually was a start.
     *      Otherwise it will leave m_Seek untouched and return false.
     */
    virtual bool readChunkStart(ZenParser::ChunkHeader& header) = 0;

    /**
     * @brief Reads the end of a chunk. Returns true if there actually was an end.
     *      Otherwise it will leave m_Seek untouched and return false.
     */
    virtual bool readChunkEnd() = 0;

    /**
       * @brief Reads a string
       */
    virtual std::string readString() = 0;
    virtual bool        readString(char* buf, size_t size) = 0;

    void readEntry(const char* name, std::string&   target, bool optional=false) { readEntryImpl(name, &target,              0, ZVT_STRING   ,optional); }
    void readEntry(const char* name, uint8_t&       target, bool optional=false) { readEntryImpl(name, &target, sizeof(target), ZVT_BYTE     ,optional); }
    void readEntry(const char* name, bool&          target, bool optional=false) { readEntryImpl(name, &target, sizeof(target), ZVT_BOOL     ,optional); }
    void readEntry(const char* name, uint16_t&      target, bool optional=false) { readEntryImpl(name, &target, sizeof(target), ZVT_WORD     ,optional); }
    void readEntry(const char* name, int16_t&       target, bool optional=false) { readEntryImpl(name, &target, sizeof(target), ZVT_WORD     ,optional); }
    void readEntry(const char* name, uint32_t&      target, bool optional=false) { readEntryImpl(name, &target, sizeof(target), ZVT_INT      ,optional); }
    void readColor(const char* name, uint32_t&      target, bool optional=false) { readEntryImpl(name, &target, sizeof(target), ZVT_COLOR    ,optional); }
    void readEntry(const char* name, int32_t&       target, bool optional=false) { readEntryImpl(name, &target, sizeof(target), ZVT_INT      ,optional); }
    void readEntry(const char* name, float&         target, bool optional=false) { readEntryImpl(name, &target, sizeof(target), ZVT_FLOAT    ,optional); }
    void readEntry(const char* name, ZMath::float2& target, bool optional=false) { readEntryImpl(name, &target, sizeof(target), ZVT_RAW_FLOAT,optional); }
    void readEntry(const char* name, ZMath::float3& target, bool optional=false) { readEntryImpl(name, &target, sizeof(target), ZVT_VEC3     ,optional); }
    void readEntry(const char* name, ZMath::float4& target, bool optional=false) { readEntryImpl(name, &target, sizeof(target), ZVT_RAW_FLOAT,optional); }
    void readEntry(const char* name, ZMath::Matrix& target, bool optional=false) { readEntryImpl(name, &target, sizeof(target), ZVT_RAW_FLOAT,optional); }
    void readEntry(const char* name, Daedalus::ZString& tg, bool optional=false) { std::string s; this->readEntry(name,s,optional); tg = Daedalus::ZString(std::move(s)); }

    void readEntry(const char* name, void* target, size_t size, bool optional=false) { readEntryImpl(name, target, size, ZVT_RAW, optional); }

    /**
       * @brief Reads the type of a single entry
       */
    virtual void readEntryType(EZenValueType& type, size_t& size) = 0;

  protected:
    /**
       * @brief Reads data of the expected type. Throws if the read type is not the same as specified and not 0
       */
    virtual void        readEntryImpl(const char* name, void* target, size_t targetSize, EZenValueType expectedType, bool optional=false) = 0;

    bool                parseHeader(ZenParser::ChunkHeader& header, const char* vobDescriptor, size_t vobDescriptorLen);
    ZenParser::ZenClass parseClassName(const char* name, size_t len);
    static size_t       valueTypeSize(EZenValueType t);

private:
    /**
       * @brief Parser-Object this operates on
       */
    ZenParser* m_pParser;

friend class ParserImplBinary;
friend class ParserImplBinSafe;
friend class ParserImplASCII;
  };
}  // namespace ZenLoad
