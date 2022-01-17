#pragma once
#include <vector>
#include "zTypes.h"
#include "utils/mathlib.h"

namespace VDFS
{
  class FileIndex;
}

namespace ZenLoad
{
  class ZenParser;
  class zCModelPrototype
  {
  public:
    struct Animation
    {
      std::string animationName;
      unsigned int layer;
      std::string nextAnimation;
      float blendIn, blendOut;

      enum Flags
      {
        MoveObject   =  1,
        RotateObject =  2,
        WaitEnd      =  4,
        Fly          =  8,
        Idle         = 16
      };

      unsigned int flags;
      std::string ascName;
      bool aniReversed;
      int startFrame, endFrame;
    };

    zCModelPrototype() = default;

    /**
      * @brief Loads the mesh from the given VDF-Archive
      */
    zCModelPrototype(const std::string& fileName, const VDFS::FileIndex& fileIndex);

    /**
      * @brief Reads the mesh-object from the given binary stream
      * @param parser ZenParser object
      */
    void readObjectData(ZenParser& parser);

    /**
      * @return List of animations registered here
      */
    std::vector<Animation> getAnimations()
    {
        return m_Animations;
    }

  private:
    std::vector<Animation> m_Animations;
  };
}  // namespace ZenLoad
