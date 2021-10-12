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

    static const char* const MaterialGroupNames[MaterialGroup::NUM_MAT_GROUPS] = {
        "UNDEF",
        "METAL",
        "STONE",
        "WOOD",
        "EARTH",
        "WATER",
        "SNOW",
    };

    class zCMaterial
    {
    public:
        /**
         * Converts the given material-group enum value to the regarding string
         */
        static const char* getMatGroupString(MaterialGroup group)
        {
          if(group>=MaterialGroup::NUM_MAT_GROUPS)
            return MaterialGroupNames[MaterialGroup::UNDEF];
          return MaterialGroupNames[group];
        }

        /**
         * Reads this object from an internal zen
         */
        static zCMaterialData readObjectData(ZenParser& parser, uint16_t version = 0);
    };

}  // namespace ZenLoad
