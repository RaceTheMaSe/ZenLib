#pragma once
#include "parserImpl.h"

namespace ZenLoad
{
class ParserImplASCII : public ParserImpl
  {
  friend ZenParser;

public:
  ParserImplASCII(ZenParser* parser);

  /**
     * @brief Read the implementation specific header and stores it in the parsers main-header struct
     */
  void readImplHeader() override;

  /**
    * @brief Read the start of a chunk. [...] Returns true if there actually was a start.
    *      Otherwise it will leave m_Seek untouched and return false.
    */
  bool readChunkStart(ZenParser::ChunkHeader& header) override;

  /**
    * @brief Reads the end of a chunk. Returns true if there actually was an end.
    *      Otherwise it will leave m_Seek untouched and return false.
    */
  bool readChunkEnd() override;

  /**
     * @brief Reads a string
     */
  std::string readString() override;
  bool        readString(char* buf, size_t size) override;

protected:
  /**
     * @brief Reads data of the expected type. Throws if the read type is not the same as specified and not 0
     */
  void readEntryImpl(const char* name, void* target, size_t targetSize, EZenValueType expectedType = ZVT_0, bool optional=false) override;

  /**
    * @brief Reads the type of a single entry
    */
  void readEntryType(EZenValueType& type, size_t& size) override;

private:
  EZenValueType parseType(const char* type) const;
  ZMath::float3 parseVec3(const char* line) const;
  void          parseFloatVec(const char* line, float* target, size_t targetSize) const;
  void          parseColor(const char* line, std::uint8_t* target, size_t targetSize) const;
  void          parseRawVec(const char* line, std::uint8_t* target, size_t targetSize) const;
  };
}  // namespace ZenLoad
