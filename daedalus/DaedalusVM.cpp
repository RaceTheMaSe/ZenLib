//
// Created by andre on 09.05.16.
//

#include "DaedalusVM.h"
#include "DaedalusExcept.h"

#include <map>
#include <cassert>
#include <algorithm>

using namespace ZenLoad;
using namespace Daedalus;

DaedalusVM::DaedalusVM(const std::vector<uint8_t>& data)
  :DaedalusVM(data.data(),data.size()){
  }

DaedalusVM::DaedalusVM(const uint8_t* pDATFileData, size_t numBytes)
    : m_DATFile(pDATFileData, numBytes) {
  m_Stack.reserve(1024);
  m_SelfId   = m_DATFile.getSymbolIndexByName("self");
  m_OtherId  = m_DATFile.getSymbolIndexByName("other");
  m_VictimId = m_DATFile.getSymbolIndexByName("victim");
  m_ItemId   = m_DATFile.getSymbolIndexByName("item");
  }

void DaedalusVM::eval(size_t PC, bool initScripts, std::function<void(size_t)> loadProgress) {
  int32_t a=0;
  int32_t b=0;

  while(true) {
    const PARStackOpCode& op = nextInstruction(PC);
    switch (op.op) {
      case EParOp_Plus: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a+b);
        break;
        }
      case EParOp_Minus: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a-b);
        break;
        }
      case EParOp_Multiply: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a*b);
        break;
        }
      case EParOp_Divide:{
        a = popDataValue();
        b = popDataValue();
        if(b==0)
          terminateScript<BadMath>();
        pushInt(a/b);
        break;
        }
      case EParOp_Mod: {
        a = popDataValue();
        b = popDataValue();
        if(b==0)
          terminateScript<BadMath>();
        pushInt(a%b);
        break;
        }
      case EParOp_BinOr: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a | b);
        break;
        }
      case EParOp_BinAnd: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a & b);
        break;
        }
      case EParOp_Less: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a < b ? 1 : 0);
        break;
        }
      case EParOp_Greater: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a > b ? 1 : 0);
        break;
        }
      case EParOp_AssignInt: {
        auto&   a = popIntVar();
        int32_t b = popDataValue();
        a = b;
        break;
        }
      case EParOp_LogOr:
        a = popDataValue();
        b = popDataValue();
        pushInt((a || b) ? 1 : 0);
        break;
      case EParOp_LogAnd:
        a = popDataValue();
        b = popDataValue();
        pushInt((a && b) ? 1 : 0);
        break;
      case EParOp_ShiftLeft: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a << b);
        break;
        }
      case EParOp_ShiftRight: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a >> b);
        break;
        }
      case EParOp_LessOrEqual: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a <= b ? 1 : 0);
        break;
        }
      case EParOp_Equal: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a == b ? 1 : 0);
        break;
        }
      case EParOp_NotEqual: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a != b ? 1 : 0);
        break;
        }
      case EParOp_GreaterOrEqual: {
        a = popDataValue();
        b = popDataValue();
        pushInt(a >= b ? 1 : 0);
        break;
        }
      case EParOp_AssignAdd:{
        int32_t& v = popIntVar();
        int32_t  a = popDataValue();
        v += a;
        break;
        }
      case EParOp_AssignSubtract: {
        int32_t& v = popIntVar();
        int32_t  a = popDataValue();
        v -= a;
        break;
        }
      case EParOp_AssignMultiply: {
        int32_t& v = popIntVar();
        int32_t  a = popDataValue();
        v *= a;
        break;
        }
      case EParOp_AssignDivide: {
        int32_t& v = popIntVar();
        int32_t  b = popDataValue();
        if(b==0)
          terminateScript<BadMath>();
        v /= b;
        break;
        }
      case EParOp_Unary_Plus:
        pushInt(+popDataValue());
        break;
      case EParOp_Unary_Minus:
        pushInt(-popDataValue());
        break;
      case EParOp_Unary_Not:
        pushInt(!popDataValue());
        break;
      case EParOp_Unary_Negate:
        pushInt(~popDataValue());
        break;
      case EParTok_Ret:
        return;
      case EParTok_Call: {
        std::function<void(DaedalusVM&)>* f=nullptr;
        {
          auto it = m_InternalsByIndex.find(op.address);
          if(it!=m_InternalsByIndex.end())
            f = &it->second;
        }

        auto currentInstance = m_Instance;
        
        if(initScripts)
          m_NumFunctionCalls++;

        if(f!=nullptr && *f) {
          CallStackFrame frame(*this, op.address, CallStackFrame::Address);
          (*f)(*this);
          m_NumFunctionCalls++;
          } else {
          CallStackFrame frame(*this, op.address, CallStackFrame::Address);
          eval(size_t(op.address),initScripts,loadProgress);
          }

        if(initScripts)
          loadProgress(m_NumFunctionCalls);

        m_Instance = currentInstance;
        break;
        }

      case EParTok_CallExternal: {
        std::function<void(DaedalusVM&)>* f=nullptr;
        if(size_t(op.symbol)<m_ExternalsByIndex.size()){
          f = &m_ExternalsByIndex[size_t(op.symbol)];
          }

        auto currentInstance = m_Instance;

        if(f!=nullptr && *f) {
          CallStackFrame frame(*this, op.symbol, CallStackFrame::SymbolIndex);
          if(initScripts)
            m_NumFunctionCalls++;
          (*f)(*this);
          if(initScripts)
            loadProgress(m_NumFunctionCalls);
          } else {
          CallStackFrame frame(*this, op.symbol, CallStackFrame::SymbolIndex);
          m_OnUnsatisfiedCall(*this);
          }

        m_Instance = currentInstance;
        break;
        }

      case EParTok_PushInt:
        pushInt(op.value);
        break;
      case EParTok_PushVar:
        pushVar(size_t(op.symbol),0);
        break;
      case EParTok_PushInstance:
        pushInt(op.symbol);
        //pushVar(size_t(op.symbol));
        break;  //TODO: Not sure about this
      case EParTok_AssignString: {
        ZString& s1 = popStringVar();
        ZString  s2 = popString();
        s1 = s2;
        break;
        }
      case EParTok_AssignStringRef:
        LogError() << "EParTok_AssignStringRef not implemented!";
        break;
      case EParTok_AssignFunc: {
        auto&   a = popIntVar();
        int32_t b = popDataValue();
        a = b;
        break;
        }
      case EParTok_AssignFloat: {
        float& a = popFloatVar();
        float  b = popFloatValue();
        a = b;
        break;
        }
      case EParTok_AssignInstance: {
        auto& sa = popVar();
        auto& sb = popVar();

        sa.instance = sb.instance;
        break;
        }
      case EParTok_Jump:
        PC = size_t(op.address);
        break;
      case EParTok_JumpIf:
        a = popDataValue();
        if(a==0)
          PC = size_t(op.address);
        break;

      case EParTok_SetInstance:
        setCurrentInstance(size_t(op.symbol));
        break;
      case EParTok_PushArrayVar:{
        pushVar(size_t(op.symbol), op.index);
        break;
        }
      }
    }
  }

const PARStackOpCode& DaedalusVM::nextInstruction(size_t& pc) {
  const PARStackOpCode& op = m_DATFile.getStackOpCode(pc);
  pc += op.opSize;
  return op;
  }

void DaedalusVM::registerExternalFunction(const char* symName, const std::function<void(DaedalusVM&)>& fn) {
  if(m_DATFile.hasSymbolName(symName)) {
    size_t s = m_DATFile.getSymbolIndexByName(symName);
    if(m_ExternalsByIndex.size()<=s)
      m_ExternalsByIndex.resize(s+1);
    m_ExternalsByIndex[s] = fn;
    }
  }

void DaedalusVM::registerInternalFunction(const char* symName, const std::function<void (DaedalusVM&)>& fn) {
  if(m_DATFile.hasSymbolName(symName)) {
    auto& s = m_DATFile.getSymbolByName(symName);
    registerInternalFunction(s.address,fn);
    }
  }

void DaedalusVM::registerInternalFunction(size_t address, const std::function<void (DaedalusVM&)>& fn) {
  m_InternalsByIndex[address] = fn;
  }

void DaedalusVM::unregisterExternalFunction(const char* symName) {
  if(m_DATFile.hasSymbolName(symName)) {
    size_t s = m_DATFile.getSymbolIndexByName(symName);
    m_ExternalsByIndex[s] = nullptr;
    }
  }

void DaedalusVM::registerUnsatisfiedLink(const std::function<void (DaedalusVM &)> &fn) {
  m_OnUnsatisfiedCall = fn;
  }

void DaedalusVM::pushInt(uint32_t value) {
  m_Stack.emplace_back(reinterpret_cast<int32_t&>(value));
  }

void DaedalusVM::pushInt(int32_t value) {
  m_Stack.emplace_back(value);
  }

template <typename T>
T DaedalusVM::popDataValue() {
  // Default behavior of the ZenGin is to pop a 0, if nothing is on the stack.
  if(m_Stack.size()<=m_StackGuard)
    return static_cast<T>(0);

  auto top = m_Stack.back();
  m_Stack.pop_back();
  if(top.tag==EParTok_PushVar){
    auto& sym = m_DATFile.getSymbolByIndex(size_t(top.i32));
    return sym.getValue<T>(top.id,top.inst);
    }
  return top.i32;
  }

void DaedalusVM::pushVar(size_t index, uint32_t arrIdx) {
  auto& sym = m_DATFile.getSymbolByIndex(index);(void)sym;
  auto ptr  = getCurrentInstanceDataPtr();
  m_Stack.emplace_back(ptr,(int32_t)index,arrIdx);
  }

uint32_t DaedalusVM::popVar(uint32_t& arrIdx) {
  if(m_Stack.empty()){
    arrIdx=0;
    return 0;
    }

  auto top = m_Stack.back();
  m_Stack.pop_back();
  arrIdx = top.id;
  return uint32_t(top.i32);
  }

void DaedalusVM::setReturn(int32_t v) {
  pushInt(v);
  }

void DaedalusVM::setReturn(const ZString &v) {
  pushString(v);
  }

void DaedalusVM::setReturn(float f) {
  m_Stack.emplace_back(f);
  }

void DaedalusVM::setReturnVar(int32_t v) {
  pushVar(size_t(v));
  }

uint32_t DaedalusVM::popUInt() {
  return uint32_t(popInt());
  }

int32_t DaedalusVM::popInt() {
  if(m_Stack.empty())
    return 0;

  auto top = m_Stack.back();
  m_Stack.pop_back();
  if(top.tag==EParTok_PushVar){
    auto& sym = m_DATFile.getSymbolByIndex(size_t(top.i32));
    return sym.getValue<int32_t>(top.id,top.inst);
    }
  return top.i32;
  }

float DaedalusVM::popFloat() {
  if(m_Stack.empty())
    return 0;

  auto top = m_Stack.back();
  m_Stack.pop_back();
  if(top.tag==EParTok_PushVar){
    auto& sym = m_DATFile.getSymbolByIndex(size_t(top.i32));
    return sym.getFloat(top.id,top.inst);
    }
  return top.f;
  }

ZString DaedalusVM::popString() {
  if(m_Stack.empty())
    return {};

  auto top = m_Stack.back();
  m_Stack.pop_back();
  if(top.tag==EParTok_PushVar){
    auto& sym = m_DATFile.getSymbolByIndex(size_t(top.i32));
    return sym.getString(top.id,top.inst);
    }

  return top.s;
  }

int32_t& DaedalusVM::popIntVar() {
  static int32_t err=0;
  if(m_Stack.empty())
    return err;

  auto top = m_Stack.back();
  m_Stack.pop_back();
  if(top.tag==EParTok_PushVar){
    auto& sym = m_DATFile.getSymbolByIndex(size_t(top.i32));
    return sym.getInt(top.id,top.inst);
    }
  return err;
  }

float& DaedalusVM::popFloatVar() {
  static float err=0;
  if(m_Stack.empty())
    return err;

  auto top = m_Stack.back();
  m_Stack.pop_back();
  if(top.tag==EParTok_PushVar){
    auto& sym = m_DATFile.getSymbolByIndex(size_t(top.i32));
    return sym.getFloat(top.id,top.inst);
    }
  return err;
  }

ZString& DaedalusVM::popStringVar() {
  static ZString err;
  if(m_Stack.empty())
    return err;

  auto top = m_Stack.back();
  m_Stack.pop_back();
  if(top.tag==EParTok_PushVar){
    auto& sym = m_DATFile.getSymbolByIndex(size_t(top.i32));
    return sym.getString(top.id,top.inst);
    }

  return err;
  }

float DaedalusVM::popFloatValue() {
  auto top = m_Stack.back();
  m_Stack.pop_back();
  return top.f;
  }

PARSymbol &DaedalusVM::popVar() {
  uint32_t arr = 0;
  uint32_t idx = popVar(arr);
  return m_DATFile.getSymbolByIndex(idx);
  }

void DaedalusVM::pushVar(const char* symName) {
  size_t idx = m_DATFile.getSymbolIndexByName(symName);
  pushVar(idx);
  }

void DaedalusVM::pushString(const ZString &value) {
  m_Stack.emplace_back(value);
  }

void DaedalusVM::setInstance(const char* instSymbol, GEngineClasses::Instance *h, EInstanceClass instanceClass) {
  PARSymbol& s = m_DATFile.getSymbolByName(instSymbol);
  s.instance.set(h,instanceClass);
  }

void DaedalusVM::setInstance(const size_t instSymbol, GEngineClasses::Instance *h, EInstanceClass instanceClass) {
  PARSymbol& s = m_DATFile.getSymbolByIndex(instSymbol);
  s.instance.set(h,instanceClass);
  }

void DaedalusVM::clearReferences(GEngineClasses::Instance &h) {
  if(h.useCount==0)
    return;

  if(m_Instance.get()==&h)
    m_Instance = nullptr;

  for(size_t i=0;i<m_DATFile.getSymTable().symbols.size();++i){
    auto& s = m_DATFile.getSymbolByIndex(i);
    if(s.instance.get()==&h){
      s.instance = nullptr;
      if(h.useCount==0)
        return;
      }
    }
  }

void DaedalusVM::clearReferences(EInstanceClass h) {
  if(m_Instance.instanceOf(h))
    m_Instance = nullptr;

  for(size_t i=0;i<m_DATFile.getSymTable().symbols.size();++i){
    auto& s = m_DATFile.getSymbolByIndex(i);
    if(s.instance.instanceOf(h)){
      s.instance = nullptr;
      }
    }
  }

void DaedalusVM::initializeInstance(GEngineClasses::Instance &instance, size_t symIdx, EInstanceClass classIdx) {
  PARSymbol& s = m_DATFile.getSymbolByIndex(symIdx);

  if(s.properties.elemProps.type!=(uint32_t)EParType_Instance)
    terminateScript<InvalidCall>();
  // Enter address into instance-symbol
  s.instance.set(&instance,classIdx);

  auto currentInstance = m_Instance;
  setCurrentInstance(symIdx);

  PARSymbol selfCpy;
  // Particle and Menu VM do not have a self symbol
  if(m_SelfId!=size_t(-1)) {
    selfCpy = globalSelf();  // Copy of "self"-symbol
    // Set self
    setInstance(m_SelfId, &instance, classIdx);
    }

  // Place the assigning symbol into the instance
  instance.instanceSymbol = symIdx;

  // Run script code to initialize the object
  runFunctionBySymIndex(symIdx,false,{});

  if(m_SelfId!=size_t(-1))
    globalSelf() = selfCpy;

  m_Instance = currentInstance;
  }

void DaedalusVM::setCurrentInstance(size_t symIdx) {
  auto& sym = m_DATFile.getSymbolByIndex(symIdx);
  m_InstanceSym = &sym;
  m_Instance    = sym.instance;
  }

GEngineClasses::Instance *DaedalusVM::getCurrentInstanceDataPtr() {
  return m_Instance.get();
  }

std::vector<std::string> DaedalusVM::getCallStack() {
  std::vector<std::string> symbolNames;
  auto frame=m_CallStack;
  while(frame) {
    symbolNames.push_back(nameFromFunctionInfo(std::make_pair(frame->address, CallStackFrame::Address)));
    frame = frame->calee;
    }
  return symbolNames;
  }

const std::string &DaedalusVM::currentCall() {
  if(m_CallStack)
    return nameFromFunctionInfo(std::make_pair(m_CallStack->address, CallStackFrame::Address));

  static std::string n = "<no function>";
  return n;
  }

PARSymbol &DaedalusVM::globalSelf() {
  return m_DATFile.getSymbolByIndex(m_SelfId);
  }

PARSymbol &DaedalusVM::globalOther() {
  return m_DATFile.getSymbolByIndex(m_OtherId);
  }

PARSymbol &DaedalusVM::globalVictim() {
  return m_DATFile.getSymbolByIndex(m_VictimId);
  }

PARSymbol &DaedalusVM::globalItem() {
  return m_DATFile.getSymbolByIndex(m_ItemId);
  }

void DaedalusVM::disAsm(size_t symIdx) {
  auto& functionSymbol = getDATFile().getSymbolByIndex(symIdx);
  auto address         = functionSymbol.address;
  if(address == 0)
    return;

  LogInfo() << "[" << functionSymbol.name <<"]";
  size_t PC = address;
  while(true) {
    const PARStackOpCode& op = nextInstruction(PC);
    switch (op.op) {
      case EParOp_Plus:
        LogInfo() << "EParOp_Plus";
        break;
      case EParOp_Minus:
        LogInfo() << "EParOp_Minus";
        break;
      case EParOp_Multiply:
        LogInfo() << "EParOp_Multiply";
        break;
      case EParOp_Divide:
        LogInfo() << "EParOp_Divide";
        break;
      case EParOp_Mod:
        LogInfo() << "EParOp_Mod";
        break;
      case EParOp_BinOr:
        LogInfo() << "EParOp_BinOr";
        break;
      case EParOp_BinAnd:
        LogInfo() << "EParOp_BinAnd";
        break;
      case EParOp_Less:
        LogInfo() << "EParOp_Less";
        break;
      case EParOp_Greater:
        LogInfo() << "EParOp_Greater";
        break;
      case EParOp_AssignInt:
        LogInfo() << "EParOp_AssignInt";
        break;
      case EParOp_LogOr:
        LogInfo() << "EParOp_LogOr";
        break;
      case EParOp_LogAnd:
        LogInfo() << "EParOp_LogAnd";
        break;
      case EParOp_ShiftLeft:
        LogInfo() << "EParOp_ShiftLeft";
        break;
      case EParOp_ShiftRight:
        LogInfo() << "EParOp_ShiftRight";
        break;
      case EParOp_LessOrEqual:
        LogInfo() << "EParOp_LessOrEqual";
        break;
      case EParOp_Equal:
        LogInfo() << "EParOp_Equal";
        break;
      case EParOp_NotEqual:
        LogInfo() << "EParOp_NotEqual";
        break;
      case EParOp_GreaterOrEqual:
        LogInfo() << "EParOp_GreaterOrEqual";
        break;
      case EParOp_AssignAdd:
        LogInfo() << "EParOp_AssignAdd";
        break;
      case EParOp_AssignSubtract:
        LogInfo() << "EParOp_AssignSubtract";
        break;
      case EParOp_AssignMultiply:
        LogInfo() << "EParOp_AssignMultiply";
        break;
      case EParOp_AssignDivide:
        LogInfo() << "EParOp_AssignDivide";
        break;
      case EParOp_Unary_Plus:
        LogInfo() << "EParOp_Unary_Plus";
        break;
      case EParOp_Unary_Minus:
        LogInfo() << "EParOp_Unary_Minus";
        break;
      case EParOp_Unary_Not:
        LogInfo() << "EParOp_Unary_Not";
        break;
      case EParOp_Unary_Negate:
        LogInfo() << "EParOp_Unary_Negate";
        break;
      // above are operations, below are tokens
      case EParTok_Ret:
        LogInfo() << "EParOp_Ret";
        return;
      case EParTok_Call:
        LogInfo() << "EParTok_Call " << op.address;
        break;
      case EParTok_CallExternal:
        LogInfo() << "EParTok_CallExternal " << op.symbol;
        break;
      case EParTok_PushInt:
        LogInfo() << "EParTok_PushInt " << op.value;
        break;
      case EParTok_PushVar:
        LogInfo() << "EParTok_PushVar " << op.symbol;
        break;
      case EParTok_PushInstance:
        LogInfo() << "EParTok_PushInstance " << op.symbol;
        break;
      case EParTok_AssignString:
        LogInfo() << "EParTok_AssignString";
        break;
      case EParTok_AssignStringRef:
        LogError() << "EParTok_AssignStringRef not implemented!";
        break;
      case EParTok_AssignFunc:
        LogInfo() << "EParOp_AssignFunc";
        break;
      case EParTok_AssignFloat:
        LogInfo() << "EParTok_AssignFloat";
        break;
      case EParTok_AssignInstance:
        LogInfo() << "EParTok_AssignInstance";
        break;
      case EParTok_Jump:
        LogInfo() << "EParTok_Jump " << op.address;
        break;
      case EParTok_JumpIf:
        LogInfo() << "EParTok_JumpIf " << op.address;
        break;
      case EParTok_SetInstance:
        LogInfo() << "EParTok_SetInstance " << op.symbol;
        break;
      case EParTok_PushArrayVar:
        LogInfo() << "EParTok_PushArrayVar " << op.symbol << " " << op.index;
        break;
      default:
        LogInfo() << "[bad instr]";
        return;
      }
    }
  }

int32_t DaedalusVM::runFunctionBySymIndex(size_t symIdx, bool initScript, std::function<void(size_t)> progressFunc) {
  if(symIdx==size_t(-1))
    return 0;

  bool hasRet=false;
  {
  CallStackFrame frame(*this, int32_t(symIdx), CallStackFrame::SymbolIndex);
  if(frame.address == 0) {
    return -1;
    }
  std::function<void(DaedalusVM&)>* f=nullptr;
  {
    auto it = m_InternalsByIndex.find(frame.address);
    if(it!=m_InternalsByIndex.end())
      f = &it->second;
  }
  if(initScript)
    m_NumFunctionCalls=0;

  // Execute the instructions
  if(f==nullptr) {
    eval(frame.address,initScript,progressFunc);
    } else {
    (*f)(*this);
    }
  hasRet = frame.hasReturnVal;
  }

  int32_t ret = hasRet ? popDataValue() : 0;
  return ret;
  }

const std::string& DaedalusVM::nameFromFunctionInfo(DaedalusVM::CallStackFrame::FunctionInfo functionInfo) {
  switch(functionInfo.second) {
    case CallStackFrame::SymbolIndex: {
      auto functionSymbolIndex = functionInfo.first;
      return getDATFile().getSymbolByIndex(functionSymbolIndex).name;
      }
    case CallStackFrame::Address: {
      auto address = functionInfo.first;
      auto functionSymbolIndex = getDATFile().getFunctionIndexByAddress(address);
      if (functionSymbolIndex != static_cast<size_t>(-1))
        return getDATFile().getSymbolByIndex(functionSymbolIndex).name;
        }
      }
  static std::string err;
  err = "unknown function with address: " + std::to_string(functionInfo.first);
  return err;
  }

template<class T>
[[noreturn]]
void DaedalusVM::terminateScript(){
  std::vector<std::string> stk;

  auto s = m_CallStack;
  while(s!=nullptr) {
    stk.emplace_back(s->nameHint);
    s = s->calee;
    }
  throw T(std::move(stk));
  }

DaedalusVM::CallStackFrame::CallStackFrame(DaedalusVM& vm, int32_t addressOrIndex, AddressType addrType)
    : calee(vm.m_CallStack), prevStackGuard(vm.m_StackGuard), vm(vm) {
  auto  symIdx         = addrType==SymbolIndex ? size_t(addressOrIndex)
                                               : vm.getDATFile().getFunctionIndexByAddress(size_t(addressOrIndex));
  auto& functionSymbol = vm.getDATFile().getSymbolByIndex(symIdx);
  address              = functionSymbol.address;
  if(address == 0)
    return;
  nameHint = functionSymbol.name.c_str();

  if(vm.m_Stack.size()<functionSymbol.properties.elemProps.count)
    vm.terminateScript<InconsistentState>();
  vm.m_StackGuard = vm.m_Stack.size() - functionSymbol.properties.elemProps.count;
  hasReturnVal    = functionSymbol.hasEParFlag(EParFlag_Return);

  // entering function
  vm.m_CallStack = this;
  }

DaedalusVM::CallStackFrame::~CallStackFrame() {
  // adjust stack
  size_t expSize = vm.m_StackGuard + (hasReturnVal ? 1 : 0);
  if(hasReturnVal && prevStackGuard==vm.m_Stack.size())
    vm.pushInt(0);

  if(vm.m_Stack.size()>expSize) {
    if(hasReturnVal) {
      Stk top = vm.m_Stack.back();
      while(vm.m_Stack.size()>=expSize)
        vm.m_Stack.pop_back();
      vm.m_Stack.push_back(top);
      } else {
      }
    while(vm.m_Stack.size()>expSize)
      vm.m_Stack.pop_back();
    }

  vm.m_CallStack  = calee;
  vm.m_StackGuard = prevStackGuard;
  }
