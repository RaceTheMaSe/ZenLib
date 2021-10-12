#include "fileIndex.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <regex>
#include <cassert>
#include <physfs.h>
#include "../lib/physfs/extras/ignorecase.h"
#include "utils/logger.h"

using namespace VDFS;

namespace internal
{
    // Must be initialized with the 0th argument passed to the executable for PhysFS.
    static std::string argv0;

    // We need to do some poor-mans-refcounting to be able to know when we
    // need to init and deinit physfs.
    static size_t numAliveIndices = 0;
}  // namespace internal

FileIndex::FileIndex()
{
#ifndef __ANDROID__ // init should have happened before and is not done with an internal::argv0 string object but the PHYSFS_AndroidInit struct
    if (internal::argv0.empty())
    {
        auto const error = "VDFS not intialized! Please call 'initVDFS' before using!";
        LogError() << error;
        throw std::runtime_error(error);
    }
#endif

    if (!PHYSFS_isInit())
        if(!PHYSFS_init(internal::argv0.c_str()))
        {
          const PHYSFS_ErrorCode err = PHYSFS_getLastErrorCode();
          const char* errstr = (err!=0) ? PHYSFS_getErrorByCode(err) : nullptr;

          char error[256]={};
          std::snprintf((char*)error,sizeof(error),"Failed to initialize PHYSFS!(%s)",errstr ? errstr : "");

          LogError() << error;
          throw std::runtime_error((char*)error);
        }

    internal::numAliveIndices++;

#ifndef __ANDROID__
    assert(!internal::argv0.empty());
#endif
}

FileIndex::~FileIndex()
{
    assert(internal::numAliveIndices != 0);
    internal::numAliveIndices--;

    if (internal::numAliveIndices == 0 && PHYSFS_isInit())
        PHYSFS_deinit();
}

void FileIndex::initVDFS(const char* argv0)
{
    assert(internal::argv0.empty());

    internal::argv0 = argv0;
#ifdef __ANDROID__
    PHYSFS_init(argv0);
#endif
}

bool FileIndex::loadVDF(const std::u16string &vdf, const std::string &mountPoint)
{
     std::string buf(vdf.size()*4,'\0');
     PHYSFS_utf8FromUtf16(reinterpret_cast<const PHYSFS_uint16*>(vdf.c_str()),&buf[0],buf.size());
     return loadVDF(buf,mountPoint);
}

/**
* @brief Loads a VDF-File and initializes everything
*/
bool FileIndex::loadVDF(const std::string& vdf, const std::string& mountPoint)
{
    if (!PHYSFS_mount(vdf.c_str(), mountPoint.c_str(), 1))
    {
        LogInfo() << "Couldn't load VDF-Archive " << vdf << ": " << PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
        return false;
    }

    return true;
}

bool FileIndex::mountFolder(const std::string& path, const std::string& mountPoint)
{
    if (!PHYSFS_mount(path.c_str(), mountPoint.c_str(), 1))
    {
        LogInfo() << "Couldn't mount directory " << path << ": " << PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
        return false;
    }

    return true;
}

/**
* @brief Fills a vector with the data of the given file
*/
bool FileIndex::getFileData(const std::string& file, std::vector<uint8_t>& data) const
{
    return getFileData(file.c_str(),data);
}

/**
* @brief Fills a vector with the data of the given file
*/
bool FileIndex::getFileData(const char* file, std::vector<uint8_t>& data) const
{
    std::string upperedStr;
    char        upperedC[64] = {};
    char*       uppered      = (char*)upperedC;

    if(std::strlen(file)<64){
      std::strcpy((char*)upperedC,file); // cheap upper-case
      } else {
      upperedStr = file;
      uppered    = &upperedStr[0];
      }

    for(size_t i=0;uppered[i];++i) {
      auto c = uppered[i];
      if('a'<=c && c<='z')
        uppered[i] = char(c+'A'-'a');
      }

    PHYSFS_File* handle = PHYSFS_openRead(uppered);
    if(handle==nullptr)
        return false;

    auto length = PHYSFS_fileLength(handle);
    data.resize(length);
    if (PHYSFS_readBytes(handle, data.data(), length) < length)
    {
        LogInfo() << "Cannot read file " << file << ": " << PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
        PHYSFS_close(handle);
        return false;
    }
    PHYSFS_close(handle);
    return true;
}

bool FileIndex::getFileDataSameCase(const char* file, std::vector<uint8_t>& data) const {
    PHYSFS_File* handle = PHYSFS_openRead(file);
    if(handle==nullptr)
        return false;

    auto length = PHYSFS_fileLength(handle);
    data.resize(length);
    if (PHYSFS_readBytes(handle, data.data(), length) < length)
    {
        LogInfo() << "Cannot read file " << file << ": " << PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
        PHYSFS_close(handle);
        return false;
    }
    PHYSFS_close(handle);
    return true;
}

bool FileIndex::hasFile(const std::string& file) const
{
    std::string upperedStr;
    char        upperedC[64] = {};
    char*       uppered      = (char*)upperedC;

    if(file.size()<64){
      std::strcpy((char*)upperedC,file.c_str()); // cheap upper-case
      } else {
      upperedStr = file;
      uppered    = &upperedStr[0];
      }

    for(size_t i=0;uppered[i];++i) {
      auto c = uppered[i];
      if('a'<=c && c<='z')
        uppered[i] = char(c+'A'-'a');
      }
    PHYSFS_Stat st={};
    return PHYSFS_stat(uppered,&st)!=0;
}

bool FileIndex::hasFileCaseSensitive(const std::string& file) const
{
    std::string upperedStr;
    char        upperedC[64] = {};
    char*       uppered      = (char*)upperedC;

    if(file.size()<64){
      std::strcpy((char*)upperedC,file.c_str()); // cheap upper-case
      } else {
      upperedStr = file;
      uppered    = &upperedStr[0];
      }

    PHYSFS_Stat st={};
    return PHYSFS_stat(uppered,&st)!=0;
}

int64_t FileIndex::getLastModTime(const std::u16string &vdf)
{
    std::string buf(vdf.size()*4,'\0');
    PHYSFS_utf8FromUtf16(reinterpret_cast<const PHYSFS_uint16*>(vdf.c_str()),&buf[0],buf.size());
    return getLastModTime(buf);
}

int64_t VDFS::FileIndex::getLastModTime(const std::string& name)
{
    int64_t result = -1;
    std::ifstream infile(name);
    if (infile.good())
    {
        std::string firstLine;
        std::getline(infile, firstLine);
        struct std::tm tm{};
        std::smatch match;

        // Regex of the datetime used by G1 and G2 e.g. (Thu, 19 Dec 2002 19:24:42 GMT)
        std::regex rgxG1G2("\\(.*, ([[:digit:]]*) (.*) ([[:digit:]]*) ([[:digit:]]*):([[:digit:]]*):([[:digit:]]*) GMT\\)");

        // Regex of the datetime used by G1 e.g. 19.06.2001  18:58.06
        std::regex rgxG1("([[:digit:]]*)\\.([[:digit:]]*)\\.([[:digit:]]*)  ([[:digit:]]*):([[:digit:]]*).([[:digit:]]*)");

        if (std::regex_search(firstLine, match, rgxG1G2))
        {
            std::istringstream datetime(match[1].str() + "-" + match[2].str() + "-" + match[3].str() + " " + match[4].str() + ":" + match[5].str() + ":" + match[6].str());

            datetime >> std::get_time(&tm, "%d-%b-%Y %H:%M:%S");
            if (!datetime.fail())
                result = std::mktime(&tm);
        }
        else if (std::regex_search(firstLine, match, rgxG1))
        {
            std::istringstream datetime(match[1].str() + "-" + match[2].str() + "-" + match[3].str() + " " + match[4].str() + ":" + match[5].str() + ":" + match[6].str());

            datetime >> std::get_time(&tm, "%d-%m-%Y %H:%M:%S");
            if (!datetime.fail())
                result = std::mktime(&tm);
        }

        infile.close();
    }

    return result;
}

std::vector<std::string> FileIndex::getKnownFiles(const std::string& path) const
{
    std::string filePath(path);
    std::vector<std::string> vec;
    bool exists = PHYSFSEXT_locateCorrectCase(&filePath[0]) == 0;
    if (!exists)
    {
        return vec;
    }

    char** files = PHYSFS_enumerateFiles(filePath.c_str());
    char** i = nullptr;
    for (i = files; *i != nullptr; i++)
        vec.emplace_back(*i);
    PHYSFS_freeList(files);
    return vec;
}

void FileIndex::finalizeLoad()
{
}
