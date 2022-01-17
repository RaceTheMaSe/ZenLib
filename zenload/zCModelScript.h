#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ZenLoad
{
  enum
  {
    ANIEVENT_MAX_NUM_VALUES = 4
  };

  enum EModelScriptAniFlags
  {
    /// Animation moves model in world space
    MSB_MOVE_MODEL   = 0x00000001,
    /// Animation rotates model in world space
    MSB_ROTATE_MODEL = 0x00000002,
    /// Animation is queued after the current ani on layer instead of started immediately
    MSB_QUEUE_ANI    = 0x00000004,
    /// Don't stick to ground
    MSB_FLY          = 0x00000008,
    /// Idle animation
    MSB_IDLE         = 0x00000010
  };

  enum EModelScriptAniDir
  {
    MSB_FORWARD,
    MSB_BACKWARD
  };

  // https://worldofplayers.ru/threads/37708/
  enum EModelScriptAniDef : uint8_t
  {
    DEF_NULL,
    DEF_CREATE_ITEM,
    DEF_INSERT_ITEM,
    DEF_REMOVE_ITEM,
    DEF_DESTROY_ITEM,
    DEF_PLACE_ITEM,
    DEF_EXCHANGE_ITEM,

    DEF_FIGHTMODE,
    DEF_PLACE_MUNITION,
    DEF_REMOVE_MUNITION,
    DEF_DRAWSOUND,
    DEF_UNDRAWSOUND,
    DEF_SWAPMESH,

    DEF_DRAWTORCH,
    DEF_INV_TORCH,
    DEF_DROP_TORCH,

    DEF_HIT_LIMB,
    DEF_DIR,
    DEF_DAM_MULTIPLY,
    DEF_PAR_FRAME,
    DEF_OPT_FRAME,
    DEF_HIT_END,
    DEF_WINDOW,

    DEF_LAST
  };

  enum EFightMode : uint8_t
  {
    FM_NONE,
    FM_FIST,
    FM_1H,
    FM_2H,
    FM_BOW,
    FM_CBOW,
    FM_MAG,
    FM_LAST
  };

  struct zCModelScriptAni
  {
    /// Add + ".MAN" for the animation data
    std::string        m_Name;
    uint32_t           m_Layer       = 0;
    std::string        m_Next;
    float              m_BlendIn     = 0;
    float              m_BlendOut    = 0;
    uint32_t           m_Flags       = 0;
    std::string        m_Asc;
    EModelScriptAniDir m_Dir         = MSB_FORWARD;
    int32_t            m_FirstFrame  = 0;
    int32_t            m_LastFrame   = 0;
    float              m_MaxFps      = 0.0f;
    float              m_Speed       = 0.0f;
    float              m_ColVolScale = 1.0f;
  };

  struct zCModelScriptMeshAndThree
  {
    /// Add + ".MAN" for the animation data
    std::string m_Name;
    bool        m_Disabled = false;
  };

  struct zCModelScriptAniAlias
  {
    /// Add + ".MAN" for the animation data
    std::string        m_Name;
    uint32_t           m_Layer    = 0;
    std::string        m_Next;
    float              m_BlendIn  = 0;
    float              m_BlendOut = 0;
    uint32_t           m_Flags    = 0;
    std::string        m_Alias;
    EModelScriptAniDir m_Dir      = MSB_FORWARD;
  };

  struct zCModelScriptAniBlend
  {
    std::string m_Name;
    uint32_t    m_Layer    = 0;
    std::string m_Next;
    float       m_BlendIn  = 0;
    float       m_BlendOut = 0;
  };

  struct zCModelScriptAniSync
  {
    std::string m_Name;
    uint32_t    m_Layer = 0;
    std::string m_Next;
  };

  struct zCModelScriptAniCombine
  {
    std::string m_Name;
    uint32_t    m_Layer     = 0;
    std::string m_Next;
    float       m_BlendIn   = 0;
    float       m_BlendOut  = 0;
    uint32_t    m_Flags     = 0;
    std::string m_Asc;
    uint32_t    m_LastFrame = 0;
  };

  struct zCModelScriptAniDisable
  {
    std::string m_Name;
  };

  struct zCModelScriptEventTag
  {
    int32_t     m_Frame;
    std::string m_Tag;
    std::string m_Argument;
  };

  struct zCModelEvent
  {
    int32_t              m_Frame = 0;
    EModelScriptAniDef   m_Def   = DEF_NULL;
    EFightMode           m_Fmode = FM_NONE;
    std::vector<int32_t> m_Int;
    std::string          m_Slot, m_Item;
    std::string          m_Slot2;
  };

  struct zCModelTag
  {
    EModelScriptAniDef   m_Def   = DEF_NULL;
    std::string          m_Slot;
  };

  struct zCModelScriptEventMMStartAni
  {
    int32_t     m_Frame=0;
    std::string m_Animation;
    std::string m_Node;
  };

  struct zCModelScriptEventSfx
  {
    int32_t     m_Frame = 0;
    std::string m_Name;

    /**
      * If non-zero, will overwrite the default sound range of the character
      */
    float m_Range = 0.0f;

    /**
      * If true, the sound shall be played in a new "empty slot", which means
      * that it should not overwrite the currently playing sound.
      */
    bool m_EmptySlot = false;
  };

  struct zCModelScriptEventPfx
  {
    int32_t     m_Frame      = 0;
    int32_t     m_Num        = 0;
    std::string m_Name;
    std::string m_Pos;
    bool        m_isAttached = false;
  };
  struct zCModelScriptEventPfxStop
  {
    int32_t m_Frame = 0;
    int32_t m_Num   = 0;
  };

  /*
    * Camera shake / earthquake / tremor
    */
  struct zCModelScriptEventCamTremor
  {
    int32_t m_Frame      = 0;
    int32_t m_Range      = 0; // radius  - values seen 1000, 2000
    int32_t m_InnerRange = 0; // innerRange - values seen 500, 700, 1400 (unsure, or period / frequency)
    int32_t m_minTime    = 0; // time       - values seen 2,5 - blendOut / fade start
    int32_t m_maxTime    = 0; // unsure     - values seen 5,8 - just some strength or endTime, delta always 3 .... amplitude value and maybe only y variable ... first two values are ranges, second two could be time values or amplitude ... or aniComb quadrants?
    // FIXME: zCEarthQuake has default values radius=200, timeSec=5 and amplitudeCM=2,30,2(vec)
  };

  struct zCModelScript
  {
    int32_t     m_Version = 0;
    std::string m_Folder;
  };

  struct zcModelSource
  {
    int32_t m_Year    = 0;
    int16_t m_Month   = 0;
    int16_t m_Day     = 0;
    int16_t m_Hour    = 0;
    int16_t m_Minutes = 0;
    int32_t m_Seconds = 0;
    std::string m_File;
  };
}  // namespace ZenLoad
