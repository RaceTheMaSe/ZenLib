
#include "zCCSLib.h"
#include <algorithm>
#include "parserImpl.h"
#include "zenParser.h"
#include "zenParserPropRead.h"
#include <cassert>
#include "zenload/zTypes.h"
#include <utils/logger.h>
#include <vdfs/fileIndex.h>

using namespace ZenLoad;

std::map<std::string, size_t> zCCSLib::m_MessagesByName;
std::vector<oCMsgConversation> zCCSLib::conversations;

zCCSLib::zCCSLib(const std::string& fileName, const VDFS::FileIndex& fileIndex, ZenLoad::ZenParser::FileVersion version) {
  try {
    ZenLoad::ZenParser parser(fileName,fileIndex);
    readObjectData(parser,version);
    }
  catch (std::exception& e) {
    LogError() << e.what();
    return;
    }
  }

zCCSLib::zCCSLib(ZenParser &parser, ZenLoad::ZenParser::FileVersion version) {
  readObjectData(parser,version);
  }

static void readEventMessage(zCEventMessage& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSAtomicBlock& parent) {
  auto& rd = *parser.getImpl();
  if(version==ZenParser::FileVersion::Gothic1)
    rd.readEntry("subType", reinterpret_cast<uint8_t&>(block.subType));
  else
    rd.readEntry("subType", block.subType);
  }

static void readScreenFx(zCEventScreenFX& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSAtomicBlock& parent) {
  readEventMessage(block,parser,header,version,parent);
  block.type = zCEventMessage::EventMsgType::MT_ScreenFX;
  auto& rd = *parser.getImpl();
  rd.readEntry("duration", block.duration);
  rd.readColor("color"   , block.color);
  rd.readEntry("fovDeg"  , block.fovDeg);
  }

static void readNpcMessage(oCNpcMessage& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSAtomicBlock& parent) {
  readEventMessage(block,parser,header,version,parent);
  block.type = zCEventMessage::EventMsgType::MT_Npc;
  if(parser.peekChar()=='['){
    ZenParser::ChunkHeader blkHdr={};
    parser.readChunkStart(blkHdr);
    // FIXME: read targetVob if there is any
    parser.readChunkEnd();
    }
  }

static void readCSCamera_EventMsg(zCCSCamera_EventMsg& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSAtomicBlock& parent) {
  readEventMessage(block,parser,header,version,parent);
  block.type = zCEventMessage::EventMsgType::MT_MsgCSCamera;
  auto& rd = *parser.getImpl();
  switch (block.subType) {
    case (uint32_t)CamEventTypes::EV_CAM_SET_DURATION:
    case (uint32_t)CamEventTypes::EV_CAM_SET_TO_TIME:
    case (uint32_t)CamEventTypes::EV_CAM_PLAY: {
      rd.readEntry("time", block.time, true); break;
      }
    case (uint32_t)CamEventTypes::EV_CAM_GOTO_KEY: {
      rd.readEntry("time", block.time, true);
      rd.readEntry("key", block.key, true);
      rd.readEntry("kfType", block.kfType, true);
      break;
      }
    case (uint32_t)CamEventTypes::EV_CAM_NOTHING:
    case (uint32_t)CamEventTypes::EV_CAM_PAUSE:
    case (uint32_t)CamEventTypes::EV_CAM_RESUME:
    case (uint32_t)CamEventTypes::EV_CAM_STOP:
    default: break;
    }
  }

static void readCSCamera_EventMsgActivate(zCCSCamera_EventMsgActivate& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSAtomicBlock& parent) {
  readEventMessage(block,parser,header,version,parent);
  block.type = zCEventMessage::EventMsgType::MT_MsgCSCameraActivate;
  auto& rd = *parser.getImpl();
  rd.readEntry("refVobName", block.refVobName);
  }
  
static void readEventCore(zCEventCore& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSAtomicBlock& parent) {
  readEventMessage(block,parser,header,version,parent);
  block.type = zCEventMessage::EventMsgType::MT_Core;
  auto& rd = *parser.getImpl();
  rd.readEntry("damage", block.damage, true);
  rd.readEntry("damageType", block.damageType, true);
  }

static void readEventCommon(zCEventCommon& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSAtomicBlock& parent) {
  readEventMessage(block,parser,header,version,parent);
  block.type = zCEventMessage::EventMsgType::MT_Common;
  }

static void readEventMover(zCEventMover& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSAtomicBlock& parent) {
  readEventMessage(block,parser,header,version,parent);
  block.type = zCEventMessage::EventMsgType::MT_Mover;
  }

static void readEventMsgAttack(oCMsgAttack& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSAtomicBlock& parent) {
  readNpcMessage(block,parser,header,version,parent);
  block.type = zCEventMessage::EventMsgType::MT_MsgAttack;
  auto& rd = *parser.getImpl();
  rd.readEntry("combo", block.combo, true);
  }

static void readEventMsgCutscene(zCEvMsgCutscene& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSAtomicBlock& parent) {
  readEventMessage(block,parser,header,version,parent);
  block.type = zCEventMessage::EventMsgType::MT_MsgCutscene;
  }

static void readEventMsgConversation(oCMsgConversation& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSAtomicBlock& parent) {
  readNpcMessage(block,parser,header,version,parent);
  block.type = zCEventMessage::EventMsgType::MT_MsgConversation;
  auto& rd = *parser.getImpl();
  switch (block.subType) {
    case (uint32_t)ConversationEventTypes::EV_PLAYSOUND:
    case (uint32_t)ConversationEventTypes::EV_CUTSCENE:
    case (uint32_t)ConversationEventTypes::EV_PLAYANISOUND: {
      rd.readEntry("text", block.text,true);
      rd.readEntry("name", block.name,true);
      break;
      }
    case (uint32_t)ConversationEventTypes::EV_OUTPUTSVM:
    case (uint32_t)ConversationEventTypes::EV_PLAYANI_FACE:
    case (uint32_t)ConversationEventTypes::EV_PLAYANI: {
      rd.readEntry("name", block.name, true);
      break;
      }
    // case (uint32_t)ConversationEventTypes::EV_TARGETVOBNAME: // FIXME: check again, have missed something
    case (uint32_t)ConversationEventTypes::EV_POINTAT:
    case (uint32_t)ConversationEventTypes::EV_QUICKLOOK:
    case (uint32_t)ConversationEventTypes::EV_LOOKAT: {
      rd.readEntry("targetVobName", block.targetVobName, true);
      break;
      }
    case (uint32_t)ConversationEventTypes::EV_OUTPUT: {
      rd.readEntry("number", block.number, true);
      break;
      }
    case (uint32_t)ConversationEventTypes::EV_ASK:
    case (uint32_t)ConversationEventTypes::EV_WAITTILLEND:
    case (uint32_t)ConversationEventTypes::EV_WAITFORQUESTION:
    case (uint32_t)ConversationEventTypes::EV_STOPLOOKAT:
    case (uint32_t)ConversationEventTypes::EV_STOPPOINTAT:
    case (uint32_t)ConversationEventTypes::EV_PLAYANI_NOOVERLAY:
    case (uint32_t)ConversationEventTypes::EV_PROCESSINFOS:
    case (uint32_t)ConversationEventTypes::EV_STOPPROCESSINFOS:
    case (uint32_t)ConversationEventTypes::EV_OUTPUTSVM_OVERLAY:
    case (uint32_t)ConversationEventTypes::EV_SNDPLAY:
    default: break;
    }

    if(!block.name.empty())
      zCCSLib::addMessageByName(block.name.c_str(),std::move(block));
  }

static void readEventMsgDamage(oCMsgDamage& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSAtomicBlock& parent) {
  readNpcMessage(block,parser,header,version,parent);
  block.type = zCEventMessage::EventMsgType::MT_MsgDamage;
  }

static void readEventMsgMagic(oCMsgMagic& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSAtomicBlock& parent) {
  readNpcMessage(block,parser,header,version,parent);
  block.type = zCEventMessage::EventMsgType::MT_MsgMagic;
  auto& rd = *parser.getImpl();
  rd.readEntry("what"        , block.what, true);
  rd.readEntry("level"       , block.level, true);
  rd.readEntry("removeSymbol", block.removeSymbol, true);
  }

static void readEventMsgManipulate(oCMsgManipulate& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSAtomicBlock& parent) {
  readNpcMessage(block,parser,header,version,parent);
  block.type = zCEventMessage::EventMsgType::MT_MsgManipulate;
  auto& rd = *parser.getImpl();
  switch (block.subType) {
    case (uint32_t)ManipulateEventTypes::EV_USEITEMTOSTATE:
    case (uint32_t)ManipulateEventTypes::EV_USEMOB: {
      rd.readEntry("schemeName" , block.schemeName,true);
      rd.readEntry("targetState", block.targetState,true);
      break;
      }
    case (uint32_t)ManipulateEventTypes::EV_USEITEM:
    case (uint32_t)ManipulateEventTypes::EV_CALLSCRIPT:
    case (uint32_t)ManipulateEventTypes::EV_EQUIPITEM:
    case (uint32_t)ManipulateEventTypes::EV_TAKEMOB:
    case (uint32_t)ManipulateEventTypes::EV_DROPMOB:
    case (uint32_t)ManipulateEventTypes::EV_DROPVOB: {
      rd.readEntry("schemeName", block.schemeName, true); break;
      }
    case (uint32_t)ManipulateEventTypes::EV_TAKEVOB: {
      rd.readEntry("targetVobName", block.targetVobName, true); break;
      }
    case (uint32_t)ManipulateEventTypes::EV_THROWVOB:
    case (uint32_t)ManipulateEventTypes::EV_EXCHANGE:
    default: break;
    }
  }

static void readEventMsgMovement(oCMsgMovement& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSAtomicBlock& parent) {
  readNpcMessage(block,parser,header,version,parent);
  block.type = zCEventMessage::EventMsgType::MT_MsgMovement;
  auto& rd = *parser.getImpl();
  switch (block.subType) {
    case (uint32_t)MovementEventTypes::EV_SETWALKMODE: {
      rd.readEntry("targetMode", block.targetMode, true);
      }
    case (uint32_t)MovementEventTypes::EV_GOTOPOS:
    case (uint32_t)MovementEventTypes::EV_JUMP: {
      rd.readEntry("targetPos", block.targetPos, true); break;
      }
    case (uint32_t)MovementEventTypes::EV_GOTOVOB:
    case (uint32_t)MovementEventTypes::EV_TURNTOVOB:
    case (uint32_t)MovementEventTypes::EV_TURNAWAY:
    case (uint32_t)MovementEventTypes::EV_WHIRLAROUND: {
      rd.readEntry("targetVobName", block.targetVobName, true); break;
      }
    case (uint32_t)MovementEventTypes::EV_GOROUTE:
    case (uint32_t)MovementEventTypes::EV_BEAMTO:
    case (uint32_t)MovementEventTypes::EV_GOTOFP: {
      rd.readEntry("targetName", block.targetName, true); break;
      }
    case (uint32_t)MovementEventTypes::EV_TURN: {
      rd.readEntry("angle", block.angle, true); break;
      }
    case (uint32_t)MovementEventTypes::EV_TURNTOPOS: {
      rd.readEntry("targetPos", block.targetPos, true); break;
      }
    case (uint32_t)MovementEventTypes::EV_ROBUSTTRACE:
    case (uint32_t)MovementEventTypes::EV_STANDUP:
    case (uint32_t)MovementEventTypes::EV_CANSEENPC:
    case (uint32_t)MovementEventTypes::EV_STRAFE:
    case (uint32_t)MovementEventTypes::EV_DODGE:
    case (uint32_t)MovementEventTypes::EV_ALIGNTOFP:
    default: break;
    }
  }

static void readEventMsgState(oCMsgState& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSAtomicBlock& parent) {
  readNpcMessage(block,parser,header,version,parent);
  block.type = zCEventMessage::EventMsgType::MT_Npc;
  }

static void readEventMsgUseItem(oCMsgUseItem& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSAtomicBlock& parent) {
  readNpcMessage(block,parser,header,version,parent);
  block.type = zCEventMessage::EventMsgType::MT_MsgUseItem;
  }

static void readEventMsgWeapon(oCMsgWeapon& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSAtomicBlock& parent) {
  readNpcMessage(block,parser,header,version,parent);
  block.type = zCEventMessage::EventMsgType::MT_MsgWeapon;
  auto& rd = *parser.getImpl();
  rd.readEntry("targetMode", block.targetMode, true);
  }

static void readEventMusicController(zCEventMusicControler& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSAtomicBlock& parent) {
  readEventMessage(block,parser,header,version,parent);
  block.type = zCEventMessage::EventMsgType::MT_MusicController;
  }

static void readAtomicBlock(zCCSAtomicBlock& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSBlock& parent) {
  // assumed to only contain one event block
  ZenParser::ChunkHeader blkHdr;
  parser.readChunkStart(blkHdr);
  auto& rd = *parser.getImpl();
  switch(blkHdr.classId) {
    case ZenParser::ZenClass::zCEventMessage: {
      zCEventMessage message;
      readEventMessage(message,parser,blkHdr,version,block);
      block.command = message;
      break;
      }
    case ZenParser::ZenClass::zCEventScreenFX: {
      zCEventScreenFX screenFX;
      readScreenFx(screenFX,parser,blkHdr,version,block);
      block.command = screenFX;
      break;
      }
    case ZenParser::ZenClass::oCNpcMessage: {
      oCNpcMessage npc;
      readNpcMessage(npc,parser,blkHdr,version,block);
      block.command = npc;
      break;
      }
    case ZenParser::ZenClass::zCCSCamera_EventMsg: {
      zCCSCamera_EventMsg cam;
      readCSCamera_EventMsg(cam,parser,blkHdr,version,block);
      block.command = cam;
      break;
      }
    case ZenParser::ZenClass::zCCSCamera_EventMsgActivate: {
      zCCSCamera_EventMsgActivate camActivate;
      readCSCamera_EventMsgActivate(camActivate,parser,blkHdr,version,block);
      block.command = camActivate;
      break;
      }
    case ZenParser::ZenClass::zCEventCommon: {
      zCEventCommon common;
      readEventCommon(common,parser,blkHdr,version,block);
      block.command = common;
      break;
      }
    case ZenParser::ZenClass::zCEventCore: {
      zCEventCore core;
      readEventCore(core,parser,blkHdr,version,block);
      block.command = core;
      break;
      }
    case ZenParser::ZenClass::zCMover: {
      zCEventMover mover;
      readEventMover(mover,parser,blkHdr,version,block);
      block.command = mover;
      break;
      }
    case ZenParser::ZenClass::oCMsgAttack: {
      oCMsgAttack attack;
      readEventMsgAttack(attack,parser,blkHdr,version,block);
      block.command = attack;
      break;
      }
    case ZenParser::ZenClass::ocMsgCutscene: {
      zCEvMsgCutscene cutscene;
      readEventMsgCutscene(cutscene,parser,blkHdr,version,block);
      block.command = cutscene;
      break;
      }
    case ZenParser::ZenClass::oCMsgConversation: {
      oCMsgConversation conversation;
      readEventMsgConversation(conversation,parser,blkHdr,version,block);
      block.command = conversation;
      break;
      }
    case ZenParser::ZenClass::oCMsgDamage: {
      oCMsgDamage damage;
      readEventMsgDamage(damage,parser,blkHdr,version,block);
      block.command = damage;
      break;
      }
    case ZenParser::ZenClass::oCMsgMagic: {
      oCMsgMagic magic;
      readEventMsgMagic(magic,parser,blkHdr,version,block);
      block.command = magic;
      break;
      }
    case ZenParser::ZenClass::oCMsgManipulate: {
      oCMsgManipulate manipulate;
      readEventMsgManipulate(manipulate,parser,blkHdr,version,block);
      block.command = manipulate;
      break;
      }
    case ZenParser::ZenClass::oCMsgMovement: {
      oCMsgMovement movement;
      readEventMsgMovement(movement,parser,blkHdr,version,block);
      block.command = movement;
      break;
      }
    case ZenParser::ZenClass::oCMsgState: {
      oCMsgState state;
      readEventMsgState(state,parser,blkHdr,version,block);
      block.command = state;
      break;
      }
    case ZenParser::ZenClass::oCMsgUseItem: {
      oCMsgUseItem useItem;
      readEventMsgUseItem(useItem,parser,blkHdr,version,block);
      block.command = useItem;
      break;
      }
    case ZenParser::ZenClass::oCMsgWeapon: {
      oCMsgWeapon weapon;
      readEventMsgWeapon(weapon,parser,blkHdr,version,block);
      block.command = weapon;
      break;
      }
    case ZenParser::ZenClass::zCMusicControler: {
      zCEventMusicControler musicController;
      readEventMusicController(musicController,parser,blkHdr,version,block);
      block.command = musicController;
      break;
      }
    default: {
      LogInfo() << "Unrecognized block " << blkHdr.name << " (" << blkHdr.classId << ") in zCCSAtomicBlock";
      break;
      }
    }
  rd.readEntry("synchronized", block.synchronized,true);
  parser.readChunkEnd();
  }

static void readCSSyncBlock(zCCSSyncBlock& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSBlock& parent);
static void readCSBlock(zCCSBlock& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version) {
  uint32_t numOfBlocks = 0;
  ReadObjectProperties(parser,
  Prop("blockName"  , block.blockName),
  Prop("numOfBlocks", numOfBlocks));
  block.time.resize(numOfBlocks);
  for(size_t i=0;i<numOfBlocks;++i) {
    ReadObjectProperties(parser,Prop((std::string("subBlock")+std::to_string(i)).c_str(),block.time[i]));
    zCCSBlock csBlock;
    ZenParser::ChunkHeader blkHdr;
    parser.readChunkStart(blkHdr);
    switch(blkHdr.classId) {
      case ZenParser::ZenClass::zCCSAtomicBlock: {
        zCCSAtomicBlock atomic;
        readAtomicBlock(atomic,parser,blkHdr,version,block);
        block.atomicBlocks.emplace_back(atomic);
        parser.readChunkEnd();
        break;
        }
      case ZenParser::ZenClass::zCCSSyncBlock: {
        zCCSSyncBlock syncBlock;
        readCSSyncBlock(syncBlock,parser,blkHdr,version,block);
        block.syncBlocks.emplace_back(syncBlock);
        parser.readChunkEnd();
        break;
        }
      default: {
        LogInfo() << "Unrecognized block " << blkHdr.name << " (" << blkHdr.classId << ") in CSSyncBlock";
        }
      }
    }
  }

static void readCSSyncBlock(zCCSSyncBlock& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSBlock& parent) {
  readCSBlock(block,parser,header,version);
  uint32_t numOfAss=0;
  ReadObjectProperties(parser,Prop("numOfAss", numOfAss));
  block.roleAss.resize(numOfAss);
  for(size_t i=0;i<numOfAss;++i)
    ReadObjectProperties(parser, Prop((std::string("roleAss")+std::to_string(i)).c_str(),block.roleAss[i]));
  }

static void readCSProps(zCCSProps& props, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSBlock& parent) {
  ZenParser::ChunkHeader roleHdr;
  parser.readChunkStart(roleHdr);

  ReadObjectProperties(parser,
  Prop("globalCutscene"   , props.globalCutscene),
  Prop("csLoop"           , props.csLoop),
  Prop("hasToBeTriggerd"  , props.hasToBeTriggered),
  Prop("distance"         , props.distance),
  Prop("range"            , props.range),
  Prop("numLockedBlocks"  , props.numLockedBlocks),
  Prop("runBehaviour"     , reinterpret_cast<uint32_t&>(props.runBehaviour)),
  Prop("runBehaviourValue", props.runBehaviourValue),
  Prop("StageName"        , props.stageName),
  Prop("scriptFuncOnStop" , props.scriptFuncOnStop));

  parser.readChunkEnd();
  }

static void readCSRole(zCCSRole& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSBlock& parent) {
  ZenParser::ChunkHeader roleHdr;
  parser.readChunkStart(roleHdr);

  ReadObjectProperties(parser,
  Prop("mustBeAlive", block.mustBeAlive),
  Prop("roleName"   , block.roleName),
  Prop("roleType"   , reinterpret_cast<uint32_t&>(block.roleType)));
  
  ZenParser::ChunkHeader integratedVobHdr;
  parser.readChunkStart(integratedVobHdr);
  // add parsing when there is data and move to own function ... in my test there was none
  parser.readChunkEnd();
  
  parser.readChunkEnd();
  }

static void readCSRoleVob(zCCSRoleVob& block, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version, zCCSBlock& parent) {
  }

static void readCutsceneData(zCCutscene& cutscene, ZenParser& parser, const ZenParser::ChunkHeader &header, ZenParser::FileVersion version) {
  readCSBlock(cutscene,parser,header,version);
  readCSProps(cutscene.props,parser,header,version,cutscene);
  uint32_t numOfRoles = 0;
  parser.getImpl()->readEntry("numOfRoles", numOfRoles);
  for(size_t i=0;i<numOfRoles;++i) {
    zCCSRole role;
    readCSRole(role,parser,header,version,cutscene);
    cutscene.roles.emplace_back(role);
    }
  uint32_t numOfRoleVobs = 0;
  parser.getImpl()->readEntry("numOfRoleVobs", numOfRoleVobs);
  for(size_t i=0;i<numOfRoleVobs;++i) {
    zCCSRoleVob roleVob;
    readCSRoleVob(roleVob,parser,header,version,cutscene);
    cutscene.roleVobs.emplace_back(roleVob);
    }
  ZenParser::ChunkHeader mainRoleHr;
  parser.readChunkStart(mainRoleHr);
  // FIXME: read if there is data here ... in my simple test there is not ... havent looked into how vobs could be addressed in spacer context
  parser.readChunkEnd();
  }

void zCCSLib::readObjectData(ZenParser& parser, ZenLoad::ZenParser::FileVersion version) {
  parser.readHeader();

  ZenParser::ChunkHeader libHeader;
  parser.readChunkStart(libHeader);

  if(libHeader.classId == ZenParser::zCCSBlock) {
    readCutsceneData(cutsceneData,parser,libHeader,version);
    return;
    }
  else if(libHeader.classId != ZenParser::zCCSLib)
    throw std::runtime_error("Failed to read zCCSLib, neither zCCSLib nor zCCutscene");

  // FIXME: this clears data so the same stuff can be added again, implies a zCCSLib can only load one CSLib and possibly multiple cutscenes with collision for text strings
  conversations.clear();
  m_MessagesByName.clear();

  uint32_t NumOfItems=0;
  parser.getImpl()->readEntry("NumOfItems", NumOfItems);

  LogInfo() << "Reading " << NumOfItems << " blocks";

  for (uint32_t i = 0; i < NumOfItems; i++) {
    ZenParser::ChunkHeader blkHdr;
    parser.readChunkStart(blkHdr);

    if(blkHdr.classId!=ZenParser::zCCSBlock) {
      LogInfo() << "Unrecognized block " << blkHdr.name << " (" << blkHdr.classId << ") in CSLib";
      parser.skipChunk();
      continue;
      }

    zCCSBlock blk;
    readCSBlock(blk,parser,blkHdr,version);
    parser.readChunkEnd();
    }
  }

void zCCSLib::addMessageByName(const std::string& name, oCMsgConversation&& msg) {
  auto nameUppered = name;
  std::transform(nameUppered.begin(), nameUppered.end(), nameUppered.begin(), ::toupper);
  const auto& fileExtension = nameUppered.find(".WAV"); // or make it even more generalized by using a fileext helper function here
  if(fileExtension!=std::string::npos && !name.empty()) {
    nameUppered=nameUppered.substr(0,fileExtension);
    m_MessagesByName[nameUppered] = conversations.size();
    conversations.emplace_back(std::move(msg));
    }
  }

const oCMsgConversation& zCCSLib::getMessageByName(const Daedalus::ZString& name) {
  std::string nameUppered = name.c_str();
  std::transform(nameUppered.begin(), nameUppered.end(), nameUppered.begin(), ::toupper);
  assert(m_MessagesByName.find(nameUppered) != m_MessagesByName.end());
  size_t idx = m_MessagesByName[nameUppered];
  assert(idx<conversations.size());
  return conversations[idx];
  }

bool zCCSLib::messageExists(const Daedalus::ZString& name) const {
  std::string nameUppered = name.c_str();
  std::transform(nameUppered.begin(), nameUppered.end(), nameUppered.begin(), ::toupper);
  return m_MessagesByName.find(nameUppered) != m_MessagesByName.end();
  }
