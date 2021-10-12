#pragma once

#include "parserImpl.h"
#include "zenParser.h"

namespace ZenLoad
{
    /**
      * @brief Templated function-calls to read the right type of data
      *      and convert it to a string for the given type
      */
    template <typename T>
    inline void read(ZenParser& /*p*/, T& /*outData*/, const char* /*exName*/);

    template <>
    inline void read<float>(ZenParser& p, float& outData, const char* exName)
    {
        p.getImpl()->readEntry(exName, outData);
    }

    template <>
    inline void read<bool>(ZenParser& p, bool& outData, const char* exName)
    {
      p.getImpl()->readEntry(exName, outData);
    }

    template <>
    inline void read<uint32_t>(ZenParser& p, uint32_t& outData, const char* exName)
    {
      p.getImpl()->readEntry(exName, outData);
    }

    template <>
    inline void read<int32_t>(ZenParser& p, int32_t& outData, const char* exName)
    {
      p.getImpl()->readEntry(exName, outData);
    }

    template <>
    inline void read<uint16_t>(ZenParser& p, uint16_t& outData, const char* exName)
    {
        p.getImpl()->readEntry(exName, outData);
    }

    template <>
    inline void read<uint8_t>(ZenParser& p, uint8_t& outData, const char* exName)
    {
      p.getImpl()->readEntry(exName, outData);
    }

    template <>
    inline void read<ZMath::float2>(ZenParser& p, ZMath::float2& outData, const char* exName)
    {
        p.getImpl()->readEntry(exName, outData);
    }

    template <>
    inline void read<ZMath::float3>(ZenParser& p, ZMath::float3& outData, const char* exName)
    {
        p.getImpl()->readEntry(exName, outData);
    }

    template <>
    inline void read<ZMath::float4>(ZenParser& p, ZMath::float4& outData, const char* exName)
    {
        p.getImpl()->readEntry(exName, outData);
    }

    template <>
    inline void read<ZMath::Matrix>(ZenParser& p, ZMath::Matrix& outData, const char* exName)
    {
        p.getImpl()->readEntry(exName, outData);
    }

    template <>
    inline void read<std::string>(ZenParser& p, std::string& outData, const char* exName)
    {
      p.getImpl()->readEntry(exName, outData);
    }

    template <>
    inline void read<Daedalus::ZString>(ZenParser& p, Daedalus::ZString& outData, const char* exName)
    {
      p.getImpl()->readEntry(exName, outData);
    }

    template <typename... T>
    static void ReadObjectProperties(ZenParser& ZenParser, std::pair<const char*, T*>... d)
    {
        auto fn = [&ZenParser](auto pair) {
            // Read the given datatype from the file
            read<typename std::remove_pointer<decltype(pair.second)>::type>(ZenParser, *pair.second, pair.first);
        };

        auto x = {(fn(d), 0)...};
        (void)x;
    }

    template <typename S>
    static std::pair<const char*, S*> Prop(const char* t, S& s)
    {
        return std::make_pair(t, &s);
    }
}  // namespace ZenLoad
