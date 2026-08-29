#include "font/Font.h"

namespace eng
{
    int Font::GetSize() const
    {
        return m_size;
    }

    const GlyphDescription& Font::GetGlyphDescription(char asciiCode) const
    {
        return m_descriptions[static_cast<unsigned char>(asciiCode)];
    }

    const std::shared_ptr<Texture>& Font::GetTexture() const
    {
        return m_texture;
    }
}