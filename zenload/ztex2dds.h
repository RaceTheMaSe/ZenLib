#pragma once
#include <vector>
#include <cinttypes>
#include <cstddef>
#include "ztex.h"
#include "daedalus/ZString.h"

namespace ZenLoad
{
  /**
    * @brief Modified ZTEX to DDS conversion
    */
  int convertZTEX2DDS(const std::vector<uint8_t>& ztexData, std::vector<uint8_t>& ddsData, bool optionForceARGB = false, int* pOutWidth = nullptr, int* pOutHeight = nullptr);
  
  /**
    * @brief Modified ZTEX to DDS conversion - ZenParser variant
    */
  int convertZTEX2DDS(ZenParser& parser, std::vector<uint8_t>& ddsData, bool optionForceARGB = false);

    /**
	 * @return Total offset (including header) of where the given mip-level starts inside ddsData
	 */
    size_t getMipFileOffsetFromDDS(const std::vector<uint8_t>& ddsData, int mip);

  enum class DXTLevel
  {
    DXT1,
    DXT3,
    DXT5,
    Unknown
  };

  /**
    * @return The DXT compression level of the given DDS file
    */
  DXTLevel getDXTLevelFromDDS(const std::vector<uint8_t>& ddsData);

  /**
    * @param ddsData Loaded dds
    * @return surface info of the given dds
    */
  tagDDSURFACEDESC2 getSurfaceDesc(const std::vector<uint8_t>& ddsData);
}  // namespace ZenLoad
