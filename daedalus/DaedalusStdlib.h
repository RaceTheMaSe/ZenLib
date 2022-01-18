#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "daedalus/ZString.h"

namespace Daedalus {
  class DaedalusVM;

  /**
    * Basic gametype. Needed for registering C_Class members and sky configuration in REGoth
    */
  enum class GameType {
    GT_Gothic1,
    GT_Gothic2
    };

  namespace GEngineClasses
  {
    // const int MAX_CHAPTER      = 5;
    const int MAX_MISSIONS        = 5;
    const int MAX_HITCHANCE       = 5;
    // const int ATR_INDEX_MAX    = 8;
    const int ITM_TEXT_MAX        = 6;
    const int GIL_MAX             = 66;
    const int VFX_NUM_USERSTRINGS = 5;

    enum {
      DAM_INVALID =   0,
      DAM_BARRIER =   1,
      DAM_BLUNT   =   2,
      DAM_EDGE    =   4,
      DAM_FIRE    =   8,
      DAM_FLY     =  16,
      DAM_MAGIC   =  32,
      DAM_POINT   =  64,
      DAM_FALL    = 128
      };
  
    enum {
      DAM_INDEX_BARRIER = 0,  // nur der Vollstandigkeit und Transparenz wegen hier definiert ( _NICHT_ verwenden )
      DAM_INDEX_BLUNT   = 1,
      DAM_INDEX_EDGE,
      DAM_INDEX_FIRE,
      DAM_INDEX_FLY,
      DAM_INDEX_MAGIC,
      DAM_INDEX_POINT,
      DAM_INDEX_FALL,// nur der Vollstandigkeit und Transparenz wegen hier definiert ( _NICHT_ verwenden )
      DAM_INDEX_MAX
      };

    enum {
      PROT_BARRIER   = DAM_INDEX_BARRIER,
      PROT_BLUNT     = DAM_INDEX_BLUNT,
      PROT_EDGE      = DAM_INDEX_EDGE,
      PROT_FIRE      = DAM_INDEX_FIRE,
      PROT_FLY       = DAM_INDEX_FLY,
      PROT_MAGIC     = DAM_INDEX_MAGIC,
      PROT_POINT     = DAM_INDEX_POINT,
      PROT_FALL      = DAM_INDEX_FALL,
      PROT_INDEX_MAX = DAM_INDEX_MAX
      };

    enum ENPCType : int32_t {
      NPCTYPE_AMBIENT     = 0,
      NPCTYPE_MAIN        = 1,
      NPCTYPE_FRIEND      = 2,
      NPCTYPE_OCAMBIENT   = 3,
      NPCTYPE_OCMAIN      = 4,
      NPCTYPE_BL_AMBIENT  = 5,
      NPCTYPE_TAL_AMBIENT = 6,
      NPCTYPE_BL_MAIN     = 7
      };

    enum class ENPCTypeG1 : int32_t {
      NPCTYPE_AMBIENT      = 0,
      NPCTYPE_MAIN         = 1,
      NPCTYPE_GUARD        = 2,
      NPCTYPE_FRIEND       = 3,
      NPCTYPE_MINE_AMBIENT = 4,
      NPCTYPE_MINE_GUARD   = 5,
      NPCTYPE_OW_AMBIENT   = 6,
      NPCTYPE_OW_GUARD     = 7,
      NPCTYPE_ROGUE        = 8
      };

    enum Move : int32_t {
      MOVE_RUN          = 1,  // Opponents in my focus + who stands in between? (G)
      MOVE_RUNBACK      = 2,  // Opponents in my focus
      MOVE_JUMPBACK     = 3,
      MOVE_TURN         = 4,  // Always up to opponents in focus (never interrupted by new action, at most by enemy attack)
      MOVE_STRAFE       = 5,  // (Direction is decided by the native-code)
      MOVE_ATTACK       = 6,  // in ComboZone = combo / in the run = storm attack?
      MOVE_SIDEATTACK   = 7,
      MOVE_FRONTATTACK  = 8,
      MOVE_TRIPLEATTACK = 9,
      MOVE_WHIRLATTACK  = 10,
      MOVE_MASTERATTACK = 11,
      MOVE_TURNTOHIT    = 15, // Version > 0.92 => No Effect
      MOVE_PARADE       = 17, // (keine Attacke = oben)
      MOVE_STANDUP      = 18,
      MOVE_WAIT         = 19,
      MOVE_WAIT_LONGER  = 23, // Achtung: Muss 23 sein, da im Programm noch einige unbenutze Moves zwischen 19 und 22 gibt
      MOVE_WAIT_EXT     = 24, // Achtung: Muss 24 sein, da im Programm noch einige unbenutze Moves zwischen 19 und 22 gibt

      MAX_MOVE          = 6,
      MAX_FIGHTAI       = 50  // Number of existing Fight-AIs // Constant NOT redundant in the native-code, can be changed in scripts as required
      };

    // Music transition types
    enum ETransitionType : int32_t {
      TRANSITION_TYPE_NULL = 0,
      TRANSITION_TYPE_NONE = 1,
      TRANSITION_TYPE_GROOVE = 2,
      TRANSITION_TYPE_FILL = 3,
      TRANSITION_TYPE_BREAK = 4,
      TRANSITION_TYPE_INTRO = 5,
      TRANSITION_TYPE_END = 6,
      TRANSITION_TYPE_ENDANDINTRO = 7
      };

    enum ESubTransitionType : int32_t {
      TRANSITION_SUB_TYPE_NULL = 0,
      TRANSITION_SUB_TYPE_IMMEDIATE = 1,
      TRANSITION_SUB_TYPE_BEAT = 2,
      TRANSITION_SUB_TYPE_MEASURE = 3
      };

    namespace MenuConstants {
      const int MAX_USERSTRINGS = 10;
      const int MAX_ITEMS = 150;
      const int MAX_EVENTS = 10;
      const int MAX_SEL_ACTIONS = 5;
      const int MAX_USERVARS = 4;
      // const int SEL_EVENT_UNDEF = 0;

      enum ESelEvent {
        SEL_EVENT_EXECUTE  = 1,
        SEL_EVENT_CHANGED  = 2,
        SEL_EVENT_LEAVE    = 3,
        SEL_EVENT_TIMER    = 4,
        SEL_EVENT_CLOSE    = 5,
        SEL_EVENT_INIT     = 6,
        SEL_EVENT_SEL_PREV = 7,
        SEL_EVENT_SEL_NEXT = 8
        };

      enum ESelAction {
        SEL_ACTION_UNDEF        = 0,
        SEL_ACTION_BACK         = 1,
        SEL_ACTION_STARTMENU    = 2,
        SEL_ACTION_STARTITEM    = 3,
        SEL_ACTION_CLOSE        = 4,
        SEL_ACTION_CONCOMMANDS  = 5,
        SEL_ACTION_PLAY_SOUND   = 6,
        SEL_ACTION_EXECCOMMANDS = 7
        };
      }  // namespace MenuConstants

    struct Instance {
      Instance() = default;
      size_t  instanceSymbol=0;
      int32_t useCount      =0;

      /**
        * Space for the user to save something. Will not be touched at all by ZenLib.
        * Note: This is set to nullptr after creation.
        */
      void* userPtr=nullptr;
      };

    struct C_ParticleFX : Instance {
      // 1) Emitter: zeitliches  Austoss-Verhalten, particles-per-second
      float   ppsValue        =0.f;
      ZString ppsScaleKeys_S;
      int32_t ppsIsLooping    =0;
      int32_t ppsIsSmooth     =0;
      float   ppsFPS          =0.f;
      ZString ppsCreateEm_S;
      float   ppsCreateEmDelay=0.f;

      // 2) Emitter: raeumliches Austoss-Verhalten
      ZString shpType_S;              //  "point, line, box, circle, sphere, mesh"
      ZString shpFOR_S;               //  "object,world"
      ZString shpOffsetVec_S;
      ZString shpDistribType_S;       //  "RAND, UNIFORM, WALK"
      float   shpDistribWalkSpeed=0.f;
      int32_t shpIsVolume=0;
      ZString shpDim_S;               //  "", "30", "10 20 30", "30", "30", "" //  line: nur 1 Dimension !=0 // shape Dimensions
      ZString shpMesh_S;              //  "cross.3ds"
      int32_t shpMeshRender_B=0;
      ZString shpScaleKeys_S;         //  "[1.0] [0.8 0.9 0.2] [1.0]"
      int32_t shpScaleIsLooping=0;
      int32_t shpScaleIsSmooth=0;
      float   shpScaleFPS=0.f;

      // 3) Partikel: Start Richtung/Speed:
      ZString dirMode_S;          //  "DIR, TARGET, MESH_POLY"
      ZString dirFOR_S;           //  "OBJECT, WORLD"
      ZString dirModeTargetFOR_S;
      ZString dirModeTargetPos_S; //  "30 23 67"
      float   dirAngleHead=0;
      float   dirAngleHeadVar=0;
      float   dirAngleElev=0;
      float   dirAngleElevVar=0;
      float   velAvg=0;
      float   velVar=0;

      // 4) Partikel: Lebensdauer
      float lspPartAvg;
      float lspPartVar;

      // 5) Partikel: Flugverhalten (gravity, nicht-linear?, mesh-selfRot?,..)
      // grav: a) nur Y, b) XYZ, c) auf Ziel zu steuern
      // ZString  flyMode_S;                //  "LINEAR, LIN_SINUS,.."
      // flyMeshSelfRotSpeedMin, flyMeshSelfRotSpeedMax
      ZString flyGravity_S;
      int32_t flyCollDet_B=0;

      // 6) Partikel: Visualisierung
      ZString visName_S;            //  "NAME_V0_A0.TGA/.3DS"  (Variation, Animation)
      ZString visOrientation_S;     //  "NONE, VELO"
      int32_t visTexIsQuadPoly=1;   //  0=triMesh, 1=quadMesh
      float   visTexAniFPS=0.f;
      int32_t visTexAniIsLooping=0; //  0=oneShot, 1=looping
      // color    (nur Tex, lifeSpan-Sync)
      ZString visTexColorStart_S;
      ZString visTexColorEnd_S;
      // size-ani    (nur Tex, lifeSpan-Sync)
      ZString visSizeStart_S;
      float   visSizeEndScale=0;
      // alpha    (lifeSpan-Sync)
      ZString visAlphaFunc_S;
      float   visAlphaStart     =0.f;
      float   visAlphaEnd       =0.f;

      // 7) misc effects

      // trail
      float   trlFadeSpeed      =0.f;
      ZString trlTexture_S;
      float   trlWidth          =0.f;

      // marks
      float   mrkFadeSpeed      =0;
      ZString mrkTexture_S;
      float   mrkSize           =0;

      // flocking
      ZString flockMode;
      float   flockStrength     =0.f;

      // local frame of reference override
      // calculates the position of the particles each frame relative to the emitters pos/rot
      // can be expensive
      // WARNING: in comb with flyCollDet_B this can be a performance-hog deluxe
      int32_t useEmittersFOR    =0;

      // optional you can set a valid timeperiod in which this pfx should be rendered (e.g. "8 22": should be rendererd from 8 to 22 o clock")
      ZString timeStartEnd_S;

      // with the next setting you can define weather this pfx is an ambient pfx, thus can be disabled in the gothic.ini with the value [ENGINE]/noAmbientPFX
      int32_t m_bIsAmbientPFX=0;
      };

    struct CFx_Base : Instance {
      ZString      visName_S;
      ZString      visSize_S;
      float        visAlpha=0.f;
      ZString      visAlphaBlendFunc_S;
      float        visTexAniFPS=0.f;
      int32_t      visTexAniIsLooping=0;
      ZString      emTrjMode_S; // none, target, line, spline, random
      ZString      emTrjOriginNode;
      ZString      emTrjTargetNode;
      float        emTrjTargetRange=0.f;
      float        emTrjTargetAzi=0.f;
      float        emTrjTargetElev=0.f;
      int32_t      emTrjNumKeys=0;
      int32_t      emTrjNumKeysVar=0;
      float        emTrjAngleElevVar=0.f;
      float        emTrjAngleHeadVar=0.f;
      float        emTrjKeyDistVar=0.f;
      ZString      emTrjLoopMode_S;
      ZString      emTrjEaseFunc_S;
      float        emTrjEaseVel=0.f;
      float        emTrjDynUpdateDelay=0.f;
      int32_t      emTrjDynUpdateTargetOnly=0;
      ZString      emFXCreate_S;
      ZString      emFXInvestOrigin_S;
      ZString      emFXInvestTarget_S;
      float        emFXTriggerDelay=0.f;
      int32_t      emFXCreatedOwnTrj=0;
      ZString      emActionCollDyn_S;   // CREATE, BOUNCE, CREATEONCE, NORESP, COLLIDE
      ZString      emActionCollStat_S;  // CREATE, BOUNCE, CREATEONCE, NORESP, COLLIDE, CREATEQUAD
      ZString      emFXCollStat_S;
      ZString      emFXCollDyn_S;
      ZString      emFXCollDynPerc_S;
      ZString      emFXCollStatAlign_S; // TRAJECTORY, COLLISIONNORMAL
      ZString      emFXCollDynAlign_S;
      float        emFXLifeSpan=0.f;
      int32_t      emCheckCollision=0;
      int32_t      emAdjustShpToOrigin=0;
      float        emInvestNextKeyDuration=0.f;
      float        emFlyGravity=0.f;
      ZString      emSelfRotVel_S;
      ZString      userString[VFX_NUM_USERSTRINGS];
      ZString      lightPresetName;
      ZString      sfxID;
      int32_t      sfxIsAmbient=0;
      int32_t      sendAssessMagic=0;
      float        secsPerDamage=0.f;
      };

    // particle effect related key vars
    struct C_ParticleFXEmitKey : Instance {
      // vars which influence all particles all time
      ZString visName_S;
      float   visSizeScale=0.f;
      float   scaleDuration=0.f;     // time to reach full scale at this key for relevant vars (size, alpha, etc.)

      float   pfx_ppsValue=0.f;
      int     pfx_ppsIsSmoothChg=0;  // changes pps smoothing of pfx if set to 1 and pfx pps scale keys are set
      int     pfx_ppsIsLoopingChg=0; // changes looping of pfx if set to 1
      float   pfx_scTime=0.f;
      ZString pfx_flyGravity_S;

      // vars which influence particles at creation level only
      ZString pfx_shpDim_S;
      int     pfx_shpIsVolumeChg=0;  // changes volume rendering of pfx if set to 1
      float   pfx_shpScaleFPS=0.f;
      float   pfx_shpDistribWalkSpeed=0.f;
      ZString pfx_shpOffsetVec_S;
      ZString pfx_shpDistribType_S;
      ZString pfx_dirMode_S;
      ZString pfx_dirFOR_S;
      ZString pfx_dirModeTargetFOR_S;
      ZString pfx_dirModeTargetPos_S;
      float   pfx_velAvg=0.f;
      float   pfx_lspPartAvg=0.f;
      float   pfx_visAlphaStart=0.f;

      ZString lightPresetName;
      float   lightRange=0.f;
      ZString sfxID;
      int     sfxIsAmbient=0;
      ZString emCreateFXID;

      float   emFlyGravity=0.f;
      ZString emSelfRotVel_S;
      ZString emTrjMode_S;
      float   emTrjEaseVel=0.f;
      int     emCheckCollision=0;
      float   emFXLifeSpan=0.f;
      };

    struct C_Menu : Instance {
      enum EFlags : int32_t {
        MENU_OVERTOP       =  1,
        MENU_EXCLUSIVE     =  2,
        MENU_NOANI         =  4,
        MENU_DONTSCALE_DIM =  8,
        MENU_DONTSCALE_POS = 16,
        MENU_ALIGN_CENTER  = 32,
        MENU_SHOW_INFO     = 64
        };

      C_Menu() = default;
      ZString backPic;
      ZString backWorld;
      int32_t posx=0, posy=0;
      int32_t dimx=0, dimy=0;
      int32_t alpha=0;
      ZString musicTheme;
      int32_t eventTimerMSec=0;
      ZString items[MenuConstants::MAX_ITEMS];
      int32_t flags=0;
      int32_t defaultOutGame=0;
      int32_t defaultInGame=0;
      };

    struct C_Menu_Item : Instance {
      enum EFlags : int32_t {
        IT_CHROMAKEYED   =     1,
        IT_TRANSPARENT   =     2,
        IT_SELECTABLE    =     4,
        IT_MOVEABLE      =     8,
        IT_TXT_CENTER    =    16,
        IT_DISABLED      =    32,
        IT_FADE          =    64,
        IT_EFFECTS_NEXT  =   128,
        IT_ONLY_OUT_GAME =   256,
        IT_ONLY_IN_GAME  =   512,
        IT_PERF_OPTION   =  1024,
        IT_MULTILINE     =  2048,
        IT_NEEDS_APPLY   =  4096,
        IT_NEEDS_RESTART =  8192,
        IT_EXTENDED_MENU = 16384
        };

      enum EType : int32_t {
        MENU_ITEM_UNDEF     = 0,
        MENU_ITEM_TEXT      = 1,
        MENU_ITEM_SLIDER    = 2,
        MENU_ITEM_INPUT     = 3,
        MENU_ITEM_CURSOR    = 4,
        MENU_ITEM_CHOICEBOX = 5,
        MENU_ITEM_BUTTON    = 6,
        MENU_ITEM_LISTBOX   = 7
        };

      C_Menu_Item() = default;
      ZString     fontName;
      ZString     text[MenuConstants::MAX_USERSTRINGS];
      ZString     backPic;
      ZString     alphaMode;
      int32_t     alpha=0;
      int32_t     type =0;
      int32_t     onSelAction[MenuConstants::MAX_SEL_ACTIONS]={};
      ZString     onSelAction_S[MenuConstants::MAX_SEL_ACTIONS];
      ZString     onChgSetOption;
      ZString     onChgSetOptionSection;

      int32_t     onEventAction[MenuConstants::MAX_EVENTS]={};
      int32_t     posx=0, posy=0;
      int32_t     dimx=0, dimy=0;  // -1 = AUTODETECT (FONTWISE)
      float       sizeStartScale=0.f;
      int32_t     flags=0;
      float       openDelayTime=0;
      float       openDuration=0;
      float       userfloat[MenuConstants::MAX_USERVARS]={};
      ZString     userString[MenuConstants::MAX_USERVARS];
      int32_t     frameSizeX=0;
      int32_t     frameSizeY=0;
      ZString     hideIfOptionSectionSet;
      ZString     hideIfOptionSet;
      int32_t     hideOnValue=0;
      };

    struct C_Npc : Instance {
      enum EAttributes : int32_t {
        EATR_HITPOINTS      = 0,
        EATR_HITPOINTSMAX   = 1,
        EATR_MANA           = 2,
        EATR_MANAMAX        = 3,
        EATR_STRENGTH       = 4,
        EATR_DEXTERITY      = 5,
        EATR_REGENERATEHP   = 6,
        EATR_REGENERATEMANA = 7,

        EATR_MAX            = 8
        };

      enum ENPCFlag : int32_t {
        EFLAG_NONE      = 0,
        EFLAG_FRIENDS   = 1 << 0,
        EFLAG_IMMORTAL  = 1 << 1,
        EFLAG_GHOST     = 1 << 2,
        EFLAG_PROTECTED = 1 << 10
        };

      C_Npc() = default;

      int32_t  id=0;
      ZString  name[5];
      ZString  slot;
      ZString  effect;
      ENPCType npcType = NPCTYPE_AMBIENT;
      ENPCFlag flags   = EFLAG_NONE;
      int32_t  attribute[EATR_MAX]={};
      int32_t  hitChance[MAX_HITCHANCE]={};
      int32_t  protection[PROT_INDEX_MAX]={};
      int32_t  damage[DAM_INDEX_MAX]={};
      int32_t  damagetype=0;
      int32_t  guild=0;
      int32_t  level=0;
      uint32_t mission[MAX_MISSIONS]={};
      int32_t  fight_tactic=0;
      int32_t  weapon=0;

      int32_t  voice=0;
      int32_t  voicePitch=0;
      int32_t  bodymass=0;
      uint32_t daily_routine=0;
      uint32_t start_aistate=0;

      // **********************
      // Spawn
      // **********************
      ZString spawnPoint;
      int32_t spawnDelay=0;

      // **********************
      // SENSES
      // **********************
      int32_t senses=0;
      int32_t senses_range=0;

      // **********************
      // Feel free to use
      // **********************
      int32_t aivar[100]={};
      ZString wp;

      // **********************
      // Experience dependant
      // **********************
      int32_t exp=0;
      int32_t exp_next=0;
      int32_t lp=0;

      // If this is set to TRUE, the Npc can't be interrupted in any action (e.g. BS_FLAG_INTERRUPTABLE for anis is being ignored)
      int32_t bodyStateInterruptableOverride=0;
      // if "noFocus" is set to TRUE, the focus name and health bar will not be drawn of this nsc (hi, stefan!)
      int32_t noFocus=0;
     };

    struct C_Mission : Instance {
      ZString name;  //  Name des Auftrages
      ZString description;
      int32_t duration;  //  Max. Dauer in Tageszeiten
      int32_t important;

      uint32_t offerConditions;
      uint32_t offer;
      uint32_t successConditions;
      uint32_t success;
      uint32_t failureConditions;
      uint32_t failure;
      uint32_t obsoleteConditions;
      uint32_t obsolete;
      uint32_t running;
      };

    struct C_Item : Instance {
      // Categories, found in the "mainflag"-field
      enum Categories {
        ITM_CAT_NONE   = 1 << 0,
        ITM_CAT_NF     = 1 << 1,
        ITM_CAT_FF     = 1 << 2,
        ITM_CAT_MUN    = 1 << 3,
        ITM_CAT_ARMOR  = 1 << 4,
        ITM_CAT_FOOD   = 1 << 5,
        ITM_CAT_DOCS   = 1 << 6,
        ITM_CAT_POTION = 1 << 7,
        ITM_CAT_LIGHT  = 1 << 8,
        ITM_CAT_RUNE   = 1 << 9,
        ITM_CAT_MAGIC  = 1u << 31,
        ITM_CAT_EQUIPABLE = ITM_CAT_NF | ITM_CAT_FF | ITM_CAT_ARMOR | ITM_CAT_RUNE | ITM_CAT_MAGIC
        };

      enum Flags {
        ITEM_DAG      = 1 << 13,   //  Dagger
        ITEM_SWD      = 1 << 14,   //  Waffe wird wie ein Schwert behandelt
        ITEM_AXE      = 1 << 15,   //  Waffe wird wie eine Axt behandelt
        ITEM_2HD_SWD  = 1 << 16,   //  Waffe wird wie ein Zweihänder behandelt
        ITEM_2HD_AXE  = 1 << 17,   //  Waffe wird wie eine Streitaxt behandelt
        ITEM_BOW      = 1 << 19,   //  Waffe wird wie ein Bogen behandelt
        ITEM_CROSSBOW = 1 << 20,   //  Waffe wird wie eine Armbrust behandelt
        ITEM_AMULET   = 1 << 22,
        ITEM_RING     = 1 << 11,
        ITEM_BELT     = 1 << 24,
        ITEM_MISSION  = 1 << 12
        };

      C_Item()=default;

      // Für alle Items
      int32_t id=0;
      ZString name;
      ZString nameID;
      int32_t hp=0;
      int32_t hp_max=0;

      uint32_t mainflag=0, flags=0; // Hauptflag und weitere Flags
      int32_t weight=0, value=0;

      // Für Waffen
      int32_t damageType=0;  // Welche Schadensarten
      int32_t damageTotal=0;
      int32_t damage[DAM_INDEX_MAX]={};

      // Für Rüstungen
      int32_t wear=0;
      int32_t protection[PROT_INDEX_MAX]={};

      // Für Nahrung
      int32_t nutrition=0;  // HP-Steigerung bei Nahrung

      // Benötigte Attribute zum Benutzen des Items
      enum {
          COND_ATR_MAX = 3
        };
      int32_t cond_atr  [COND_ATR_MAX]={};
      int32_t cond_value[COND_ATR_MAX]={};

      // Attribute, die bei anlegen des Items verändert werden
      enum {
          CHANGE_ATR_MAX = 3
        };
      int32_t change_atr  [CHANGE_ATR_MAX]={};
      int32_t change_value[CHANGE_ATR_MAX]={};

      // Parserfunktionen
      uint32_t magic=0;       // Parserfunktion zum "Magie Header"
      uint32_t on_equip=0;    // Parserfunktion, wenn Item equipped wird.
      uint32_t on_unequip=0;  // Parserfunktion, wenn Item unequipped wird.
      uint32_t on_state[4]={};

      // Besitzer
      uint32_t owner=0;          // Besitzer : Instanz-Name
      int32_t  ownerGuild=0;     // Besitzer : Gilde
      int32_t  disguiseGuild=0;  // Zur Schau getragene Gilde durch Verkleidung

      // Die 3DS-Datei
      ZString visual;

      // Veränderung des NSC-Meshes beim Anlegen dieses Gegenstandes
      ZString visual_change;  //  ASC - File
      ZString effect;
      int32_t visual_skin=0;

      ZString scemeName;
      int32_t material=0;
      // ZString  pfx                ;    // Magic Weapon PFX
      int32_t munition=0;  // Instance of Munition

      int32_t spell=0;
      int32_t range=0;

      int32_t mag_circle=0;

      ZString description;
      ZString text[ITM_TEXT_MAX];
      int32_t count[ITM_TEXT_MAX]={};

      // inventory darstellungs geschichten, wird nur benutzt, falls von 0 abweichend
      int32_t inv_zbias=0;    // wie weit ist das item im inventory richtung far plane verschoben (integer scale 100=1)
      int32_t inv_rotx=0;     // wieviel grad um die x achse ist das item im inv gedreht
      int32_t inv_roty=0;     // wieviel grad um die y achse ist das item im inv gedreht
      int32_t inv_rotz=0;     // wieviel grad um die z achse ist das item im inv gedreht
      int32_t inv_animate=0;  // soll das item in inv rotiert werden
      };

    struct C_Focus : Instance {
      /// für NSCs
      float npc_longrange{};             //  Zurufweite  ( 20 m )
      float npc_range1{}, npc_range2{};  //  Reichweite
      float npc_azi{};                   //  Azimuth    ( Seitenwinkel )
      float npc_elevdo{}, npc_elevup{};  //  Elevation  ( Höhenwinkel  )
      int32_t npc_prio{};                //  Priorität

      /// für ITEMs
      float item_range1{}, item_range2{};  //  Reichweite
      float item_azi{};                    //  Azimuth    ( Seitenwinkel )
      float item_elevdo{}, item_elevup{};  //  Elevation  ( Höhenwinkel  )
      int32_t item_prio{};                 //  Priorität

      /// für MOBs
      float mob_range1{}, mob_range2{};  //  Reichweite
      float mob_azi{};                   //  Azimuth    ( Seitenwinkel )
      float mob_elevdo{}, mob_elevup{};  //  Elevation  ( Höhenwinkel  )
      int32_t mob_prio{};                //  Priorität
      };

    struct SubChoice {
      ZString  text;
      uint32_t functionSym=0;
      };

    struct C_Info : Instance {
      C_Info()=default;
      int32_t  npc=0;
      int32_t  nr=0;
      int32_t  important=0;  //  Wichtig Flag -> ansprechen
      uint32_t condition=0;
      uint32_t information=0;
      ZString  description;
      int32_t  trade=0; // bool32-flag
      int32_t  permanent=0;
      std::vector<SubChoice> subChoices;
      void addChoice(const SubChoice& subChoice) {
        subChoices.insert(subChoices.begin(), subChoice);
        }
      void removeChoice(std::size_t index) {
        subChoices.erase(subChoices.begin() + int(index));
        }
      };

    struct C_Spell : Instance {
      float   time_per_mana=0;       // Zeit pro investierten Manapunkt (ms)
      int32_t damage_per_level=0;    // Schaden pro Level
      int32_t damageType=0;          // CAN BE ONLY ONE DAMAGE TYPE
      int32_t spellType=0;           // Good, Neutral or Bad
      int32_t canTurnDuringInvest=0;
      int32_t canChangeTargetDuringInvest=0;
      int32_t isMultiEffect=0;       // Effect Class is oCVisFX_MultiTarget if set to 1 (e.g. the effect can have multiple trajectorys (massdeath)
      int32_t targetCollectAlgo=0;
      int32_t targetCollectType=0;
      int32_t targetCollectRange=0;
      int32_t targetCollectAzi=0;
      int32_t targetCollectElev=0;
      };

    struct C_ItemReact : Instance {
      int32_t  npc             =0;
      int32_t  trade_item      =0;
      int32_t  trade_amount    =0;
      int32_t  requested_cat   =0;
      int32_t  requested_item  =0;
      int32_t  requested_amount=0;
      uint32_t reaction        =0;
      };

    struct C_SFX : Instance {
      ZString file;
      // pitch offset in semi-tones
      int32_t pitchOff = 0;
      // semitone variance
      int32_t pitchVar = 0;
      // 0 .. 127
      int32_t vol = 64;
      int32_t loop = 0;
      int32_t loopStartOffset = 0;
      int32_t loopEndOffset = 0;
      float   reverbLevel = 0.0f;
      ZString pfxName;
     };

    struct C_MusicTheme : Instance {
      ZString            file;
      float              vol=0.f;
      int32_t            loop=0;
      float              reverbMix=0;
      float              reverbTime=0;
      ETransitionType    transType=TRANSITION_TYPE_NULL;
      ESubTransitionType transSubType=TRANSITION_SUB_TYPE_NULL;
      };

    struct C_MusicJingle : Instance {
      ZString            name;
      int32_t            loop=0;
      float              vol=0.f;
      int32_t            transSubType=TRANSITION_SUB_TYPE_NULL;
      };

    struct C_GilValues : Instance {
      C_GilValues() {
        std::fill(std::begin(jumpup_height)     ,std::end(jumpup_height)     , 200);
        std::fill(std::begin(blood_disabled)    ,std::end(blood_disabled)    ,   0);
        std::fill(std::begin(blood_max_distance),std::end(blood_max_distance),1000);
        std::fill(std::begin(blood_amount)      ,std::end(blood_amount)      ,  10);
        std::fill(std::begin(blood_flow)        ,std::end(blood_flow)        ,   0);
        std::fill(std::begin(blood_emitter)     ,std::end(blood_emitter)     , "PFX_BLOOD");
        std::fill(std::begin(blood_texture)     ,std::end(blood_texture)     , "ZBLOODSPLAT2.TGA");
        std::fill(std::begin(turn_speed)        ,std::end(turn_speed)        , 150);
        }
      int32_t  water_depth_knee   [GIL_MAX]={};
      int32_t  water_depth_chest  [GIL_MAX]={};
      int32_t  jumpup_height      [GIL_MAX]={}; //  DEFAULT = 200;
      // int32_t  JUMPUP_FORCE       [GIL_MAX]={};
      int32_t  swim_time          [GIL_MAX]={};
      int32_t  dive_time          [GIL_MAX]={};
      int32_t  step_height        [GIL_MAX]={};
      int32_t  jumplow_height     [GIL_MAX]={};
      int32_t  jumpmid_height     [GIL_MAX]={};
      int32_t  slide_angle        [GIL_MAX]={};
      int32_t  slide_angle2       [GIL_MAX]={};
      int32_t  disable_autoroll   [GIL_MAX]={}; //  DEFAULT = 0                 ;  0 = Autoroll  enabled  / 1 = Autoroll  disabled
      int32_t  surface_align      [GIL_MAX]={}; //  DEFAULT = 0                 ;  0 = Alignment disabled  / 1 = Alignment enabled
      int32_t  climb_heading_angle[GIL_MAX]={};
      int32_t  climb_horiz_angle  [GIL_MAX]={};
      int32_t  climb_ground_angle [GIL_MAX]={};
      int32_t  fight_range_base   [GIL_MAX]={};
      int32_t  fight_range_fist   [GIL_MAX]={};
      int32_t  fight_range_g      [GIL_MAX]={};
      int32_t  fight_range_1hs    [GIL_MAX]={};
      int32_t  fight_range_1ha    [GIL_MAX]={};
      int32_t  fight_range_2hs    [GIL_MAX]={};
      int32_t  fight_range_2ha    [GIL_MAX]={};
      int32_t  falldown_height    [GIL_MAX]={}; //                                 Wie tief Fallen ohne Schaden ?
      int32_t  falldown_damage    [GIL_MAX]={}; //                                 Schaden für jeden weiteren angefangenen Meter.
      int32_t  blood_disabled     [GIL_MAX]={}; //  DEFAULT = 0                 ;  Blut ganz ausschalten (z.B. bei Sekletten) ?
      int32_t  blood_max_distance [GIL_MAX]={}; //  DEFAULT = 1000              ;  Wie weit spritzt das Blut (in cm) ?
      int32_t  blood_amount       [GIL_MAX]={}; //  DEFAULT = 10                ;  Wie viel Blut ?
      int32_t  blood_flow         [GIL_MAX]={}; //  DEFAULT = 0                 ;  Soll es sich langsam ausbreiten ?
      ZString  blood_emitter      [GIL_MAX]={}; //  DEFAULT = "PFX_BLOOD"       ;  Welcher Partikel-Emitter ?
      ZString  blood_texture      [GIL_MAX]={}; //  DEFAULT = "ZBLOODSPLAT2.TGA";  Welche Textur ?
      int32_t  turn_speed         [GIL_MAX]={}; //  DEFAULT = 150               ;
      };

    struct C_FightAI : Instance {
      Move move[MAX_MOVE]={};
      };

    struct CCamSys : Instance {
      // angles -180 to 180
      float bestRange     = 2.f;
      float minRange      = 1.99f;
      float maxRange      = 4.1f;
      float bestElevation = 0.f;
      float minElevation  = 0.f;
      float maxElevation  = 89.f;
      float bestAzimuth   = 0.f;
      float minAzimuth    = -90.f;
      float maxAzimuth    = 90.f;
      float bestRotZ      = 0.f;
      float minRotZ       = 0.f;
      float maxRotZ       = 0.f;
      float rotOffsetX    = 20.f;
      float rotOffsetY    = 0.f;
      float rotOffsetZ    = 0.f;
      float targetOffsetX = 0.f;
      float targetOffsetY = 0.f;
      float targetOffsetZ = 0.f;
      // dynamic
      float veloTrans     = 40.f; // velocity while easing   to best position
      float veloRot       = 2.f;  // velocity while rotating to best orientation
      // bool32 options
      int32_t  translate  = 1; // rotate around target, 1: on, 0:off
      int32_t  rotate     = 1; // rotate around target, 1: on, 0:off
      int32_t  collision  = 1; // disable collision for this mode (ideal pos must be near player)
      };

    struct C_SVM : Instance {
      ZString MILGreetings;
      ZString PALGreetings;
      ZString Weather;
      ZString IGetYouStill;
      ZString DieEnemy;
      ZString DieMonster;
      ZString Addon_DieMonster;
      ZString Addon_DieMonster2;
      ZString DirtyThief;
      ZString HandsOff;
      ZString SheepKiller;
      ZString SheepKillerMonster;
      ZString YouMurderer;
      ZString DieStupidBeast;
      ZString YouDareHitMe;
      ZString YouAskedForIt;
      ZString ThenIBeatYouOutOfHere;
      ZString WhatDidYouDoInThere;
      ZString WillYouStopFighting;
      ZString KillEnemy;
      ZString EnemyKilled;
      ZString MonsterKilled;
      ZString Addon_MonsterKilled;
      ZString Addon_MonsterKilled2;
      ZString ThiefDown;
      ZString RumfummlerDown;
      ZString SheepAttackerDown;
      ZString KillMurderer;
      ZString StupidBeastKilled;
      ZString NeverHitMeAgain;
      ZString YouBetterShouldHaveListened;
      ZString GetUpAndBeGone;
      ZString NeverEnterRoomAgain;
      ZString ThereIsNoFightingHere;
      ZString SpareMe;
      ZString RunAway;
      ZString Alarm;
      ZString Guards;
      ZString Help;
      ZString GoodMonsterKill;
      ZString GoodKill;
      ZString notnow;
      ZString RunCoward;
      ZString GetOutOfHere;
      ZString WhyAreYouInHere;
      ZString YesGoOutOfHere;
      ZString WhatsThisSupposedToBe;
      ZString YouDisturbedMySlumber;
      ZString ITookYourGold;
      ZString ShitNoGold;
      ZString ITakeYourWeapon;
      ZString WhatAreYouDoing;
      ZString LookingForTroubleAgain;
      ZString StopMagic;
      ZString ISaidStopMagic;
      ZString WeaponDown;
      ZString ISaidWeaponDown;
      ZString WiseMove;
      ZString NextTimeYoureInForIt;
      ZString OhMyHead;
      ZString TheresAFight;
      ZString OhMyGodItsAFight;
      ZString GoodVictory;
      ZString NotBad;
      ZString OhMyGodHesDown;
      ZString CheerFriend01;
      ZString CheerFriend02;
      ZString CheerFriend03;
      ZString Ooh01;
      ZString Ooh02;
      ZString Ooh03;
      ZString WhatWasThat;
      ZString GetOutOfMyBed;
      ZString Awake;
      ZString abs_commander;
      ZString abs_monastery;
      ZString abs_farm;
      ZString abs_good;
      ZString sheepkiller_crime;
      ZString attack_crime;
      ZString theft_crime;
      ZString murder_crime;
      ZString pal_city_crime;
      ZString mil_city_crime;
      ZString city_crime;
      ZString mona_crime;
      ZString farm_crime;
      ZString oc_crime;
      ZString toughguy_attacklost;
      ZString toughguy_attackwon;
      ZString toughguy_playerattack;
      ZString gold_1000;
      ZString gold_950;
      ZString gold_900;
      ZString gold_850;
      ZString gold_800;
      ZString gold_750;
      ZString gold_700;
      ZString gold_650;
      ZString gold_600;
      ZString gold_550;
      ZString gold_500;
      ZString gold_450;
      ZString gold_400;
      ZString gold_350;
      ZString gold_300;
      ZString gold_250;
      ZString gold_200;
      ZString gold_150;
      ZString gold_100;
      ZString gold_90;
      ZString gold_80;
      ZString gold_70;
      ZString gold_60;
      ZString gold_50;
      ZString gold_40;
      ZString gold_30;
      ZString gold_20;
      ZString gold_10;
      ZString Smalltalk01;
      ZString Smalltalk02;
      ZString Smalltalk03;
      ZString Smalltalk04;
      ZString Smalltalk05;
      ZString Smalltalk06;
      ZString Smalltalk07;
      ZString Smalltalk08;
      ZString Smalltalk09;
      ZString Smalltalk10;
      ZString Smalltalk11;
      ZString Smalltalk12;
      ZString Smalltalk13;
      ZString Smalltalk14;
      ZString Smalltalk15;
      ZString Smalltalk16;
      ZString Smalltalk17;
      ZString Smalltalk18;
      ZString Smalltalk19;
      ZString Smalltalk20;
      ZString Smalltalk21;
      ZString Smalltalk22;
      ZString Smalltalk23;
      ZString Smalltalk24;
      ZString Smalltalk25;
      ZString Smalltalk26;
      ZString Smalltalk27;
      ZString Smalltalk28;
      ZString Smalltalk29;
      ZString Smalltalk30;
      ZString NoLearnNoPoints;
      ZString NoLearnOverPersonalMAX;
      ZString NoLearnYoureBetter;
      ZString YouLearnedSomething;
      ZString unterstadt;
      ZString oberstadt;
      ZString tempel;
      ZString markt;
      ZString galgen;
      ZString kaserne;
      ZString hafen;
      ZString whereto;
      ZString oberstadt_2_unterstadt;
      ZString unterstadt_2_oberstadt;
      ZString unterstadt_2_tempel;
      ZString unterstadt_2_hafen;
      ZString tempel_2_unterstadt;
      ZString tempel_2_markt;
      ZString tempel_2_galgen;
      ZString markt_2_tempel;
      ZString markt_2_kaserne;
      ZString markt_2_galgen;
      ZString galgen_2_tempel;
      ZString galgen_2_markt;
      ZString galgen_2_kaserne;
      ZString kaserne_2_markt;
      ZString kaserne_2_galgen;
      ZString hafen_2_unterstadt;
      ZString Dead;
      ZString Aargh_1;
      ZString Aargh_2;
      ZString Aargh_3;
      ZString addon_wrongarmor;
      ZString addon_wrongarmor_sld;
      ZString addon_wrongarmor_mil;
      ZString addon_wrongarmor_kdf;
      ZString addon_noarmor_bdt;
      ZString addon_diebandit;
      ZString addon_dirtypirate;
      ZString SC_HeyTurnAround;
      ZString SC_HeyTurnAround02;
      ZString SC_HeyTurnAround03;
      ZString SC_HeyTurnAround04;
      ZString SC_HeyWaitASecond;
      ZString DoesntWork;
      ZString PickBroke;
      ZString NeedKey;
      ZString NoMorePicks;
      ZString NoPickLockTalent;
      ZString NoSweeping;
      ZString picklockorkeymissing;
      ZString keymissing;
      ZString picklockmissing;
      ZString neveropen;
      ZString missingitem;
      ZString dontknow;
      ZString nothingtoget;
      ZString nothingtoget02;
      ZString nothingtoget03;
      ZString healshrine;
      ZString heallastshrine;
      ZString irdoraththereyouare;
      ZString scopensirdorathbook;
      ZString scopenslastdoor;
      ZString trade_1;
      ZString trade_2;
      ZString trade_3;
      ZString verstehe;
      ZString foundtreasure;
      ZString cantunderstandthis;
      ZString cantreadthis;
      ZString stoneplate_1;
      ZString stoneplate_2;
      ZString stoneplate_3;
      ZString cough;
      ZString hui;
      //--- notr
      ZString Addon_ThisLittleBastard;
      ZString addon_openadanostemple;
      ZString attentat_addon_description;
      ZString attentat_addon_description2;
      ZString attentat_addon_pro;
      ZString attentat_addon_contra;
      ZString mine_addon_description;
      ZString addon_summonancientghost;
      ZString addon_ancientghost_notnear;
      ZString addon_gold_description;
      //--- gothic1
      ZString watchyouraim;
      ZString watchyouraimangry;
      ZString letsforgetourlittlefight;
      ZString strange;
      ZString diemortalenemy;
      ZString nowwait;
      ZString nowwaitintruder;
      ZString youstillnothaveenough;
      ZString youattackedmycharge;
      ZString iwillteachyourespectforforeignproperty;
      ZString youkilledoneofus;
      ZString berzerk;
      ZString youllbesorryforthis;
      ZString yesyes;
      ZString shitwhatamonster;
      ZString wewillmeetagain;
      ZString nevertrythatagain;
      ZString itookyourore;
      ZString shitnoore;
      ZString youviolatedforbiddenterritory;
      ZString youwannafoolme;
      ZString whatdidyouinthere;
      ZString intruderalert;
      ZString behindyou;
      ZString heyheyhey;
      ZString cheerfight;
      ZString cheerfriend;
      ZString ooh;
      ZString yeahwelldone;
      ZString hedefeatedhim;
      ZString hedeservedit;
      ZString hekilledhim;
      ZString itwasagoodfight;
      ZString friendlygreetings;
      ZString algreetings;
      ZString magegreetings;
      ZString sectgreetings;
      ZString thereheis;
      ZString nolearnovermax;
      ZString nolearnyoualreadyknow;
      ZString heyyou;
      ZString whatdoyouwant;
      ZString isaidwhatdoyouwant;
      ZString makeway;
      ZString outofmyway;
      ZString youdeaforwhat;
      ZString lookaway;
      ZString okaykeepit;
      ZString whatsthat;
      ZString thatsmyweapon;
      ZString giveittome;
      ZString youcankeepthecrap;
      ZString theykilledmyfriend;
      ZString suckergotsome;
      ZString suckerdefeatedebr;
      ZString suckerdefeatedgur;
      ZString suckerdefeatedmage;
      ZString suckerdefeatednov_guard;
      ZString suckerdefeatedvlk_guard;
      ZString youdefeatedmycomrade;
      ZString youdefeatednov_guard;
      ZString youdefeatedvlk_guard;
      ZString youstolefromme;
      ZString youstolefromus;
      ZString youstolefromebr;
      ZString youstolefromgur;
      ZString stolefrommage;
      ZString youkilledmyfriend;
      ZString youkilledebr;
      ZString youkilledgur;
      ZString youkilledmage;
      ZString youkilledocfolk;
      ZString youkilledncfolk;
      ZString youkilledpsifolk;
      ZString getthingsright;
      ZString youdefeatedmewell;
      ZString om;
      };
    }  // namespace GEngineClasses

  /**
    * @brief Links the classes known to the engine to the VM.
    */
  void registerGothicEngineClasses(DaedalusVM& vm);
  }  // namespace Daedalus
