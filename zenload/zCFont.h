//
// Created by andre on 21.10.16.
//

#pragma once
#include <map>
#include "zTypes.h"

namespace VDFS
{
    class FileIndex;
}

namespace ZenLoad
{
    class ZenParser;
    class zCFont
    {
    public:
        /**
         * Max number of letters that a font can have (file format limitation)
         */
        static const int FONT_NUM_MAX_LETTERS = 256;

        /**
         * Everything found in the FNT-file
         */
        struct FontInfo
        {
            std::string   fontName;                            // Name of the font-file?
            uint32_t      fontHeight=0;                        // Height of the glyphs
            uint8_t       glyphWidth[FONT_NUM_MAX_LETTERS]={}; // Widths of the single glyphs
            ZMath::float2 fontUV1[FONT_NUM_MAX_LETTERS]={};    // Top-left corner
            ZMath::float2 fontUV2[FONT_NUM_MAX_LETTERS]={};    // Bottom-right corner
        };

        zCFont(const char* fileName, const VDFS::FileIndex& fileIndex);
        zCFont(zCFont&)=delete;
        zCFont(zCFont&&)=delete;
        virtual ~zCFont()=default;
        zCFont& operator=(zCFont&)=delete;
        zCFont& operator=(zCFont&&)=delete;

        /**
         * @return Whether this font was correctly loaded
         */
        bool isValid() const { return m_Info.fontHeight != 0; }

        /**
         * @return Information loaded from the font-file
         */
        const FontInfo& getFontInfo() const { return m_Info; }

    private:
        /**
         * Parses a .FNT-File given as a binary blob
         * @param fntData .FNT-File given as a binary blob
         * @return Success
         */
        bool parseFNTData(const std::vector<uint8_t>& fntData);

        // Everything found in the FNT-file
        FontInfo m_Info;
    };
}  // namespace ZenLoad
