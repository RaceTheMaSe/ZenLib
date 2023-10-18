#include "parserImpl.h"

#include <cctype>

ZenLoad::ParserImpl::ParserImpl(ZenParser* parser)
    : m_pParser(parser)
  {
  }

bool ZenLoad::ParserImpl::parseHeader(ZenLoad::ZenParser::ChunkHeader& header, const char* vobDescriptor, size_t vobDescriptorLen) {
  if(vobDescriptor==nullptr || vobDescriptorLen<=2)
    return false;
  if(vobDescriptor[0]!='[' || vobDescriptor[vobDescriptorLen-1]!=']')
    return false;

  header.objectID = 0;
  header.size     = 0;  // Doesn't matter in ASCII
  header.version  = 0;
  header.name.clear();

  /* Examples:
   * [% oCWorld:zCWorld 64513 0]
   * [VobTree % 0 0]
   * [% zCVob 52224 1]
   * [% zCVobLight:zCVob 39168 9]
   * [visual zCProgMeshProto 64513 2]
   * [visual zCParticleFX 0 6]
   * [visual zCMesh 64513 19]
   * [visual \xA7 0 2]
   * [% \xA7 0 25111]
   *
   * Syntax:
   *  <object-name | '%'> <class-name | '%' | \xA7> [version] [objId]
   * */
  enum Token:uint8_t {
    T_Name    = 0,
    T_Class   = 1,
    T_Version = 2,
    T_Id      = 3,
    };

  vobDescriptor++;
  vobDescriptorLen-=2;

  Token tokType = T_Name;
  for(size_t i=0; i<vobDescriptorLen; ++i) {
    if(vobDescriptor[i]==' ' || vobDescriptor[i]=='\t')
      continue;
    const char* tok = &vobDescriptor[i];
    size_t      len = 0;
    while(i<vobDescriptorLen && !(vobDescriptor[i]==' ' || vobDescriptor[i]=='\t')) {
      ++i;
      ++len;
      }

    switch(tokType) {
      case T_Name: {
        tokType = T_Class;
        if(len==1 && *tok=='%')
          break;
        header.name = std::string(tok,len);
        break;
        }
      case T_Class: {
        tokType          = T_Version;
        if(len==1 && *tok=='%')
          break;
        //if(len==1 && *tok=='\xA7')
        //  break; // reference
        header.classId   = parseClassName(tok,len);
        break;
        }
      case T_Version: {
        tokType        = T_Id;
        header.version = (uint16_t)std::atoi(tok);
        break;
        }
      case T_Id: {
        header.objectID = std::atoi(tok);
        break;
        }
      }
    }

  return true;
  }

ZenLoad::ZenParser::ZenClass ZenLoad::ParserImpl::parseClassName(const char* name, size_t len) {
  static const std::initializer_list<std::pair<const char*,ZenLoad::ZenParser::ZenClass>> cls =
  {
    {"",                                                 ZenParser::zUnknown                    },
    {"\xA7",                                             ZenParser::zReference                  },
    {"\xa7",                                             ZenParser::zReference                  },
    {"zCCSLib",                                          ZenParser::zCCSLib                     },
    {"zCCSBlock",                                        ZenParser::zCCSBlock                   },
    {"zCCSAtomicBlock",                                  ZenParser::zCCSAtomicBlock             },
    {"oCMsgConversation:oCNpcMessage:zCEventMessage",    ZenParser::oCMsgConversation           },
    {"oCMsgMovement:oCNpcMessage:zCEventMessage",        ZenParser::oCMsgMovement               },
    {"zCCSCamera_EventMsg:zCEventMessage",               ZenParser::zCCSCamera_EventMsg         },
    {"zCDecal",                                          ZenParser::zCDecal                     },
    {"zCProgMeshProto",                                  ZenParser::zCProgMeshProto             },
    {"zCParticleFX",                                     ZenParser::zCParticleFX                },
    {"zCMesh",                                           ZenParser::zCMesh                      },
    {"zCModel",                                          ZenParser::zCModel                     },
    {"zCMorphMesh",                                      ZenParser::zCMorphMesh                 },
    {"oCWorld:zCWorld",                                  ZenParser::zCWorld                     },
    {"zCWayNet",                                         ZenParser::zCWayNet                    },
    {"zCWaypoint",                                       ZenParser::zCWaypoint                  },
    {"zCVob",                                            ZenParser::zCVob                       },
    {"zCVobLevelCompo:zCVob",                            ZenParser::zCVobLevelCompo             },
    {"oCItem:zCVob",                                     ZenParser::oCItem                      },
    {"oCMOB:zCVob",                                      ZenParser::oCMOB                       },
    {"oCMobInter:oCMOB:zCVob",                           ZenParser::oCMobInter                  },
    {"oCMobBed:oCMobInter:oCMOB:zCVob",                  ZenParser::oCMobBed                    },
    {"oCMobFire:oCMobInter:oCMOB:zCVob",                 ZenParser::oCMobFire                   },
    {"oCMobLadder:oCMobInter:oCMOB:zCVob",               ZenParser::oCMobLadder                 },
    {"oCMobSwitch:oCMobInter:oCMOB:zCVob",               ZenParser::oCMobSwitch                 },
    {"oCMobWheel:oCMobInter:oCMOB:zCVob",                ZenParser::oCMobWheel                  },
    {"oCMobContainer:oCMobInter:oCMOB:zCVob",            ZenParser::oCMobContainer              },
    {"oCMobDoor:oCMobInter:oCMOB:zCVob",                 ZenParser::oCMobDoor                   },
    {"zCPFXControler:zCVob",                             ZenParser::zCPFXController             },
    {"zCVobAnimate:zCVob",                               ZenParser::zCVobAnimate                },
    {"zCVobLensFlare:zCVob",                             ZenParser::zCVobLensFlare              },
    {"zCVobLight:zCVob",                                 ZenParser::zCVobLight                  },
    {"zCVobLightPreset",                                 ZenParser::zCVobLightPreset            },
    {"zCVobSpot:zCVob",                                  ZenParser::zCVobSpot                   },
    {"zCVobStartpoint:zCVob",                            ZenParser::zCVobStartpoint             },
    {"zCVobSound:zCVob",                                 ZenParser::zCVobSound                  },
    {"zCVobSoundDaytime:zCVobSound:zCVob",               ZenParser::zCVobSoundDaytime           },
    {"oCZoneMusic:zCVob",                                ZenParser::oCZoneMusic                 },
    {"oCZoneMusicDefault:oCZoneMusic:zCVob",             ZenParser::oCZoneMusicDefault          },
    {"zCZoneReverb:zCVob",                               ZenParser::zCZoneReverb                },
    {"zCZoneReverbDefault:zCZoneReverb:zCVob",           ZenParser::zCZoneReverbDefault         },
    {"zCTouchAnimate:zCVob",                             ZenParser::zCTouchAnimate              },
    {"zCTouchAnimateSound:zCTouchAnimate:zCVob",         ZenParser::zCTouchAnimateSound         },
    {"zCZoneZFog:zCVob",                                 ZenParser::zCZoneZFog                  },
    {"zCZoneZFogDefault:zCZoneZFog:zCVob",               ZenParser::zCZoneZFogDefault           },
    {"zCZoneVobFarPlane:zCVob",                          ZenParser::zCZoneVobFarPlane           },
    {"zCZoneVobFarPlaneDefault:zCZoneVobFarPlane:zCVob", ZenParser::zCZoneVobFarPlaneDefault    },
    {"zCMessageFilter:zCVob",                            ZenParser::zCMessageFilter             },
    {"zCCodeMaster:zCVob",                               ZenParser::zCCodeMaster                },
    {"zCTrigger:zCVob",                                  ZenParser::zCTrigger                   },
    {"zCTriggerList:zCTrigger:zCVob",                    ZenParser::zCTriggerList               },
    {"oCTriggerScript:zCTrigger:zCVob",                  ZenParser::oCTriggerScript             },
    {"zCMover:zCTrigger:zCVob",                          ZenParser::zCMover                     },
    {"oCTriggerChangeLevel:zCTrigger:zCVob",             ZenParser::oCTriggerChangeLevel        },
    {"zCTriggerWorldStart:zCVob",                        ZenParser::zCTriggerWorldStart         },
    {"zCTriggerUntouch:zCVob",                           ZenParser::zCTriggerUntouch            },
    {"zCCSCamera:zCVob",                                 ZenParser::zCCSCamera                  },
    {"zCCamTrj_KeyFrame:zCVob",                          ZenParser::zCCamTrj_KeyFrame           },
    {"oCTouchDamage:zCTouchDamage:zCVob",                ZenParser::oCTouchDamage               },
    {"zCEarthquake:zCVob",                               ZenParser::zCEarthquake                },
    {"zCAICamera",                                       ZenParser::zCAICamera                  },
    {"zCMoverControler:zCVob",                           ZenParser::zCMoverController           },
    {"zCVobScreenFX:zCVob",                              ZenParser::zCVobScreenFX               },
    {"zCVobStair:zCVob",                                 ZenParser::zCVobStair                  },
    {"oCCSTrigger:zCTrigger:zCVob",                      ZenParser::oCCSTrigger                 },
    {"zCMusicControler:zCVob",                           ZenParser::zCMusicControler            },
    {"oCVisualFX:zCVob",                                 ZenParser::ocVisualFX                  },
    {"oCVisFX_MultiTarget:oCVisualFX:zCVob",             ZenParser::ocVisFX_MultiTarget         },
    {"zCLensFlareFX",                                    ZenParser::zCLensFlareFX               },
    {"zCEventManager",                                   ZenParser::zCEventManager              },
    {"zCCutscene:zCCSBlock",                             ZenParser::zCCSBlock                   },
    {"zCCSRole",                                         ZenParser::zCCSRole                    },
    {"oCCSProps:zCCSProps",                              ZenParser::oCCSProps                   },
    {"zCCSSyncBlock:zCCSBlock",                          ZenParser::zCCSSyncBlock               },
    {"oCMsgManipulate:oCNpcMessage:zCEventMessage",      ZenParser::oCMsgManipulate             },
    {"zCEventScreenFX:zCEventMessage",                   ZenParser::zCEventScreenFX             },
    {"zCCSCamera_EventMsgActivate:zCEventMessage",       ZenParser::zCCSCamera_EventMsgActivate },
    {"zCCSCamera_EventMsg:zCEventMessage",               ZenParser::zCCSCamera_EventMsg         },
    {"oCMsgAttack:oCNpcMessage:zCEventMessage",          ZenParser::oCMsgAttack                 },
    {"ocMsgDamage:oCNpcMessage:zCEventMessage",          ZenParser::oCMsgDamage                 },
    {"oCMsgMagic:oCNpcMessage:zCEventMessage",           ZenParser::oCMsgMagic                  },
    {"oCMsgState:oCNpcMessage:zCEventMessage",           ZenParser::oCMsgState                  },
    {"oCMsgUseItem:oCNpcMessage:zCEventMessage",         ZenParser::oCMsgUseItem                },
    {"oCMsgWeapon:oCNpcMessage:zCEventMessage",          ZenParser::oCMsgWeapon                 },
    {"oCMobMsg:zCEventMessage",                          ZenParser::ocMobMsg                    },
    {"zCEvMsgCutscene:zCEventMessage",                   ZenParser::zCEvMsgCutscene             },
    {"zCEventCommon:zCEventMessage",                     ZenParser::zCEventCommon               },
    {"zCEventCore:zCEventMessage",                       ZenParser::zCEventCore                 },
    {"zCEventMover:zCEventMessage",                      ZenParser::zCEventMover                },
    {"zCEventMusicControler:zCEventMessage",             ZenParser::zCEventMusicControler       }
  };

  for(auto& i:cls) {
    if(std::strlen(i.first)==len && std::memcmp(i.first,name,len)==0)
      return i.second;
    }
  return ZenParser::zUnknown;
  }

size_t ZenLoad::ParserImpl::valueTypeSize(ZenLoad::ParserImpl::EZenValueType type) {
  switch(type) {
    // Byte sized
    case ZVT_BOOL:
    case ZVT_BYTE:
    case ZVT_ENUM:
      return sizeof(uint8_t);
    case ZVT_WORD:
      return sizeof(uint16_t);
    // 32-bit
    case ZVT_INT:
    case ZVT_HASH:
    case ZVT_FLOAT:
    case ZVT_COLOR:
      return sizeof(uint32_t);
    case ZVT_VEC3:
      return sizeof(float)*3;
    // Raw
    case ZVT_RAW_FLOAT:
    case ZVT_RAW:
    case ZVT_STRING:
    case ZVT_0:
      return 0;
    case ZVT_10:
    case ZVT_11:
    case ZVT_12:
    case ZVT_13:
    case ZVT_14:
    case ZVT_15:
      return 0;
    }
  return 0;
  }
