#include <cctype>
#include <cmath>

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <utils/logger.h>

#include "modelScriptParser.h"
#include "zenParser.h"

using namespace ZenLoad;

MdsParserTxt::MdsParserTxt(ZenParser &zen)
  :zen(zen) {
  buf.reserve(8);
  }

MdsParserTxt::TokType MdsParserTxt::nextTok(std::string& buf) {
  buf.clear();
  bool loop=true;
  while(loop && zen.getRemainBytes()>0) {
    loop = false;
    const char first = zen.readChar();

    if(first==' ' || first=='\t' || first==':' /* fps and range semicolon*/) {
      loop = true;
      }
    else if(first=='/' && zen.peekChar()=='/') {
      zen.readChar();
      while(zen.getRemainBytes()>0) {
        const char cur = zen.readChar();
        if(cur=='\n')
          break;
        }
      loop = true;
      }
    else if(first=='\n' || first=='\r') {
      if(zen.peekChar()=='\n')
        zen.setSeek(zen.getSeek()+1);
      return TokType::TK_CRLF;
      }
    else if(('a'<=first && first<='z') || ('A'<=first && first<='Z') || first=='_' || first=='*' || first=='.' ) {
      buf.push_back(first);
      while(zen.getRemainBytes()>0) {
        const char cur = zen.peekChar();
        if(('a'<=cur && cur<='z') || ('A'<=cur && cur<='Z') || cur=='*' || cur=='.' || cur=='_' || ('0'<=cur && cur<='9')) {
          buf.push_back(zen.readChar());
          } else {
          break;
          }
        }
      return TokType::TK_Name;
      }
    else if('0'<=first && first<='9') {
      bool flt=false;
      buf.push_back(first);
      while(zen.getRemainBytes()>0) {
        const char cur = zen.peekChar();
        if(('0'<=cur && cur<='9') || (cur=='.' && !flt)) {
          buf.push_back(zen.readChar());
          if(cur=='.')
            flt=true;
          } else {
          break;
          }
        }
      return TokType::TK_Num;
      }
    else if(first=='\"') {
      while(zen.getRemainBytes()>0) {
        const char cur = zen.readChar();
        if(cur=='\"' || cur==')') // NOTE: closing bracket is for mal-formed input where the second apostrophe is missing - *eventSFXGrnd	(72	"Turn	) in Wolf.mds
          break;
        if(cur=='\r' || cur=='\n') { // NOTE: second approach working around this also merged...don't know if counterproductive
          // Invalid syntax
          break; // G1 wolf.mds is missing closing " - need to workaround it
          }
        buf.push_back(cur);
        }
      for(size_t i=0;i<buf.size();) { // sanitize input
        if(buf[i]=='\t')
          buf[i]=' ';
        else
          ++i;
        }
      while(buf.size()>0 && std::isspace(buf[buf.size()-1])) // clean trailing whitespaces
          buf.erase(buf.size()-1);
      return TokType::TK_String;
      }
    else if(first=='(') {
      return TokType::TK_BracketL;
      }
    else if(first==')') {
      return TokType::TK_BracketR;
      }
    else if(first=='{') {
      return TokType::TK_Begin;
      }
    else if(first=='}') {
      return TokType::TK_End;
      }
    }

  return TokType::TK_Null;
  }

MdsParser::Chunk MdsParserTxt::supportedChunks() const {
  if(buf=="Model" || buf=="model")
    return CHUNK_MODEL;
  if(buf=="meshAndTree" || buf=="MeshAndTree")
    return CHUNK_MESH_AND_TREE;
  if(buf=="registerMesh")
    return CHUNK_REGISTER_MESH;
  if(buf=="aniEnum")
    return CHUNK_ANI_ENUM;
  if(buf=="ani")
    return CHUNK_ANI;
  if(buf=="aniComb")
    return CHUNK_ANI_COMB;
  if(buf=="aniDisable")
    return CHUNK_ANI_DISABLE;
  if(buf=="aniAlias")
    return CHUNK_ANI_ALIAS;
  if(buf=="aniBlend")
    return CHUNK_ANI_BLEND;
  if(buf=="*eventSFX")
    return CHUNK_EVENT_SFX;
  if(buf=="*eventSFXGrnd")
    return CHUNK_EVENT_SFX_GRND;
  if(buf=="*eventPFX")
    return CHUNK_EVENT_PFX;
  if(buf=="*eventPFXStop")
    return CHUNK_EVENT_PFX_STOP;
  if(buf=="*eventTag")
    return CHUNK_EVENT_TAG;
  if(buf=="*eventMMStartAni")
    return CHUNK_EVENT_MMSTARTANI;
  if(buf=="*eventCamTremor")
    return CHUNK_EVENT_CAMTREMOR;
  if(buf=="modelTag")
    return CHUNK_MODEL_TAG;
  if(buf=="aniComb")
    return CHUNK_ANI_COMB;
  if(buf=="aniDisable" || buf=="AniDisable")
    return CHUNK_ANI_DISABLE;
  return CHUNK_ERROR;
  }

MdsParser::Chunk MdsParser::matchingChunkClose(const MdsParser::Chunk& in) {
  switch(in) {
    case MdsParser::CHUNK_ANI_ENUM:     return MdsParser::CHUNK_ANI_ENUM_END;
    case MdsParser::CHUNK_ANI_EVENTS:   return MdsParser::CHUNK_ANI_EVENTS_END;
    case MdsParser::CHUNK_MODEL:        return MdsParser::CHUNK_MODEL_END;
    case MdsParser::CHUNK_MODEL_SCRIPT: return MdsParser::CHUNK_MODEL_SCRIPT_END;
    default:                            return MdsParser::CHUNK_ERROR;
    }
  }

MdsParser::Chunk MdsParserTxt::beginChunk() {
  while(zen.getRemainBytes()>0) {
    const TokType tt = nextTok(buf);
    if(tt==TK_Begin) {
      tokenStack.push(tt);
      openChunks.push(chunks.top());
      }
    else if(tt==TK_End) {
      if(tokenStack.size() == 0 || tokenStack.top()!=TK_Begin)
        LogInfo() << "Mds Syntax error";
      tokenStack.pop();
      openChunks.pop();
      }
    else if(tt==TK_BracketL) {
      tokenStack.push(tt);
      }
    else if(tt==TK_BracketR || (tt==TK_CRLF && tokenStack.size() && tokenStack.top()==TK_BracketL /* cope with missing closing bracket*/)) {
      if(tokenStack.size() == 0 || tokenStack.top()!=TK_BracketL)
        LogInfo() << "Mds Syntax error";
      tokenStack.pop();
      }
    else if(tt==TK_Name) {
      const auto& c = supportedChunks();
      if(c!=CHUNK_ERROR) {
        if(chunks.size())
          chunks.pop();
       chunks.push(c);
        }
      return c;
      }
    }
  return CHUNK_EOF;
  }

void MdsParserTxt::beginArgs() {
  while(zen.getRemainBytes()>0) {
    const TokType tt = nextTok(buf);
    if(tt==TK_BracketL) {
      tokenStack.push(tt);
      return;
      }
    else
      LogInfo() << "Mds Syntax error";
    }
  }

void MdsParserTxt::endArgs() {
  while(zen.getRemainBytes()>0) {
    size_t seek = zen.getSeek();
    const TokType tt = nextTok(buf);
    if(tt==TK_BracketR) {
      const auto& t = tokenStack.top();
      if(tokenStack.top()!=TK_BracketL){
        LogInfo() << "Mds Syntax error" << t << " and " << tt;
        zen.setSeek(seek); // unget
        }
      tokenStack.pop();
      return;
      }
    // this implies no arguments across multiple lines - may be only valid for vanilla mds files, other authors might save those files differently
    else if(tt==TK_CRLF && tokenStack.top()==TK_BracketL) {
      tokenStack.pop();
      // LogInfo() << "Input file mal-formed - closing bracket in input file missing: " << (uint8_t)tt << " - buffer content: " << buf << " line: " << lineNumber;
      zen.setSeek(seek);
      return;
      }
    else {
      LogInfo() << "Input file mal-formed - or parser broken because of bugfix attempts: " << (uint8_t)tt << " - buffer content: " << buf;
      zen.setSeek(seek);
      return;
      }
    }
  }

void MdsParserTxt::endChunk() {}

void MdsParserTxt::implReadItem(MdsParserTxt::TokType dest, bool optional) {
  while(zen.getRemainBytes()>0) {
    const size_t seek = zen.getSeek();
    const TokType tt = nextTok(buf);
    if(tt==dest)
      return;
    if(optional) {
      zen.setSeek(seek); // unget
      buf.clear();
      return;
      }
    }
  buf.clear();
  }

std::string MdsParserTxt::readStr() {
  implReadItem(TK_String);
  return std::move(buf);
  }

std::string MdsParserTxt::readKeyword() {
  implReadItem(TK_Name);
  return std::move(buf);
  }

uint32_t MdsParserTxt::readDWord() {
  return uint32_t(readI32());
  }

int32_t MdsParserTxt::readI32() {
  implReadItem(TK_Num);
  if(buf.size()>0)
    return (int32_t)std::atol(buf.c_str());
  return 0;
  }

int16_t MdsParserTxt::readI16() {
  implReadItem(TK_Num);
  if (buf.size() > 0)
      return (int16_t)std::atoi(buf.c_str());
  return 0;
  }

float MdsParserTxt::readFloat() {
  implReadItem(TK_Num);
  if(buf.size()>0)
    return float(std::atof(buf.c_str()));
  return 0;
  }

std::string MdsParserTxt::readOptStr() {
  implReadItem(TK_String,true);
  return std::move(buf);
  }

std::string MdsParserTxt::readOptKeyword() {
  implReadItem(TK_Name,true);
  return std::move(buf);
  }

uint32_t MdsParserTxt::readOptDWord() {
  return uint32_t(readOptI32());
  }

int32_t MdsParserTxt::readOptI32() {
  implReadItem(TK_Num,true);
  if(buf.size()>0)
    return (int32_t)std::atol(buf.c_str());
  return 0;
  }

int16_t MdsParserTxt::readOptI16() {
  implReadItem(TK_Num,true);
  if (buf.size() > 0)
    return (int16_t)std::atoi(buf.c_str());
  return 0;
  }

float MdsParserTxt::readOptFloat() {
  implReadItem(TK_Num,true);
  if(buf.size()>0)
    return float(std::atof(buf.c_str()));
  return 0;
  }

void MdsParserTxt::readMeshAndTree() {
  std::string mesh = readStr();
  std::string hint = readOptKeyword();

  meshAndThree.m_Name     = mesh;
  meshAndThree.m_Disabled = (hint=="DONT_USE_MESH");
  }

// NOTE: those extra getSeek and setSeek are there for mal-formed cases. Happened that a closing bracket is missing, the token type TK_Name matches, so even if optional seek is not reset inside the implRead() function
float MdsParserTxt::readOptFps() {
  size_t seek = zen.getSeek();
  implReadItem(TK_Name,true);
  if(buf.compare("FPS")!=0) {
    zen.setSeek(seek);
    return 0;
    }
  implReadItem(TK_Num,true);
  if(buf.size()>0)
    return float(std::atof(buf.c_str()));
  return 0;
  }

float MdsParserTxt::readOptCvs() {
  size_t seek = zen.getSeek();
  implReadItem(TK_Name,true);
  if(buf.compare("CVS")!=0) {
    zen.setSeek(seek);
    return 0;
    }
  implReadItem(TK_Num,true);
  if(buf.size()>0)
    return float(std::atof(buf.c_str()));
  return 0;
  }

float MdsParserTxt::readOptRange() {
  size_t seek = zen.getSeek();
  implReadItem(TK_Name,true);
  if(buf.compare("r")!=0) {
    zen.setSeek(seek);
    return 0;
    }
  implReadItem(TK_Num,true);
  if(buf.size()>0)
    return float(std::atof(buf.c_str()));
  return 0;
  }


MdsParserBin::MdsParserBin(ZenParser &zen)
  :zen(zen) {
  }

MdsParser::Chunk MdsParserBin::beginChunk() {
  if(zen.getFileSize()<=zen.getSeek()+sizeof(BinaryChunkInfo))
    return MdsParser::CHUNK_EOF;

  BinaryChunkInfo chunk{};
  zen.readStructure(chunk);
  chunkEnd = uint32_t(zen.getSeek())+chunk.length;
  return MdsParser::Chunk(chunk.id);
  }

void MdsParserBin::endChunk() {
  zen.setSeek(chunkEnd);
  }

std::string MdsParserBin::readStr() {
  return zen.readLine(false);
  }

std::string MdsParserBin::readKeyword() {
  return zen.readLine(false);
  }

uint32_t MdsParserBin::readDWord() {
  return zen.readBinaryDWord();
  }

int32_t MdsParserBin::readI32() {
  return int32_t(zen.readBinaryDWord());
  }

int16_t MdsParserBin::readI16() {
  return int16_t(zen.readBinaryWord());
  }

float MdsParserBin::readFloat() {
  return zen.readBinaryFloat();
  }

void MdsParserBin::readMeshAndTree() {
  const bool dontUseMesh = readDWord()!=0;

  meshAndThree.m_Name     = readStr();
  meshAndThree.m_Disabled = dontUseMesh;
  }


MdsParser::Chunk MdsParser::parse() {
  while(true) {
    const Chunk ch = beginChunk();
    switch(ch) {
      case CHUNK_EOF:
        break;
      case CHUNK_ERROR:
        LogInfo() << "Mds Error chunk";
        break;
      case CHUNK_MLID_MODELMESH:
      case CHUNK_MLID_MDM_SOURCE:
      case CHUNK_MLID_MDM_NODEMESHES:
      case CHUNK_MLID_MDM_SOFSKINLIST:
        break;
      case CHUNK_SOURCE:
        beginArgs();
        readSource();
        endArgs();
        break;
      case CHUNK_MODEL:
        beginArgs();
        readModel();
        endArgs();
        break;
      case CHUNK_MODEL_SCRIPT:
        beginArgs();
        readModelScript();
        endArgs();
        break;
      case CHUNK_MODEL_END:
      case CHUNK_MODEL_SCRIPT_END:
      case CHUNK_ANI_ENUM:
      case CHUNK_ANI_ENUM_END:
      case CHUNK_ANI_EVENTS:
      case CHUNK_ANI_EVENTS_END:
        break;
      case CHUNK_ANI_COMB:
        beginArgs();
        readAniComb();
        endArgs();
        break;
      case CHUNK_ANI_DISABLE:
        beginArgs();
        readAniDisable();
        endArgs();
        break;
      case CHUNK_ANI_BLEND:
        beginArgs();
        readAniBlend();
        endArgs();
        break;
      case CHUNK_MESH_AND_TREE:
        beginArgs();
        readMeshAndTree();
        endArgs();
        break;
      case CHUNK_REGISTER_MESH:
        beginArgs();
        readRegisterMesh();
        endArgs();
        break;
      case CHUNK_ANI:
        beginArgs();
        readAni();
        endArgs();
        break;
      case CHUNK_ANI_ALIAS:
        beginArgs();
        readAniAlias();
        endArgs();
        break;
      case CHUNK_EVENT_SFX:
        beginArgs();
        readSfx(sfx);
        endArgs();
        break;
      case CHUNK_EVENT_SFX_GRND:
        beginArgs();
        readSfx(gfx);
        endArgs();
        break;
      case CHUNK_EVENT_PFX:
        beginArgs();
        readPfx(pfx);
        endArgs();
        break;
      case CHUNK_EVENT_PFX_GRND:
        beginArgs();
        readPfx(gpfx);
        endArgs();
        break;
      case CHUNK_EVENT_PFX_STOP:
        beginArgs();
        readPfxStop();
        endArgs();
        break;
      case CHUNK_EVENT_TAG:
        beginArgs();
        readEvent(eventTag);
        endArgs();
        break;
      case CHUNK_MODEL_TAG:
        beginArgs();
        readModelTag();
        endArgs();
        break;
      case CHUNK_EVENT_MMSTARTANI:
        beginArgs();
        readMMStart();
        endArgs();
        break;
      case CHUNK_EVENT_CAMTREMOR:
        beginArgs();
        readCamTremor();
        endArgs();
        break;
      case CHUNK_ANI_SYNC:
      case CHUNK_ANI_BATCH:
      case CHUNK_ANI_MAX_FPS:
      case CHUNK_EVENT_SET_MESH:
      case CHUNK_EVENT_SWAP_MESH:
        break;
      }
    endChunk();
    return ch;
    }
  }

uint32_t MdsParser::makeAniFlags(const std::string& flag_str) {
  uint32_t flags = 0;
  for(auto ch : flag_str) {
    switch(ch) {
      case 'M':
        flags |= MSB_MOVE_MODEL;
        break;
      case 'R':
        flags |= MSB_ROTATE_MODEL;
        break;
      case 'E':
        flags |= MSB_QUEUE_ANI;
        break;
      case 'F':
        flags |= MSB_FLY;
        break;
      case 'I':
        flags |= MSB_IDLE;
        break;
      case '.':
        break;
      }
    }
  return flags;
  }

EModelScriptAniDir MdsParser::makeAniDir(const std::string& str){
  return (!str.empty() && str[0] == 'R') ? MSB_BACKWARD : MSB_FORWARD;
  }

void MdsParser::readRegisterMesh() {
  std::string mesh = readStr();
  meshesASC.emplace_back(std::move(mesh));
  }

void MdsParser::readAni() {
  ani.m_Name        = readStr();
  ani.m_Layer       = readDWord();
  ani.m_Next        = readStr();
  ani.m_BlendIn     = readFloat();
  ani.m_BlendOut    = readFloat();
  ani.m_Flags       = makeAniFlags(readKeyword());
  ani.m_Asc         = readStr();
  ani.m_Dir         = makeAniDir(readKeyword());
  ani.m_FirstFrame  = readI32();
  ani.m_LastFrame   = readI32();
  ani.m_MaxFps      = readOptFps();
  ani.m_Speed       = readOptFloat();
  ani.m_ColVolScale = readOptCvs();
  }

void MdsParser::readAniComb() {
  comb.m_Name      = readStr();
  comb.m_Layer     = readDWord();
  comb.m_Next      = readStr();
  comb.m_BlendIn   = readFloat();
  comb.m_BlendOut  = readFloat();
  comb.m_Flags     = makeAniFlags(readKeyword());
  comb.m_Asc       = readStr();
  comb.m_LastFrame = readDWord();
  }

void MdsParser::readAniAlias() {
  alias.m_Name     = readStr();
  alias.m_Layer    = readDWord();
  alias.m_Next     = readStr();
  alias.m_BlendIn  = readFloat();
  alias.m_BlendOut = readFloat();
  alias.m_Flags    = makeAniFlags(readKeyword());
  alias.m_Alias    = readStr();
  alias.m_Dir      = makeAniDir(readOptKeyword());
  }

void MdsParser::readAniBlend() {
  blend.m_Name     = readStr();
  blend.m_Next     = readStr();
  blend.m_BlendIn  = readOptFloat();
  blend.m_BlendOut = readOptFloat();
  }

void MdsParser::readAniDisable() {
  disable.m_Name = readStr();
  }

void MdsParser::readModel() {
  modelName = readStr();
  }

void MdsParser::readModelScript() {
  script.m_Version = readI32();
  script.m_Folder  = readStr();
  }

void MdsParser::readSource() {
  source.m_Year     = readI32();
  source.m_Month    = readI16();
  source.m_Day      = readI16();
  source.m_Hour     = readI16();
  source.m_Minutes  = readI16();
  source.m_Seconds  = readI32();
  source.m_File     = readStr();
  }

void MdsParser::readSfx(std::vector<zCModelScriptEventSfx> &out) {
  zCModelScriptEventSfx s;
  s.m_Frame     = readI32();
  s.m_Name      = readStr();
  s.m_Range     = readOptRange();
  s.m_EmptySlot = readOptKeyword()=="EMPTY_SLOT" || readOptStr()=="EMPTY_SLOT";
  out.emplace_back(std::move(s));
  }

void MdsParser::readPfx(std::vector<zCModelScriptEventPfx> &out) {
  zCModelScriptEventPfx p;
  p.m_Frame = readI32();
  p.m_Num   = readOptI32();
  p.m_Name  = readStr();
  p.m_Pos   = readOptStr();
  p.m_isAttached = readOptKeyword()=="ATTACH" || readOptStr()=="ATTACH";
  out.emplace_back(std::move(p));
  }

void MdsParser::readPfxStop() {
  zCModelScriptEventPfxStop p;
  p.m_Frame = readI32();
  p.m_Num   = readI32();
  pfxStop.emplace_back(p);
  }

void MdsParser::readAniDef(EModelScriptAniDef& m_Def) {
  std::string str = readStr();
  for(auto& c:str)
    c = char(std::toupper(c));

  // https://worldofplayers.ru/threads/37708/
  if(str=="DEF_CREATE_ITEM")
    m_Def=DEF_CREATE_ITEM;
  else if(str=="DEF_INSERT_ITEM")
    m_Def=DEF_INSERT_ITEM;
  else if(str=="DEF_REMOVE_ITEM")
    m_Def=DEF_REMOVE_ITEM;
  else if(str=="DEF_DESTROY_ITEM")
    m_Def=DEF_DESTROY_ITEM;
  else if(str=="DEF_PLACE_ITEM")
    m_Def=DEF_PLACE_ITEM;
  else if(str=="DEF_EXCHANGE_ITEM")
    m_Def=DEF_EXCHANGE_ITEM;
  else if(str=="DEF_FIGHTMODE")
    m_Def=DEF_FIGHTMODE;
  else if(str=="DEF_PLACE_MUNITION")
    m_Def=DEF_PLACE_MUNITION;
  else if(str=="DEF_REMOVE_MUNITION")
    m_Def=DEF_REMOVE_MUNITION;
  else if(str=="DEF_DRAWSOUND")
    m_Def=DEF_DRAWSOUND;
  else if(str=="DEF_UNDRAWSOUND")
    m_Def=DEF_UNDRAWSOUND;
  else if(str=="DEF_SWAPMESH")
    m_Def=DEF_SWAPMESH;
  else if(str=="DEF_DRAWTORCH")
    m_Def=DEF_DRAWTORCH;
  else if(str=="DEF_INV_TORCH")
    m_Def=DEF_INV_TORCH;
  else if(str=="DEF_DROP_TORCH")
    m_Def=DEF_DROP_TORCH;
  else if(str=="DEF_HIT_LIMB")
    m_Def=DEF_HIT_LIMB;
  else if(str=="DEF_DIR")
    m_Def=DEF_DIR;
  else if(str=="DEF_DAM_MULTIPLY")
    m_Def=DEF_DAM_MULTIPLY;
  else if(str=="DEF_PAR_FRAME")
    m_Def=DEF_PAR_FRAME;
  else if(str=="DEF_OPT_FRAME")
    m_Def=DEF_OPT_FRAME;
  else if(str=="DEF_HIT_END")
    m_Def=DEF_HIT_END;
  else if(str=="DEF_WINDOW")
    m_Def=DEF_WINDOW;
  }

void MdsParser::readEvent(std::vector<zCModelEvent> &out) {
  zCModelEvent evt;
  evt.m_Frame = readOptI32();

  readAniDef(evt.m_Def);
  if(evt.m_Def==DEF_NULL)
    return;

  std::string str;
  switch(evt.m_Def) {
    case DEF_NULL:
    case DEF_LAST:
      break;
    case DEF_CREATE_ITEM:
    case DEF_EXCHANGE_ITEM:
      evt.m_Slot = readOptStr();
      evt.m_Item = readOptStr();
      break;
    case DEF_INSERT_ITEM:
    case DEF_PLACE_MUNITION:
      evt.m_Slot = readOptStr();
      break;
    case DEF_REMOVE_ITEM:
      evt.m_Item = readOptStr();
      break;
    case DEF_DESTROY_ITEM:
    case DEF_PLACE_ITEM:
    case DEF_REMOVE_MUNITION:
      break; // vanilla scripts dont have item specified, leave it out just in case reading would mess up something
    case DEF_DRAWSOUND:
    case DEF_UNDRAWSOUND:
      evt.m_Slot = readOptStr();
      break;
    case DEF_FIGHTMODE:
      evt.m_Fmode = readFMode();
      break;
    case DEF_SWAPMESH:
      evt.m_Slot  = readOptStr();
      evt.m_Slot2 = readOptStr();
      break;
    case DEF_DRAWTORCH:
    case DEF_INV_TORCH:
    case DEF_DROP_TORCH:
      break;
    case DEF_HIT_LIMB:
      evt.m_Slot  = readOptStr();
      break;
    case DEF_DIR:
      evt.m_Slot  = readOptStr(); // FIXME: unused - values seen are "O" - Oben=Up, "U" - Unten=Down - or in this context "from above" and "from below" probably
      break;
    case DEF_DAM_MULTIPLY:
    case DEF_PAR_FRAME:
    case DEF_OPT_FRAME:
    case DEF_HIT_END:
    case DEF_WINDOW:{
      str = readStr();
      std::stringstream ss(str);
      while(!ss.eof()) {
        int frame=0;
        ss >> frame;
        if(!ss.good() && !ss.eof())
          break;
        evt.m_Int.push_back(frame);
        }
      break;
      }
    }

  out.emplace_back(std::move(evt));
  }

void MdsParser::readModelTag() {
  readAniDef(modelTag.m_Def);
  modelTag.m_Slot = readStr();
  }

void MdsParser::readMMStart() {
  zCModelScriptEventMMStartAni mm;
  mm.m_Frame     = readI32();
  mm.m_Animation = readStr();
  mm.m_Node      = readOptStr();
  mmStartAni.emplace_back(std::move(mm));
  }

void MdsParser::readCamTremor() {
  zCModelScriptEventCamTremor ct;
  ct.m_Frame      = readI32();
  ct.m_Range      = readI32();
  ct.m_InnerRange = readI32();
  ct.m_minTime    = readI32();
  ct.m_maxTime    = readI32();
  camTremors.emplace_back(ct);
  }

EFightMode MdsParser::readFMode() {
  auto s = readStr();
  if(s=="")
    return FM_NONE;
  if(s=="FIST")
    return FM_FIST;
  if(s=="1H" || s=="1h")
    return FM_1H;
  if(s=="2H" || s=="2h")
    return FM_2H;
  if(s=="BOW")
    return FM_BOW;
  if(s=="CBOW")
    return FM_CBOW;
  if(s=="MAG")
    return FM_MAG;
  return FM_NONE;
  }

std::string MdsParser::readOptStr() {
  return readStr();
  }

std::string MdsParser::readOptKeyword() {
  return readKeyword();
  }

uint32_t MdsParser::readOptDWord() {
  return readDWord();
  }

int32_t MdsParser::readOptI32() {
  return readI32();
  }

float MdsParser::readOptFloat() {
  return readFloat();
  }

int16_t MdsParser::readOptI16() {
  return readI16();
  }

float MdsParser::readOptFps() {
  return readOptFloat();
  }

float MdsParser::readOptCvs() {
  return readOptFloat();
  }

float MdsParser::readOptRange() {
  return readOptFloat();
  }
