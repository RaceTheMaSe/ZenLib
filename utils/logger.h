#pragma once
#include <functional>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <cstdio>

#define USE_LOG

/** Replace some HRESULT-Functionality used by this */
#if !(defined(WIN32) || defined(_WIN32))
#define FAILED(x) (x != 0)
#else
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
// HACK: Work around windows.h messing this up with its define
#ifdef DialogBox
#undef DialogBox
#endif
#endif

#if defined(__ANDROID__)
#include <android/log.h>
#endif

#ifdef _MSC_VER
#define FUNCTION_SIGNATURE __FUNCSIG__
#elif defined(__GNUC__)
#define FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#endif

/** Log if a boolen expression returned false */
#define LEB(x)                   \
{                                \
  if (!x)                        \
  {                              \
    LogError() << #x " failed!"; \
  }                              \
}

/** Log if a boolen expression returned false and return false in calling function */
#define LEB_R(x)                 \
{                                \
  if (!x)                        \
  {                              \
    LogError() << #x " failed!"; \
    return false;                \
  }                              \
}

/** Logging macros */
#define LogInfo()  ZenLib::Log("Info", __FILE__, __LINE__, FUNCTION_SIGNATURE,    false, ZenLib::Log::EMessageType::MT_Info)
#define LogWarn()  ZenLib::Log("Warning", __FILE__, __LINE__, FUNCTION_SIGNATURE,  true, ZenLib::Log::EMessageType::MT_Warning)
#define LogError() ZenLib::Log("Error", __FILE__, __LINE__, FUNCTION_SIGNATURE,    true, ZenLib::Log::EMessageType::MT_Error)

namespace ZenLib
{
#ifdef USE_LOG
    class Log
    {
    public:
      enum EMessageType
      {
        MT_Info,
        MT_Warning,
        MT_Error
      };

      /** Append starting information here and wait for more messages using the <<-operator */
      Log(const char* type, const char* file, int line, const char* function, bool includeInfo = false, EMessageType typeID = MT_Info)
      {
        m_TypeID = typeID;
        if(includeInfo)
          (*this) << file << "(" << line << "):"
                          << "[" << function << "] ";  // {0}({1}): <message here>
        else
          (*this) << type << ": ";

      }
      Log(Log&)=delete;
      Log(Log&&)=delete;
      Log& operator=(Log&)=delete;
      Log& operator=(Log&&)=delete;

      /** Flush the log message to wherever we need to */
      ~Log()
      {
          Flush();
      }

      /** STL stringstream feature */
      template <typename T>
      inline Log& operator<<(const T& obj)
      {
          m_Message << obj;
          return *this;
      }

      template <typename T>
      inline Log& operator<<(const std::list<T>& obj)
      {
          if (obj.empty())
          {
              m_Message << "[]";
              return *this;
          }

          m_Message << "[\n";

          for (const T& s : obj)
          {
              m_Message << " - " << s << '\n';
          }

          m_Message << ']';
          return *this;
      }

      template <typename T>
      inline Log& operator<<(const std::vector<T>& obj)
      {
        if (obj.empty()) {
          m_Message << "[]";
          return *this;
          }

        m_Message << "[\n";

        for (const T& s : obj)
          m_Message << " - " << s << '\n';

        m_Message << ']';
        return *this;
      }

      inline Log& operator<<(std::wostream& (*fn)(std::wostream&))
      {
        m_Message << fn;
        return *this;
      }

      /** Sets the function to be called when a log should be printed */
      static void SetLogCallback(std::function<void(EMessageType type, const char* line)> fn)
      {
        s_LogCallback = std::move(fn);
      }

      /** Sets the function to be called when a log should be printed */
      static void SetLogCallback(const std::function<void(const std::string&)>& fn)
      {
        s_LogCallback = [fn](EMessageType, const char* line) { fn(line); };
      }

    private:

      /** Called when the object is getting destroyed, which happens immediately if simply calling the constructor of this class */
      inline void Flush()
      {
        // Do callback
        if(s_LogCallback)
          s_LogCallback(m_TypeID,m_Message.str().c_str()); else
          defaultOuput (m_TypeID,m_Message.str().c_str());
      }

      inline void defaultOuput(EMessageType type, const char* line)
      {
#if defined(WIN32) || defined(_WIN32)
        (void)type;
        OutputDebugStringA(line);
        OutputDebugStringA("\n");
#elif defined(__ANDROID__)
      switch(type)
      {
        case MT_Info:
            __android_log_print(ANDROID_LOG_INFO, "ZenLib", "%s", line);
          break;

        case MT_Warning:
            __android_log_print(ANDROID_LOG_WARN, "ZenLib", "%s", line);
          break;

        case MT_Error:
            __android_log_print(ANDROID_LOG_ERROR, "ZenLib", "%s", line);
          break;
        }
#else
        switch(type)
        {
          case MT_Info:
            std::cout << line << std::endl;
            break;

          case MT_Warning:
            //   std::cerr << line << std::endl;
            //   break;

          case MT_Error:
            std::cerr << line << std::endl;
            break;
        }
#endif
      }

      static std::function<void(EMessageType type, const char* line)> s_LogCallback;

      EMessageType      m_TypeID;   // Type of log
      std::stringstream m_Message;  // Text to write into the logfile
    };
#else

    class Log
    {
    public:
      Log(const char* Type, const char* File, int Line, const char* Function, bool bIncludeInfo = false, UINT MessageBox = 0) {}

      ~Log() {}

      /** Clears the logfile */
      static void Clear() {}

      /** STL stringstream feature */
      template <typename T>
      inline Log& operator<<(const T& obj)
      {
        return *this;
      }

      inline Log& operator<<(std::wostream& (*fn)(std::wostream&))
      {
        return *this;
      }

      /** Called when the object is getting destroyed, which happens immediately if simply calling the constructor of this class */
      inline void Flush() {}

      /** Sets the function to be called when a log should be printed */
      static void SetLogCallback(std::function<void(const std::string&)> fn) {}

    private:
    };

#endif

}  // namespace Utils
