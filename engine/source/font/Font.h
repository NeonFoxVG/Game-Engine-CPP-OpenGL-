#pragma once

#include "graphics/Texture.h"

namespace eng
{
    struct GlyphDescription
    {
        int x0, y0;
        int x1, y1;

        int width;
        int height;

        int advance;
        int xOffset = 0;
        int yOffset = 0;
    };

    class Font
    {
    public:
        int GetSize() const;
        const GlyphDescription& GetGlyphDescription(char asciiCode) const;
        const std::shared_ptr<Texture>& GetTexture() const;

    private:
        int m_size = 0;
        GlyphDescription m_descriptions[128];
        std::shared_ptr<Texture> m_texture;

        friend class FontManager;
    };
}