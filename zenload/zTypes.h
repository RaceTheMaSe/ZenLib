#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <cinttypes>
#include "utils/mathlib.h"
#include "daedalus/ZString.h"

namespace ZenLoad
{
    enum class WorldVersion : uint16_t
    {
      VERSION_G1_08k = 0xFC01,
      VERSION_G26fix = 0x9900
    };

    /**
   * Used by zCVobSound
   */
    enum SoundMode : uint8_t
    {
      SM_LOOPING = 0,
      SM_ONCE,
      SM_RANDOM
    };

    /**
   * Used by zCVobSound
   */
    enum SoundVolType : uint8_t
    {
      SVT_SPHERE = 0,
      SVT_ELLIPSOID
    };

    enum MutateType : uint8_t
    {
      MT_NONE = 0,
      MT_TRIGGER,
      MT_UNTRIGGER,
      MT_ENABLE,
      MT_DISABLE,
      MT_TOGGLE_ENABLED
    };

    enum MoverBehavior : uint32_t
    {
      STATE_TOGGLE = 0,
      STATE_TRIGGER_CTRL,
      STATE_OPEN_TIMED,
      NSTATE_LOOP,
      NSTATE_SINGLE_KEYS
    };

    enum PosLerpType : uint32_t
    {
      CURVE  = 0,
      LINEAR
    };

    enum SpeedType : uint32_t
    {
      SEG_CONST          = 0,
      SLOW_START_END     = 1,
      SLOW_START         = 2,
      SLOW_END           = 3,
      SEG_SLOW_START_END = 4,
      SEG_SLOW_START     = 5,
      SEG_SLOW_END       = 6,
    };

    enum MoverMessage : uint8_t
    {
      GOTO_KEY_FIXED_DIRECTLY = 0,
      GOTO_KEY_FIXED_ORDER,
      GOTO_KEY_NEXT,
      GOTO_KEY_PREV
    };

    enum MotionType : uint8_t
    {
      MOTION_TYPE_UNDEF = 0,
      MOTION_TYPE_SMOOTH,
      MOTION_TYPE_LINEAR,
      MOTION_TYPE_STEP,
      MOTION_TYPE_SLOW,
      MOTION_TYPE_FAST,
      MOTION_TYPE_CUSTOM
    };

    enum FrameOfReference : uint8_t
    {
      FOR_WORLD = 0,
      FOR_OBJECT
    };

    enum LoopMode : uint8_t
    {
      LM_NONE = 0,
      LM_PINGPONG
    };

    enum SplineLerpMode : uint8_t
    {
      PATH_UNDEFINED = 0,
      PATH,
      PATH_IGNOREROLL,
      PATH_ROT_SAMPLES
    };

    enum FlareType : uint8_t
    {
      FT_NONE = 0,
      FT_CORONA,
      FT_GLOW,
      FT_FLARE
    };

    enum VisualCamAlign : uint8_t
    {
      CA_NONE  = 0,
      CA_YAW,
      CA_ALL
    };

    enum AnimMode : uint8_t
    {
      AM_NONE  = 0,
      AM_WIND,
      AM_WIND2
    };

    enum CutsceneRole : uint8_t
    {
      CSRequired = 0,
      CSRdelayable,
      CSRnonrequired
    };

    enum CSRunBehaviour : uint8_t
    {
      RUN_ALWAYS = 0,
      RUN_TIMES,
      RUN_PER_HOUR,
      RUN_PER_DAY
    };

    enum class ScreenFXEventTypes : uint8_t
    {
      BLEND_FADE_IN = 0,
      BLEND_FADE_OUT,
      CINEMA_FADE_IN,
      CINEMA_FADE_OUT,
      FOV_MORPH
    };

    enum class ManipulateEventTypes : uint8_t
    {
      EV_TAKEVOB = 0,
      EV_DROPVOB,
      EV_THROWVOB,
      EV_EXCHANGE,
      EV_USEMOB,
      EV_USEITEM,
      EV_INSERTINTERACTITEM,
      EV_REMOVEINTERACTITEM,
      EV_CREATEINTERACTITEM,
      EV_DESTROYINTERACTITEM,
      EV_PLACEINTERACTITEM,
      EV_EXCHANGEINTERACTITEM,
      EV_USEMOBWITHITEM,
      EV_CALLSCRIPT,
      EV_EQUIPITEM,
      EV_USEITEMTOSTATE,
      EV_TAKEMOB,
      EV_DROPMOB
    };

    enum class ConversationEventTypes : uint32_t
    {
      EV_PLAYANISOUND = 0,
      EV_PLAYANI,
      EV_PLAYSOUND,
      EV_LOOKAT,
      EV_OUTPUT,
      EV_OUTPUTSVM,
      EV_CUTSCENE,
      EV_WAITTILLEND,
      EV_ASK,
      EV_WAITFORQUESTION,
      EV_STOPLOOKAT,
      EV_STOPPOINTAT,
      EV_POINTAT,
      EV_QUICKLOOK,
      EV_PLAYANI_NOOVERLAY,
      EV_PLAYANI_FACE,
      EV_PROCESSINFOS,
      EV_STOPPROCESSINFOS,
      EV_OUTPUTSVM_OVERLAY,
      EV_SNDPLAY
    };

    enum class MovementEventTypes : uint8_t
    {
      EV_ROBUSTTRACE = 0,
      EV_GOTOPOS,
      EV_GOTOVOB,
      EV_GOROUTE,
      EV_TURN,
      EV_TURNTOPOS,
      EV_TURNTOVOB,
      EV_TURNAWAY,
      EV_JUMP,
      EV_SETWALKMODE,
      EV_WHIRLAROUND,
      EV_STANDUP,
      EV_CANSEENPC,
      EV_STRAFE,
      EV_GOTOFP,
      EV_DODGE,
      EV_BEAMTO,
      EV_ALIGNTOFP
    };

    enum class EventCoreTypes : uint8_t
    {
      TRIGGER = 0,
      UNTRIGGER,
      TOUCH,
      UNTOUCH,
      TOUCHLEVEL,
      DAMAGE
    };

    enum class EventAttackTypes : uint8_t
    {
      EV_ATTACKFORWARD = 0,
      EV_ATTACKLEFT,
      EV_ATTACKRIGHT,
      EV_ATTACKRUN,
      EV_ATTACKFINISH,
      EV_PARADE,
      EV_AIMAT,
      EV_SHOOTAT,
      EV_STOPAIM,
      EV_DEFEND
    };

    enum class CamEventTypes : uint8_t
    {
      EV_CAM_NOTHING = 0,
      EV_CAM_PLAY,
      EV_CAM_PAUSE,
      EV_CAM_RESUME,
      EV_CAM_STOP,
      EV_CAM_GOTO_KEY,
      EV_CAM_SET_DURATION,
      EV_CAM_SET_TO_TIME
    };

    enum class CamActivateEventTypes : uint8_t
    {
      EV_CAM_NOTHING = 0,
      EV_CAM_DEACTIVATE,
      EV_CAM_SETPOSREFERENCE,
      EV_CAM_SETTARGETREFERENCE
    };

    enum class EventMagicTypes : uint8_t
    {
      EV_OPEN = 0,
      EV_CLOSE,
      EV_MOVE,
      EV_INVEST,
      EV_CAST,
      EV_SETLEVEL,
      EV_SHOWSYMBOL,
      EV_SETFRONTSPELL,
      EV_CASTSPELL,
      EV_READY,
      EV_UNREADY
    };

    enum class EventStateTypes : uint8_t
    {
      EV_STARTSTATE = 0,
      EV_WAIT,
      EV_SETNPCTOSTATE,
      EV_SETTIME,
      EV_APPLYTIMEDOVERLAY
    };

    enum class EventWeaponTypes : uint8_t
    {
      EV_DRAWWEAPON = 0,
      EV_DRAWWEAPON1,
      EV_DRAWWEAPON2,
      EV_REMOVEWEAPON,
      EV_REMOVEWEAPON1,
      EV_REMOVEWEAPON2,
      EV_CHOOSEWEAPON,
      EV_FORCEREMOVEWEAPON,
      EV_ATTACK,
      EV_EQUIPBESTWEAPON,
      EV_EQUIPBESTARMOR,
      EV_UNEQUIPWEAPONS,
      EV_UNEQUIPARMOR,
      EV_EQUIPARMOR
    };

    using SectorIndex = uint32_t;
    enum : uint32_t { SECTOR_INDEX_INVALID = uint32_t(-1) };

  /**
   * @brief Maximum amount of nodes a skeletal mesh can render
   */
    const size_t MAX_NUM_SKELETAL_NODES = 96;

    struct WorldVertex
    {
      ZMath::float3 Position;
      ZMath::float3 Normal;
      ZMath::float2 TexCoord;
      uint32_t      Color{};
      bool operator==(const WorldVertex& other)
      {
        return (Position==other.Position &&
                Normal  ==other.Normal   &&
                TexCoord==other.TexCoord);
      }
    };

    struct SkeletalVertex
    {
      ZMath::float3 Normal;
      ZMath::float2 TexCoord;
      uint32_t      Color{};
      ZMath::float3 LocalPositions[4];
      unsigned char BoneIndices[4]{};
      float         Weights[4]{};
    };

    struct zMAT3
    {
      float v[3][3];

      ZMath::float3 getUpVector()    const { return {v[0][1], v[1][1], v[2][1]}; }
      ZMath::float3 getRightVector() const { return {v[0][0], v[1][0], v[2][0]}; }
      ZMath::float3 getAtVector()    const { return {v[0][2], v[1][2], v[2][2]}; }

      void setAtVector(const ZMath::float3& a) {
        v[0][2] = a.x;
        v[1][2] = a.y;
        v[2][2] = a.z;
        }
      void setUpVector(const ZMath::float3& a) {
        v[0][1] = a.x;
        v[1][1] = a.y;
        v[2][1] = a.z;
        }
      void setRightVector(const ZMath::float3& a) {
        v[0][0] = a.x;
        v[1][0] = a.y;
        v[2][0] = a.z;
        }

      ZMath::Matrix toMatrix(const ZMath::float3& position = ZMath::float3(0, 0, 0)) const {
        ZMath::Matrix m = ZMath::Matrix::CreateIdentity();
        m.Up(getUpVector());

        ZMath::float3 fwd = getAtVector();
        fwd.x *= -1.0f;
        fwd.y *= -1.0f;
        fwd.z *= -1.0f;

        m.Forward(fwd);
        m.Right(getRightVector());
        m.Translation(position);

        return m;
        }
    };

    struct zCModelAniSample
    {
      ZMath::float3 position;
      ZMath::float4 rotation;  // Quaternion
    };

    /**
   * @brief Base for an object parsed from a zen-file.
   *      Contains a map of all properties and their values as text
   */
    struct ParsedZenObject
    {
    };

    /**
     * @brief All kinds of information found in a zCMaterial
     */
    struct zCMaterialData : public ParsedZenObject
    {
      std::string matName;
      uint8_t     matGroup=0;
      uint32_t    color=0xFFFFFFFF;
      float       smoothAngle=0.f;
      std::string texture;
      std::string texScale;
      float       texAniFPS=0.f;
      uint8_t     texAniMapMode=0;
      std::string texAniMapDir;
      bool        noCollDet=false;
      bool        noLighmap=false;
      uint8_t     loadDontCollapse=0;
      std::string detailObject;
      float       detailTextureScale=0.f;
      uint8_t     forceOccluder=0;
      uint8_t     environmentMapping=0;
      float       environmentalMappingStrength=0.f;
      uint8_t     waveMode=0;
      uint8_t     waveSpeed=0;
      float       waveMaxAmplitude=0.f;
      float       waveGridSize=0.f;
      uint8_t     ignoreSun=0;
      uint8_t     alphaFunc=0;
      ZMath::float2 defaultMapping={};
    };

    struct zCVisualData : public ParsedZenObject
    {
    };

    struct zCAIBaseData : public ParsedZenObject
    {
    };

    struct TriggerListEntry
    {
      std::string triggerTarget;
      float       fireDelay=0;
    };

    struct zCOBBox3D {
      ZMath::float3 center;
      ZMath::float3 axis[3];
      ZMath::float3 extend;
      std::vector<zCOBBox3D> childs;
    };

    struct zCLightMap {
      ZMath::float3 origin;
      ZMath::float3 uvUp;
      ZMath::float3 uvRight;
      uint32_t texIndex; // pointer to zTexture in original
    };

    //#pragma pack(push, 1)
    /**
     * @brief Data of zCVob
     */
    struct zCVobData : public ParsedZenObject
    {
      enum EVobType
      {
        VT_Unknown  = -1,
        VT_zCVob    = 0,
        VT_zCVobLevelCompo,
        VT_ocDummyVobGenerator,
        VT_oCItem,
        VT_oCNpc,
        VT_oCMOB,
        VT_oCMobInter,
        VT_oCMobDoor,
        VT_oCMobBed,
        VT_oCMobFire,
        VT_oCMobItemSlot,
        VT_oCMobLadder,
        VT_oCMobSwitch,
        VT_oCMobWheel,
        VT_oCMobContainer,
        VT_zCVobLight,
        VT_zCVobSound,
        VT_zCVobSoundDaytime,
        VT_oCZoneMusic,
        VT_oCZoneMusicDefault,
        VT_zCMessageFilter,
        VT_zCCodeMaster,
        VT_zCTrigger,
        VT_zCTriggerList,
        VT_zCTriggerScript,
        VT_oCTriggerChangeLevel,
        VT_zCTriggerWorldStart,
        VT_oCTriggerTeleport,
        VT_oCCSTrigger,
        VT_zCMover,
        VT_zCVobStartpoint,
        VT_zCVobSpot,
        VT_zCVobWaypoint,
        VT_zCPFXController,
        VT_oCTouchDamage,
        VT_zCTriggerUntouch,
        VT_zCZoneReverb,
        VT_zCZoneReverbDefault,
        VT_ocZoneFog,
        VT_ocZoneFogDefault,
        VT_zCZoneVobFarPlane,
        VT_zCZoneVobFarPlaneDefault,
        VT_zCVobAnimate,
        VT_zCVobLensFlare,
        VT_zCTouchAnimate,
        VT_zCTouchAnimateSound,
        VT_zCMoverController,
        VT_zCVobLightPreset,
        VT_zCCSCamera,
        VT_zCCamTraj_KeyFrame,
        VT_zCEarthQuake,
        VT_zCMusicControler,
        VT_ocVisualFX,
        VT_ocVisFX_MultiTarget,
        VT_zCVobStair,
        VT_zCVobScreenFX,
        VT_zCLensFlareFX,
        VT_zCLensFlare,
        VT_zReference
      };

      EVobType       vobType = VT_zCVob;
      uint32_t       vobObjectID = uint32_t(-1);

      uint32_t       pack = 0;
      std::string    presetName;
      ZMath::float3  bbox[2]               = {};
      ZMath::Matrix  rotationMatrix        = {};
      zMAT3          rotationMatrix3x3     = {};
      ZMath::Matrix  worldMatrix           = {};
      ZMath::float3  position              = {};
      std::string    vobName;
      std::string    visual;
      bool           showVisual            = false;
      VisualCamAlign visualCamAlign        = VisualCamAlign::CA_NONE;
      AnimMode       visualAniMode         = AnimMode::AM_NONE;
      float          visualAniModeStrength = 0;
      float          vobFarClipScale       = 0;
      bool           cdStatic              = false;
      bool           cdDyn                 = false;
      bool           staticVob             = false;
      uint8_t        dynamicShadow         = 0;
      int32_t        zBias                 = 0;
      bool           isAmbient             = false;

      // References
      size_t visualReference              = 0;
      size_t aiReference                  = 0;
      size_t eventMgrReference            = 0;

      bool   physicsEnabled               = false;

      struct
      {
        struct
        {
          std::string   name;
          ZMath::float2 decalDim         = {};
          ZMath::float2 decalOffset      = {};
          bool          decal2Sided      = false;
          int32_t       decalAlphaFunc   = 0;
          float         decalTexAniFPS   = 0;
          uint8_t       decalAlphaWeight = 0;
          bool          ignoreDayLight   = false;
        } zCDecal;
      } visualChunk;

      // Sub-classes
      struct
      {
        std::string instanceName;
      } oCItem;

      struct
      {
        std::string focusName;
        int32_t     hitpoints       = 0;
        int32_t     damage          = 0;
        bool        moveable        = false;
        bool        takeable        = false;
        bool        focusOverride   = false;
        uint32_t    soundMaterial   = 0;
        std::string visualDestroyed;
        std::string owner;
        std::string ownerGuild;
        bool        isDestroyed     = false;
      } oCMOB;

      struct
      {
        int32_t     state         = 0;
        int32_t     stateTarget   = 0;
        int32_t     stateNum      = 0;
        std::string triggerTarget;
        std::string useWithItem;
        std::string conditionFunc;
        std::string onStateFunc;
        bool        rewind        = false;
      } oCMobInter;

      struct
      {
        std::string fireSlot;
        std::string fireVobtreeName;
      } oCMobFire;

      struct
      {
        bool        locked = false;
        std::string keyInstance;
        std::string pickLockStr;
        std::string contains;
      } oCMobLockable;

      struct
      {
        std::string contains;
      } oCMobContainer;

      struct
      {
        std::string lightPresetInUse;
        uint32_t    lightType     = 0;
        float       range         = 0;
        uint32_t    color         = 0;
        float       spotConeAngle = 0;
        bool        lightStatic   = false;
        uint32_t    lightQuality  = 0;
        std::string lensflareFX;
        struct {
          bool                  turnedOn        = false;
          std::vector<float>    rangeAniScale;
          float                 rangeAniFPS     = 0;
          bool                  rangeAniSmooth  = true;
          std::vector<uint32_t> colorAniList;
          float                 colorAniListFPS = true;
          bool                  colorAniSmooth  = true;
          bool                  canMove         = true;
          } dynamic;
      } zCVobLight;

      struct
      {
        float        sndVolume       = 0;
        SoundMode    sndMode         = SM_LOOPING;
        float        sndRandDelay    = 0;
        float        sndRandDelayVar = 0;
        bool         sndStartOn      = false;
        bool         sndAmbient3D    = false;
        bool         sndObstruction  = false;
        float        sndConeAngle    = 0;
        SoundVolType sndVolType      = SVT_SPHERE;
        float        sndRadius       = 0;
        std::string  sndName;
      } zCVobSound;

      struct
      {
        float       sndStartTime = 0.f;
        float       sndEndTime   = 0.f;
        std::string sndName2;
      } zCVobSoundDaytime;

      struct
      {
        bool     enabled     = false;
        uint32_t priority    = 0;
        bool     ellipsoid   = false;
        float    reverbLevel = 0;
        float    volumeLevel = 0;
        bool     loop        = false;
      } oCZoneMusic;

      struct
      {
        uint8_t  reverbPreset   = 0;
        float    reverbWeight   = 0;
        float    innerRangePerc = 0;
      } zCZoneReverb;

      struct
      {
        float fogRangeCenter = 0;
        float innerRangePerc = 0;
        uint32_t fogColor    = 0;
        bool  fadeOutSky     = false;
        bool  overrideColor  = false;
      } zCZoneZFog;

      struct
      {
        float farPlaneZ      = 0;
        float innerRangePerc = 0;
      } zCZoneVobFarPlane;

      struct
      {
        bool startOn = false;
      } zCVobAnimate;

      struct
      {
        std::string lensFlareFx;
      } zCVobLensFlare;

      struct
      {
        std::string sfxTouch;
      } zCTouchAnimateSound;

      struct
      {
        std::string              triggerTarget;
        bool                     orderRelevant       = false;
        bool                     firstFalseIsFailure = false;
        bool                     untriggerCancels    = false;
        std::string              triggerTargetFailure;
        std::vector<std::string> slaveVobName;
      } zCCodeMaster;

      struct
      {
        std::string triggerTarget;
        MutateType  onTrigger=MT_NONE;
        MutateType  onUntrigger=MT_NONE;
      } zCMessageFilter;

      struct
      {
        std::string triggerTarget;
        uint8_t     flags             = 0;
        uint8_t     filterFlags       = 0;
        std::string respondToVobName;
        int32_t     numCanBeActivated = 0;
        float       retriggerWaitSec  = 0;
        float       damageThreshold   = 0;
        float       fireDelaySec      = 0;
      } zCTrigger;

      struct
      {
        std::string triggerTarget;
      } zCTriggerUntouch;

      struct
      {
        std::string  triggerTarget;
        MoverMessage moverMessage = GOTO_KEY_FIXED_DIRECTLY;
        int32_t      gotoFixedKey = 0;
      } zCMoverController;

      struct
      {
        int32_t                       listProcess = 0;
        std::vector<TriggerListEntry> list;
      } zCTriggerList;

      struct
      {
        std::string scriptFunc;
        std::string triggerTarget;
      } zCTriggerScript;

      struct
      {
        MoverBehavior moverBehavior=MoverBehavior::NSTATE_LOOP;
        float         touchBlockerDamage = 0;
        float         stayOpenTimeSec    = 0;
        bool          moverLocked        = false;
        bool          autoLinkEnable     = false;
        bool          autoRotate         = false;
        float         moveSpeed          = 0;
        PosLerpType   posLerpType        = PosLerpType::CURVE;
        SpeedType     speedType          = SpeedType::SEG_CONST;
        std::vector<zCModelAniSample> keyframes;
        std::string   sfxOpenStart;
        std::string   sfxOpenEnd;
        std::string   sfxMoving;
        std::string   sfxCloseStart;
        std::string   sfxCloseEnd;
        std::string   sfxLock;
        std::string   sfxUnlock;
        std::string   sfxUseLocked;
      } zCMover;

      struct
      {
        std::string levelName;
        std::string startVobName;
      } oCTriggerChangeLevel;

      struct
      {
        std::string triggerTarget;
        bool        fireOnlyFirstTime = false;
      } oCTriggerWorldStart;

      struct
      {
        std::string pfxName;
        bool        killVobWhenDone = true;
        bool        pfxStartOn      = true;
      } zCPFXController;

      struct
      {
        float damage = 0;

        struct {
          bool barrier = false;
          bool blunt   = false;
          bool edge    = false;
          bool fire    = false;
          bool fly     = false;
          bool magic   = false;
          bool point   = false;
          bool fall    = false;
          } touchDamage;

        float damageRepeatDelaySec = 0;
        float damageVolDownScale   = 0;
        int   damageCollType       = 0;
      } oCTouchDamage;

      struct
      {
        float      time                = 0.f;
        float      angleRollDeg        = 0.f;
        float      camFOVScale         = 1.f;
        MotionType motionType          = MotionType::MOTION_TYPE_SLOW;
        MotionType motionTypeFOV       = MotionType::MOTION_TYPE_SMOOTH;
        MotionType motionTypeRoll      = MotionType::MOTION_TYPE_SMOOTH;
        MotionType motionTypeTimeScale = MotionType::MOTION_TYPE_SMOOTH;
        struct
        {
          float tension     = 0;
          float bias        = 0;
          float continuity  = 0;
          float timeScale   = 1.f;
          bool  timeIsFixed = false;
        } details;
        ZMath::Matrix originalPose;
      } zCCamTraj_KeyFrame;

      struct
      {
        FrameOfReference camTrj                         = FrameOfReference::FOR_WORLD;
        FrameOfReference targetTrj                      = FrameOfReference::FOR_WORLD;
        LoopMode         loopMode                       = LoopMode::LM_NONE;
        SplineLerpMode   splLerpMode                    = SplineLerpMode::PATH;
        bool             ignoreFORVobRotCam             = false;
        bool             ignoreFORVobRotTarget          = false;
        bool             adaptToSurroundings            = false;
        bool             easeToFirstKey                 = false;
        bool             easeFromLastKey                = false;
        float            totalTime                      = 0;
        std::string      autoCamFocusVobName;
        bool             autoCamPlayerMovable           = false;
        bool             autoCamUntriggerOnLastKey      = false;
        float            autoCamUntriggerOnLastKeyDelay = 0.f;
        int              sleepMode                      = 0;
        float            nextOnTimer                    = 0.f;
        bool             paused                         = false;
        bool             started                        = false;
        bool             gotoTimeMode                   = false;
        float            csTime                         = 0.f;
        std::vector<zCVobData> camSpline;
        std::vector<zCVobData> targetSpline;
      } zCCSCamera;

      struct
      {
        float         radius      = 0.f;
        float         timeSec     = 0.f;
        ZMath::float3 amplitudeCM = {};
      } zCEarthQuake;

      struct
      {
        std::string texName;
        FlareType   type      = FT_CORONA;
        float       size      = 0.f;
        float       alpha     = 0.f;
        float       rangeMin  = 0.f;
        float       rangeMax  = 0.f;
        float       posScale  = 0.f;
        float       fadeScale = 0.f;
      } zCFlare;

      struct
      {
        std::string name;
        float       fadeScale = 0.f;
      } zcLensFlareFX;

      std::vector<zCVobData> childVobs;
    };

    struct zCPortal
    {
      /**
       * Names of the sectors facing front/back.
       * An empty string means that there is no sector. (ie. at the border from an inside-room to the outside world)
       *
       */
      std::string frontSectorName;
      std::string backSectorName;

      SectorIndex frontSectorIndex=SECTOR_INDEX_INVALID; // Index to zCBspTreeData::sectors. Can be SECTOR_INDEX_INVALID.
      SectorIndex backSectorIndex =SECTOR_INDEX_INVALID;  // Index to zCBspTreeData::sectors. Can be SECTOR_INDEX_INVALID.
    };

    /**
     * A Sector defines inside-regions in an outdoor world. Using sectors, you can check if the player
     * entered the house of an other person, for example.
     */
    struct zCSector
    {
      std::string           name;
      std::vector<uint32_t> bspNodeIndices;         // Indices to zCBspTreeData::leafIndices
      std::vector<uint32_t> portalPolygonIndices;   // Indices to polygons of the worldmesh
    };

    struct zCBspNode
    {
      enum : size_t
      {
        INVALID_NODE = static_cast<uint32_t>(-1)
      };

      ZMath::float4 plane{};
      uint32_t front  = zCBspNode::INVALID_NODE;
      uint32_t back   = zCBspNode::INVALID_NODE;
      uint32_t parent = zCBspNode::INVALID_NODE;

      ZMath::float3 bbox3dMin{}, bbox3dMax{};

      size_t treePolyIndex = size_t(-1);
      size_t numPolys      = size_t(-1);
      bool   lodFlag       = false;
      ZMath::float3 light{};

      bool isLeaf() const
      {
        return front == INVALID_NODE && back == INVALID_NODE;
      }
    };

    struct zCBspTreeData
    {
      enum TreeMode
      {
        Indoor = 0,
        Outdoor = 1
      };

      /**
        * Whether this tree is an indoor or outdoor location
        */
      TreeMode               mode = TreeMode::Indoor;

      std::vector<zCBspNode> nodes;
      std::vector<uint32_t>  leafIndices;
      std::vector<uint32_t>  treePolyIndices;
      std::vector<uint32_t>  portalPolyIndices;

      std::vector<zCSector>  sectors;
      std::vector<zCPortal>  portals;

      uint32_t               version;
    };

    struct zCBspTreeData2
    {
      enum TreeMode
      {
        Indoor  = 0,
        Outdoor = 1
      };

      /**
        * Whether this tree is an indoor or outdoor location
        */
      TreeMode mode;

      std::vector<zCBspNode> nodes;
      std::vector<uint8_t>   leafIndices;
      std::vector<uint16_t>  treePolyIndices;
    };
    //#pragma pack(pop)

    // the various message types
    struct zCEventMessage : public ParsedZenObject
    {
      enum class EventMsgType : uint8_t
      {
        MT_Plain = 0,
        MT_oCMobMsg,
        MT_Common,
        MT_Core,
        MT_Mover,
        MT_MusicController,
        MT_Npc,
        MT_MsgCutscene,
        MT_MsgCSCamera,
        MT_MsgCSCameraActivate,
        MT_MsgAttack,
        MT_MsgDamage,
        MT_MsgMagic,
        MT_MsgState,
        MT_MsgUseItem,
        MT_MsgWeapon,
        MT_MsgMovement,
        MT_MsgConversation,
        MT_MsgManipulate,
        MT_ScreenFX
      };
      EventMsgType type    = EventMsgType::MT_Plain;
      uint32_t     subType = 0;
    };
    
    struct oCMobMsg : public zCEventMessage {};
    struct zCEvMsgCutscene : public zCEventMessage {};
    struct zCEventCommon : public zCEventMessage {};
    struct zCEventCore : public zCEventMessage
    {
      float    damage     = 0.f;
      uint32_t damageType = 0;
    };

    struct zCEventMover: public zCEventMessage {};
    struct zCEventMusicControler : public zCEventMessage {};
    struct zCCSCamera_EventMsg : public zCEventMessage
    {
      float    time   = 0.f;
      uint32_t key    = 0;
      uint32_t kfType = 0;
    };

    struct zCCSCamera_EventMsgActivate : public zCEventMessage
    {
      std::string refVobName = {};
    };

    struct oCNpcMessage : public zCEventMessage
    {
      uint32_t      targetMode    = 0;
      std::string   targetVobName = {};
      std::string   targetName    = {};
      ZMath::float3 targetPos     = {};
      float         angle         = 0.f;
      uint32_t      number        = 0;
    };

    struct oCMsgAttack : public oCNpcMessage
    {
      uint32_t combo = 0;
    };

    struct oCMsgConversation : public oCNpcMessage
    {
      Daedalus::ZString text = {};
      Daedalus::ZString name = {};
    };

    struct oCMsgDamage : public oCNpcMessage {};
    struct oCMsgMagic : public oCNpcMessage
    {
      uint32_t what         = 0;
      uint32_t level        = 0;
      bool     removeSymbol = false;
    };

    struct oCMsgManipulate : public oCNpcMessage
    {
      std::string schemeName  = {};
      int         targetState = 0;
    };

    struct oCMsgMovement : public oCNpcMessage {};

    struct oCMsgState : public oCNpcMessage {};
    struct oCMsgUseItem : public oCNpcMessage {};
    struct oCMsgWeapon : public oCNpcMessage {};
    struct zCEventScreenFX : public zCEventMessage
    {
      float    duration = 0.f;
      uint32_t color    = 0;
      float    fovDeg   = 0.f;
    };

    struct zCCSRole : public ParsedZenObject
    {
      bool         mustBeAlive = false;
      std::string  roleName    = {};
      CutsceneRole roleType    = CutsceneRole::CSRequired;
      // integratedVob
    };

    struct zCCSRoleVob : public zCVobData
    {
    };

    struct zCCSBlockSync : public ParsedZenObject
    {
      std::vector<uint32_t> roleAss = {};
    };

    struct zCCSAtomicBlock : public ParsedZenObject
    {
      float          time         = 0.f;
      zCEventMessage command;
      bool           synchronized = false;
    };

    struct zCCSBlock : public ParsedZenObject
    {
      std::string                  blockName = {};
      std::vector<float>           time;
      std::vector<ParsedZenObject> syncBlocks; // FIXME: those have the time values ... to unify this, the parser has to be specialized / optimized / reworked around cssyncblock, child sync block and atomic block nesting
      std::vector<zCCSAtomicBlock> atomicBlocks;
    };

    struct zCCSSyncBlock : public zCCSBlock
    {
      std::vector<zCCSAtomicBlock> atomicSyncBlocks;
      std::vector<uint32_t>        roleAss;
    };

    struct zCCSProps
    {
      bool           globalCutscene    = false;
      bool           csLoop            = false;
      bool           hasToBeTriggered  = false;
      float          distance          = 500.f;
      float          range             = 2000.f;
      uint32_t       numLockedBlocks   = 0;
      CSRunBehaviour runBehaviour      = CSRunBehaviour::RUN_ALWAYS;
      uint32_t       runBehaviourValue = 0;
      std::string    stageName         = {};
      std::string    scriptFuncOnStop;
    };

    struct zCCutscene : public zCCSBlock
    {
      zCCSProps                  props;
      std::vector<zCCSRole>      roles;
      std::vector<zCCSRoleVob>   roleVobs;
      zCVobData                  mainRoleVob;
    };

    struct zCEventManagerData : public ParsedZenObject
    {
      bool       cleared;
      bool       active;
      zCCutscene cutsceneData;
    };

    struct zCCSLibData : public zCCSBlock
    {
      uint32_t numItems = 0;
    };

    struct zCWaypointData : public ParsedZenObject
    {
      std::string   wpName;
      int32_t       waterDepth;
      bool          underWater;
      ZMath::float3 position;
      ZMath::float3 direction;
    };

    struct zCWayNetData : public ParsedZenObject
    {
      uint32_t                               waynetVersion;
      std::vector<zCWaypointData>            waypoints;
      std::vector<std::pair<size_t, size_t>> edges;
    };

    /**
  * @brief All kinds of information found in a oCWorld
  */
    struct oCWorldData : public ParsedZenObject
    {
      std::vector<zCVobData> rootVobs;
      zCWayNetData           waynet       = {};
      zCBspTreeData          bspTree      = {};
      size_t                 numVobsTotal = 0;
    };

#pragma pack(push, 1)
    // Information about the whole file we are reading here
    struct BinaryFileInfo
    {
      uint32_t version;
      uint32_t size;
    };

    // Information about a single chunk
    struct BinaryChunkInfo
    {
      uint16_t id;
      uint32_t length;
    };

    /**
   * @brief Merged flags-struct to contain all flags from all versions of the files
   */
    struct PolyFlags
    {
      uint8_t portalPoly          : 2;
      uint8_t occluder            : 1;
      uint8_t sectorPoly          : 1;
      uint8_t mustRelight         : 1;
      uint8_t portalIndoorOutdoor : 1;
      uint8_t ghostOccluder       : 1;
      uint8_t noDynLightNear      : 1;
      uint16_t sectorIndex        :16;
      uint8_t lodFlag             : 1;
      uint8_t normalMainAxis      : 2;
    };

    struct PolyFlags2_6fix
    {
      uint8_t portalPoly          : 2;
      uint8_t occluder            : 1;
      uint8_t sectorPoly          : 1;
      uint8_t mustRelight         : 1;
      uint8_t portalIndoorOutdoor : 1;
      uint8_t ghostOccluder       : 1;
      uint8_t noDynLightNear      : 1;
      uint16_t sectorIndex        :16;

      PolyFlags2_6fix() 
      : portalPoly(0),
        occluder(0),
        sectorPoly(0),
        mustRelight(0),
        portalIndoorOutdoor(0),
        ghostOccluder(0),
        noDynLightNear(0),
        sectorIndex(0) { }

      PolyFlags2_6fix(const PolyFlags& p) 
      : portalPoly(p.portalPoly),
        occluder(p.occluder),
        sectorPoly(p.sectorPoly),
        mustRelight(p.mustRelight),
        portalIndoorOutdoor(p.portalIndoorOutdoor), 
        ghostOccluder(p.ghostOccluder),
        noDynLightNear(p.noDynLightNear),
        sectorIndex(p.sectorIndex) { }

      PolyFlags generify() const
      {
        PolyFlags p{};
        memset(&p, 0, sizeof(p));

        p.portalPoly          = portalPoly;
        p.occluder            = occluder;
        p.sectorPoly          = sectorPoly;
        p.mustRelight         = mustRelight;
        p.portalIndoorOutdoor = portalIndoorOutdoor;
        p.ghostOccluder       = ghostOccluder;
        p.noDynLightNear      = noDynLightNear;
        p.sectorIndex         = sectorIndex;

        return p;
      }
    };

    struct PolyFlags1_08k
    {
      uint8_t portalPoly          : 2;
      uint8_t occluder            : 1;
      uint8_t sectorPoly          : 1;
      uint8_t lodFlag             : 1;
      uint8_t portalIndoorOutdoor : 1;
      uint8_t ghostOccluder       : 1;
      uint8_t normalMainAxis      : 2;
      uint16_t sectorIndex        :16;

      PolyFlags1_08k()
      : portalPoly(0),
        occluder(0),
        sectorPoly(0),
        lodFlag(0),
        portalIndoorOutdoor(0),
        ghostOccluder(0),
        normalMainAxis(0),
        sectorIndex(0) { }

      PolyFlags1_08k(const PolyFlags& p)
      : portalPoly(p.portalPoly),
        occluder(p.occluder),
        sectorPoly(p.sectorPoly),
        lodFlag(p.lodFlag),
        portalIndoorOutdoor(p.portalIndoorOutdoor), 
        ghostOccluder(p.ghostOccluder),
        normalMainAxis(p.normalMainAxis),
        sectorIndex(p.sectorIndex) { }

      PolyFlags generify() const
      {
        PolyFlags p{};
        memset(&p, 0, sizeof(p));

        p.portalPoly          = portalPoly;
        p.occluder            = occluder;
        p.sectorPoly          = sectorPoly;
        p.lodFlag             = lodFlag;
        p.portalIndoorOutdoor = portalIndoorOutdoor;
        p.ghostOccluder       = ghostOccluder;
        p.normalMainAxis      = normalMainAxis;
        p.sectorIndex         = sectorIndex;

        return p;
      }
    };

    struct zTMSH_FeatureChunk
    {
      float uv[2]{};
      uint32_t lightStat{};
      ZMath::float3 vertNormal;
    };

#pragma pack(pop)

    /**
  * @brief Information about a triangle in the World. Contains whether the triangle 
  *      belongs to an outside/inside location, the static lighting colors of the edges,
  *      material-information and to which sector this belongs, amongst others
  */
    struct WorldTriangle
    {
      /**
        * @brief Returns whether this triangle is outside or inside
        */
      bool isOutside() const { return !flags.sectorPoly; }

      /**
        * @brief Returns the interpolated lighting value for the given position on the triangle
        */

      ZMath::float4 interpolateLighting(const ZMath::float3& /*position*/) const {
        /*float u,v,w;
        ZMath::barycentric(position, vertices[0].Position, vertices[1].Position, vertices[2].Position, u, v, w);

        ZMath::float4 c[3];
        c[0].fromABGR8(vertices[0].Color);
        c[1].fromABGR8(vertices[1].Color);
        c[2].fromABGR8(vertices[2].Color);

        return u * c[0] + v * c[1] + w * c[2];*/

        return {1, 1, 1, 1};  // TODO: Implementation missing without GLM
        }

      /**
        * @brief Flags taken from the original ZEN-File
        */
      PolyFlags flags{};

      /**
        * @brief Index to the lightmap stored in the zCMesh
        */
      int16_t lightmapIndex = -1;

      /**
        * @brief Vertices belonging to this triangle
        */
      WorldVertex vertices[3]{};

      /**
        * @brief Index of submesh correlated with this triangle
        */
      int16_t submeshIndex = -1;
    };

    /**
  * @brief Simple generic packed mesh, containing all useful information of a (lod-level of) zCMesh and zCProgMeshProto
  */
    // FIXME: Probably move this to renderer-package
    struct PackedMesh
    {
      struct SubMesh
      {
        zCMaterialData        material;
        size_t                indexOffset = 0;
        size_t                indexSize   = 0;
        std::vector<int16_t>  triangleLightmapIndices;  // Index values to the texture found in zCMesh
      };

      std::vector<WorldTriangle> triangles;  // Use index / 3 to access these
      std::vector<WorldVertex>   vertices;
      std::vector<uint32_t>      indices;
      std::vector<uint32_t>      verticesId; // only for morph meshes
      std::vector<SubMesh>       subMeshes;
      ZMath::float3              bbox[2];
      bool                       isUsingAlphaTest = false;
    };

    struct PackedSkeletalMesh
    {
      struct SubMesh
      {
        zCMaterialData material;
        size_t         indexOffset = 0;
        size_t         indexSize   = 0;
      };

      ZMath::float3               bbox[2];
      std::vector<SkeletalVertex> vertices;
      std::vector<uint32_t>       indices;
      std::vector<SubMesh>        subMeshes;
    };

#pragma pack(push, 4)

    struct VobObjectInfo
    {
      ZMath::Matrix worldMatrix;
      ZMath::float4 color;
    };

    struct SkeletalMeshInstanceInfo
    {
      ZMath::Matrix worldMatrix;
      ZMath::Matrix nodeTransforms[MAX_NUM_SKELETAL_NODES];
      ZMath::float4 color;
    };

    struct zDate
    {
      uint32_t year;
      uint16_t month;
      uint16_t day;
      uint16_t hour;
      uint16_t minute;
      uint16_t second;
    };

    struct zTPlane
    {
      float distance{};
      ZMath::float3 normal;
    };
#pragma pack(pop)

    struct zWedge
    {
      ZMath::float3 m_Normal;
      ZMath::float2 m_Texcoord;
      uint16_t m_VertexIndex{};
    };

    struct zTriangle
    {
      uint16_t m_Wedges[3];
    };

    struct zTriangleEdges
    {
      uint16_t m_Edges[3];
    };

    struct zEdge
    {
      uint16_t m_Wedges[2];
    };

#pragma pack(push, 4)
    struct zTNodeWedgeNormal
    {
      ZMath::float3 m_Normal;
      int m_NodeIndex{};
    };

    struct zTLODParams
    {
      float m_LodStrength;
      float m_ZDisplace2;
      float m_MorphPercent;
      int32_t m_MinNumVertices;
    };

#pragma pack(pop)

#pragma pack(push, 1)
    class zCModelNodeInst
    {
    public:
    };

    struct zTMdl_AniSample
    {
      uint16_t rotation[3];  // 16bit quantized euler angles
      uint16_t position[3];  // 16bit quantized float3
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct zTWeightEntry
    {
      // Weight and position of the vertex.
      // This vertexposition is in the local space of the joint-Matrix!
      float weight{};
      ZMath::float3 localVertexPosition;

      // Nodeindex this belongs to
      unsigned char nodeIndex{};
    };
#pragma pack(pop)

    enum zTMdl_AniEventType
    {
      zMDL_EVENT_TAG,
      zMDL_EVENT_SOUND,
      zMDL_EVENT_SOUND_GRND,
      zMDL_EVENT_ANIBATCH,
      zMDL_EVENT_SWAPMESH,
      zMDL_EVENT_HEADING,
      zMDL_EVENT_PFX,
      zMDL_EVENT_PFX_GRND,
      zMDL_EVENT_PFX_STOP,
      zMDL_EVENT_SETMESH,
      zMDL_EVENT_MM_STARTANI,
      zMDL_EVENT_CAM_TREMOR
    };
}  // namespace ZenLoad
