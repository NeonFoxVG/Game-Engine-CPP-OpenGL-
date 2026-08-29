#include "scene/components/ui/TextComponent.h"
#include "scene/components/ui/CanvasComponent.h"
#include "scene/components/ui/RectTransformComponent.h"
#include "font/Font.h"
#include "Engine.h"

namespace eng
{
    void TextComponent::LoadProperties(const nlohmann::json& json)
    {
        const std::string text = json.value("text", "");
        SetText(text);

        if (json.contains("font"))
        {
            const auto& fontObj = json["font"];
            std::string path = fontObj.value("path", "");
            int fontSize = fontObj.value("size", 12);
            SetFont(path, fontSize);
        }

        if (json.contains("color"))
        {
            const auto& colorObj = json["color"];
            glm::vec4 color(
                colorObj.value("r", 1.0f),
                colorObj.value("g", 1.0f),
                colorObj.value("b", 1.0f),
                colorObj.value("a", 1.0f)
            );
            SetColor(color);
        }
    }

    void TextComponent::Render(CanvasComponent* canvas)
    {
        if (m_text.empty() || !m_font || !canvas)
        {
            return;
        }

        int width = m_font->GetTexture()->GetWidth();
        int height = m_font->GetTexture()->GetHeight();

        auto pos = GetPivotPos();
        float xOrigin = pos.x;
        float yOrigin = pos.y;

        for (const auto c : m_text)
        {
            const auto& desc = m_font->GetGlyphDescription(c);

            float x1 = static_cast<float>(xOrigin);
            float y1 = static_cast<float>(yOrigin - desc.height + desc.yOffset);
            float x2 = x1 + static_cast<float>(desc.width);
            float y2 = y1 + static_cast<float>(desc.height);

            float u1 = static_cast<float>(desc.x0) / static_cast<float>(width);
            float v1 = static_cast<float>(desc.y0) / static_cast<float>(height);
            float u2 = static_cast<float>(desc.x1) / static_cast<float>(width);
            float v2 = static_cast<float>(desc.y1) / static_cast<float>(height);

            xOrigin += desc.advance;

            canvas->DrawRect(
                glm::vec2(x1, y1), glm::vec2(x2, y2),
                glm::vec2(u1, v2), glm::vec2(u2, v1),
                m_font->GetTexture().get(), m_color
            );

        }
    }

    const std::string& TextComponent::GetText() const
    {
        return m_text;
    }

    void TextComponent::SetText(const std::string& text)
    {
        m_text = text;
    }

    const glm::vec4& TextComponent::GetColor() const
    {
        return m_color;
    }

    void TextComponent::SetColor(const glm::vec4& color)
    {
        m_color = color;
    }

    const std::shared_ptr<Font>& TextComponent::GetFont() const
    {
        return m_font;
    }

    void TextComponent::SetFont(const std::shared_ptr<Font>& font)
    {
        m_font = font;
    }

    void TextComponent::SetFont(const std::string& path, int size)
    {
        m_font = Engine::GetInstance().GetFontManager().GetFont(path, size);
    }

    glm::vec2 TextComponent::GetPivotPos()
    {
        auto rt = GetOwner()->GetComponent<RectTransformComponent>();
        auto pos = rt ? rt->GetScreenPosition() : GetOwner()->GetWorldPosition2D();

        glm::vec2 rect(0.0f);
        for (const auto c : m_text)
        {
            const auto& d = m_font->GetGlyphDescription(c);
            rect.x += static_cast<float>(d.advance);
            rect.y = std::max(rect.y, static_cast<float>(d.height));
        }

        if (rt)
        {
            pos -= rect * rt->GetPivot();
        }

        pos.x = std::round(pos.x);
        pos.y = std::round(pos.y);
        return pos;
    }
}