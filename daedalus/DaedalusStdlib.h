#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace Daedalus
{
    class DaedalusVM;

    /**
	 * Basic gametype. Needed for registering C_Class members and sky configuration in REGoth
	 */
    enum class GameType
    {
        GT_Gothic1,
        GT_Gothic2
    };

    namespace GEngineClasses
    {
        const int MAX_CHAPTER = 5;
        const int MAX_MISSIONS = 5;
        const int MAX_HITCHANCE = 5;
        const int ATR_INDEX_MAX = 8;
        const int ITM_TEXT_MAX = 6;
        const int GIL_MAX = 66;

        enum {
          DAM_INDEX_BARRIER = 0,  //				 nur der Vollstandigkeit und Transparenz wegen hier definiert ( _NICHT_ verwenden )
          DAM_INDEX_BLUNT   = 1,
          DAM_INDEX_EDGE,
          DAM_INDEX_FIRE,
          DAM_INDEX_FLY,
          DAM_INDEX_MAGIC,
          DAM_INDEX_POINT,
          DAM_INDEX_FALL,//				 nur der Vollstandigkeit und Transparenz wegen hier definiert ( _NICHT_ verwenden )
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

        enum Move : int32_t {
          MOVE_RUN          = 1,  // Opponents in my focus + who stands in between? (G)
          MOVE_RUNBACK      = 2, 	// Opponents in my focus
          MOVE_JUMPBACK     = 3,
          MOVE_TURN         = 4,  // Always up to opponents in focus (never interrupted by new action, at most by enemy attack)
          MOVE_STRAFE       = 5,  // (Direction is decided by the native-code)
          MOVE_ATTACK       = 6,  // in ComboZone = combo / in the run = storm attack?
          MOVE_SIDEATTACK   = 7,
          MOVE_FRONTATTACK  = 8,
          MOVE_TRIPLEATTACK = 9,
          MOVE_WHIRLATTACK  = 10,
          MOVE_MASTERATTACK = 11,
          MOVE_TURNTOHIT    = 15,	// Version > 0.92 => No Effect
          MOVE_PARADE       = 17,	// (keine Attacke = oben)
          MOVE_STANDUP      = 18,
          MOVE_WAIT         = 19,
          MOVE_WAIT_LONGER  = 23,	// Achtung: Muss 23 sein, da im Programm noch einige unbenutze Moves zwischen 19 und 22 gibt
          MOVE_WAIT_EXT     = 24,	// Achtung: Muss 24 sein, da im Programm noch einige unbenutze Moves zwischen 19 und 22 gibt

          MAX_MOVE          = 6,
          MAX_FIGHTAI       = 50, // Number of existing Fight-AIs // Constant NOT redundant in the native-code, can be changed in scripts as required
          };

        // Music transition types
        enum ETransitionType
        {
            TRANSITION_TYPE_NONE = 1,
            TRANSITION_TYPE_GROOVE = 2,
            TRANSITION_TYPE_FILL = 3,
            TRANSITION_TYPE_BREAK = 4,
            TRANSITION_TYPE_INTRO = 5,
            TRANSITION_TYPE_END = 6,
            TRANSITION_TYPE_ENDANDINTRO = 7
        };

        enum ESubTransitionType
        {
            TRANSITION_SUB_TYPE_IMMEDIATE = 1,
            TRANSITION_SUB_TYPE_BEAT = 2,
            TRANSITION_SUB_TYPE_MEASURE = 3
        };

        namespace MenuConstants
        {
            const int MAX_USERSTRINGS = 10;
            const int MAX_ITEMS = 150;
            const int MAX_EVENTS = 10;
            const int MAX_SEL_ACTIONS = 5;
            const int MAX_USERVARS = 4;
            const int SEL_EVENT_UNDEF = 0;

            enum ESelEvent
            {
                SEL_EVENT_EXECUTE = 1,
                SEL_EVENT_CHANGED = 2,
                SEL_EVENT_LEAVE = 3,
                SEL_EVENT_TIMER = 4,
                SEL_EVENT_CLOSE = 5,
                SEL_EVENT_INIT = 6,
                SEL_EVENT_SEL_PREV = 7,
                SEL_EVENT_SEL_NEXT = 8,
            };

            enum ESelAction
            {
                SEL_ACTION_UNDEF = 0,
                SEL_ACTION_BACK = 1,
                SEL_ACTION_STARTMENU = 2,
                SEL_ACTION_STARTITEM = 3,
                SEL_ACTION_CLOSE = 4,
                SEL_ACTION_CONCOMMANDS = 5,
                SEL_ACTION_PLAY_SOUND = 6,
                SEL_ACTION_EXECCOMMANDS = 7,
            };
        }  // namespace MenuConstants

        struct Instance {
          Instance() = default;
          size_t instanceSymbol=0;

          /**
             * Space for the user to save something. Will not be touched at all by ZenLib.
             * Note: This is set to nullptr after creation.
             */
          void* userPtr=nullptr;
          };

        struct C_ParticleFX : Instance
        {
            // 1) Emitter: zeitliches  Austoss-Verhalten, particles-per-second
            float ppsValue;
            std::string ppsScaleKeys_S;
            int32_t ppsIsLooping;
            int32_t ppsIsSmooth;
            float ppsFPS;
            std::string ppsCreateEm_S;
            float ppsCreateEmDelay;

            // 2) Emitter: raeumliches Austoss-Verhalten
            std::string shpType_S;  //	"point, line, box, circle, sphere, mesh"
            std::string shpFOR_S;   //	"object,world"
            std::string shpOffsetVec_S;
            std::string shpDistribType_S;  //	"RAND, UNIFORM, WALK"
            float shpDistribWalkSpeed;
            int32_t shpIsVolume;
            std::string shpDim_S;   //	"", "30", "10 20 30", "30", "30", "" //	line: nur 1 Dimension !=0 // shape Dimensions
            std::string shpMesh_S;  //	"cross.3ds"
            int32_t shpMeshRender_B;
            std::string shpScaleKeys_S;  //	"[1.0] [0.8 0.9 0.2] [1.0]"
            int32_t shpScaleIsLooping;
            int32_t shpScaleIsSmooth;
            float shpScaleFPS;

            // 3) Partikel: Start Richtung/Speed:
            std::string dirMode_S;  //	"DIR, TARGET, MESH_POLY"
            std::string dirFOR_S;   //	"OBJECT, WORLD"
            std::string dirModeTargetFOR_S;
            std::string dirModeTargetPos_S;  //	"30 23 67"
            float dirAngleHead;
            float dirAngleHeadVar;
            float dirAngleElev;
            float dirAngleElevVar;
            float velAvg;
            float velVar;

            // 4) Partikel: Lebensdauer
            float lspPartAvg;
            float lspPartVar;

            // 5) Partikel: Flugverhalten (gravity, nicht-linear?, mesh-selfRot?,..)
            // grav: a) nur Y, b) XYZ, c) auf Ziel zu steuern
            // std::string  flyMode_S;								//	"LINEAR, LIN_SINUS,.."
            // flyMeshSelfRotSpeedMin, flyMeshSelfRotSpeedMax
            std::string flyGravity_S;
            int32_t flyCollDet_B;

            // 6) Partikel: Visualisierung
            std::string visName_S;         //	"NAME_V0_A0.TGA/.3DS"	(Variation, Animation)
            std::string visOrientation_S;  //	"NONE, VELO"
            int32_t visTexIsQuadPoly;      //	0=triMesh, 1=quadMesh
            float visTexAniFPS;
            int32_t visTexAniIsLooping;  //	0=oneShot, 1=looping
            // color		(nur Tex, lifeSpan-Sync)
            std::string visTexColorStart_S;
            std::string visTexColorEnd_S;
            // size-ani		(nur Tex, lifeSpan-Sync)
            std::string visSizeStart_S;
            float visSizeEndScale;
            // alpha		(lifeSpan-Sync)
            std::string visAlphaFunc_S;
            float visAlphaStart;
            float visAlphaEnd;

            // 7) misc effects

            // trail
            float trlFadeSpeed;
            std::string trlTexture_S;
            float trlWidth;

            // marks
            float mrkFadeSpeed;
            std::string mrkTexture_S;
            float mrkSize;

            // flocking
            std::string flockMode;
            float flockStrength;

            // local frame of reference override
            // calculates the position of the particles each frame relative to the emitters pos/rot
            // can be expensive
            // WARNING: in comb with flyCollDet_B this can be a performance-hog deluxe
            int32_t useEmittersFOR;

            // optional you can set a valid timeperiod in which this pfx should be rendered (e.g. "8 22": should be rendererd from 8 to 22 o clock")
            std::string timeStartEnd_S;

            // with the next setting you can define weather this pfx is an ambient pfx, thus can be disabled in the gothic.ini with the value [ENGINE]/noAmbientPFX
            int32_t m_bIsAmbientPFX;
        };

        struct C_Menu : Instance
        {
            enum EFlags : int32_t
            {
                MENU_OVERTOP = 1,
                MENU_EXCLUSIVE = 2,
                MENU_NOANI = 4,
                MENU_DONTSCALE_DIM = 8,
                MENU_DONTSCALE_POS = 16,
                MENU_ALIGN_CENTER = 32,
                MENU_SHOW_INFO = 64,
            };

            C_Menu() = default;
            std::string backPic;
            std::string backWorld;
            int32_t posx=0, posy=0;
            int32_t dimx=0, dimy=0;
            int32_t alpha=0;
            std::string musicTheme;
            int32_t eventTimerMSec=0;
            std::string items[MenuConstants::MAX_ITEMS];
            int32_t flags=0;
            int32_t defaultOutGame=0;
            int32_t defaultInGame=0;
        };

        struct C_Menu_Item : Instance
        {
            enum EFlags : int32_t
            {
                IT_CHROMAKEYED = 1,
                IT_TRANSPARENT = 2,
                IT_SELECTABLE = 4,
                IT_MOVEABLE = 8,
                IT_TXT_CENTER = 16,
                IT_DISABLED = 32,
                IT_FADE = 64,
                IT_EFFECTS_NEXT = 128,
                IT_ONLY_OUT_GAME = 256,
                IT_ONLY_IN_GAME = 512,
                IT_PERF_OPTION = 1024,
                IT_MULTILINE = 2048,
                IT_NEEDS_APPLY = 4096,
                IT_NEEDS_RESTART = 8192,
                IT_EXTENDED_MENU = 16384,
            };

            enum EType : int32_t
            {
                MENU_ITEM_UNDEF = 0,
                MENU_ITEM_TEXT = 1,
                MENU_ITEM_SLIDER = 2,
                MENU_ITEM_INPUT = 3,
                MENU_ITEM_CURSOR = 4,
                MENU_ITEM_CHOICEBOX = 5,
                MENU_ITEM_BUTTON = 6,
                MENU_ITEM_LISTBOX = 7,
            };

            C_Menu_Item() = default;
            std::string fontName;
            std::string text[MenuConstants::MAX_USERSTRINGS];
            std::string backPic;
            std::string alphaMode;
            int32_t     alpha=0;
            int32_t     type =0;
            int32_t     onSelAction[MenuConstants::MAX_SEL_ACTIONS]={};
            std::string onSelAction_S[MenuConstants::MAX_SEL_ACTIONS];
            std::string onChgSetOption;
            std::string onChgSetOptionSection;

            int32_t onEventAction[MenuConstants::MAX_EVENTS]={};
            int32_t posx=0, posy=0;
            int32_t dimx=0, dimy=0;  // -1 = AUTODETECT (FONTWISE)
            float sizeStartScale=0.f;
            int32_t flags=0;
            float openDelayTime=0;
            float openDuration=0;
            float userfloat[MenuConstants::MAX_USERVARS]={};
            std::string userString[MenuConstants::MAX_USERVARS];
            int32_t frameSizeX=0;
            int32_t frameSizeY=0;
            std::string hideIfOptionSectionSet;
            std::string hideIfOptionSet;
            int32_t hideOnValue=0;
        };

        struct C_Npc : Instance
        {
            enum EAttributes : int32_t
            {
                EATR_HITPOINTS = 0,
                EATR_HITPOINTSMAX = 1,
                EATR_MANA = 2,
                EATR_MANAMAX = 3,
                EATR_STRENGTH = 4,
                EATR_DEXTERITY = 5,
                EATR_REGENERATEHP = 6,
                EATR_REGENERATEMANA = 7,

                EATR_MAX = 8
            };

            enum ENPCFlag : int32_t
            {
                EFLAG_NONE = 0,
                EFLAG_FRIENDS = 1 << 0,
                EFLAG_IMMORTAL = 1 << 1,
                EFLAG_GHOST = 1 << 2,
                EFLAG_PROTECTED = 1 << 10,
            };

            C_Npc()
            {
                id = 0;
                npcType = 0;
                flags = EFLAG_NONE;
                memset(attribute, 0, sizeof(attribute));
                memset(hitChance, 0, sizeof(hitChance));
                memset(protection, 0, sizeof(protection));
                memset(damage, 0, sizeof(damage));
                memset(mission, 0, sizeof(mission));
                damagetype = 0;
                guild = 0;
                level = 0;
                fight_tactic = 0;
                weapon = 0;
                voice = 0;
                voicePitch = 0;
                bodymass = 0;
                daily_routine = 0;
                start_aistate = 0;
                spawnDelay = 0;
                senses = 0;
                senses_range = 0;
                memset(aivar, 0, sizeof(aivar));
                exp = 0;
                exp_next = 0;
                lp = 0;
                bodyStateInterruptableOverride = 0;
                noFocus = 0;
            }

            int32_t id;
            std::string name[5];
            std::string slot;
            std::string effect;
            int32_t npcType;
            ENPCFlag flags;
            int32_t attribute[EATR_MAX];
            int32_t hitChance[MAX_HITCHANCE];
            int32_t protection[PROT_INDEX_MAX];
            int32_t damage[DAM_INDEX_MAX];
            int32_t damagetype;
            int32_t guild, level;
            uint32_t mission[MAX_MISSIONS];
            int32_t fight_tactic;
            int32_t weapon;

            int32_t voice;
            int32_t voicePitch;
            int32_t bodymass;
            uint32_t daily_routine;
            uint32_t start_aistate;

            // **********************
            // Spawn
            // **********************
            std::string spawnPoint;
            int32_t spawnDelay;

            // **********************
            // SENSES
            // **********************
            int32_t senses;
            int32_t senses_range;

            // **********************
            // Feel free to use
            // **********************
            int32_t aivar[100];
            std::string wp;

            // **********************
            // Experience dependant
            // **********************
            int32_t exp;
            int32_t exp_next;
            int32_t lp;

            // If this is set to TRUE, the Npc can't be interrupted in any action (e.g. BS_FLAG_INTERRUPTABLE for anis is being ignored)
            int32_t bodyStateInterruptableOverride;
            // if "noFocus" is set to TRUE, the focus name and health bar will not be drawn of this nsc (hi, stefan!)
            int32_t noFocus;
        };

        struct C_Mission : Instance
        {
            std::string name;  //	Name des Auftrages
            std::string description;
            int32_t duration;  //	Max. Dauer in Tageszeiten
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

        struct C_Item : Instance
        {
            // Categories, found in the "mainflag"-field
            enum Categories
            {
                ITM_CAT_NONE = 1 << 0,
                ITM_CAT_NF = 1 << 1,
                ITM_CAT_FF = 1 << 2,
                ITM_CAT_MUN = 1 << 3,
                ITM_CAT_ARMOR = 1 << 4,
                ITM_CAT_FOOD = 1 << 5,
                ITM_CAT_DOCS = 1 << 6,
                ITM_CAT_POTION = 1 << 7,
                ITM_CAT_LIGHT = 1 << 8,
                ITM_CAT_RUNE = 1 << 9,
                ITM_CAT_MAGIC = 1 << 31,
                ITM_CAT_EQUIPABLE = ITM_CAT_NF | ITM_CAT_FF | ITM_CAT_ARMOR | ITM_CAT_RUNE | ITM_CAT_MAGIC,
            };

            enum Flags
            {
                ITEM_DAG = 1 << 13,       //  Dagger
                ITEM_SWD = 1 << 14,       //	Waffe wird wie ein Schwert behandelt
                ITEM_AXE = 1 << 15,       //	Waffe wird wie eine Axt behandelt
                ITEM_2HD_SWD = 1 << 16,   //	Waffe wird wie ein Zweihänder behandelt
                ITEM_2HD_AXE = 1 << 17,   //	Waffe wird wie eine Streitaxt behandelt
                ITEM_BOW = 1 << 19,       //	Waffe wird wie ein Bogen behandelt
                ITEM_CROSSBOW = 1 << 20,  //	Waffe wird wie eine Armbrust behandelt
                ITEM_AMULET = 1 << 22,
                ITEM_RING = 1 << 11,
                ITEM_BELT = 1 << 24,
                ITEM_MISSION = 1 << 12,
            };

            C_Item()
                : on_state{}
                , count{}
                , cond_atr{}
                , cond_value{}
                , change_atr{}
                , change_value{}
                , protection{}
                , damage{}
            {
                id = 0;
                hp = 0;
                hp_max = 0;
                mainflag = 0;
                flags = 0;
                weight = 0;
                value = 0;
                damageType = 0;
                damageTotal = 0;
                wear = 0;
                nutrition = 0;
                magic = 0;
                on_equip = 0;
                on_unequip = 0;
                owner = 0;
                ownerGuild = 0;
                disguiseGuild = 0;
                visual_skin = 0;
                material = 0;
                munition = 0;
                spell = 0;
                range = 0;
                mag_circle = 0;
                inv_zbias = 0;
                inv_rotx = 0;
                inv_roty = 0;
                inv_rotz = 0;
                inv_animate = 0;
                amount = 0;
            }

            // Für alle Items
            int32_t id;
            std::string name, nameID;
            int32_t hp, hp_max;

            int32_t mainflag, flags;  //	Hauptflag und weitere Flags
            int32_t weight, value;

            // Für Waffen
            int32_t damageType;  //	Welche Schadensarten
            int32_t damageTotal;
            int32_t damage[DAM_INDEX_MAX];

            // Für Rüstungen
            int32_t wear;
            int32_t protection[PROT_INDEX_MAX];

            // Für Nahrung
            int32_t nutrition;  //	HP-Steigerung bei Nahrung

            // Benötigte Attribute zum Benutzen des Items
            enum
            {
                COND_ATR_MAX = 3
            };
            int32_t cond_atr[COND_ATR_MAX];
            int32_t cond_value[COND_ATR_MAX];

            // Attribute, die bei anlegen des Items verändert werden
            enum
            {
                CHANGE_ATR_MAX = 3
            };
            int32_t change_atr[CHANGE_ATR_MAX];
            int32_t change_value[CHANGE_ATR_MAX];

            // Parserfunktionen
            uint32_t magic;       //	Parserfunktion zum "Magie Header"
            uint32_t on_equip;    //	Parserfunktion, wenn Item equipped wird.
            uint32_t on_unequip;  //	Parserfunktion, wenn Item unequipped wird.
            uint32_t on_state[4];

            // Besitzer
            uint32_t owner;         //	Besitzer : Instanz-Name
            int32_t ownerGuild;     //	Besitzer : Gilde
            int32_t disguiseGuild;  //	Zur Schau getragene Gilde durch Verkleidung

            // Die 3DS-Datei
            std::string visual;

            // Veränderung des NSC-Meshes beim Anlegen dieses Gegenstandes
            std::string visual_change;  //	ASC - File
            std::string effect;
            int32_t visual_skin;

            std::string scemeName;
            int32_t material;
            // std::string	pfx								;		//	Magic Weapon PFX
            int32_t munition;  //	Instance of Munition

            int32_t spell;
            int32_t range;

            int32_t mag_circle;

            std::string description;
            std::string text[ITM_TEXT_MAX];
            int32_t count[ITM_TEXT_MAX];

            // inventory darstellungs geschichten, wird nur benutzt, falls von 0 abweichend
            int32_t inv_zbias;    //  wie weit ist das item im inventory richtung far plane verschoben (integer scale 100=1)
            int32_t inv_rotx;     //  wieviel grad um die x achse ist das item im inv gedreht
            int32_t inv_roty;     //  wieviel grad um die y achse ist das item im inv gedreht
            int32_t inv_rotz;     //  wieviel grad um die z achse ist das item im inv gedreht
            int32_t inv_animate;  //  soll das item in inv rotiert werden

            // REGoth member, number of items
            uint32_t amount;
        };

        struct C_Focus : Instance
        {
            /// für NSCs
            float npc_longrange;           //	Zurufweite	( 20 m )
            float npc_range1, npc_range2;  //	Reichweite
            float npc_azi;                 //	Azimuth		( Seitenwinkel )
            float npc_elevdo, npc_elevup;  //	Elevation	( Höhenwinkel  )
            int32_t npc_prio;              //	Priorität

            /// für ITEMs
            float item_range1, item_range2;  //	Reichweite
            float item_azi;                  //	Azimuth		( Seitenwinkel )
            float item_elevdo, item_elevup;  //	Elevation	( Höhenwinkel  )
            int32_t item_prio;               //	Priorität

            /// für MOBs
            float mob_range1, mob_range2;  //	Reichweite
            float mob_azi;                 //	Azimuth		( Seitenwinkel )
            float mob_elevdo, mob_elevup;  //	Elevation	( Höhenwinkel  )
            int32_t mob_prio;              //	Priorität
        };

        struct SubChoice
        {
            std::string text;
            uint32_t functionSym;
        };

        struct C_Info : Instance
        {
            C_Info()=default;
            int32_t npc=0;
            int32_t nr=0;
            int32_t important=0;  //	Wichtig Flag -> ansprechen
            uint32_t condition=0;
            uint32_t information=0;
            std::string description;
            int32_t trade=0; // bool32-flag
            int32_t permanent=0;
            std::vector<SubChoice> subChoices;
            void addChoice(const SubChoice& subChoice)
            {
                subChoices.insert(subChoices.begin(), subChoice);
            }
            void removeChoice(std::size_t index)
            {
                subChoices.erase(subChoices.begin() + index);
            }
        };

        struct C_Spell : Instance
        {
            float   time_per_mana;       // Zeit pro investierten Manapunkt (ms)
            int32_t damage_per_level;  // Schaden pro Level
            int32_t damageType;        // CAN BE ONLY ONE DAMAGE TYPE
            int32_t spellType;         // Good, Neutral or Bad
            int32_t canTurnDuringInvest;
            int32_t canChangeTargetDuringInvest;
            int32_t isMultiEffect;  // Effect Class is oCVisFX_MultiTarget if set to 1 (e.g. the effect can have multiple trajectorys (massdeath)
            int32_t targetCollectAlgo;
            int32_t targetCollectType;
            int32_t targetCollectRange;
            int32_t targetCollectAzi;
            int32_t targetCollectElev;
        };

        struct C_ItemReact : Instance
        {
            int32_t npc;
            int32_t trade_item;
            int32_t trade_amount;
            int32_t requested_cat;
            int32_t requested_item;
            int32_t requested_amount;
            uint32_t reaction;
        };

        struct C_SFX : Instance
        {
            std::string file;
            // pitch offset in semi-tones
            int32_t pitchOff = 0;
            // semitone variance
            int32_t pitchVar = 0;
            // 0 .. 127
            int32_t vol = 64;
            int32_t loop = 0;
            int32_t loopStartOffset = 0;
            int32_t loopEndOffset = 0;
            float reverbLevel = 0.0f;
            std::string pfxName;
        };

        struct C_MusicTheme : Instance {
          std::string file;
          float       vol=0.f;
          int32_t     loop=0;
          float       reverbMix=0;
          float       reverbTime=0;
          int32_t     transType=0;
          int32_t     transSubType=0;
          };

        struct C_GilValues : Instance {
          C_GilValues(){
            for(auto& i:jumpup_height)      i = 200;
            for(auto& i:blood_max_distance) i = 1000;
            for(auto& i:blood_amount)       i = 10;
            for(auto& i:turn_speed)         i = 150;
            }
          int32_t      water_depth_knee   [GIL_MAX]={};
          int32_t      water_depth_chest  [GIL_MAX]={};
          int32_t      jumpup_height      [GIL_MAX]={};		//	DEFAULT = 200;
          // int32_t      JUMPUP_FORCE       [GIL_MAX]={};
          int32_t      swim_time          [GIL_MAX]={};
          int32_t      dive_time          [GIL_MAX]={};
          int32_t      step_height			  [GIL_MAX]={};
          int32_t      jumplow_height     [GIL_MAX]={};
          int32_t      jumpmid_height     [GIL_MAX]={};
          int32_t      slide_angle        [GIL_MAX]={};
          int32_t      slide_angle2       [GIL_MAX]={};
          int32_t      disable_autoroll   [GIL_MAX]={}; //	DEFAULT = 0					;  0 = Autoroll  enabled	/ 1 = Autoroll  disabled
          int32_t      surface_align      [GIL_MAX]={}; //	DEFAULT = 0					;  0 = Alignment disabled	/ 1 = Alignment enabled
          int32_t      climb_heading_angle[GIL_MAX]={};
          int32_t      climb_horiz_angle  [GIL_MAX]={};
          int32_t      climb_ground_angle [GIL_MAX]={};
          int32_t      fight_range_base   [GIL_MAX]={};
          int32_t      fight_range_fist   [GIL_MAX]={};
          int32_t      fight_range_g      [GIL_MAX]={};
          int32_t      fight_range_1hs    [GIL_MAX]={};
          int32_t      fight_range_1ha    [GIL_MAX]={};
          int32_t      fight_range_2hs    [GIL_MAX]={};
          int32_t      fight_range_2ha    [GIL_MAX]={};
          int32_t      falldown_height    [GIL_MAX]={}; //									Wie tief Fallen ohne Schaden ?
          int32_t      falldown_damage    [GIL_MAX]={}; //									Schaden für jeden weiteren angefangenen Meter.
          int32_t      blood_disabled     [GIL_MAX]={}; //	DEFAULT = 0					;	Blut ganz ausschalten (z.B. bei Sekletten) ?
          int32_t      blood_max_distance [GIL_MAX]={}; //	DEFAULT = 1000				;	Wie weit spritzt das Blut (in cm) ?
          int32_t      blood_amount       [GIL_MAX]={}; //	DEFAULT = 10				;	Wie viel Blut ?
          int32_t      blood_flow         [GIL_MAX]={}; //	DEFAULT = 0					;	Soll es sich langsam ausbreiten ?
          std::string  blood_emitter      [GIL_MAX]={}; //	DEFAULT = "PFX_BLOOD"		;	Welcher Partikel-Emitter ?
          std::string  blood_texture      [GIL_MAX]={}; //	DEFAULT = "ZBLOODSPLAT2.TGA";	Welche Textur ?
          int32_t      turn_speed         [GIL_MAX]={}; //	DEFAULT = 150				;
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
    }  // namespace GEngineClasses

    /**
     * @brief Links the classes known to the engine to the VM.
     */
    void registerGothicEngineClasses(DaedalusVM& vm);
}  // namespace Daedalus
