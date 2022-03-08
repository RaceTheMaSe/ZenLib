#pragma once

#include <functional>
#include <queue>
#include <set>
#include <stack>
#include "DATFile.h"
#include "DaedalusGameState.h"
#include "DaedalusStdlib.h"
#include "ZString.h"

namespace Daedalus {
class DaedalusVM {
  public:
    DaedalusVM(const std::vector<uint8_t>& data);
    DaedalusVM(const uint8_t* pDATFileData, size_t numBytes);

    void    eval(size_t pcInit, bool initScripts, std::function<void(size_t)> f);
    int32_t runFunctionBySymIndex(size_t symIdx, bool initScript, std::function<void(size_t)> f);

    void registerExternalFunction(const char *symName, const std::function<void(DaedalusVM&)>& fn);
    void registerInternalFunction(const char *symName, const std::function<void(DaedalusVM&)>& fn);
    void registerInternalFunction(size_t addr,         const std::function<void(DaedalusVM&)>& fn);
    void unregisterExternalFunction(const char *symName);
    void registerUnsatisfiedLink (const std::function<void(DaedalusVM&)> &fn);

    void pushInt(uint32_t value);
    void pushInt(int32_t value);
    void pushVar(size_t index, uint32_t arrIdx = 0);
    void pushVar(const char *symName);
    void pushString(const ZString& str);

    void setReturn(int32_t v);
    void setReturn(const ZString& v);
    void setReturn(float f);
    void setReturnVar(int32_t v);

    uint32_t     popUInt();
    int32_t      popInt();
    float        popFloat();
    ZString      popString();


    int32_t&     popIntVar();
    float&       popFloatVar();
    ZString&     popStringVar();

    PARSymbol&   popVar();
    uint32_t     popVar(uint32_t& arrIdx);

    void setInstance(const char * instSymbol, GEngineClasses::Instance *h, EInstanceClass instanceClass);
    void setInstance(const size_t instSymbol, GEngineClasses::Instance *h, EInstanceClass instanceClass);
    void clearReferences(GEngineClasses::Instance& h);
    void clearReferences(EInstanceClass h);
    void initializeInstance(GEngineClasses::Instance& instance, size_t symIdx, EInstanceClass classIdx);

    GEngineClasses::Instance*     getCurrentInstanceDataPtr();
    DATFile&                      getDATFile()       { return m_DATFile; }
    const DATFile&                getDATFile() const { return m_DATFile; }
    std::vector<std::string>      getCallStack();
    const std::string&            currentCall();

    PARSymbol&                    globalSelf();
    PARSymbol&                    globalOther();
    PARSymbol&                    globalVictim();
    PARSymbol&                    globalItem();

    size_t                        numFunctionCalls() const { return m_NumFunctionCalls; };
    void                          disAsm(size_t symIdx);

  private:
    template <typename T = int32_t>
    T           popDataValue();
    float       popFloatValue();

    const PARStackOpCode &nextInstruction(size_t& pc);
    void setCurrentInstance(size_t symIdx);

    class CallStackFrame final {
      public:
        enum AddressType : uint8_t {
          Address,
          SymbolIndex
          };
        using FunctionInfo = std::pair<size_t, CallStackFrame::AddressType>;

        CallStackFrame(DaedalusVM& vm, int32_t addressOrIndex, AddressType addrType);
        CallStackFrame(CallStackFrame&)=delete;
        CallStackFrame(CallStackFrame&&)=delete;
        ~CallStackFrame();
        CallStackFrame& operator=(CallStackFrame&)=delete;
        CallStackFrame& operator=(CallStackFrame&&)=delete;

        CallStackFrame* const calee         =nullptr;
        const char*           nameHint      =nullptr;
        size_t                address       =0;
        bool                  hasReturnVal  =false;

      private:
        size_t                prevStackGuard=0;
        DaedalusVM&           vm;
      };

    DATFile m_DATFile;

    struct Stk {
      Stk(int32_t i):i32(i){}
      Stk(float   fIn):f(fIn){}
      Stk(ZString sIn):s(std::move(sIn)){}
      Stk(void* instIn,int32_t i,uint32_t idIn):i32(i),tag(EParTok_PushVar),id(idIn),inst(instIn){}

      union {
        int32_t i32;
        float   f;
        };
      ZString  s;
      EParOp   tag=EParTok_PushInt;
      uint32_t id =0;
      void*    inst=nullptr;
      };

    struct Stack {
      Stack(std::vector<Stk>& s):storage(s),zero(s.size()){}

      std::vector<Stk> storage;
      size_t           zero=0;
      };

    const std::string &nameFromFunctionInfo(CallStackFrame::FunctionInfo functionInfo);

    template<class T>
    [[noreturn]] void terminateScript();

    std::vector<Stk>                                             m_Stack;
    size_t                                                       m_StackGuard=0;

    // contains linked list of stack frames
    CallStackFrame*                                              m_CallStack=nullptr;
    std::vector<std::function<void(DaedalusVM&)>>                m_ExternalsByIndex;
    std::unordered_map<size_t,std::function<void(DaedalusVM&)>>  m_InternalsByIndex;
    std::function<void(DaedalusVM&)>                             m_OnUnsatisfiedCall;

    InstancePtr                                                  m_Instance;
    PARSymbol*                                                   m_InstanceSym = nullptr;

    size_t                                                       m_SelfId   = size_t(-1);
    size_t                                                       m_OtherId  = size_t(-1);
    size_t                                                       m_VictimId = size_t(-1);
    size_t                                                       m_ItemId   = size_t(-1);

    size_t                                                       m_NumFunctionCalls=0;
  };
}  // namespace Daedalus
