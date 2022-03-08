//
// Created by andre on 09.05.16.
//

#include "DATFile.h"
#include "zenload/zenParser.h"

#include <algorithm>
#include <map>
#include <cctype>
#include <utils/logger.h>

using namespace ZenLoad;
using namespace Daedalus;

DATFile::DATFile(const uint8_t* pData, size_t numBytes){
  ZenLoad::ZenParser input(pData, numBytes);
  uint8_t version = input.readBinaryByte();
  (void)version;
  readSymTable(input);
  //printToLog();
  }

void DATFile::printToLog() {
  for(size_t i=0;i<getSymTable().symbols.size();++i){
    auto& s = getSymbolByIndex(i);
    if(s.isEParType(EParType::EParType_Instance))
      LogInfo() << s.name;
    }
  }

void DATFile::readSymTable(ZenLoad::ZenParser& input) {
  uint32_t count = input.readBinaryDWord();
  m_SymTable.symbolsByName.reserve(count);
  m_SymTable.symbols  .resize(count);
  m_SymTable.sortTable.resize(count);
  input.readBinaryRaw(m_SymTable.sortTable.data(), sizeof(uint32_t) * count);

  // Read symbols
  for(uint32_t i=0; i<count; i++) {
    PARSymbol s;
    uint32_t named = input.readBinaryDWord();

    if(named) {
      uint8_t b = input.readBinaryByte();
      while(b != 0x0A) {
        if(b != 0xFF)  // FIXME: This happens at INSTANCE_HELP?
          s.name += (char)b;
        b = input.readBinaryByte();
        }
      }

    input.readBinaryRaw(&s.properties, sizeof(s.properties));

    if(0 == (s.properties.elemProps.flags & (uint32_t)EParFlag_ClassVar)) {
      switch((uint32_t)s.properties.elemProps.type) {
        case EParType_Float:
          s.floatData.resize(s.properties.elemProps.count);
          input.readBinaryRaw(s.floatData.data(), sizeof(float) * s.floatData.size());
          break;

        case EParType_Int:
          s.intData.resize(s.properties.elemProps.count);
          input.readBinaryRaw(s.intData.data(), sizeof(uint32_t) * s.intData.size());
          break;

        case EParType_String:
          s.strData.resize(s.properties.elemProps.count);
          for(uint32_t j = 0; j < s.properties.elemProps.count; j++)
            s.strData[j] = ZString(input);
          break;

        case EParType_Class:
          s.classOffset = static_cast<int32_t>(input.readBinaryDWord());
          break;

        case EParType_Func:
        case EParType_Prototype:
        case EParType_Instance:
          s.address = static_cast<int32_t>(input.readBinaryDWord());
          if (s.properties.elemProps.flags & (uint32_t)EParFlag_External)
            {;}
          break;
        }
      }

    s.parent = static_cast<int32_t>(input.readBinaryDWord());

    if(!s.name.empty())
      m_SymTable.symbolsByName.emplace_back(s.name, i);

    // check for callable object
    if(s.isEParType(EParType_Prototype) ||    // is a Prototype
       (s.isEParType(EParType_Func)           // or a function, which
        && !s.hasEParFlag(EParFlag_ClassVar)  // is no class-member (skip class members of type func)
        && s.hasEParFlag(EParFlag_Const)))    // and is const (skip function params of type func)
      {
      m_SymTable.functionsByAddress.emplace(s.address, i);
      }

    m_SymTable.symbols[i] = std::move(s);
    }

  std::sort(m_SymTable.symbolsByName.begin(),m_SymTable.symbolsByName.end(),[](auto& l,auto& r){
    return compareNoCase(l.first.c_str(),r.first.c_str())<0;
    });
  readStack(input);
  }

void DATFile::readStack(ZenLoad::ZenParser& input) {
  size_t stackSize = input.readBinaryDWord();
  size_t seek      = 0;

  while(input.getSeek()<input.getFileSize()) {
    PARStackOpCode s={};
    s.op = static_cast<EParOp>(input.readBinaryByte());
    seek += sizeof(uint8_t);
    s.opSize = sizeof(uint8_t);

    switch(s.op) {
      case EParTok_PushInt:
        s.value = int32_t(input.readBinaryDWord());
        s.opSize += sizeof(int32_t);
        break;
      case EParTok_Call:
      case EParTok_Jump:
      case EParTok_JumpIf:
        s.address = int32_t(input.readBinaryDWord());
        s.opSize += sizeof(int32_t);
        break;
      case EParTok_CallExternal:
      case EParTok_PushVar:
      case EParTok_PushInstance:
      case EParTok_SetInstance:
        s.symbol = int32_t(input.readBinaryDWord());
        s.opSize += sizeof(int32_t);
        break;
      case EParTok_PushArrayVar:
        s.symbol = int32_t(input.readBinaryDWord());
        s.opSize += sizeof(int32_t);
        s.index = input.readBinaryByte();
        s.opSize += sizeof(uint8_t);
        break;
      default:
        break;
      }

    seek += s.opSize;
    code.push_back(s);
    }

  size_t pc=0;
  pcCode.resize(stackSize);
  for(auto& i:code) {
    for(size_t r=0;r<i.opSize;++r)
      pcCode[pc+r] = &i;
    pc+=i.opSize;
    }
  }

int DATFile::compareNoCase(const char *a, const char *b) {
  for(;*a;++a,++b) {
    auto aa = std::toupper(*a);
    auto bb = std::toupper(*b);
    if(aa<bb)
      return -1;
    if(aa>bb)
      return 1;
    }
  if(0<*b)
    return -1;
  if(0>*b)
    return 1;
  return 0;
  }

namespace Daedalus
{
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_Npc>()
    {
        return IC_Npc;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_Mission>()
    {
        return IC_Mission;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_Info>()
    {
        return IC_Info;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_Item>()
    {
        return IC_Item;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_ItemReact>()
    {
        return IC_ItemReact;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_Focus>()
    {
        return IC_Focus;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_Menu>()
    {
        return IC_Menu;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_Menu_Item>()
    {
        return IC_MenuItem;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_SFX>()
    {
        return IC_Sfx;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_ParticleFX>()
    {
        return IC_Pfx;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_MusicTheme>()
    {
        return IC_MusicTheme;
    };
    template <>
    EInstanceClass enumFromClass<Daedalus::GEngineClasses::C_GilValues>()
    {
        return IC_GilValues;
    };
}  // namespace Daedalus

PARSymbol& DATFile::getSymbolByName(const char *symName) {
  size_t id=getSymbolIndexByName(symName);
  if(id==size_t(-1)) {
    LogWarn() << "symbol " << symName << " not found";
    static PARSymbol ps;
    return ps;
    }
  return m_SymTable.symbols[id];
  }

bool DATFile::hasSymbolName(const char* symName) {
  return getSymbolIndexByName(symName)!=size_t(-1);
  }

size_t DATFile::getSymbolIndexByName(const char* symName) {
  if(symName==nullptr)
    return size_t(-1);
  auto it = std::lower_bound(m_SymTable.symbolsByName.begin(),m_SymTable.symbolsByName.end(),symName,[](auto& l,const char* r){
    return compareNoCase(l.first.c_str(),r)<0;
    });
  if(it==m_SymTable.symbolsByName.end())
    return size_t(-1);
  if(compareNoCase(it->first.c_str(),symName)!=0)
    return size_t(-1);
  return it->second;
  }

PARSymbol& DATFile::getSymbolByIndex(size_t idx) {
  if(idx<m_SymTable.symbols.size())
    return m_SymTable.symbols[idx];
  static PARSymbol err;
  return err;
  }

size_t DATFile::getFunctionIndexByAddress(size_t address) {
  auto it = m_SymTable.functionsByAddress.find(address);
  if (it != m_SymTable.functionsByAddress.end())
    return it->second;
  else
    return static_cast<size_t>(-1);
  }

const PARStackOpCode& DATFile::getStackOpCode(size_t pc) {
  if(pc<pcCode.size())
    return *pcCode[pc];
  static PARStackOpCode s={};
  s.op=EParTok_Ret;
  return s;
  }

size_t DATFile::addSymbol() {
  m_SymTable.symbols.emplace_back();
  return m_SymTable.symbols.size() - 1;
  }

void DATFile::iterateSymbolsOfClass(const char* className, const std::function<void(size_t, PARSymbol&)>& callback) {
  constexpr auto none = uint32_t{0xFFFFFFFF};
  // First, find the parent-symbol
  size_t baseSym = getSymbolIndexByName(className);

  for(size_t i=0; i<m_SymTable.symbols.size(); i++) {
    PARSymbol& s = getSymbolByIndex(i);
    if(s.parent==none || s.properties.elemProps.type!=(uint32_t)EParType_Instance)
      continue;

    if((s.properties.elemProps.flags & (uint32_t)Daedalus::EParFlag::EParFlag_Const)==0)
      continue;  // filters out variables of type C_NPC or C_ITEM

    PARSymbol& p = getSymbolByIndex(s.parent);
    uint32_t pBase = s.parent;

    // In case this is also just a prototype, go deeper one more level
    if(p.properties.elemProps.type==(uint32_t)EParType_Prototype && p.parent!=none) {
      pBase = p.parent;
      }

    // If the parent-classes match, we found an instance of our class
    if(baseSym==pBase)
      callback(i, s);
    }
  }

int32_t& PARSymbol::getInt(size_t idx, void* baseAddr) {
  return getValue<int32_t>(idx, baseAddr);
  }

ZString& PARSymbol::getString(size_t idx, void* baseAddr) {
  return getValue<ZString>(idx, baseAddr);
  }

float& PARSymbol::getFloat(size_t idx, void* baseAddr) {
  return getValue<float>(idx, baseAddr);
  }

const int32_t& PARSymbol::getInt(size_t idx, void* baseAddr) const {
  return getValue<int32_t>(idx, baseAddr);
  }

const ZString& PARSymbol::getString(size_t idx, void* baseAddr) const {
  return getValue<ZString>(idx, baseAddr);
  }

const float& PARSymbol::getFloat(size_t idx, void* baseAddr) const {
  return getValue<float>(idx, baseAddr);
  }

#if (defined(__GNUC__) && !defined(ANDROID) && !defined(__clang__)) // FIXME: crude way of getting it to compile with ndk clang 9 for android where __GNUC__ is also defined, gcc 9/10 and clang 10-14 for desktop
template <>
DataContainer<int32_t>& PARSymbol::getDataContainer() {
  return this->intData;
  }

template <>
DataContainer<float>& PARSymbol::getDataContainer() {
  return this->floatData;
  }

template <>
DataContainer<ZString>& PARSymbol::getDataContainer() {
  return this->strData;
  }
#endif
