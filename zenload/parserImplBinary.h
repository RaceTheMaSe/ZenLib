#pragma once
#include "parserImpl.h"

namespace ZenLoad
{
class ParserImplBinary : public ParserImpl
  {
  friend ZenParser;

public:
  ParserImplBinary(ZenParser* parser);

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
  };
}  // namespace ZenLoad
