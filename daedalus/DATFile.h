#pragma once

#include <map>
#include <string>
#include <vector>
#include <daedalus/DaedalusStdlib.h>
#include <utils/logger.h>
#include <cassert>
#include <cstddef>
#include <algorithm>
#include <unordered_map>

#include "ZString.h"

namespace Daedalus {
enum EInstanceClass {
  IC_None,
  IC_Npc,
  IC_Mission,
  IC_Info,
  IC_Item,
  IC_ItemReact,
  IC_Focus,
  IC_Menu,
  IC_MenuItem,
  IC_Sfx,
  IC_Pfx,
  IC_Vfx,
  IC_FXEmitKey,
  IC_MusicTheme,
  IC_MusicJingle,
  IC_GilValues,
  IC_FightAi,
  IC_CamSys,
  IC_Spell,
  IC_Svm
  };

class InstancePtr final {
  public:
    InstancePtr() = default;
    InstancePtr(const InstancePtr& other):cls(other.cls),ptr(other.ptr){
      if(ptr)
        ptr->useCount++;
      }

    InstancePtr(InstancePtr&& other) noexcept :cls(other.cls),ptr(other.ptr){
      other.ptr = nullptr;
      }

    ~InstancePtr() {
      if(ptr)
        ptr->useCount--;
      }

    void set(GEngineClasses::Instance *h, EInstanceClass c) {
      if(h)
        h->useCount++;
      if(ptr)
        ptr->useCount--;
      cls = c;
      ptr = h;
      }

    Daedalus::GEngineClasses::Instance*       get()       { return ptr; }
    const Daedalus::GEngineClasses::Instance* get() const { return ptr; }

    InstancePtr& operator = (const std::nullptr_t&) {
      if(ptr)
        ptr->useCount--;
      cls = EInstanceClass::IC_None;
      ptr = nullptr;
      return *this;
      }

    InstancePtr& operator = (const InstancePtr& other) {
      if(&other==this)
        return *this;
      if(other.ptr){
        other.ptr->useCount++;
        if(other.ptr->useCount<0)
          LogWarn() << "Instance pointer useCount negative";
        }
      if(ptr){
        ptr->useCount--;
        if(ptr->useCount<0)
          LogWarn() << "Instance pointer useCount negative";
        }
      cls = other.cls;
      ptr = other.ptr;
      return *this;
      }

    InstancePtr& operator = (InstancePtr&& other) noexcept {
      std::swap(ptr,other.ptr);
      std::swap(cls,other.cls);
      return *this;
      }

    bool instanceOf(const EInstanceClass& c) const {
      return cls==c;
      }

  private:
    Daedalus::EInstanceClass            cls = EInstanceClass::IC_None;
    Daedalus::GEngineClasses::Instance* ptr = nullptr;
  };

template <class C_Class>
EInstanceClass enumFromClass();

enum EParFlag {
  EParFlag_Const    = 1 << 0,
  EParFlag_Return   = 1 << 1,
  EParFlag_ClassVar = 1 << 2,
  EParFlag_External = 1 << 3,
  EParFlag_Merged   = 1 << 4
  };

enum EParType {
  EParType_Void      = 0,
  EParType_Float     = 1,
  EParType_Int       = 2,
  EParType_String    = 3,
  EParType_Class     = 4,
  EParType_Func      = 5,
  EParType_Prototype = 6,
  EParType_Instance  = 7
  };

enum EParOp : uint8_t
  {
  EParOp_Add = 0,              // a + b
  EParOp_Subract = 1,          // a - b
  EParOp_Multiply = 2,         // a * b
  EParOp_Divide = 3,           // a / b
  EParOp_Mod = 4,              // a % b
  EParOp_BinOr = 5,            // a | b
  EParOp_BinAnd = 6,           // a & b
  EParOp_Less = 7,             // a < b
  EParOp_Greater = 8,          // a > b
  EParOp_AssignInt = 9,        // a = b
  EParOp_LogOr = 11,           // a || b
  EParOp_LogAnd = 12,          // a && b
  EParOp_ShiftLeft = 13,       // a << b
  EParOp_ShiftRight = 14,      // a >> b
  EParOp_LessOrEqual = 15,     // a <= b
  EParOp_Equal = 16,           // a == b
  EParOp_NotEqual = 17,        // a != b
  EParOp_GreaterOrEqual = 18,  // a >= b
  EParOp_AssignAdd = 19,       // a += b (a = a + b)
  EParOp_AssignSubtract = 20,  // a -= b (a = a - b)
  EParOp_AssignMultiply = 21,  // a *= b (a = a * b)
  EParOp_AssignDivide = 22,    // a /= b (a = a / b)
  EParOp_Plus = 30,            // +a
  EParOp_Minus = 31,           // -a
  EParOp_Not = 32,             // !a
  EParOp_Negate = 33,          // ~a
  //  EParOp_LeftBracket     = 40,    // '('
  //  EParOp_RightBracket    = 41,    // ')'
  //  EParOp_Semicolon       = 42,    // ';'
  //  EParOp_Comma           = 43,    // ','
  //  EParOp_CurlyBracket    = 44,    // '{', '}'
  //  EParOp_None            = 45,
  //  EParOp_Float           = 51,
  //  EParOp_Var             = 52,
  //  EParOp_Operator        = 53,
  EParOp_Ret = 60,
  EParOp_Call = 61,
  EParOp_CallExternal = 62,
  //  EParOp_PopInt          = 63,
  EParOp_PushInt = 64,
  EParOp_PushVar = 65,
  //  EParOp_PushString      = 66,
  EParOp_PushInstance = 67,
  //  EParOp_PushIndex       = 68,
  //  EParOp_PopVar          = 69,
  EParOp_AssignString = 70,
  EParOp_AssignStringRef = 71,
  EParOp_AssignFunc = 72,
  EParOp_AssignFloat = 73,
  EParOp_AssignInstance = 74,
  EParOp_Jump = 75,
  EParOp_JumpIf = 76,
  EParOp_SetInstance = 80,
  //  EParOp_Skip            = 90,
  //  EParOp_Label           = 91,
  //  EParOp_Func            = 92,
  //  EParOp_FuncEnd         = 93,
  //  EParOp_Class           = 94,
  //  EParOp_ClassEnd        = 95,
  //  EParOp_Instance        = 96,
  //  EParOp_InstanceEnd     = 97,
  //  EParOp_String          = 98,
  //  EParOp_Array           = 180,  // EParOp_Var + 128
  EParOp_PushArrayVar = 245  // EParOp_PushVar + EParOp_Array
  };

template<class T>
class DataContainer final {
  public:
    DataContainer() = default;
    DataContainer(DataContainer&) = delete;
    DataContainer(DataContainer&& oth) noexcept
      : val0(oth.val0), val(std::move(oth.val)), sz(oth.sz) {
      oth.sz = 0;
      }
    ~DataContainer() = default;

    DataContainer& operator = (DataContainer&& oth) noexcept {
      val0 = oth.val0;
      std::swap(val,oth.val);
      std::swap(sz,oth.sz);
      return *this;
      }

    DataContainer& operator = (const DataContainer& oth) {
      if(this==&oth)
        return *this;
      if(oth.sz>1)
        val.reset(new T[oth.sz]); else
        val.reset(nullptr);
      const T* src = oth.data();
      T*       dst = this->data();
      for(size_t i=0; i<oth.sz; ++i)
        dst[i] = src[i];
      sz = oth.sz;
      return *this;
      }

    size_t   size() const { return sz; }
    T&       operator[](size_t i)       { return sz<=1 ? val0 : val[i]; }
    const T& operator[](size_t i) const { return sz<=1 ? val0 : val[i]; }

    T*       data()       { return sz<=1 ? &val0 : val.get(); }
    const T* data() const { return sz<=1 ? &val0 : val.get(); }

    void     resize(size_t size) {
      if(size>1) {
        std::unique_ptr<T[]> n{new T[size]};
        T*                   d = data();
        for(size_t i=0; i<size && i<sz; ++i)
          n[i] = d[i];
        val = std::move(n);
        } else {
        if(sz>0)
          val0 = (*this)[0];
        val.reset(nullptr);
        }
      sz = size;
      }
  private:
    T                    val0 = 0;
    std::unique_ptr<T[]> val;
    size_t               sz = 0;
  };

struct PARSymbol {
  PARSymbol() = default;

  struct Properties {
    int32_t offClsRet;  // Offset (ClassVar) | Size (Class) | ReturnType (Func)
    struct
      {
      uint32_t count : 12;  // Count:12, Type:4 (EParType_), Flags:6 (EParFlag_), Space: 1, Reserved:9
      uint32_t type : 4;    // EParType_*
      uint32_t flags : 6;   // EParFlag_*
      uint32_t space : 1;
      uint32_t reserved : 9;
      } elemProps;

    struct
      {
      uint32_t value : 19;  // Value:19, Reserved:13
      uint32_t reserved : 13;
      } fileIndex;

    struct
      {
      uint32_t value : 19;  // Value:19, Reserved:13
      uint32_t reserved : 13;
      } lineStart;

    struct
      {
      uint32_t value : 19;  // Value:19, Reserved:13
      uint32_t reserved : 13;
      } lineCount;

    struct
      {
      uint32_t value : 24;  // Value:24, Reserved:8
      uint32_t reserved : 8;
      } charStart;

    struct
      {
      uint32_t value : 24;  // Value:24, Reserved:8
      uint32_t reserved : 8;
      } charCount;
    };

  bool hasEParFlag(EParFlag eParFlag) const {
    return static_cast<bool>(properties.elemProps.flags & (uint32_t)eParFlag);
    }

  bool isEParType(EParType eParType) const {
    return properties.elemProps.type == (uint32_t)eParType;
    }

  Properties               properties={};
  std::string              name;
  DataContainer<float>     floatData;
  DataContainer<int32_t>   intData;
  DataContainer<ZString>   strData;
  int32_t                  classOffset=0;
  uint32_t                 address=0;

  // Defacto reflections. Offset of the class member to be able to access class members via name
  // Not stored in files, only valid for classes to directly write to engine memory
  int32_t                  classMemberOffset=-1;
  // Store array size of the class member var. 1 for scalar members. Useful for bounds checking.
  // Not stored in files, only valid for classes to directly write to engine memory
  uint32_t                 classMemberArraySize=0;

  // Not stored in files, only valid for classes to directly write to engine memory
  InstancePtr              instance;
  uint32_t                 parent             = 0xFFFFFFFF;  // 0xFFFFFFFF (-1) = none

  void warnIndexOutOfBounds(size_t index, size_t size) {
    LogWarn() << (const char*) "DaedalusVM: index out of range for: " << name << "[" << size << "], index = " << index;
    }

  template <class T>
  T* getClassMember(void* baseAddr) {
    return reinterpret_cast<T*>(reinterpret_cast<char*>(baseAddr) + classMemberOffset);
    }

  int32_t&       getInt(size_t idx = 0, void* baseAddr = nullptr);
  ZString&       getString(size_t idx = 0, void* baseAddr = nullptr);
  float&         getFloat(size_t idx = 0, void* baseAddr = nullptr);

  const int32_t& getInt(size_t idx = 0, void* baseAddr = nullptr) const;
  const ZString& getString(size_t idx = 0, void* baseAddr = nullptr) const;
  const float&   getFloat(size_t idx = 0, void* baseAddr = nullptr) const;

  template <typename T>
  DataContainer<T>& getDataContainer();

#if !defined(__GNUC__) && !defined(ANDROID) || (defined(__clang_major__))
  template <>
  DataContainer<int32_t>& getDataContainer() {
    return this->intData;
    }

  template <>
  DataContainer<float>& getDataContainer() {
    return this->floatData;
    }

  template <>
  DataContainer<ZString>& getDataContainer() {
    return this->strData;
    }
#endif

  template <class T>
  const T& getValue(size_t idx = 0, void* baseAddr = nullptr) const {
    return getValue<T>(idx,baseAddr);
  }

  template <class T>
  T& getValue(size_t idx = 0, void* baseAddr = nullptr) {
    bool isClassVar = this->hasEParFlag(EParFlag_ClassVar);
    if(isClassVar) {
      bool isRegistered = classMemberOffset != -1;
      if(!isRegistered) {
        LogError() << "DaedalusVM: class data member not registered: " << name;
        }
      else if (baseAddr == nullptr) {
        LogError() << "DaedalusVM: base address of C_Class is nullptr: " << name;
        static T empty={};
        return empty; // FIXME: this comes up often, so assuming that when functions get called with null objects, return zero as otherwise this messes up script logic massively - testcase: npc nek gives deadxp, because ZS_DEAD "(C_NpcIsHuman(other) && other.aivar[AIV_PARTYMEMBER])" returns some garbage value for AIV_PARTYMEMBER called with "instance other" == nullptr
        }
      else if (idx >= classMemberArraySize) {
        warnIndexOutOfBounds(idx, classMemberArraySize);
        LogError() << "DaedalusVM: index out of range for registered class data member: " << name;
        }
      else {
        return getClassMember<T>(baseAddr)[idx];
        }
      }

    auto& data = getDataContainer<T>();
    // read from symbol's data if not isClassVar or the above failed. (the latter should not happen)
    if(data.size()<=idx) {
      if(!isClassVar)  // only print error message if we did not fall through from above
        warnIndexOutOfBounds(idx, data.size());
      data.resize(idx + 1);
      }
    return data[idx];
    }

  void set(int32_t v, size_t idx = 0, void* baseAddr = nullptr)
    {
    switch ((uint32_t)properties.elemProps.type)
      {
      case EParType_Func:
        address = uint32_t(v);

        if (baseAddr && classMemberOffset != -1)
          *reinterpret_cast<int32_t*>(reinterpret_cast<char*>(baseAddr) + classMemberOffset + (sizeof(int32_t) * idx)) = v;
        break;
      default:
        assert(false);
      }
    }
  };

struct PARSymTable {
  std::vector<uint32_t>                       sortTable;
  std::vector<PARSymbol>                      symbols;
  std::vector<std::pair<std::string, size_t>> symbolsByName;
  std::unordered_map<size_t, size_t>          functionsByAddress;
  };

struct PARStack {
  size_t stackOffset;
  size_t stackSize;
  };

struct PARStackOpCode {
  EParOp op;

  /**
   * These are valid depending on "op"
   */
  union {
    int32_t address;  // EParOp_Call, EParOp_Jump, EParOp_JumpIf,
    int32_t symbol;   // EParOp_CallExternal, EParOp_PushVar, EParOp_PushInstance, EParOp_SetInstance, EParOp_PushArrayVar
    int32_t value;    // EParOp_PushInt
    };
  uint8_t index;      // EParOp_PushArrayVar
  size_t  opSize;     // Size of this operation
  };

struct PARInstance {
  std::map<std::string, uint32_t> symbolsByName;
  };

class DATFile {
  public:
    DATFile()=default;
    DATFile(const uint8_t* pData, size_t numBytes);

    bool           hasSymbolName       (const char* symName);
    PARSymbol&     getSymbolByName     (const char* symName);
    size_t         getSymbolIndexByName(const char* symName);
    PARSymbol&     getSymbolByIndex    (size_t idx);

    size_t         getFunctionIndexByAddress(size_t address);
    void           iterateSymbolsOfClass(const char *className, const std::function<void(size_t, PARSymbol&)>& callback);
    const PARStackOpCode &getStackOpCode(size_t pc);
    size_t         addSymbol();

    const PARSymTable& getSymTable() const { return m_SymTable; }

    void           printToLog();

    template<class T,class M>
    void registerMember(const char* name,T M::* field, bool checkIfExists){
      if(checkIfExists && !hasSymbolName(name))
        return;
      auto& parSymbol                = getSymbolByName(name);
      parSymbol.classMemberOffset    = int32_t(offsetOf(field));
      parSymbol.classMemberArraySize = uint32_t(std::max<size_t>(1u,std::extent<T>()));
      }

  private:
    void readSymTable(ZenLoad::ZenParser& input);
    void readStack   (ZenLoad::ZenParser& input);

    template<class T,class M>
    static ptrdiff_t offsetOf(T M::* field){
      char        tmp = 0;
      const char* f   = reinterpret_cast<const char*>(&(reinterpret_cast<M*>(&tmp)->*field));
      return static_cast<ptrdiff_t>(f-&tmp);
      }

    static int compareNoCase(const char* a,const char* b);

    PARSymTable                  m_SymTable;

    std::vector<PARStackOpCode>  code;
    std::vector<PARStackOpCode*> pcCode;
  };

}  // namespace Daedalus
