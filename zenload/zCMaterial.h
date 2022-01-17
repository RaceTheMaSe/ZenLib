#pragma once

#include "zTypes.h"

namespace ZenLoad
{
    class ZenParser;

    enum MaterialGroup : uint8_t
    {
      UNDEF = 0,
      METAL,
      STONE,
      WOOD,
      EARTH,
      WATER,
      SNOW,
      NUM_MAT_GROUPS
    };

    class zCMaterial
    {
    public:
      /**
        * Converts the given material-group enum value to the regarding string
        */
      static const char* getMatGroupString(MaterialGroup group);

      /**
        * Reads this object from an internal zen
        */
      static zCMaterialData readObjectData(ZenParser& parser, uint16_t version = 0);
    };

}  // namespace ZenLoad
