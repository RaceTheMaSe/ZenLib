#pragma once
#include <map>
#include "zTypes.h"
#include "zenParser.h"

namespace VDFS
{
    class FileIndex;
}

namespace Daedalus
{
     class ZString;
}

namespace ZenLoad
{
    class zCCSLib
    {
    public:
        /**
     * @brief Loads the file from the given VDF-Archive
     */
        zCCSLib() = default;
        zCCSLib(const std::string& fileName, const VDFS::FileIndex& fileIndex, ZenLoad::ZenParser::FileVersion version);
        zCCSLib(ZenParser &parser,ZenLoad::ZenParser::FileVersion version);

        /**
        * Reads this object from an internal zen
        */
        void readObjectData(ZenParser& parser, ZenLoad::ZenParser::FileVersion version);

        /**
         * @return the message of the given name
         */
        const oCMsgConversation& getMessageByName(const Daedalus::ZString& name);

        /**
         * @return true if the message was found
         */
        bool messageExists(const Daedalus::ZString& name) const;

        const zCCSLibData& getData()     const { return m_Data; }

        const zCCutscene&  getCutscene() const { return cutsceneData; }

        /**
         * Message indices by their names
         */
        static std::map<std::string, size_t> m_MessagesByName;
        
        static void addMessageByName(const std::string& name, oCMsgConversation&& msg);

    private:
        /**
         * Loaded data straight from the file
         */
        zCCSLibData m_Data;

        zCCutscene cutsceneData;

        static std::vector<oCMsgConversation> conversations;
    };
}  // namespace ZenLoad
