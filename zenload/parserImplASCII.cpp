#include "parserImplASCII.h"

#include <algorithm>
#include <cctype>
#include "utils/logger.h"

using namespace ZenLoad;

ParserImplASCII::ParserImplASCII(ZenParser* parser)
    : ParserImpl(parser)
{
}

/**
 * @brief Read the start of a chunk. [...]
 */
bool ParserImplASCII::readChunkStart(ZenParser::ChunkHeader& header)
{
    size_t seek = m_pParser->getSeek();
    // Check chunk-header
    m_pParser->skipSpaces();

    // Early exit if this is a chunk-end or not a chunk header
    if(m_pParser->m_Data[m_pParser->m_Seek] != '[' || m_pParser->m_Data[m_pParser->m_Seek + 1] == ']')
    {
      m_pParser->setSeek(seek);
      return false;
    }

    size_t tmpSeek = m_pParser->m_Seek;
    while (m_pParser->m_Data[tmpSeek] != ']')
    {
        if (m_pParser->m_Data[tmpSeek] == '\r' || m_pParser->m_Data[tmpSeek] == '\n')
            throw std::runtime_error("Invalid vob descriptor");

        ++tmpSeek;
    }

    // Parse chunk-header
    char        vobDescStk[256] = {};
    std::string vobDescHeap;
    const char* vobDesc = (const char*)vobDescStk;

    if(!readString((char*)vobDescStk,sizeof(vobDescStk))) {
      vobDescHeap = readString();
      vobDesc     = vobDescHeap.c_str();
      }

    if(parseHeader(header,vobDesc,std::strlen(vobDesc))) {
      // Skip the last newline
      m_pParser->skipSpaces();
      return true;
      }
    m_pParser->setSeek(seek);
    return false;
}

/**
 * @brief Read the end of a chunk. []
 */
bool ParserImplASCII::readChunkEnd()
{
    size_t seek = m_pParser->getSeek();

    m_pParser->skipSpaces();
    char l[3] = {};
    if(!readString((char*)l,3) || l[0]!='[' || l[1]!=']') {
      m_pParser->setSeek(seek);  // Next property isn't a string or the end
      return false;
      }

    m_pParser->skipSpaces();
    return true;
}

/**
 * @brief Read the implementation specific header and stores it in the parsers main-header struct
 */
void ParserImplASCII::readImplHeader()
{
    if (!m_pParser->skipString("objects"))
        throw std::runtime_error("Object count missing");

    m_pParser->m_Header.objectCount = m_pParser->readIntASCII();

    if (!m_pParser->skipString("END"))
        throw std::runtime_error("No END in header(2)");

    m_pParser->skipSpaces();
}

/**
 * @brief Reads a string
 */
std::string ParserImplASCII::readString()
{
  return m_pParser->readLine();
}

bool ParserImplASCII::readString(char* buf, size_t size)
{
  return m_pParser->readLine(buf,size);
}

/**
 * @brief Reads data of the expected type. Throws if the read type is not the same as specified and not 0
 */
void ParserImplASCII::readEntryImpl(const char* expectedName, void* target, size_t targetSize, EZenValueType expectedType, bool optional)
{
    m_pParser->skipSpaces();
    std::string lineHeap;
    char        lineStk[256] = {};
    char*       line = (char*)lineStk;
    size_t seek = m_pParser->getSeek();
    if(!readString((char*)lineStk,sizeof(lineStk)))
    {
      lineHeap = m_pParser->readLine();
      line     = lineHeap.size()>0 ? &lineHeap[0] : (char*)lineStk;
    }

    size_t lnSize = std::strlen(line);
    if(lnSize>=1) {
      if(line[0]=='[') {
        if(optional) {
          m_pParser->setSeek(seek); // reset seek
          return;
          }
        else
          throw std::runtime_error(std::string("Chunk end instead of: ") + expectedName);
        }
      }

    /* Examples:
     * childs0=int:1
     *
     * Syntax:
     * <name>=<type>:<value>
     * */
    const char* name  = line;
    const char* type  = "";
    const char* value = "";

    size_t i=0;
    for(; i<lnSize; ++i) {
      if(line[i]=='=') {
        line[i] = '\0';
        if(i+1<lnSize)
          type = line+i+1;
        break;
        }
      }

    for(; i<lnSize; ++i) {
      if(line[i]==':') {
        line[i] = '\0';
        if(i+1<lnSize)
          value = line+i+1;
        break;
        }
      }

    if(expectedName!=nullptr && expectedName[0]!='\0') {
      for(size_t i=0; ; ++i) {
        char e = expectedName[i];
        char n = name[i];
        if('A'<=e && e<='Z')
          e = char(e-'A'+'a');
        if('A'<=n && n<='Z')
          n = char(n-'A'+'a');
        if(e!=n) {
          if(optional) {
            m_pParser->setSeek(seek);
            return;
            }
          else
            throw std::runtime_error(std::string("Value name does not match expected name. Value:") + name + " Expected: " + expectedName);
          }
        if(expectedName[i]=='\0')
          break;
        }
      }

    auto realType = parseType(type);
    if(realType==ZVT_ENUM) {
      switch(targetSize) {
        case 1:
          realType = ZVT_BYTE;
          break;
        case 2:
          realType = ZVT_WORD;
          break;
        case 4:
          realType = ZVT_INT;
          break;
        }
      }

    if(realType==ZVT_RAW_FLOAT && expectedType==ZVT_RAW)
      expectedType = ZVT_RAW_FLOAT;
    else if(realType==ZVT_INT && expectedType==ZVT_BYTE)
      realType = ZVT_BYTE;
    else if(realType==ZVT_INT && expectedType==ZVT_WORD)
      realType = ZVT_WORD;

    if(expectedType!=realType && realType!=ZVT_0)
      throw std::runtime_error(std::string("Valuetype name does not match expected type. Value:") + expectedName);

    switch(realType) {
      case ZVT_0:
        break;
      case ZVT_STRING:
        if(optional && strcmp(value,"[]")==0) { // read optional entry at end of block and revert if this is chunkEnd
          m_pParser->setSeek(seek);
          value = nullptr;
          }
        *reinterpret_cast<std::string*>(target) = value;
        break;
      case ZVT_INT:
        *reinterpret_cast<int32_t*>(target) = std::stoi(value);
        break;
      case ZVT_FLOAT:
        *reinterpret_cast<float*>(target) = std::stof(value);
        break;
      case ZVT_BYTE:
        *reinterpret_cast<uint8_t*>(target) = static_cast<uint8_t>(std::stoi(value));
        break;
      case ZVT_WORD:
        *reinterpret_cast<int16_t*>(target) = static_cast<int16_t>(std::stoi(value));
        break;
      case ZVT_BOOL:
        *reinterpret_cast<bool*>(target) = std::stoi(value) != 0;
        break;
      case ZVT_VEC3:
        *reinterpret_cast<ZMath::float3*>(target) = parseVec3(value);
        break;
      case ZVT_COLOR:
        parseColor(value,reinterpret_cast<uint8_t*>(target),targetSize);
        break;
      case ZVT_RAW_FLOAT:
        parseFloatVec(value,reinterpret_cast<float*>(target),targetSize);
        break;
      case ZVT_RAW:
        parseRawVec(value,reinterpret_cast<uint8_t*>(target),targetSize);
        break;
      case ZVT_10:
      case ZVT_11:
      case ZVT_12:
      case ZVT_13:
      case ZVT_14:
      case ZVT_15:
        break;
      case ZVT_ENUM:
        *reinterpret_cast<uint8_t*>(target) = std::stoi(value);
        break;
      case ZVT_HASH:
        break;
      }
}

/**
* @brief Reads the type of a single entry
*/
void ParserImplASCII::readEntryType(EZenValueType& outtype, size_t& size)
{
  m_pParser->skipSpaces();
  std::string lineHeap;
  char        lineStk[256] = {};
  char*       line = (char*)lineStk;
  if(!readString((char*)lineStk,sizeof(lineStk)))
  {
    lineHeap = m_pParser->readLine();
    line     = lineHeap.size()>0 ? &lineHeap[0] : (char*)lineStk;
  }

  size_t lnSize = std::strlen(line);
  if(lnSize>=1) {
    // FIXME?
    if(line[0]=='[')
      return;
    }

  /* Examples:
   * childs0=int:1
   *
   * Syntax:
   * <name>=<type>:<value>
   * */
  const char* name  = line;
  const char* type  = "";

  size_t i=0;
  for(; i<lnSize; ++i) {
    if(line[i]=='=') {
      line[i] = '\0';
      if(i+1<lnSize)
        type = line+i+1;
      break;
      }
    }

  for(; i<lnSize; ++i) {
    if(line[i]==':') {
      line[i] = '\0';
      break;
      }
    }

  (void)name;
  outtype = parseType(type);
  size    = 0;
  }

ParserImpl::EZenValueType ParserImplASCII::parseType(const char* type) const
{
  if(std::strcmp(type,"int")==0)
    return ZVT_INT;
  if(std::strcmp(type,"float")==0)
    return ZVT_FLOAT;
  if(std::strcmp(type,"string")==0)
    return ZVT_STRING;
  if(std::strcmp(type,"byte")==0)
    return ZVT_BYTE;
  if(std::strcmp(type,"word")==0)
    return ZVT_WORD;
  if(std::strcmp(type,"bool")==0)
    return ZVT_BOOL;
  if(std::strcmp(type,"vec3")==0)
    return ZVT_VEC3;
  if(std::strcmp(type,"color")==0)
    return ZVT_COLOR;
  if(std::strcmp(type,"rawFloat")==0)
    return ZVT_RAW_FLOAT;
  if(std::strcmp(type,"raw")==0)
    return ZVT_RAW;
  if(std::strcmp(type,"enum")==0)
    return ZVT_ENUM;
  if(std::memcmp(type,"enum",4)==0) // version 0 enums have all types appended with semicolons
    return ZVT_ENUM;
  throw std::runtime_error("Unknown type");
}

ZMath::float3 ParserImplASCII::parseVec3(const char* line) const
{
  float ret[3] = {};
  parseFloatVec(line,(float*)ret,sizeof(ret));
  return {ret[0],ret[1],ret[2]};
}

void ParserImplASCII::parseFloatVec(const char* line, float* target, size_t targetSize) const
{
  targetSize /= sizeof(float);
  for(size_t i=0; i<targetSize; ++i) {
    if(*line=='\0')
      break;
    while(*line==' ' || *line=='\t')
      ++line;
    size_t pos = 0;
    target[i] = std::stof(line,&pos);
    line+=pos;
    }
}

void ParserImplASCII::parseColor(const char* line, uint8_t* target, size_t targetSize) const
{
  for(size_t i=0; i<targetSize; ++i) {
    if(*line=='\0')
      break;
    while(*line==' ' || *line=='\t')
      ++line;
    size_t pos = 0;
    target[i] = std::stoi(line,&pos);
    line+=pos;
    }
}

void ParserImplASCII::parseRawVec(const char* line, uint8_t* target, size_t targetSize) const
{
  for(size_t i=0; i<targetSize; ++i)
  {
    if(line[0]=='\0' || line[1]=='\0')
      break;
    int c[2] = {};
    for(int r=0; r<2; ++r) {
      if('0'<=line[r] && line[r]<='9')
        c[r] = line[r]-'0';
      else if(('a'<=line[r] && line[r]<='f') ||
              ('A'<=line[r] && line[r]<='F'))
        c[r] = line[r]-'a'+10;
      }
    line+=2;
    target[i] = c[0]*16+c[1];
  }
}

