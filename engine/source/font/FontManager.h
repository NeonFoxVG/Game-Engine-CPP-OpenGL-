#pragma once

#include "font/Font.h"

#include <memory>
#include <unordered_map>

typedef struct FT_LibraryRec_* FT_Library;

namespace eng
{
    class FontManager
    {
    public:
        ~FontManager();
        void Init();
        std::shared_ptr<Font> GetFont(const std::string& path, int size);

    private:
        FT_Library m_fontLibrary = nullptr;

        using FontFamily = std::unordered_map<int, std::shared_ptr<Font>>;
        std::unordered_map<std::string, FontFamily> m_fonts;
    };
}