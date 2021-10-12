#pragma once

#include <zenload/zCModelAni.h>

namespace ZenLoad
{
    class zCModelAni;
    class ZenParser;

    /** Streaming parser for .MAN files.
 */
    class ManParser
    {
    public:
        using Samples = std::vector<zCModelAniSample>;
        using NodeIndex = std::vector<uint32_t>;

        enum EChunkType
        {
            CHUNK_ERROR,
            CHUNK_EOF,

            CHUNK_HEADER = 0xA020,
            CHUNK_RAWDATA = 0xA090,

        };

        ManParser(ZenParser& zen);

     /** Returns the parsed header.
     *
     * Call this if parse() returns CHUNK_HEADER.
     *
     * @return The header read during the last call to parse().
     */
        const zCModelAni::ModelAniHeader& header() const { return m_Header; }

     /** Returns the parsed samples.
     *
     * Call this if parse() returns CHUNK_RAWDATA.
     *
     * @return The samples read during the last call to parse().
     */
        const Samples& samples() const { return m_Samples; }

     /** Returns the parsed nodes.
     *
     * Call this if parse() returns CHUNK_RAWDATA.
     *
     * @return The nodes read during the last call to parse().
     */
        const NodeIndex& nodeIndex() const { return m_NodeIndex; }

        EChunkType parse();

    private:
        ZenParser& m_Zen;

        zCModelAni::ModelAniHeader m_Header = zCModelAni::ModelAniHeader();
        Samples m_Samples;
        NodeIndex m_NodeIndex;

        void readHeader();
        void readRawData();
    };

}  // namespace ZenLoad
